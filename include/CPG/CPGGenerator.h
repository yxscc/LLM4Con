#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include <string>
#include <fstream>

#include "CPG.h"
#include "Node.h"
#include "Edge.h"

using namespace std;
namespace fs = std::filesystem;


class CPGGenerator {
private:
    CPG *cpg;

public:
    CPGGenerator() {
        cpg = new CPG();
    }
    ~CPGGenerator() { //delete cpg; 
    }

    fs::path generateCPG(std::string dir);
    
    //std::string convertToBC(const string& dir);

    CPG * buildCPGByDot(std::string dir);

    CPG * buildLLVMCPG(std::string dir);

};

