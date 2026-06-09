// CPG.h
#ifndef CPG_H
#define CPG_H

#include "Node.h"
#include "Edge.h"
#include "Util/PathUtils.h"
#include <vector>
#include <memory>
#include <filesystem>
#include <iostream>
#include <cstring>
#include <regex>

namespace fs = std::filesystem;

namespace llvm {
    class Function;
    class Instruction;
} // namespace llvm

typedef std::unordered_set<Node*> CPGNodeSet;

class CPG {
private:
    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<std::unique_ptr<Edge>> m_edges;
    std::unordered_map<std::string, Node*> id2Node; // 根据ID查找节点
    CPGNodeSet mainNodes; // 主函数节点
    
    std::unordered_map<std::string, CPGNodeSet> type2Nodes; // 根据类型查找节点
    std::unordered_map<std::string, CPGNodeSet> file2Methods; // 根据名称查找节点

public:
    CPG() {}
    ~CPG() = default;

    void addNode(std::unique_ptr<Node> node);
    void addEdge(std::unique_ptr<Edge> edge);
    Node* findNode(const std::string& id) const;

    // CPGNodeSet getNodes() const { return nodes; }
    // std::unordered_set<Edge*> getEdges() const { return edges; }

    CPGNodeSet getMainNodes() const { return mainNodes; }
    bool isMainNode(Node* node) const { return mainNodes.find(node) != mainNodes.end(); }

    bool isLoopBeginNode(Node* node) const;

    bool isConditionBeginNode(Node* node) const;

    CPGNodeSet getNodesByType(std::string type) const {
        if(type2Nodes.find(type) == type2Nodes.end()) {
            return CPGNodeSet();
        }
        return type2Nodes.at(type);
    }

    CPGNodeSet getMethodsByFileName(std::string fileName) const {
        std::string result = fileName;
    
        if (result.compare(0, 2, "..") == 0) {
            result = result.substr(2);
        } else if (result.compare(0, 1, ".") == 0) {
            result = result.substr(1);
        }
    
        auto it = file2Methods.find(result);
        if (it != file2Methods.end()) {
            return it->second;
        }

        for (const auto& [key, methods] : file2Methods) {
            if(key.empty()) continue;
            if (PathUtils::arePathsLikelySameFile(key, result)) {
                return methods;
            }
        }

        // Layer 4 fallback: basename-only match. The existing
        // arePathsLikelySameFile is too strict because it walks segments
        // from the tail and rejects as soon as a parent directory differs.
        // For kernel CVEs the IR debug-info path (e.g. "block/blk-ioc.c")
        // and the CPG path (e.g. "src/blk-ioc.c") share only the base
        // filename, so both Layer 1 (exact) and the segment fuzzy match
        // miss. We accept basename equality as a last-resort alias and
        // collect every CPG entry whose filename ends with the requested
        // basename. This only runs after Layer 1/2/3 have all failed, so
        // the existing baseline behaviour is preserved.
        std::string base = std::filesystem::path(result).filename().string();
        if (base.empty()) return CPGNodeSet();
        CPGNodeSet aggregated;
        for (const auto& [key, methods] : file2Methods) {
            if (key.empty()) continue;
            std::string keyBase = std::filesystem::path(key).filename().string();
            if (keyBase == base) {
                aggregated.insert(methods.begin(), methods.end());
            }
        }
        return aggregated;
    }

    // Generate the Linux syscall-entry naming variants for a given "base"
    // name. The kernel's SYSCALL_DEFINE macro produces several alias
    // functions in the IR (architecture-specific prefixes, internal helpers)
    // that all refer to the same C-level body. The CPG, however, typically
    // only contains the unprefixed base. This helper lets findMethod fall
    // back through the common variants so the IR entry name can still be
    // mapped to the CPG method node.
    // Generate fall-back name variants for LLVM-mangled / decorated symbols.
    // LLVM passes (and certain GCC compatibility layers) decorate function
    // names with suffixes that the C-level CPG never sees. Examples:
    //   foo.123, foo.llvm.7BAD1234, foo.part.0, foo.isra.5,
    //   foo.constprop.1, foo.cold, __llvm.foo, local_foo
    // After Layer 1 (exact) and Layer 2 (syscall prefix) miss, try each
    // demangled candidate so the IR-side decorated name still maps to the
    // bare CPG method.
    static std::vector<std::string> demangleVariants(const std::string& name) {
        std::vector<std::string> out;
        out.push_back(name);
        static const std::regex kSuffix(
            R"((\.(?:llvm\.\w+|part\.\d+|isra\.\d+|constprop\.\d+|cold|\d+))+$)");
        static const std::regex kPrefix(R"(^(?:__llvm\.|local_))");
        std::string s = std::regex_replace(name, kSuffix, "");
        if (s != name) out.push_back(s);
        std::string p = std::regex_replace(s, kPrefix, "");
        if (p != s) out.push_back(p);
        return out;
    }

    static std::vector<std::string> syscallNameVariants(const std::string& name) {
        std::vector<std::string> out;
        out.push_back(name);
        // Well-known prefixes produced by SYSCALL_DEFINE* macro expansion.
        static const char* kPrefixes[] = {
            "__x64_sys_", "__ia32_sys_", "__arm64_sys_", "__arm_sys_",
            "__riscv_sys_", "__powerpc_sys_", "__se_sys_", "__do_sys_",
            "SyS_", "sys_"
        };
        std::string base = name;
        for (const char* p : kPrefixes) {
            std::size_t plen = std::strlen(p);
            if (base.size() > plen && base.compare(0, plen, p) == 0) {
                base = base.substr(plen);
                break;
            }
        }
        if (base != name) out.push_back(base);
        // Also try each prefix applied to the stripped base, so that when
        // the CPG has one form and the IR has another we can find either.
        for (const char* p : kPrefixes) {
            std::string candidate = std::string(p) + base;
            if (candidate != name) out.push_back(candidate);
        }
        return out;
    }

    // Build the full ordered candidate list across Layer 1 (exact),
    // Layer 2 (syscall variants) and Layer 3 (LLVM-mangling demangle).
    // Layer 3 candidates are appended AFTER Layer 1/2 so the existing
    // baseline name resolution is never overruled.
    static std::vector<std::string> allNameCandidates(const std::string& name) {
        std::vector<std::string> out = syscallNameVariants(name);
        std::unordered_set<std::string> seen(out.begin(), out.end());
        for (const std::string& v : demangleVariants(name)) {
            if (seen.insert(v).second) out.push_back(v);
        }
        return out;
    }

    // Returns true when a method node is a pure forward declaration / stub:
    // it carries a signature in CODE but no real body, which the CPG models as
    // a single CFG edge straight to the Method_return.
    static bool isStubMethodNode(Node* methodNode) {
        if (methodNode->outCFGEdges.size() != 1) return false;
        Edge* edge = *methodNode->outCFGEdges.begin();
        Node* nextNode = edge->getToNode();
        return nextNode && nextNode->getType() == "Method_return";
    }

    Node* findMethod(std::string name) const {

        auto it = type2Nodes.find("Method");
        if (it == type2Nodes.end()) return nullptr;
        const CPGNodeSet& methodNodes = it->second;

        // Pass 1 (preferred): a real definition with a CFG body. This is the
        // original behaviour, so any case that already mapped is unchanged.
        for (const std::string& candidate : allNameCandidates(name)) {
            for(Node* methodNode : methodNodes){
                if(methodNode->getName() == candidate &&
                   methodNode->properties["CODE"] != "<empty>"){
                    if(isStubMethodNode(methodNode)){
                        continue;
                    }
                    return methodNode;
                }
            }
        }

        // Pass 2 (fallback): no full-CFG node exists for this name. This is the
        // c2cpg-on-partial-kernel-source case — the function body is present in
        // the AST but the CFG layer came back degenerate (single edge), so
        // Pass 1 skipped it. Rather than dropping the entry entirely (which
        // loses the whole thread and is the dominant "Not found in CPG" recall
        // hole), accept the best AST-only node here. We pick the candidate with
        // the longest CODE: a real definition embeds its full `{ … }` body in
        // CODE, while a bare forward declaration only has the signature, so
        // longest-CODE reliably prefers the definition over a prototype.
        Node* bestAstOnly = nullptr;
        std::size_t bestCodeLen = 0;
        for (const std::string& candidate : allNameCandidates(name)) {
            for(Node* methodNode : methodNodes){
                if(methodNode->getName() != candidate) continue;
                const std::string& code = methodNode->properties["CODE"];
                if(code == "<empty>") continue;
                if(code.size() > bestCodeLen){
                    bestCodeLen = code.size();
                    bestAstOnly = methodNode;
                }
            }
            if (bestAstOnly) return bestAstOnly;
        }
        return nullptr;
    }

    std::unordered_set<Node*> findMethodsByName(std::string name) const {
        std::unordered_set<Node*> methods;
        auto it = type2Nodes.find("Method");
        if (it == type2Nodes.end()) return methods;
        const CPGNodeSet& methodNodes = it->second;

        // Try all syscall + demangle name variants so IR-side decorated
        // names (e.g. __x64_sys_foo, foo.llvm.7BAD1234) also match CPG
        // nodes named sys_foo / foo.
        std::vector<std::string> candidates = allNameCandidates(name);
        std::unordered_set<std::string> tried(candidates.begin(), candidates.end());

        for(Node* methodNode : methodNodes){
            const std::string& mname = methodNode->getName();
            if(tried.count(mname) && methodNode->properties["CODE"] != "<empty>"){
                if(methodNode->outCFGEdges.size() == 1){
                    std::unordered_set<Edge*> outEdges = methodNode->outCFGEdges;
                    Edge* edge = *outEdges.begin();
                    Node* nextNode = edge->getToNode();
                    if(nextNode->getType() == "Method_return"){
                        continue;
                    }
                }
                methods.insert(methodNode);
            }
        }
        return methods;
    }

    // Layer 5: when all lookups fail, surface the "closest" known method
    // names as suggestions for diagnostic logs / LLM feedback. Distance
    // metric is the simpler "common substring" + length-delta proxy
    // (full Levenshtein is overkill here and much slower over large CPGs).
    std::vector<std::string> findMethodSuggestions(const std::string& name,
                                                   std::size_t k = 5) const {
        auto it = type2Nodes.find("Method");
        if (it == type2Nodes.end()) return {};
        const CPGNodeSet& methodNodes = it->second;

        auto score = [](const std::string& a, const std::string& b) -> int {
            if (a.empty() || b.empty()) return -1;
            // Substring containment is the strongest signal.
            if (a.find(b) != std::string::npos) return 1000 - (int)(a.size() - b.size());
            if (b.find(a) != std::string::npos) return 1000 - (int)(b.size() - a.size());
            // Otherwise count shared prefix and shared suffix as a cheap
            // approximation of edit distance.
            int prefix = 0, suffix = 0;
            for (std::size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
                if (a[i] == b[i]) ++prefix; else break;
            }
            for (std::size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
                if (a[a.size()-1-i] == b[b.size()-1-i]) ++suffix; else break;
            }
            int delta = (int)std::abs((int)a.size() - (int)b.size());
            return prefix * 4 + suffix * 4 - delta;
        };

        std::vector<std::pair<int, std::string>> scored;
        scored.reserve(methodNodes.size());
        std::unordered_set<std::string> seen;
        for (Node* m : methodNodes) {
            const std::string& mname = m->getName();
            if (mname.empty() || !seen.insert(mname).second) continue;
            if (m->properties.count("CODE") && m->properties.at("CODE") == "<empty>") continue;
            int s = score(mname, name);
            if (s > 0) scored.emplace_back(s, mname);
        }
        std::sort(scored.begin(), scored.end(),
                  [](const auto& a, const auto& b){ return a.first > b.first; });

        std::vector<std::string> out;
        for (auto& p : scored) {
            if (out.size() >= k) break;
            out.push_back(std::move(p.second));
        }
        return out;
    }

    Node* findMethod(Node* node) const {
        bool hasCallEdge = false;
        Node* method = nullptr;
        std::string callerName = node->getName();
        std::string callFullName = node->getProperty("METHOD_FULL_NAME");
        std::string callerFile = node->getFileName();
        if (callerFile.empty()) {
            for (Edge* edge : node->inEdges) {
                if (edge->getType() == "Contains") {
                    Node* parent = edge->getFromNode();
                    if (parent && parent->getType() == "Method") {
                        callerFile = parent->getFileName();
                        break;
                    }
                }
            }
        }
        if (!callFullName.empty() && callFullName != "<empty>") {
            CPGNodeSet methodNodes = type2Nodes.at("Method");
            Node* bestFullMatch = nullptr;
            for (Node* methodNode : methodNodes) {
                std::string fullName = methodNode->getProperty("FULL_NAME");
                if (!fullName.empty() && fullName == callFullName) {
                    if (methodNode->properties["CODE"] != "<empty>") {
                        if (!callerFile.empty() &&
                            methodNode->getFileName() == callerFile) {
                            return methodNode;
                        }
                        if (!bestFullMatch) {
                            bestFullMatch = methodNode;
                        }
                    }
                }
            }
            if (bestFullMatch) {
                return bestFullMatch;
            }
        }
        for(auto edge : node->outEdges){
            std::string calleeName = edge->getToNode()->getName();
            std::string type = edge->getType();
            if(edge->getType() == "Call"
            && (calleeName.find(callerName) != std::string::npos || callerName.find(calleeName) != std::string::npos)
            && edge->getToNode()->properties["CODE"] != "<empty>"){
                if(method == nullptr){
                    method = edge->getToNode();
                    hasCallEdge = true;
                }
                else{
                    Node* temp = edge->getToNode();
                    if(temp->properties["CODE"].length() > method->properties["CODE"].length()){
                        method = temp;
                    }
                }
            }

        }
        if(hasCallEdge){
            return method;
        }
        else{
            std::string name;
            Node* fallback = findMethod(node->getName());
            return fallback;
        }
    }

    Node * findMethodByLLVMFunction(const llvm::Function* llvmFunc) const;
    
    bool hasContainsEdge(Node * callee, Node * node) const{
        for(auto edge : node->inEdges){
            if(edge->getType() == "Contains" && edge->getFromNode() == callee){
                return true;
            }
        }
        return false;
    }

};

#endif // CPG_H
