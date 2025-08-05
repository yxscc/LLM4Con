/*#include "CCPG/LSAnalysis.h"

class NullReference;
class UseAfterFree;
class DoubleFree;

class NullReferenceDetector {

    public:
        void detect();
        void printNullReferences(fs::path outputDir);

        void addNullReference(NullReference * nullReference) {
            nprs.insert(nullReference);
        }
        bool isNullReference(const SVFStmt * stmt1, const SVFStmt * stmt2);
        std::string toString() {
            return "";
        }

    private:
        std::set<NullReference *> nprs;
};

class NullReference {
    public:
        NullReference(const Context& ctx1, NodeLoc loc1, const SVF::SVFStmt* stmt1,
                const Context& ctx2, NodeLoc loc2, const SVF::SVFStmt* stmt2)
        : ctx1(ctx1), loc1(loc1), stmt1(stmt1), ctx2(ctx2), loc2(loc2), stmt2(stmt2) {}
        ~NullReference() {}

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
            ss << "Null Pointer Reference Detected:\n"
            << "├─ Thread Context 1: " << formatContext(ctx1) << "\n"
            << "│  ├─ Location: " << formatLoc(loc1) << "\n"
            << "└─ Thread Context 2: " << formatContext(ctx2) << "\n"
            << "   ├─ Location: " << formatLoc(loc2);

            return ss.str();
        }

    private:
        Context ctx1;
        NodeLoc loc1;
        const SVF::SVFStmt * stmt1;
        Context ctx2;
        NodeLoc loc2;
        const SVF::SVFStmt * stmt2;
};


*/