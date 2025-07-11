#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

#include "CPG/CPGGenerator.h"
#include "CPG/CPG.h"
#include "SVFUtil/SVFManager.h"
#include "Util/ExtAPI.h"
#include "CCPG/CCPG.h"
#include "Util/ExecutionTimer.h"
#include "Query/DataRaceDetector.h"
#include "Query/UseAfterFreeDetector.h"
#include "Query/DoubleFreeDetector.h"
#include "Query/NullReferenceDetector.h"
#include "LLMUtil/LLMClient.h"

#ifdef U
#undef U
#endif

#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/LLVMModule.h"


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
void dumpsvf(SVFG* svfg);

int main(int argc, char* argv[]) {

    // llm_client has been removed to avoid hardcoded keys.
    // You can initialize it here using environment variables or a config file.
    // llm_client::LLMClient::get_shared_instance(std::getenv("LLM_API_URL"), std::getenv("LLM_API_KEY"));

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
    //ExecutionTimer::getInstance()->stop("CCPG Analysis");

    ccpg->dump(targetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Data Race Detection");
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
nrd->printNullReferences(targetPath->getOutputDir());

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

void dumpsvf(SVFG* svfg){
    LLVMModuleSet* moduleSet = LLVMModuleSet::getLLVMModuleSet();
    //std::cout << "Number of SVFG nodes: " << svfg->getSVFGNodeSet().size() << std::endl;
    for (SVF::SVFG::const_iterator it = svfg->begin(); it != svfg->end(); ++it)
    {
        const SVF::SVFGNode* svfNode = it->second;
        if(svfNode == nullptr){
            continue;
        }
        const SVF::SVFValue *svfValue = svfNode->getValue();
        if(svfValue == nullptr){
            continue;
        }

        // 获取Value*的名字
        std::string valueName = svfValue->getName();
        // 获取Value*的sourceLoc
        std::string sourceLoc = svfValue->getSourceLoc();
        // 获取llvmValue
        const llvm::Value* llvmValue = moduleSet->getLLVMValue(svfValue);


        // 把上面输出的这些以同样的格式输出到一个txt文件中，文件生成在根目录
        std::ofstream out(PROJECT_PATH + std::string("SVFG.txt"), std::ios::app);
        out << "Node: " << svfNode->getId() << "\n";
        out << "==============\n\t" << svfNode->toString() << "\n==============\n";
        out << "\t|| SVF-Name:\t" << svfValue->getName() << "\n";
        out << "\t|| LLVM-Name:\t" << llvmValue->getName().data() << "\n";
        out << "\t|| LLVM-Val:\t" << svfValue->toString() << "\n";
        out << "\t|| SVF-Type:\t" << svfValue->getType()->toString() << "\n";
        out << "\t|| SVFSource-Loc:\t" << svfValue->getSourceLoc() << "\n";
        out << "\n";
        out.close();

    }
}