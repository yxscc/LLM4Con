// in src/llm_main.cpp

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream> // Required for std::stringstream
#include <chrono>  // For time measurement
#include <csignal> // For signal handling
#include <atomic>  // For atomic variables

#include "CPG/CPGGenerator.h"
#include "CPG/CPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "CCPG/HBGraph.h"
#include "llvm/Support/CommandLine.h"
#include "Util/ExecutionTimer.h"
#include "PhasarUtil/AnalysisManager.h"
#include "nlohmann/json.hpp"

#ifdef U
#undef U
#endif

#include "LLMUtil/AgentManager.h"
#include "LLMUtil/ThreadAPIDiscoveryAgent.h"
#include "CPG/Node.h"
#include "Query/LLMDataRaceDetector.h"
#include "Query/StatefulBugDetector.h"
#include "Query/HypothesisVerifier.h"
#include "LLMUtil/Conversation.h"
#include "LLMUtil/VerificationAgent.h"

using namespace std;
using namespace llvm;
using namespace psr;
using namespace llm_client;
using json = nlohmann::json;
namespace fs = std::filesystem;

TargetPath * TargetPath::instance = nullptr;
ExecutionTimer * ExecutionTimer::instance = nullptr;

// ============ Memory Monitoring & Checkpoint System ============
static std::atomic<bool> g_shouldExit{false};
static std::string g_checkpointFile;
static std::string g_currentPhase = "initialization";

// Get current memory usage in MB (Linux specific)
static size_t getMemoryUsageMB() {
    std::ifstream statm("/proc/self/statm");
    if (!statm.is_open()) return 0;
    size_t size, resident;
    statm >> size >> resident;
    // resident is in pages, typically 4KB each
    return (resident * 4096) / (1024 * 1024);
}

// Get available system memory in MB
static size_t getAvailableMemoryMB() {
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) return 0;
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.find("MemAvailable:") == 0) {
            size_t kb = 0;
            sscanf(line.c_str(), "MemAvailable: %zu kB", &kb);
            return kb / 1024;
        }
    }
    return 0;
}

// Write checkpoint to file
static void writeCheckpoint(const std::string& phase, const std::string& status, const std::string& extra = "") {
    g_currentPhase = phase;
    if (g_checkpointFile.empty()) return;
    
    std::ofstream ckpt(g_checkpointFile);
    if (!ckpt.is_open()) return;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    ckpt << "=== Analysis Checkpoint ===" << std::endl;
    ckpt << "Time: " << std::ctime(&time);
    ckpt << "Phase: " << phase << std::endl;
    ckpt << "Status: " << status << std::endl;
    ckpt << "Memory Usage: " << getMemoryUsageMB() << " MB" << std::endl;
    ckpt << "Available Memory: " << getAvailableMemoryMB() << " MB" << std::endl;
    if (!extra.empty()) {
        ckpt << "Details: " << extra << std::endl;
    }
    ckpt.close();
    
    std::cout << "[Checkpoint] " << phase << " - " << status 
              << " (Mem: " << getMemoryUsageMB() << "MB, Avail: " << getAvailableMemoryMB() << "MB)" << std::endl;
    std::cout.flush();
}

// Check memory and exit gracefully if running low
static bool checkMemoryAndMaybeExit(size_t thresholdMB = 2048) {
    size_t available = getAvailableMemoryMB();
    size_t used = getMemoryUsageMB();
    
    if (available < thresholdMB) {
        std::cerr << "\n[WARNING] Low memory detected! Available: " << available 
                  << "MB, Used by process: " << used << "MB" << std::endl;
        std::cerr << "[WARNING] Current phase: " << g_currentPhase << std::endl;
        std::cerr << "[WARNING] Consider increasing system memory or swap." << std::endl;
        
        writeCheckpoint(g_currentPhase, "LOW_MEMORY_WARNING", 
                       "Available: " + std::to_string(available) + "MB");
        
        // Don't exit yet, just warn. User can check checkpoint file.
        return true;
    }
    return false;
}

// Signal handler for graceful shutdown
static void signalHandler(int signum) {
    std::cerr << "\n[SIGNAL] Received signal " << signum << " during phase: " << g_currentPhase << std::endl;
    writeCheckpoint(g_currentPhase, "INTERRUPTED_BY_SIGNAL_" + std::to_string(signum));
    
    // For SIGTERM, try to exit gracefully
    if (signum == SIGTERM || signum == SIGINT) {
        g_shouldExit = true;
    }
    // Re-raise signal for default handling
    signal(signum, SIG_DFL);
    raise(signum);
}

// Install signal handlers
static void installSignalHandlers() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
}
// ============ End Memory Monitoring & Checkpoint System ============

// Command line options - can be overridden by config file
static llvm::cl::opt<std::string> ConfigFile("config",
        llvm::cl::desc("JSON config file (overrides command line options)"), llvm::cl::init(""));
static llvm::cl::opt<std::string> InputSrcDir("input-src",
        llvm::cl::desc("Input source directory"), llvm::cl::init(""));
static llvm::cl::opt<std::string> InputBCFileName("input-bc",
        llvm::cl::desc("Input bitcode file"), llvm::cl::init(""));
static llvm::cl::opt<bool> PrintTrace("print-trace", cl::desc("Print trace information"), cl::init(false));

static cl::opt<std::string> LLMProviderOpt("llm-provider", cl::desc("Choose LLM provider: openai or gemini"), cl::init("openai"));
static cl::opt<std::string> LLMApiKey("llm-key", cl::desc("API key for the chosen LLM provider"), cl::init(""));
static cl::opt<std::string> LLMModel("llm-model", cl::desc("Model name for the chosen LLM provider"), cl::init(""));
static cl::opt<std::string> LLMBaseUrl("llm-url", cl::desc("Base URL for the LLM API"), cl::init(""));
static cl::opt<std::string> EntryConfigFile("entry-config", cl::desc("Entry point config file (one function name per line, like UAFX format)"), cl::init(""));

// Debug / cost-saving modes
static cl::opt<bool> OnlyThreadEntry(
    "only-thread-entry",
    cl::desc("Only build CPG/CCPG and resolve thread entry points (build ThreadCreationTree, dump thread-creation-tree.dot), then exit. Skips all later LLM analyses to save tokens."),
    cl::init(false)
);

static cl::opt<bool> AgentMode(
    "agent-mode",
    cl::desc("Use the new hypothesis-driven DetectorAgent (single LLM session) instead of per-thread/per-pair workflow. Default: true."),
    cl::init(true)
);

static cl::opt<bool> LegacyWorkflow(
    "legacy-workflow",
    cl::desc("Force legacy per-thread-contract + per-pair workflow (overrides --agent-mode)."),
    cl::init(false)
);

// Global config structure
struct AnalysisConfig {
    std::string input_src;
    std::string input_bc;
    std::string llm_provider = "openai";
    std::string llm_key;
    std::string llm_model;
    std::string llm_url;
    std::vector<std::string> entry_points;
    bool only_thread_entry = false;
    bool print_trace = false;
    std::string call_graph_type = "OTF";  // Options: OTF, CHA, RTA, NORESOLVE
    bool agent_mode = true;               // Use hypothesis-driven agent architecture
    bool legacy_workflow = false;          // Fall back to per-thread-pair workflow
};

// Load config from JSON file
bool loadConfigFromJson(const std::string& configPath, AnalysisConfig& config) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << configPath << std::endl;
        return false;
    }
    
    try {
        json j;
        file >> j;
        
        if (j.contains("input_src")) config.input_src = j["input_src"].get<std::string>();
        if (j.contains("input_bc")) config.input_bc = j["input_bc"].get<std::string>();
        
        if (j.contains("llm")) {
            auto& llm = j["llm"];
            if (llm.contains("provider")) config.llm_provider = llm["provider"].get<std::string>();
            if (llm.contains("key")) config.llm_key = llm["key"].get<std::string>();
            if (llm.contains("model")) config.llm_model = llm["model"].get<std::string>();
            if (llm.contains("base_url")) config.llm_url = llm["base_url"].get<std::string>();
        }
        
        if (j.contains("entry_points")) {
            for (const auto& ep : j["entry_points"]) {
                config.entry_points.push_back(ep.get<std::string>());
            }
        }
        
        if (j.contains("options")) {
            auto& opts = j["options"];
            if (opts.contains("only_thread_entry")) config.only_thread_entry = opts["only_thread_entry"].get<bool>();
            if (opts.contains("print_trace")) config.print_trace = opts["print_trace"].get<bool>();
        }
        
        std::cout << "Loaded config from: " << configPath << std::endl;
        return true;
    } catch (const json::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
        return false;
    }
}

std::string convertToBC(const std::string& file);
void runLlmEvaluation(const std::string& cve_source_code_path, const fs::path& outputDir);

std::string cleanSourceCode(const std::string& source_path) {
    std::ifstream file(source_path);
    if (!file.is_open()) {
        return "[ERROR: Could not open source file]";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Remove multi-line comments
    content = std::regex_replace(content, std::regex("/\\*[\\s\\S]*?\\*/"), "");
    // Remove single-line comments
    content = std::regex_replace(content, std::regex("//.*"), "");

    std::stringstream original_ss(content);
    std::stringstream cleaned_ss;
    std::string line;

    // Remove printf lines
    while (std::getline(original_ss, line)) {
        if (line.find("printf") == std::string::npos) {
            cleaned_ss << line << "\n";
        }
    }

    return cleaned_ss.str();
}

// 新增的辅助函数，用于清理从CPG获取的代码片段
std::string cleanCodeSnippet(const std::string& code) {
    std::string cleaned_code = code;
    // 移除多行注释
    cleaned_code = std::regex_replace(cleaned_code, std::regex("/\\*[\\s\\S]*?\\*/"), "");
    // 移除单行注释
    cleaned_code = std::regex_replace(cleaned_code, std::regex("//.*"), "");

    std::stringstream original_ss(cleaned_code);
    std::stringstream final_ss;
    std::string line;

    // 移除包含printf的行
    while (std::getline(original_ss, line)) {
        if (line.find("printf") == std::string::npos) {
            final_ss << line << "\n";
        }
    }
    return final_ss.str();
}


void runZeroShotAnalysis(const std::string& source_code_path, const fs::path& outputDir) {
    std::stringstream code_ss;

    for (Thread* t : ThreadCreationTree::getInstance()->getThreads()) {
        if (t->getThreadMainFunction() && t->getThreadMainFunction()->getFuncNode()) {
            std::string func_name = t->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            std::string original_code = t->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode();
            
            // 在这里调用清理函数
            std::string cleaned_code = cleanCodeSnippet(original_code);

            code_ss << "/* Thread " << t->getId() << " Entry Function: " << func_name << " */\n";
            code_ss << cleaned_code << "\n\n";
        }
    }

    std::string relevant_code = code_ss.str();
    std::string user_prompt = "我正在分析一个并发程序，这里是它的几个关键线程的入口函数代码。请你看看其中是否存在任何恶性数据竞争或漏洞，如果有，请把它们全部报告出来，注意，你应该报告你确定的缺陷，并给出对应的代码，减少说明性文字，不用考虑修复。\n\n```c\n" + relevant_code + "\n```";

    try {
        auto llm_client = LLMClient::get_instance();
        Conversation zero_shot_convo(llm_client, "You are an expert C/C++ concurrency bug analyzer.");
        std::string llm_response = zero_shot_convo.send_message(user_prompt);
        std::ofstream result_file(outputDir / "zero_shot_analysis.txt");
        result_file << "========= Zero-Shot LLM Analysis Result =========\n\n";
        result_file << "--- PROMPT ---\n" << user_prompt << "\n\n";
        result_file << "--- RESPONSE ---\n" << llm_response << "\n";
        result_file.close();
    } catch (const std::exception& e) {
        cerr << "  - An error occurred during zero-shot analysis: " << e.what() << endl;
        std::ofstream result_file(outputDir / "zero_shot_analysis.txt");
        result_file << "An error occurred: " << e.what() << "\n";
        result_file.close();
    }
}

int main(int argc, char** argv) {
    llvm::cl::ParseCommandLineOptions(argc, argv, "LLM Concurrency Bug Detector\n");
    
    // Install signal handlers for graceful shutdown
    installSignalHandlers();
    
    // Start total time measurement
    auto total_start_time = std::chrono::high_resolution_clock::now();

    // Initialize config with defaults from command line
    AnalysisConfig config;
    config.input_src = InputSrcDir.getValue();
    config.input_bc = InputBCFileName.getValue();
    config.llm_provider = LLMProviderOpt.getValue();
    config.llm_key = LLMApiKey.getValue();
    config.llm_model = LLMModel.getValue();
    config.llm_url = LLMBaseUrl.getValue();
    config.only_thread_entry = OnlyThreadEntry.getValue();
    config.print_trace = PrintTrace.getValue();
    
    // Load config from JSON file if specified (overrides command line)
    if (!ConfigFile.getValue().empty()) {
        if (!loadConfigFromJson(ConfigFile.getValue(), config)) {
            return 1;
        }
    }
    
    // Load entry points from separate file if specified (and not already in JSON config)
    if (config.entry_points.empty() && !EntryConfigFile.getValue().empty()) {
        std::ifstream entryFile(EntryConfigFile.getValue());
        if (entryFile.is_open()) {
            std::string line;
            while (std::getline(entryFile, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                if (!line.empty() && line[0] != '#') {
                    config.entry_points.push_back(line);
                }
            }
            entryFile.close();
            std::cout << "Loaded " << config.entry_points.size() << " entry points from: " << EntryConfigFile.getValue() << std::endl;
        }
    }
    
    // Validate required fields
    if (config.input_src.empty() || config.input_bc.empty()) {
        std::cerr << "Error: input-src and input-bc are required (via command line or config file)" << std::endl;
        return 1;
    }

    LLMProvider provider;
    std::string base_url;
    std::string api_key = config.llm_key;
    
    // API key resolution priority:
    // 1. Config/Command line --llm-key
    // 2. DEFAULT_KEY environment variable
    // 3. Provider-specific env var (GEMINI_API_KEY or OPENAI_API_KEY)
    if (api_key.empty() && std::getenv("DEFAULT_KEY")) {
        api_key = std::getenv("DEFAULT_KEY");
    }
    if (api_key.empty()) {
        const char* provider_env = config.llm_provider == "gemini" ? "GEMINI_API_KEY" : "OPENAI_API_KEY";
        if (std::getenv(provider_env)) {
            api_key = std::getenv(provider_env);
        }
    }
    
    std::string model = config.llm_model;

    if (config.llm_provider == "gemini") {
        provider = LLMProvider::GEMINI;
        base_url = config.llm_url.empty() ? "https://generativelenlanguage.googleapis.com/v1beta/models/" + model + ":generateContent" : config.llm_url;
    } else {
        provider = LLMProvider::OPENAI;
        base_url = config.llm_url.empty() ? "https://jeniya.cn/v1/chat/completions" : config.llm_url;
    }

    if (api_key.empty()) {
        std::cerr << "Error: API key for " << config.llm_provider << " is not set. Use --llm-key, config file, or environment variables." << std::endl;
        return 1;
    }

    LLMClient::initialize_shared_instance(provider, base_url, api_key);
    LLMClient::get_instance()->set_model(model);

    std::string projectDir = config.input_src;
    TargetPath * targetPath = TargetPath::getInstance();
    targetPath->setTargetAbsolutePath(projectDir);
    
    // Setup checkpoint file
    g_checkpointFile = (targetPath->getOutputDir() / "analysis_checkpoint.txt").string();
    writeCheckpoint("initialization", "STARTED", "Project: " + projectDir);

    std::string source_file_path = projectDir;
    if (fs::is_directory(projectDir)) {
        for (const auto& entry : fs::directory_iterator(projectDir)) {
            if (entry.path().extension() == ".c" && entry.path().stem().string().find("_clean") == std::string::npos) {
                source_file_path = entry.path().string();
                break;
            }
        }
    }

    std::cout << "InputSrcDir: " << config.input_src << std::endl;
    std::cout << "InputBCFileName: " << config.input_bc << std::endl;
    if (!config.entry_points.empty()) {
        std::cout << "Entry points: " << config.entry_points.size() << " specified" << std::endl;
    }
    cout << "Generating CPG for project directory: " << projectDir << endl;
    writeCheckpoint("CPG_generation", "IN_PROGRESS");

    auto cpgGenerator = std::make_unique<CPGGenerator>();
    std::unique_ptr<CPG> cpg(cpgGenerator->buildCPGByDot(projectDir));
    std::string bcFile = config.input_bc == "-" ? convertToBC(projectDir) : config.input_bc;
    
    writeCheckpoint("CPG_generation", "COMPLETED");
    checkMemoryAndMaybeExit(4096);  // Warn if less than 4GB available

    writeCheckpoint("Phasar_analysis", "IN_PROGRESS", "Bitcode: " + bcFile);
    ExecutionTimer::getInstance()->start("Running whole-program pointer analysis with Phasar");
    auto pointerAnalyzer = std::make_unique<PhasarPointerAnalysis>(bcFile, config.entry_points);
    AnalysisManager::getInstance()->initialize(std::move(pointerAnalyzer));
    ExecutionTimer::getInstance()->stop("Running whole-program pointer analysis with Phasar");
    writeCheckpoint("Phasar_analysis", "COMPLETED");

    // Phase 0: Discover custom thread/lock API wrappers using LLM
    std::cout << "\n[Phase 0: Discovering Thread/Lock API Wrappers]" << std::endl;
    writeCheckpoint("Phase0_ThreadAPIDiscovery", "IN_PROGRESS");
    ExecutionTimer::getInstance()->start("Thread API Discovery");
    auto apiDiscoveryAgent = std::make_unique<llm_client::ThreadAPIDiscoveryAgent>(LLMClient::get_instance());
    int discoveredCount = apiDiscoveryAgent->discoverAndRegister(projectDir);
    ExecutionTimer::getInstance()->stop("Thread API Discovery");
    std::cout << "[Phase 0] Discovered and registered " << discoveredCount << " custom API wrapper(s).\n" << std::endl;
    writeCheckpoint("Phase0_ThreadAPIDiscovery", "COMPLETED", "Found " + std::to_string(discoveredCount) + " wrappers");

    // [DISABLED] Lazy-init race special handling - now detected through normal stateful bug flow
    // const auto& lazyInitRaces = apiDiscoveryAgent->getDetectedLazyInitRaces();

    writeCheckpoint("CCPG_Analysis", "IN_PROGRESS");
    checkMemoryAndMaybeExit(4096);
    ExecutionTimer::getInstance()->start("CCPG Analysis");
    auto ccpg = std::make_unique<CCPG>(cpg.get());
    ccpg->build();
    ExecutionTimer::getInstance()->start("HBGraph");
    HBGraph::getInstance()->build(ccpg.get());
    ExecutionTimer::getInstance()->stop("HBGraph");
    HBGraph::getInstance()->dumpDot(targetPath->getOutputDir());
    ExecutionTimer::getInstance()->stop("CCPG Analysis");
    ccpg->dump(targetPath->getOutputDir());
    writeCheckpoint("CCPG_Analysis", "COMPLETED");

    // Always dump the thread creation tree once it's available.
    ThreadCreationTree::getInstance()->printThreadCreationTree(targetPath->getOutputDir());

    if (config.only_thread_entry) {
        std::cout << "\n[Only Thread Entry Mode]\n";
        std::cout << "Thread creation tree dumped to: "
                  << (targetPath->getOutputDir() / "thread-creation-tree.dot") << "\n";
        std::cout << "Skipping contract generation / parallelism analysis / bug detection / evaluation to save tokens.\n";
        return 0;
    }

    writeCheckpoint("LLM_Analysis", "IN_PROGRESS");
    llm_client::AgentManager agentManager(ccpg.get());
    const auto candidateSharedObjects = ThreadCreationTree::getInstance()->collectCandidateSharedObjects();

    bool useAgentMode = config.agent_mode && !config.legacy_workflow;
    std::cout << "\n[Analysis Mode] " << (useAgentMode ? "Agent (hypothesis-driven)" : "Legacy (per-thread contract)") << std::endl;

    if (useAgentMode) {
        agentManager.runAnalysisAgentMode();
    } else {
        auto thread_pairs_with_analysis = agentManager.runAnalysisLegacy();
        writeCheckpoint("LLM_Analysis", "COMPLETED", "Thread pairs analyzed");

        writeCheckpoint("Bug_Detection", "IN_PROGRESS");
        query::StatefulBugDetector statefulDetector;
        statefulDetector.detect(thread_pairs_with_analysis, candidateSharedObjects, nullptr);
        statefulDetector.printResults(targetPath->getOutputDir());
        writeCheckpoint("Bug_Detection", "COMPLETED");

        std::cout << "LLM-guided analysis complete. Results are in the output directory." << std::endl;
        // Skip the hypothesis path below
        goto analysis_done;
    }

    writeCheckpoint("LLM_Analysis", "COMPLETED", "Hypotheses verified");

    {
        writeCheckpoint("Bug_Detection", "IN_PROGRESS");
        query::StatefulBugDetector statefulDetector;
        const auto& hypotheses = agentManager.getConfirmedHypotheses();

        // Phase 4.5 LLM hypothesis-verifier filter — second-pass FP triage
        // for everything the static constraint engine confirmed. Skip with
        // LACE_DISABLE_LLM_VERIFIER=1 (e.g. when running offline).
        std::unique_ptr<llm_client::VerificationAgent> hyp_verifier;
        const char* disable = std::getenv("LACE_DISABLE_LLM_VERIFIER");
        if (!hypotheses.empty() && (!disable || disable[0] == '0' || disable[0] == '\0')) {
            auto client = agentManager.getLLMClient();
            if (client) {
                hyp_verifier = std::make_unique<llm_client::VerificationAgent>(client);
            }
        }

        statefulDetector.detectFromHypotheses(hypotheses, ccpg.get(),
                                              hyp_verifier.get());
        statefulDetector.printResults(targetPath->getOutputDir());
        writeCheckpoint("Bug_Detection", "COMPLETED");
    }

    analysis_done:
    std::cout << "LLM-guided analysis complete. Results are in the output directory." << std::endl;

    // Zero-shot analysis is temporarily disabled for efficiency on large projects.
    // runZeroShotAnalysis(source_file_path, targetPath->getOutputDir());

    // Phase 4: LLM Evaluation is temporarily disabled for efficiency on large projects.
    // std::cout << "\n[Phase 4: Evaluating Results with LLM]" << std::endl;
    // runLlmEvaluation(source_file_path, targetPath->getOutputDir());
    // std::cout << "LLM-based evaluation complete. Report saved in output directory." << std::endl;

    // Print final statistics
    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(total_end_time - total_start_time);
    
    auto token_stats = LLMClient::get_instance()->get_token_stats();
    
    std::cout << "\n========== Analysis Statistics ==========" << std::endl;
    std::cout << "Total Time: " << total_duration.count() << " seconds" << std::endl;
    std::cout << "LLM API Requests: " << token_stats.total_requests << std::endl;
    std::cout << "Total Prompt Tokens: " << token_stats.total_prompt_tokens << std::endl;
    std::cout << "Total Completion Tokens: " << token_stats.total_completion_tokens << std::endl;
    std::cout << "Total Tokens: " << (token_stats.total_prompt_tokens + token_stats.total_completion_tokens) << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Save statistics to file
    std::ofstream stats_file(targetPath->getOutputDir() / "analysis_stats.txt");
    if (stats_file.is_open()) {
        stats_file << "Analysis Statistics\n";
        stats_file << "==================\n";
        stats_file << "Total Time: " << total_duration.count() << " seconds\n";
        stats_file << "LLM API Requests: " << token_stats.total_requests << "\n";
        stats_file << "Total Prompt Tokens: " << token_stats.total_prompt_tokens << "\n";
        stats_file << "Total Completion Tokens: " << token_stats.total_completion_tokens << "\n";
        stats_file << "Total Tokens: " << (token_stats.total_prompt_tokens + token_stats.total_completion_tokens) << "\n";
        stats_file.close();
    }

    return 0;
}

std::string convertToBC(const std::string& file){
    string name = file.substr(file.find_last_of("/")+1);
    fs::path projectDir = fs::path(PROJECT_PATH);
    fs::path outputDir = projectDir / "llvmbc" ;
    fs::path bcFile = outputDir / (name + ".ll");
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    if(file.find(".c") != std::string::npos){
        string convertToBCCommand = std::string("clang -S -c -fno-discard-value-names -emit-llvm -g -O0 -fno-inline ") + file + " -o " + bcFile.string();
        printf("convertToBCCommand: %s\n", convertToBCCommand.c_str());
        if (system(convertToBCCommand.c_str()) != 0) {
            cerr << "Failed to convert to BC file." << endl;
            exit(1);
        }
    } else if(file.find(".cpp") != std::string::npos){
        string convertToBCCommand = std::string("clang++ -S -c -fno-discard-value-names -emit-llvm -g -O0 -fno-inline ") + file + " -o " + bcFile.string();
        printf("convertToBCCommand: %s\n", convertToBCCommand.c_str());
        if (system(convertToBCCommand.c_str()) != 0) {
            cerr << "Failed to convert to BC file." << endl;
            exit(1);
        }
    } else {
        cerr << "Unsupported file type." << endl;
        exit(1);
    }
    return bcFile.string();
}


std::string readFileContent(const fs::path& path) {
    if (!fs::exists(path)) {
        return "[ERROR: File not found at " + path.string() + "]";
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        return "[ERROR: Could not open file " + path.string() + "]";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string getCVEIdentifier(const std::string& path_str) {
    std::regex cve_regex("(CVE-[0-9]{4,}-[0-9]{4,})");
    std::smatch match;
    if (std::regex_search(path_str, match, cve_regex)) {
        return match[1].str();
    }
    return "";
}

// Generates a tailored prompt for a single tool evaluation.
std::string buildEvaluationPrompt(const std::string& tool_name, const std::string& tool_result, const std::string& cve_id, const std::string& readme, const std::string& source_code) {
    std::stringstream prompt_ss;
    prompt_ss << "作为一名专业的漏洞分析专家，请根据以下信息，专门评估工具“" << tool_name << "”的性能。\n\n";
    prompt_ss << "## 漏洞信息\n\n";
    prompt_ss << "### CVE ID: " << cve_id << "\n\n";
    prompt_ss << "### README 描述:\n" << "```\n" << readme << "\n```\n\n";
    prompt_ss << "### 漏洞源代码:\n" << "```c\n" << source_code << "\n```\n\n";
    prompt_ss << "--------------------------------\n\n";
    prompt_ss << "## “" << tool_name << "” 的检测结果:\n" << "```\n" << (tool_result.empty() ? "未检测到任何问题。" : tool_result) << "\n```\n\n";
    prompt_ss << "--------------------------------\n\n";
    prompt_ss << "## 评估任务\n\n";
    prompt_ss << "请**只针对这一个工具**，从以下四个维度进行详细评估，并给出清晰的结论：\n\n";
    prompt_ss << "1.  **是否直接检测到漏洞？** (即检测结果是否直接指出了`README`中描述的最终漏洞，例如Use-After-Free, Double-Free等)\n";
    prompt_ss << "2.  **是否检测到作为Root Cause的数据竞争？** (如果未能直接检测到漏洞，评估其是否准确地识别出了导致该漏洞产生的根本原因——数据竞争)\n";
    prompt_ss << "3.  **数据竞争的隐蔽性如何？** (如果检测到了数据竞争，评估这个数据竞争是否容易被开发者忽略。例如，由于复杂的指针操作、间接的函数调用或是在看似无关的代码块之间发生的竞争)\n";
    prompt_ss << "4.  **误报率分析** (评估工具的误报情况。注意：良性的数据竞争也应被视为误报。请指出哪些是误报，哪些是恶性竞争)\n\n";
    prompt_ss << "请以清晰的、结构化的方式给出你的分析报告。";
    return prompt_ss.str();
}

void runLlmEvaluation(const std::string& cve_source_code_path, const fs::path& outputDir) {
    std::string cve_id = getCVEIdentifier(cve_source_code_path);
    if (cve_id.empty()) {
        std::cerr << "Could not determine CVE ID from path: " << cve_source_code_path << std::endl;
        return;
    }

    // --- 1. Gather common information ---
    std::string source_code = readFileContent(cve_source_code_path);
    std::string readme_content = readFileContent(fs::path(cve_source_code_path).parent_path() / "README.md");

    // --- 2. Read results from all tools ---
    std::string our_tool_result = readFileContent(outputDir / "stateful_bugs" / "bugs.txt");
    std::string fsam_result = readFileContent("/home/ConCodeQL/experimental_result/Fsam/mta-" + cve_id + ".md");
    std::string racerf_result = readFileContent("/home/ConCodeQL/experimental_result/RacerF/" + cve_id + "_frama-c_result.txt");

    std::map<std::string, std::string> tool_results = {
        {"Our Tool (LLM-guided Detector)", our_tool_result},
        {"Fsam", fsam_result},
        {"RacerF", racerf_result}
    };

    std::stringstream final_report_ss;
    final_report_ss << "# LLM-based Evaluation Report for " << cve_id << "\n\n";

    // --- 3. Evaluate each tool in a separate conversation ---
    for (const auto& pair : tool_results) {
        const std::string& tool_name = pair.first;
        const std::string& tool_result = pair.second;

        std::cout << "  - Evaluating " << tool_name << "..." << std::endl;
        std::string prompt = buildEvaluationPrompt(tool_name, tool_result, cve_id, readme_content, source_code);

        try {
            auto llm_client = LLMClient::get_instance();
            Conversation evaluation_convo(llm_client, "You are a professional vulnerability analysis expert.");
            std::string llm_response = evaluation_convo.send_message(prompt);

            final_report_ss << "## Evaluation for: " << tool_name << "\n\n";
            final_report_ss << llm_response << "\n\n";
            final_report_ss << "--------------------------------\n\n";

        } catch (const std::exception& e) {
            cerr << "    - An error occurred during LLM evaluation for " << tool_name << ": " << e.what() << endl;
            final_report_ss << "## Evaluation for: " << tool_name << "\n\n";
            final_report_ss << "An error occurred during evaluation: " << e.what() << "\n\n";
            final_report_ss << "--------------------------------\n\n";
        }
    }

    // --- 4. Save the combined report ---
    std::ofstream final_report_file(outputDir / "llm_evaluation_report.md");
    final_report_file << final_report_ss.str();
    final_report_file.close();
}