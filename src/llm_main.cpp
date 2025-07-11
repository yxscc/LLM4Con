#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>

#include "CPG/CPGGenerator.h"
#include "CPG/CPG.h"
#include "SVFUtil/SVFManager.h"
#include "Util/ExtAPI.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "SVF-LLVM/LLVMModule.h"
#include "CCPG/CCPG.h"
#include "Util/ExecutionTimer.h"
#include "LLMUtil/LLMClient.h"

#ifdef U
#undef U
#endif

#include "LLMUtil/FindingThreadEntryAgent.h"
#include "LLMUtil/ContractGeneratorAgent.h"
#include "CCPG/CCPGNode.h"
#include "CPG/Node.h"


using namespace std;
using namespace llvm;
using namespace SVF;

TargetPath * TargetPath::instance = nullptr;
ExecutionTimer * ExecutionTimer::instance = nullptr;

static llvm::cl::opt<std::string> InputSrcDir(cl::Positional,
        llvm::cl::desc("<input src>"), llvm::cl::init("-"));
static llvm::cl::opt<std::string> InputBCFileName(cl::Positional,
        llvm::cl::desc("<input bc>"), llvm::cl::init("-"));
static llvm::cl::opt<bool> PrintTrace("print-trace", cl::desc("Print trace information"), cl::init(false));

std::string convertToBC(const std::string& file);

int main(int argc, char** argv) {
    std::cout << "LLM Concurrency Bug Detector Initialized." << std::endl;

    int arg_num = 0;
    std::vector<char*> arg_value(argc);

    std::vector<std::string> moduleNameVec;
    LLVMUtil::processArguments(argc, argv, arg_num, arg_value.data(), moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value.data(),
                                "Whole Program Concurrency Analysis\n");

    std::string projectDir = InputSrcDir;
    TargetPath * targetPath = TargetPath::getInstance();
    targetPath->setTargetAbsolutePath(projectDir);

    std::cout << "InputSrcDir: " << InputSrcDir << std::endl;
    std::cout <<  "InputBCFileName: " << InputBCFileName << std::endl;
    
    cout << "Generating CPG for project directory: " << projectDir << endl;

    auto cpgGenerator = std::make_unique<CPGGenerator>();
    std::unique_ptr<CPG> cpg(cpgGenerator->buildCPGByDot(projectDir));
    
    std::string bcFile;
    if(InputBCFileName != "-"){
        bcFile = InputBCFileName;
        moduleNameVec.push_back(bcFile);
    }
    else{
        bcFile = convertToBC(projectDir);
        moduleNameVec.push_back(bcFile);
    }  

    ExecutionTimer::getInstance()->start("SVF Analysis");
    SVFManager::getInstance()->runSVFAnalysis(moduleNameVec);
    ExecutionTimer::getInstance()->stop("SVF Analysis");

    ExecutionTimer::getInstance()->start("CCPG Analysis");
    auto ccpg = std::make_unique<CCPG>(cpg.get());
    ccpg->build();
    ExecutionTimer::getInstance()->stop("CCPG Analysis");

    ccpg->dump(targetPath->getOutputDir());

    // --- LLM-based Analysis ---
    std::cout << "\n--- Starting LLM-based Concurrency Analysis ---" << std::endl;

    // 1. Initialize LLM Client
    auto llmClient = llm_client::LLMClient::get_shared_instance(std::getenv("LLM_API_URL"), std::getenv("LLM_API_KEY"));
    if (!llmClient) {
        std::cerr << "Failed to initialize LLM Client. Please check URL and API key. Aborting." << std::endl;
        return 1;
    }
    std::cout << "LLM Client initialized." << std::endl;

    // 2. Find all thread creation sites
    CCPGNodeSet forkNodes = ccpg->getNodesByType(ThreadAPIUtil::TYPE::FORK);
    std::cout << "Found " << forkNodes.size() << " thread creation sites." << std::endl;

    // 3. Instantiate Agents
    llm_client::FindingThreadEntryAgent entryFinder(llmClient);
    ContractGeneratorAgent contractGenerator(ccpg.get(), llmClient);
    std::vector<LLM::ConcurrencyContract> allContracts;

    // 4. Iterate over each thread creation site to find entry point and generate contract
    for (CCPGNode* forkNode : forkNodes) {
        std::cout << "\n--- Analyzing Fork Site (Node ID: " << forkNode->getId() << ") ---" << std::endl;
        
        // a. Use FindingThreadEntryAgent to get the entry function ID
        int entryFuncId = entryFinder.find_thread_entry(forkNode);
        
        if (entryFuncId != -1) {
            std::cout << "Agent identified thread entry function ID: " << entryFuncId << std::endl;
            
            // b. Use ContractGeneratorAgent to generate a contract for this thread
            auto contractOpt = contractGenerator.generateContractForThread(entryFuncId);
            
            if (contractOpt) {
                std::cout << "Successfully generated contract for thread entry ID " << entryFuncId << std::endl;
                allContracts.push_back(contractOpt.value());
            } else {
                std::cerr << "Failed to generate contract for thread entry ID " << entryFuncId << std::endl;
            }
        } else {
            std::cerr << "Could not determine thread entry for fork site " << forkNode->getId() << std::endl;
        }
    }

    // 5. Print all generated contracts
    std::cout << "\n--- All Generated Concurrency Contracts ---" << std::endl;
    if (allContracts.empty()) {
        std::cout << "No contracts were generated." << std::endl;
    }
    else {
        for (const auto& contract : allContracts) {
            std::cout << contract.toJson() << "\n" << std::endl;
        }
    }
    std::cout << "--- LLM-based Analysis Finished ---\n" << std::endl;

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
