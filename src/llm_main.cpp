#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <fstream> 
#include <regex>  


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

// Runs the zero-shot analysis and saves the result.
void runZeroShotAnalysis(const std::string& source_code_path, const fs::path& outputDir) {
    std::stringstream code_ss;

    for (Thread* t : ThreadCreationTree::getInstance()->getThreads()) {
        if (t->getThreadMainFunction()) {
            std::string func_name = t->getThreadMainFunction()->getFuncNode()->getCPGNode()->getName();
            code_ss << "/* Thread " << t->getId() << " Entry Function: " << func_name << " */\n";
            code_ss << t->getThreadMainFunction()->getFuncNode()->getCPGNode()->getCode() << "\n\n";
        }
    }
    
    std::string relevant_code = code_ss.str();
    
    std::string user_prompt = "我正在分析一个并发程序，这里是它的几个关键线程的入口函数代码。请你看看其中是否存在任何恶性数据竞争或漏洞，如果有，请把它们全部报告出来，注意，你应该报告你确定的缺陷，并给出对应的代码，减少说明性文字，不用考虑修复。\n\n```c\n" + relevant_code + "\n```";

    try {
        auto llm_client = LLMClient::get_instance();
        // Use a simple conversation object for this one-off query
        Conversation zero_shot_convo(llm_client, "You are an expert C/C++ concurrency bug analyzer.");
        std::string llm_response = zero_shot_convo.send_message(user_prompt);
        std::ofstream result_file(outputDir / "zero_shot_analysis.txt");
        result_file << "========= Zero-Shot LLM Analysis Result =========\n\n";
        result_file << "--- PROMPT ---\n";
        result_file << user_prompt << "\n\n";
        result_file << "--- RESPONSE ---\n";
        result_file << llm_response << "\n";
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
        base_url = llm_base_url_str.empty() ? "https://generativelanguage.googleapis.com/v1beta/models/" + model + ":generateContent" : llm_base_url_str;
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
        // Simple heuristic to find the .c file in the directory
        for (const auto& entry : fs::directory_iterator(projectDir)) {
            if (entry.path().extension() == ".c") {
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

    // --- LLM-based Analysis ---
    llm_client::AgentManager agentManager(ccpg.get());
    const auto candidateSharedObjects = ThreadCreationTree::getInstance()->collectCandidateSharedObjects();
    auto thread_pairs_with_analysis = agentManager.runAnalysis();

    // --- Run Detectors on LLM-generated Contracts ---
    /*std::cout << "\n[Phase 3: Detecting Data Races from Contracts]" << std::endl;
    query::LLMDataRaceDetector raceDetector;
    raceDetector.detect(thread_pairs_with_analysis);
    raceDetector.printDataRaces(targetPath->getOutputDir());*/

    // --- 2. 实例化并调用新的检测器 ---
    query::StatefulBugDetector statefulDetector;
    statefulDetector.detect(thread_pairs_with_analysis, candidateSharedObjects);
    statefulDetector.printResults(targetPath->getOutputDir());
    // ------------------------------------

    std::cout << "LLM-guided analysis complete. Results are in the output directory." << std::endl;

    runZeroShotAnalysis(source_file_path, targetPath->getOutputDir());

    return 0;
}


// 将文件或项目转换成bc文件，并输出到项目根目录的llvmbc文件夹下
std::string convertToBC(const std::string& file){
    // 获取文件名
    string name = file.substr(file.find_last_of("/")+1);
    // 获取项目根目录
    fs::path projectDir = fs::path(PROJECT_PATH);
    // 获取llvmbc文件夹路径
    fs::path outputDir = projectDir / "llvmbc" ;
    // 获取bc文件路径
    fs::path bcFile = outputDir / (name + ".ll");
    // 如果llvmbc文件夹不存在，则创建
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }


    // 将文件转换成bc文件
    // 如果是.c文件就用clang，如果是.cpp文件就用clang++
    if(file.find(".c") != std::string::npos){
        string convertToBCCommand = std::string("clang -S -c -fno-discard-value-names -emit-llvm -g -O0 -fno-inline ") + file + " -o " + bcFile.string();
        printf("convertToBCCommand: %s\n", convertToBCCommand.c_str());
        int result = system(convertToBCCommand.c_str());
        if (result != 0) {
            cerr << "Failed to convert to BC file." << endl;
            exit(1);
        }
    }
    else if(file.find(".cpp") != std::string::npos){
        string convertToBCCommand = std::string("clang++ -S -c -fno-discard-value-names -emit-llvm -g -O0 -fno-inline ") + file + " -o " + bcFile.string();
        printf("convertToBCCommand: %s\n", convertToBCCommand.c_str());
        int result = system(convertToBCCommand.c_str());
        if (result != 0) {
            cerr << "Failed to convert to BC file." << endl;
            exit(1);
        }
    }
    else{
        cerr << "Unsupported file type." << endl;
        exit(1);
    }
    
    // 优化bc文件
    /*string optBCCommand = std::string("opt -S -mem2reg ") + bcFile.string() + " -o " + bcFile.string();
    printf("optBCCommand: %s\n", optBCCommand.c_str());
    result = system(optBCCommand.c_str());
    if (result != 0) {
        cerr << "Failed to optimize BC file." << endl;
        exit(1);
    }*/

    return bcFile.string();
}
