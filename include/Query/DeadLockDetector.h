#include "CCPG/LSAnalysis.h"

class DeadLock;

class DeadLockDetector {

    public:
        void detect();
        void printDeadLocks(fs::path outputDir);

        void addDeadLock(DeadLock * deadlock) {
            deadlocks.insert(deadlock);
        }
        std::string toString() {
            return "";
        }

    private:
        std::set<DeadLock *> deadlocks;
};

class DeadLock {
    public:
        DeadLock(const Context& ctx1, NodeLoc loc1,
                const Context& ctx2, NodeLoc loc2)
        : ctx1(ctx1), loc1(loc1), locks1(locks1), ctx2(ctx2), loc2(loc2), locks2(locks2) {}
        ~DeadLock() {}

        const Context* getCtx1() const { return &ctx1; }
        const Context* getCtx2() const { return &ctx2; }
        NodeLoc getLoc1() { return loc1; }
        NodeLoc getLoc2() { return loc2; }
        const SVF::SVFStmt * getStmt1() { return stmt1; }
        const SVF::SVFStmt * getStmt2() { return stmt2; }

        std::string toString() {
            std::stringstream ss;
            
            // 格式化上下文信息
            auto formatContext = [](Context ctx) -> std::string {
                return ( ctx.getCallStack().size() != 0 ) ? ctx.toString() : "null-context";
            };
        
            // 格式化代码位置
            auto formatLoc = [](NodeLoc loc) -> std::string {
                return "L" + std::to_string(loc.getLineNumber()) 
                    + ":" + loc.getFileName();
            };
        
            // 格式化Lockset（新增部分）
            auto formatLocks = [](const std::vector<Lock*>& locks) -> std::string {
                std::stringstream locksStream;
                locksStream << "{";
                for (size_t i = 0; i < locks.size(); ++i) {
                    if (i != 0) {
                        locksStream << ", ";
                    }
                    // 直接调用锁对象的getName()方法
                    locksStream << locks[i]->getAcquire()->getCPGNode()->getCode();  // 例如输出 "mutex", "rwlock"
                }
                locksStream << "}";
                return locksStream.str();
            };
        
            // 组合最终输出
            ss << "Dead Lock Detected:\n"
            << "├─ Thread Context 1: " << formatContext(ctx1) << "\n"
            << "│  ├─ Location: " << formatLoc(loc1) << "\n"
            << "│  └─ lockset: " << formatLocks(locks1) << "\n"
            << "└─ Thread Context 2: " << formatContext(ctx2) << "\n"
            << "   ├─ Location: " << formatLoc(loc2) << "\n"  // 补全换行符
            << "   └─ lockset: " << formatLocks(locks2) << "\n";
        
            return ss.str();
        }


        
    private:
        Context ctx1;
        NodeLoc loc1;
        const SVF::SVFStmt * stmt1;
        std::vector<Lock *> locks1;
        Context ctx2;
        NodeLoc loc2;
        const SVF::SVFStmt * stmt2;
        std::vector<Lock *> locks2;
};