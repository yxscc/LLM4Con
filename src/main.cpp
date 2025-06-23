#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include <vector>
#include <string>

#include "CPG/CPGGenerator.h"
#include "CPG/CPG.h"
#include "SVFUtil/SVFManager.h"
#include "Util/ExtAPI.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "SVF-LLVM/LLVMModule.h"
#include "CCPG/CCPG.h"
#include "Util/ExecutionTimer.h"
#include "Query/DataRaceDetector.h"
#include "Query/UseAfterFreeDetector.h"
#include "Query/DoubleFreeDetector.h"
#include "Query/NullReferenceDetector.h"
#include "LLMUtil/LLMClient.h"


using namespace std;
using namespace llvm;
using namespace SVF;

SVFManager * SVFManager::instance = nullptr;
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

    llm_client::LLMClient::get_shared_instance(
        "https://ark.cn-beijing.volces.com/api/v3/chat/completions",
        "b9d76d27-14e4-4f1d-8a56-bef96c07a9a3"
    );

    int arg_num = 0;
    char **arg_value = new char*[argc];

    std::vector<std::string> moduleNameVec;
    LLVMUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value,
                                "Whole Program Concurrency Analysis\n");

    std::string projectDir = InputSrcDir;
    TargetPath * TargetPath = TargetPath::getInstance();
    TargetPath->setTargetAbsolutePath(projectDir);

    std::cout << "InputSrcDir: " << InputSrcDir << std::endl;
    std::cout <<  "InputBCFileName: " <<InputBCFileName << std::endl;
    
    cout << "Generating CPG for project directory: " << projectDir << endl;

    CPGGenerator* cpgGenerator = new CPGGenerator();
    CPG * cpg = cpgGenerator->buildCPGByDot(projectDir);
    
    std::string bcFile;
    if(InputBCFileName != "-"){
        bcFile = InputBCFileName;
    }
    else{
        bcFile = convertToBC(projectDir);
        moduleNameVec.push_back(bcFile);
    }  

    ExecutionTimer::getInstance()->start("SVF Analysis");
    LLVMModuleSet* moduleSet = LLVMModuleSet::getLLVMModuleSet();

    SVFModule* svfModule = moduleSet->buildSVFModule(moduleNameVec);
    SVFIRBuilder builder(svfModule);
    SVFIR* pag = builder.build();
    ICFG* icfg = pag->getICFG();
    Andersen* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    PTACallGraph* callgraph = ander->getCallGraph();
    VFG* vfg = new VFG(callgraph);
    SVFGBuilder svfBuilder(true);
    SVFG* svfg = svfBuilder.buildFullSVFG(ander);

    SVFManager* svfManager = SVFManager::build(svfModule, pag, ander, svfg, callgraph, icfg, vfg);
    ExecutionTimer::getInstance()->stop("SVF Analysis");

    ExecutionTimer::getInstance()->start("CCPG Analysis");
    CCPG* ccpg = new CCPG(static_cast<const CPG*>(cpg), svfManager);
    ccpg->build();
    //ExecutionTimer::getInstance()->stop("CCPG Analysis");

    ccpg->dump(TargetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Data Race Detection");
    DataRaceDetector* drd = new DataRaceDetector();
    drd->detect();
    ExecutionTimer::getInstance()->stop("Data Race Detection");
    drd->printDataRaces(TargetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Use After Free Detection");
    UseAfterFreeDetector * uafd = new UseAfterFreeDetector();
    uafd->detect();
    ExecutionTimer::getInstance()->stop("Use After Free Detection");
    uafd->printUseAfterFrees(TargetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Double Free Detection");
    DoubleFreeDetector * dfd = new DoubleFreeDetector();
    dfd->detect();
    ExecutionTimer::getInstance()->stop("Double Free Detection");
    dfd->printDoubleFrees(TargetPath->getOutputDir());

    ExecutionTimer::getInstance()->start("Null Reference Detection");
    NullReferenceDetector * nrd = new NullReferenceDetector();
    nrd->detect();
    ExecutionTimer::getInstance()->stop("Null Reference Detection");
    nrd->printNullReferences(TargetPath->getOutputDir());

    ExecutionTimer::getInstance()->printAllTimes(TargetPath->getOutputDir());

    
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



