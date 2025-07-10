#include "CCPG/LSAnalysis.h"

class DataRace;
class UseAfterFree;
class DoubleFree;

class DataRaceDetector {

    public:
        void detect();
        void printDataRaces(fs::path outputDir);

        void addDataRace(DataRace * datarace) {
            dataraces.insert(datarace);
        }
        std::string toString() {
            return "";
        }

    private:
        std::set<DataRace *> dataraces;
};

class DataRace {
    public:
        DataRace(const Context& ctx1, NodeLoc loc1, const SVF::SVFStmt* stmt1, const std::vector<Lock*>& locks1, const std::string& code1,
                const Context& ctx2, NodeLoc loc2, const SVF::SVFStmt* stmt2, const std::vector<Lock*>& locks2, const std::string& code2)
        : ctx1(ctx1), loc1(loc1), stmt1(stmt1), locks1(locks1), code1(code1), 
          ctx2(ctx2), loc2(loc2), stmt2(stmt2), locks2(locks2), code2(code2) {}
        ~DataRace() {}

        const Context* getCtx1() const { return &ctx1; }
        const Context* getCtx2() const { return &ctx2; }
        NodeLoc getLoc1() { return loc1; }
        NodeLoc getLoc2() { return loc2; }
        const SVF::SVFStmt * getStmt1() { return stmt1; }
        const SVF::SVFStmt * getStmt2() { return stmt2; }
        const std::vector<Lock*>& getLocks1() const { return locks1; }
        const std::vector<Lock*>& getLocks2() const { return locks2; }

        std::string toString() {
            std::stringstream ss;
        
            // 格式化上下文信息
            auto formatContext = [](const Context& ctx) -> std::string {
                if (ctx.getCallStack().empty()) {
                    return "null-context";
                }
                std::string contextStr = ctx.toString();
                // Replace newlines with an indented newline
                std::string result;
                std::istringstream iss(contextStr);
                std::string line;
                bool first = true;
                while(std::getline(iss, line)) {
                    if (!first) {
                        result += "\n               "; // alignment for subsequent lines
                    }
                    result += line;
                    first = false;
                }
                return result;
            };

            // 格式化代码位置和内容
            auto formatSourceInfo = [](const NodeLoc& loc) -> std::string {
                return loc.getFileName() + ":" + std::to_string(loc.getLineNumber());
            };

            // 格式化锁集
            auto formatLocks = [](const std::vector<Lock*>& locks) -> std::string {
                if (locks.empty()) return "{}";
                std::stringstream locksStream;
                locksStream << "{ ";
                for (size_t i = 0; i < locks.size(); ++i) {
                    locksStream << "`" << locks[i]->getAcquire()->getCPGNode()->getCode() << "`";
                    if (i < locks.size() - 1) locksStream << ", ";
                }
                locksStream << " }";
                return locksStream.str();
            };

            // 组合最终输出
            ss << "========== Data Race Detected ==========\n"
               << "Access 1:\n"
               << "├── Location:  " << formatSourceInfo(loc1) << "\n"
               << "│   └── Code:      " << code1 << "\n"
               << "├── Lockset:   " << formatLocks(locks1) << "\n"
               << "└── Context:   " << formatContext(ctx1) << "\n\n"
               << "Access 2:\n"
               << "├── Location:  " << formatSourceInfo(loc2) << "\n"
               << "│   └── Code:      " << code2 << "\n"
               << "├── Lockset:   " << formatLocks(locks2) << "\n"
               << "└── Context:   " << formatContext(ctx2) << "\n"
               << "========================================";

            return ss.str();
        }

    private:
        Context ctx1;
        NodeLoc loc1;
        const SVF::SVFStmt * stmt1;
        std::vector<Lock *> locks1;
        std::string code1;
        Context ctx2;
        NodeLoc loc2;
        const SVF::SVFStmt * stmt2;
        std::vector<Lock *> locks2;
        std::string code2;
};


