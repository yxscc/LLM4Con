// SVFManager.h
#ifndef SVFMANAGER_H 
#define SVFMANAGER_H

#include <set>
#include <vector>


#include "WPA/Andersen.h"
#include "MTA/MTA.h"
#include "MemoryModel/PointerAnalysisImpl.h"
#include "SVFIR/SVFValue.h"
#include "CPG/Node.h"
#include "Graphs/SVFG.h"
#include "Graphs/SVFGStat.h"
#include "Graphs/PTACallGraph.h"


namespace SVF
{

class PointerAnalysis;
class AndersenWaveDiff;
class LockAnalysis;
class SVFModule;
class SVFG;
class Andersen;
class PTACallGraph;
class ICFG;
class VFG;

class SVFManager{
private:
    SVFModule* svfModule;
    SVFIR* pag;
    Andersen* ander;
    SVFG* svfg;
    PTACallGraph* callgraph;
    ICFG* icfg;
    VFG* vfg;

    static SVFManager* instance;  // 静态实例指针

    SVFManager(SVFModule* svfModule, SVFIR* pag, Andersen* ander, SVFG* svfg, PTACallGraph* callgraph, ICFG* icfg, VFG* vfg){
        this->svfModule = svfModule;
        this->pag = pag;
        this->ander = ander;
        this->svfg = svfg;
        this->callgraph = callgraph;
        this->icfg = icfg;
        this->vfg = vfg;
    };
    ~SVFManager() {}  // 私有析构函数
    SVFManager(const SVFManager&) = delete;            // 阻止复制构造
    SVFManager& operator=(const SVFManager&) = delete; // 阻止赋值操作

public:
    /*SVFManager(SVFModule* svfModule, SVFIR* pag, Andersen* ander, SVFG* svfg, PTACallGraph* callgraph, ICFG* icfg, VFG* vfg){
        this->svfModule = svfModule;
        this->pag = pag;
        this->ander = ander;
        this->svfg = svfg;
        this->callgraph = callgraph;
        this->icfg = icfg;
        this->vfg = vfg;
    };*/

    static SVFManager* build(SVFModule* svfModule, SVFIR* pag, Andersen* ander, SVFG* svfg, PTACallGraph* callgraph, ICFG* icfg, VFG* vfg) {
        if (instance == nullptr) {
            instance = new SVFManager(svfModule, pag, ander, svfg, callgraph, icfg, vfg);
        }
        return instance;
    }

    // Accessor method to get the singleton instance
    static SVFManager* getInstance() {
        if (instance == nullptr) {
            throw std::runtime_error("SVFManager is not built yet. Call build() first.");
        }
        return instance;
    }


    // 获取SVFModule
    SVFModule* getSVFModule(){
        return svfModule;
    }

    // 获取SVFIR
    SVFIR* getSVFIR(){
        return pag;
    }

    // 获取Andersen
    Andersen* getAndersen(){
        return ander;
    }

    // 获取PTACallGraph
    PTACallGraph* getPTACallGraph(){
        return callgraph;
    }

    // 获取ICFG
    ICFG* getICFG(){
        return icfg;
    }

    // 获取VFG
    VFG* getVFG(){
        return vfg;
    }

    // 获取SVFG
    SVFG* getSVFG(){
        return svfg;
    }

};

} // End of SVF namespace

#endif // End of SVFMANAGER_H