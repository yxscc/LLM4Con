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

#include "CPG/CPGGenerator.h"
#include "CPG/CPG.h"
#include "CCPG/ThreadCreationTree.h"
#include "llvm/Support/CommandLine.h"
#include "Util/ExecutionTimer.h"
#include "PhasarUtil/AnalysisManager.h"

#ifdef U
#undef U
#endif

#include "LLMUtil/AgentManager.h"
#include "CPG/Node.h"
#include "Query/LLMDataRaceDetector.h"
#include "Query/StatefulBugDetector.h"
#include "LLMUtil/Conversation.h"

using namespace std;
using namespace llvm;
using namespace psr;
using namespace llm_client;
namespace fs = std::filesystem;

TargetPath * TargetPath::instance = nullptr;
ExecutionTimer * ExecutionTimer::instance = nullptr;

static llvm::cl::opt<std::string> InputSrcDir("input-src",
        llvm::cl::desc("Input source file"), llvm::cl::Required);
static llvm::cl::opt<std::string> InputBCFileName("input-bc",
        llvm::cl::desc("Input bitcode file"), llvm::cl::Required);
static llvm::cl::opt<bool> PrintTrace("print-trace", cl::desc("Print trace information"), cl::init(false));

static cl::opt<std::string> LLMProviderOpt("llm-provider", cl::desc("Choose LLM provider: openai or gemini"), cl::init("openai"));
static cl::opt<std::string> LLMApiKey("llm-key", cl::desc("API key for the chosen LLM provider"), cl::init(""));
static cl::opt<std::string> LLMModel("llm-model", cl::desc("Model name for the chosen LLM provider"), cl::init(""));
static cl::opt<std::string> LLMBaseUrl("llm-url", cl::desc("Base URL for the LLM API"), cl::init(""));

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

    LLMProvider provider;
    std::string base_url;
    std::string api_key_str = LLMApiKey.getValue();
    std::string api_key = api_key_str.empty() ? (std::getenv(LLMProviderOpt.getValue() == "gemini" ? "GEMINI_API_KEY" : "OPENAI_API_KEY") ? std::getenv(LLMProviderOpt.getValue() == "gemini" ? "GEMINI_API_KEY" : "OPENAI_API_KEY") : "") : api_key_str;
    std::string model = LLMModel.getValue();

    if (LLMProviderOpt.getValue() == "gemini") {
        provider = LLMProvider::GEMINI;
        std::string llm_base_url_str = LLMBaseUrl.getValue();
        base_url = llm_base_url_str.empty() ? "https://generativelenlanguage.googleapis.com/v1beta/models/" + model + ":generateContent" : llm_base_url_str;
    } else {
        provider = LLMProvider::OPENAI;
        std::string llm_base_url_str = LLMBaseUrl.getValue();
        base_url = llm_base_url_str.empty() ? "https://jeniya.cn/v1/chat/completions" : llm_base_url_str;
    }

    if (api_key.empty()) {
        std::cerr << "Error: API key for " << LLMProviderOpt << " is not set. Use --llm-key or environment variables." << std::endl;
        return 1;
    }

    LLMClient::initialize_shared_instance(provider, base_url, api_key);
    LLMClient::get_instance()->set_model(model);

    std::string projectDir = InputSrcDir;
    TargetPath * targetPath = TargetPath::getInstance();
    targetPath->setTargetAbsolutePath(projectDir);

    std::string source_file_path = projectDir;
    if (fs::is_directory(projectDir)) {
        for (const auto& entry : fs::directory_iterator(projectDir)) {
            if (entry.path().extension() == ".c" && entry.path().stem().string().find("_clean") == std::string::npos) {
                source_file_path = entry.path().string();
                break;
            }
        }
    }

    std::cout << "InputSrcDir: " << InputSrcDir << std::endl;
    std::cout <<  "InputBCFileName: " << InputBCFileName << std::endl;
    cout << "Generating CPG for project directory: " << projectDir << endl;

    auto cpgGenerator = std::make_unique<CPGGenerator>();
    std::unique_ptr<CPG> cpg(cpgGenerator->buildCPGByDot(projectDir));
    std::string bcFile = InputBCFileName == "-" ? convertToBC(projectDir) : InputBCFileName;

    ExecutionTimer::getInstance()->start("Running whole-program pointer analysis with Phasar");
    auto pointerAnalyzer = std::make_unique<PhasarPointerAnalysis>(bcFile);
    AnalysisManager::getInstance()->initialize(std::move(pointerAnalyzer));
    ExecutionTimer::getInstance()->stop("Running whole-program pointer analysis with Phasar");

    ExecutionTimer::getInstance()->start("CCPG Analysis");
    auto ccpg = std::make_unique<CCPG>(cpg.get());
    ccpg->build();
    ExecutionTimer::getInstance()->stop("CCPG Analysis");
    ccpg->dump(targetPath->getOutputDir());

    llm_client::AgentManager agentManager(ccpg.get());
    const auto candidateSharedObjects = ThreadCreationTree::getInstance()->collectCandidateSharedObjects();
    auto thread_pairs_with_analysis = agentManager.runAnalysis();

    query::StatefulBugDetector statefulDetector;
    statefulDetector.detect(thread_pairs_with_analysis, candidateSharedObjects);
    statefulDetector.printResults(targetPath->getOutputDir()); // This now saves the result to a file
    std::cout << "LLM-guided analysis complete. Results are in the output directory." << std::endl;

    runZeroShotAnalysis(source_file_path, targetPath->getOutputDir());

    std::cout << "\n[Phase 4: Evaluating Results with LLM]" << std::endl;
    runLlmEvaluation(source_file_path, targetPath->getOutputDir());
    std::cout << "LLM-based evaluation complete. Report saved in output directory." << std::endl;

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