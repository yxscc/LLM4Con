#include "SVFUtil/SVFManager.h"
#include "SVF-LLVM/LLVMModule.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "WPA/Andersen.h"
#include "Graphs/SVFG.h"
#include "Graphs/VFG.h"
#include "Graphs/PTACallGraph.h"

// Initialize static instance
SVFManager* SVFManager::instance = nullptr;

SVFManager* SVFManager::getInstance() {
    if (instance == nullptr) {
        instance = new SVFManager();
    }
    return instance;
}

SVFManager::SVFManager() :
    svfModule(nullptr),
    pag(nullptr),
    ander(nullptr),
    svfg(nullptr),
    callgraph(nullptr),
    icfg(nullptr),
    vfg(nullptr) {}

SVFManager::~SVFManager() {
    // The lifetime of SVF objects is complex.
    // SVFModule and Andersen analysis have their own release functions.
    // Other objects are often owned by the main ones.
    // A simple deletion might be enough if ownership is clear.
    delete vfg;
    delete svfg;
    // Releasing Andersen might clean up related graphs.
    if (ander) {
        SVF::AndersenWaveDiff::releaseAndersenWaveDiff();
    }
    // SVFModule is managed by LLVMModuleSet, so we don't delete it here.
}

void SVFManager::runSVFAnalysis(const std::vector<std::string>& moduleNameVec) {
    // This block is moved from main.cpp
    SVF::LLVMModuleSet* moduleSet = SVF::LLVMModuleSet::getLLVMModuleSet();

    this->svfModule = moduleSet->buildSVFModule(moduleNameVec);
    SVF::SVFIRBuilder builder(this->svfModule);
    this->pag = builder.build();
    this->icfg = this->pag->getICFG();
    this->ander = SVF::AndersenWaveDiff::createAndersenWaveDiff(this->pag);
    this->callgraph = this->ander->getCallGraph();
    this->vfg = new SVF::VFG(this->callgraph);
    SVF::SVFGBuilder svfBuilder(true);
    this->svfg = svfBuilder.buildFullSVFG(this->ander);
}
