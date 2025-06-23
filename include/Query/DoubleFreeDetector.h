#include "CCPG/LSAnalysis.h"

class DoubleFree;

class DoubleFreeDetector {

    public:
        void detect();
        void printDoubleFrees(fs::path outputDir);

        void addDoubleFree(DoubleFree * df) {
            dfs.insert(df);
        }
        std::string toString() {
            return "";
        }

    private:
        std::set<DoubleFree *> dfs;
};

class DoubleFree {
    public:
        DoubleFree(const Context& ctx1, NodeLoc loc1, const CallICFGNode * free1,
                    const Context& ctx2, NodeLoc loc2, const CallICFGNode * free2)
        : ctx1(ctx1), loc1(loc1), free1(free1), ctx2(ctx2), loc2(loc2), free2(free2) {} 
        ~DoubleFree() {}

        const Context* getCtx1() const { return &ctx1; }
        const Context* getCtx2() const { return &ctx2; }
        NodeLoc getLoc1() { return loc1; }
        NodeLoc getLoc2() { return loc2; }
        const SVF::CallICFGNode * getFree1() { return free1; }
        const SVF::CallICFGNode * getFree2() { return free2; }

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
            ss << "Double Free Detected:\n"
            << "├─ Thread Context 1: " << formatContext(ctx1) << "\n"
            << "│  ├─ Location: " << formatLoc(loc1) << "\n"
            << "│  └─ Operation: " << formatCallNode(free1) << "\n"
            << "└─ Thread Context 2: " << formatContext(ctx2) << "\n"
            << "   ├─ Location: " << formatLoc(loc2) << "\n"
            << "   └─ Operation: " << formatCallNode(free2);

            return ss.str();
        }

    private:
        Context ctx1;
        NodeLoc loc1;
        const CallICFGNode * free1;
        Context ctx2;
        NodeLoc loc2;
        const CallICFGNode * free2;
};