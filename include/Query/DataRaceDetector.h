// in: include/Query/DataRaceDetector.h

/*#ifndef DATA_RACE_DETECTOR_H
#define DATA_RACE_DETECTOR_H

#include "CCPG/LSAnalysis.h"
#include <sstream>      // For std::stringstream
#include <string>
#include <llvm/IR/Instruction.h> // <-- Include LLVM Instruction header
#include <llvm/Support/raw_ostream.h> // <-- For printing instructions

class DataRace;

class DataRaceDetector {
public:
    void detect();
    void printDataRaces(fs::path outputDir);

    void addDataRace(DataRace* datarace) {
        dataraces.insert(datarace);
    }
    std::string toString() {
        return "";
    }

private:
    std::set<DataRace*> dataraces;
};

// --- Refactored DataRace class ---
class DataRace {
public:
    // Constructor now takes llvm::Instruction*
    DataRace(const Context& ctx1, NodeLoc loc1, const llvm::Instruction* inst1, const std::vector<Lock*>& locks1, const std::string& code1,
             const Context& ctx2, NodeLoc loc2, const llvm::Instruction* inst2, const std::vector<Lock*>& locks2, const std::string& code2)
        : ctx1(ctx1), loc1(loc1), inst1(inst1), locks1(locks1), code1(code1), 
          ctx2(ctx2), loc2(loc2), inst2(inst2), locks2(locks2), code2(code2) {}
    
    ~DataRace() = default;

    // Getters for the LLVM instructions
    const llvm::Instruction* getInst1() const { return inst1; }
    const llvm::Instruction* getInst2() const { return inst2; }
    
    // Other getters remain the same
    const Context* getCtx1() const { return &ctx1; }
    const Context* getCtx2() const { return &ctx2; }
    NodeLoc getLoc1() { return loc1; }
    NodeLoc getLoc2() { return loc2; }
    const std::vector<Lock*>& getLocks1() const { return locks1; }
    const std::vector<Lock*>& getLocks2() const { return locks2; }

    // Updated toString() method
    std::string toString() {
        std::stringstream ss;

        // Helper to format an LLVM instruction to a string
        auto formatInstruction = [](const llvm::Instruction* inst) -> std::string {
            if (!inst) return "null-instruction";
            std::string instStr;
            llvm::raw_string_ostream os(instStr);
            inst->print(os);
            // The output might have leading spaces/tabs, let's trim it
            size_t first = instStr.find_first_not_of(" \t");
            if (std::string::npos == first) return instStr;
            return instStr.substr(first);
        };
        
        // (The formatContext, formatSourceInfo, and formatLocks lambdas remain the same as your version)
        auto formatContext = [](const Context& ctx) -> std::string { /* ... same as before ...  };
        auto formatSourceInfo = [](const NodeLoc& loc) -> std::string { /* ... same as before ... };
        auto formatLocks = [](const std::vector<Lock*>& locks) -> std::string { /* ... same as before ... };

        ss << "========== Data Race Detected ==========\n"
           << "Access 1:\n"
           << "├── Location:  " << formatSourceInfo(loc1) << "\n"
           << "│   ├── Code:      " << code1 << "\n"
           << "│   └── LLVM IR:   " << formatInstruction(inst1) << "\n"
           << "├── Lockset:   " << formatLocks(locks1) << "\n"
           << "└── Context:   " << formatContext(ctx1) << "\n\n"
           << "Access 2:\n"
           << "├── Location:  " << formatSourceInfo(loc2) << "\n"
           << "│   ├── Code:      " << code2 << "\n"
           << "│   └── LLVM IR:   " << formatInstruction(inst2) << "\n"
           << "├── Lockset:   " << formatLocks(locks2) << "\n"
           << "└── Context:   " << formatContext(ctx2) << "\n"
           << "========================================";

        return ss.str();
    }

private:
    Context ctx1;
    NodeLoc loc1;
    const llvm::Instruction* inst1;
    std::vector<Lock*> locks1;
    std::string code1;
    Context ctx2;
    NodeLoc loc2;
    const llvm::Instruction* inst2;
    std::vector<Lock*> locks2;
    std::string code2;
};

#endif */