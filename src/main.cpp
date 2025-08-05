#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

#include "CPG/CPGGenerator.h"
#include "CPG/CPG.h"
#include "CCPG/CCPG.h"
#include "Util/ExecutionTimer.h"
#include "Query/DataRaceDetector.h"
#include "Query/UseAfterFreeDetector.h"
#include "Query/DoubleFreeDetector.h"
#include "Query/NullReferenceDetector.h"
#include "LLMUtil/LLMClient.h"
#include "PhasarUtil/AnalysisManager.h"
#include "llvm/Support/CommandLine.h"

#ifdef U
#undef U
#endif


using namespace std;
using namespace llvm;

static cl::opt<std::string> InputSrcDir(cl::Positional,
        cl::desc("<input src>"), cl::init("-"));
static cl::opt<std::string> InputBCFileName(cl::Positional,
        cl::desc("<input bc>"), cl::init("-"));

TargetPath * TargetPath::instance = nullptr;
ExecutionTimer * ExecutionTimer::instance = nullptr;

std::string convertToBC(const std::string& file);

int main(int argc, char* argv[]) {

    // llm_client has been removed to avoid hardcoded keys.
    // You can initialize it here using environment variables or a config file.
    // llm_client::LLMClient::get_shared_instance(std::getenv("LLM_API_URL"), std::getenv("LLM_API_KEY"));

    int arg_num = 0;
    std::vector<char*> arg_value(argc);

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
    }
    else{
        bcFile = convertToBC(projectDir);
    }  

    ExecutionTimer::getInstance()->start("Running whole-program pointer analysis with Phasar");
    auto pointerAnalyzer = std::make_unique<PhasarPointerAnalysis>(bcFile);
    AnalysisManager::getInstance()->initialize(std::move(pointerAnalyzer));
    ExecutionTimer::getInstance()->stop("Running whole-program pointer analysis with Phasar");

    ExecutionTimer::getInstance()->start("CCPG Analysis");
    auto ccpg = std::make_unique<CCPG>(cpg.get());
    ccpg->build();
    //ExecutionTimer::getInstance()->stop("CCPG Analysis");

    ccpg->dump(targetPath->getOutputDir());

    /*ExecutionTimer::getInstance()->start("Data Race Detection");
    auto drd = std::make_unique<DataRaceDetector>();
    drd->detect();
    ExecutionTimer::getInstance()->stop("Data Race Detection");
    drd->printDataRaces(targetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Use After Free Detection");
    auto uafd = std::make_unique<UseAfterFreeDetector>();
    uafd->detect();
    ExecutionTimer::getInstance()->stop("Use After Free Detection");
    uafd->printUseAfterFrees(targetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Double Free Detection");
    auto dfd = std::make_unique<DoubleFreeDetector>();
    dfd->detect();
    ExecutionTimer::getInstance()->stop("Double Free Detection");
dfd->printDoubleFrees(targetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Null Reference Detection");
    auto nrd = std::make_unique<NullReferenceDetector>();
    nrd->detect();
    ExecutionTimer::getInstance()->stop("Null Reference Detection");
nrd->printNullReferences(targetPath->getOutputDir());*/

    ExecutionTimer::getInstance()->printAllTimes(targetPath->getOutputDir());

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