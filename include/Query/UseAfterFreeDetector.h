#include "CCPG/LSAnalysis.h"

class UseAfterFree;

class UseAfterFreeDetector {

    public:
        void detect();
        void printUseAfterFrees(fs::path outputDir);

        void addUseAfterFree(UseAfterFree * uaf) {
            uafs.insert(uaf);
        }
        std::string toString() {
            return "";
        }

    private:
        std::set<UseAfterFree *> uafs;
};

class UseAfterFree {
    public:
        UseAfterFree(const Context& ctx1, NodeLoc loc1, const CallICFGNode * free,
                    const Context& ctx2, NodeLoc loc2, const SVFStmt * use)
        : ctx1(ctx1), loc1(loc1), free(free), ctx2(ctx2), loc2(loc2), use(use) {} 
        ~UseAfterFree() {}

        const Context* getCtx1() const { return &ctx1; }
        const Context* getCtx2() const { return &ctx2; }
        NodeLoc getLoc1() { return loc1; }
        NodeLoc getLoc2() { return loc2; }
        const SVF::CallICFGNode * getFree() { return free; }
        const SVF::SVFStmt * getUse() { return use; }

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

            // 格式化CallICFGNode
            auto formatCallNode = [](const CallICFGNode * callNode) -> std::string {
                if (!callNode) return "null-callnode";
                return callNode->getCalledFunction()->getName();
            };
            // 格式化SVF语句
            auto formatStmt = [](const SVF::SVFStmt* stmt) -> std::string {
                if (!stmt) return "null-stmt";
                
                // 获取基础信息
                std::string type;
                std::string srcInfo = stmt->getInst()->getSourceLoc();
                
                // 关键操作信息提取
                std::string content;
                if (const LoadStmt* load = SVFUtil::dyn_cast<LoadStmt>(stmt)) {
                    type = "Load";
                    content = "Load from: " + load->getRHSVar()->toString();
                } else if (const StoreStmt* store = SVFUtil::dyn_cast<StoreStmt>(stmt)) {
                    type = "Store";
                    content = "Store to: " + store->getLHSVar()->toString();
                } else {
                    content = stmt->toString();
                }
                
                return type + " [" + srcInfo + "] " + content;
            };

            // 组合最终输出
            ss << "Use After Free Detected:\n"
            << "├─ Thread Context 1: " << formatContext(ctx1) << "\n"
            << "│  ├─ Location: " << formatLoc(loc1) << "\n"
            << "│  └─ Operation: " << formatCallNode(free) << "\n"
            << "└─ Thread Context 2: " << formatContext(ctx2) << "\n"
            << "   ├─ Location: " << formatLoc(loc2) << "\n"
            << "   └─ Operation: " << formatStmt(use);

            return ss.str();
        }

    private:
        Context ctx1;
        NodeLoc loc1;
        const CallICFGNode * free;
        Context ctx2;
        NodeLoc loc2;
        const SVFStmt * use;
};