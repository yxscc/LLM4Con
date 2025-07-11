// SVFManager.h
#ifndef SVFMANAGER_H 
#define SVFMANAGER_H

#include <string>
#include <vector>

// Forward declarations to reduce header dependencies
#include "WPA/Andersen.h"
#include "Graphs/SVFG.h"
#include "Graphs/PTACallGraph.h"
#include "Graphs/ICFG.h"
#include "Graphs/VFG.h"
#include "SVFIR/SVFModule.h"
#include "SVFIR/SVFIR.h"


class SVFManager {
public:
    // Get the singleton instance
    static SVFManager* getInstance();

    // Delete copy constructor and assignment operator
    SVFManager(const SVFManager&) = delete;
    void operator=(const SVFManager&) = delete;

    // Run the SVF analysis
    void runSVFAnalysis(const std::vector<std::string>& moduleNameVec);

    // Accessors
    SVF::SVFModule* getSVFModule() { return svfModule; }
    SVF::SVFIR* getSVFIR() { return pag; }
    SVF::Andersen* getAndersen() { return ander; }
    SVF::PTACallGraph* getPTACallGraph() { return callgraph; }
    SVF::ICFG* getICFG() { return icfg; }
    SVF::VFG* getVFG() { return vfg; }
    SVF::SVFG* getSVFG() { return svfg; }

private:
    // Private constructor and destructor
    SVFManager();
    ~SVFManager();

    SVF::SVFModule* svfModule;
    SVF::SVFIR* pag;
    SVF::Andersen* ander;
    SVF::SVFG* svfg;
    SVF::PTACallGraph* callgraph;
    SVF::ICFG* icfg;
    SVF::VFG* vfg;

    static SVFManager* instance;  // Static instance pointer
};

#endif // End of SVFMANAGER_H
