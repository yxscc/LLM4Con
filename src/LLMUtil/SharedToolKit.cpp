#include "LLMUtil/SharedToolKit.h"
#include "CCPG/CCPG.h"
#include "CCPG/CCPGNode.h"
#include "CPG/CPG.h"
#include "phasar.h"
#include "PhasarUtil/LLVMAnalyzer.h"
#include "Util/TargetPath.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

using namespace psr;

namespace llm_client {
namespace SharedToolKit {

namespace {

namespace fs = std::filesystem;

// Output bounds. A source tool that returns an unbounded blob is what pushes a
// session into its turn budget without adding information, so every reply here
// is clipped and says so.
constexpr int kMaxReadLines = 400;
constexpr int kDefaultReadLines = 200;
constexpr size_t kMaxReplyBytes = 48 * 1024;
constexpr int kMaxGrepHits = 200;
constexpr int kDefaultGrepHits = 50;
constexpr size_t kMaxGrepFileBytes = 4 * 1024 * 1024;
constexpr int kMaxGrepFiles = 20000;
constexpr int kMaxListEntries = 300;

fs::path sandboxRoot() {
    std::error_code ec;
    fs::path root = TargetPath::getInstance()->getTargetAbsolutePath();
    fs::path canon = fs::weakly_canonical(root, ec);
    return ec ? root : canon;
}

bool isInside(const fs::path& canonical, const fs::path& root) {
    const std::string c = canonical.string();
    const std::string r = root.string();
    if (c.rfind(r, 0) != 0) return false;
    return c.size() == r.size() || c[r.size()] == fs::path::preferred_separator;
}

// Resolves an agent-supplied path against the sandbox. Accepts a root-relative
// path ("net/mptcp/subflow.c"), a full path, and the leading-slash-stripped form
// that CPG node locations carry. Symlinks are resolved before the containment
// test, so a link pointing out of the tree is refused like any other escape.
std::optional<fs::path> resolveInSandbox(const std::string& raw) {
    const fs::path root = sandboxRoot();
    if (raw.empty()) return std::nullopt;

    std::vector<fs::path> attempts;
    fs::path given(raw);
    if (given.is_absolute()) {
        attempts.push_back(given);
    } else {
        attempts.push_back(root / given);
        // Node locations are emitted as absolute paths without the leading
        // separator; recover them so the agent can paste a location verbatim.
        attempts.emplace_back(fs::path("/") / given);
    }
    // A path that already contains the root anywhere (e.g. copied from a log
    // with a prefix) is re-anchored at the root.
    const std::string rootStr = root.string();
    if (const size_t at = raw.find(rootStr); at != std::string::npos) {
        attempts.emplace_back(raw.substr(at));
    }

    std::error_code ec;
    for (const fs::path& p : attempts) {
        fs::path canon = fs::weakly_canonical(p, ec);
        if (ec) { ec.clear(); continue; }
        if (isInside(canon, root) && fs::exists(canon, ec)) return canon;
        ec.clear();
    }
    return std::nullopt;
}

std::string relativeToRoot(const fs::path& p) {
    std::error_code ec;
    fs::path rel = fs::relative(p, sandboxRoot(), ec);
    return ec ? p.string() : rel.string();
}

std::string refusal(const std::string& raw) {
    nlohmann::json j = {
        {"error", "path is outside the analysed source tree or does not exist"},
        {"requested", raw},
        {"source_root", sandboxRoot().string()},
        {"hint", "pass a path relative to source_root, e.g. net/mptcp/subflow.c"}
    };
    return j.dump();
}

bool looksLikeSource(const fs::path& p) {
    static const std::vector<std::string> kExts = {
        ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hh", ".S", ".s"
    };
    const std::string ext = p.extension().string();
    return std::find(kExts.begin(), kExts.end(), ext) != kExts.end();
}

int clampInt(const nlohmann::json& args, const char* key, int fallback, int lo, int hi) {
    int v = fallback;
    if (args.contains(key) && args.at(key).is_number_integer())
        v = args.at(key).get<int>();
    return std::max(lo, std::min(hi, v));
}

std::string readFileTool(const nlohmann::json& args) {
    if (!args.contains("path") || !args.at("path").is_string())
        return R"({"error":"read_file needs a string 'path'."})";
    const std::string raw = args.at("path").get<std::string>();
    auto resolved = resolveInSandbox(raw);
    if (!resolved) return refusal(raw);
    if (!fs::is_regular_file(*resolved))
        return R"({"error":"path is not a regular file."})";

    const int start = clampInt(args, "start_line", 1, 1, 1 << 28);
    const int count = clampInt(args, "max_lines", kDefaultReadLines, 1, kMaxReadLines);

    std::ifstream in(*resolved);
    if (!in) return R"({"error":"cannot open file."})";

    std::ostringstream body;
    std::string line;
    int lineno = 0, emitted = 0;
    bool clipped = false;
    while (std::getline(in, line)) {
        ++lineno;
        if (lineno < start) continue;
        if (emitted >= count) { clipped = true; break; }
        body << lineno << '\t' << line << '\n';
        ++emitted;
        if (body.tellp() > static_cast<std::streampos>(kMaxReplyBytes)) {
            clipped = true;
            break;
        }
    }

    nlohmann::json j = {
        {"path", relativeToRoot(*resolved)},
        {"first_line", start},
        {"lines_returned", emitted},
        {"content", body.str()}
    };
    if (clipped)
        j["truncated"] = "more lines follow; call again with a later start_line";
    return j.dump();
}

std::string grepSourceTool(const nlohmann::json& args) {
    if (!args.contains("pattern") || !args.at("pattern").is_string())
        return R"({"error":"grep_source needs a string 'pattern'."})";
    const std::string pattern = args.at("pattern").get<std::string>();

    fs::path base = sandboxRoot();
    if (args.contains("path") && args.at("path").is_string() &&
        !args.at("path").get<std::string>().empty()) {
        auto resolved = resolveInSandbox(args.at("path").get<std::string>());
        if (!resolved) return refusal(args.at("path").get<std::string>());
        base = *resolved;
    }

    auto flags = std::regex::ECMAScript | std::regex::optimize;
    if (args.value("ignore_case", false)) flags |= std::regex::icase;
    std::regex re;
    try {
        re.assign(pattern, flags);
    } catch (const std::regex_error& e) {
        nlohmann::json j = {{"error", std::string("bad regex: ") + e.what()}};
        return j.dump();
    }

    const int limit = clampInt(args, "max_results", kDefaultGrepHits, 1, kMaxGrepHits);
    nlohmann::json hits = nlohmann::json::array();
    int scanned = 0;
    bool clipped = false;

    auto scanOne = [&](const fs::path& file) {
        std::error_code ec;
        if (fs::file_size(file, ec) > kMaxGrepFileBytes || ec) return;
        std::ifstream in(file);
        if (!in) return;
        std::string line;
        int lineno = 0;
        while (std::getline(in, line)) {
            ++lineno;
            if (line.size() > 2000) line.resize(2000);
            if (!std::regex_search(line, re)) continue;
            hits.push_back({{"path", relativeToRoot(file)},
                            {"line", lineno},
                            {"text", line}});
            if (static_cast<int>(hits.size()) >= limit) { clipped = true; return; }
        }
    };

    std::error_code ec;
    if (fs::is_regular_file(base)) {
        scanOne(base);
    } else {
        // follow_directory_symlink is deliberately absent: a link out of the
        // tree must not become a way to read the rest of the disk.
        fs::recursive_directory_iterator it(
            base, fs::directory_options::skip_permission_denied, ec), end;
        for (; !ec && it != end; it.increment(ec)) {
            if (static_cast<int>(hits.size()) >= limit || scanned >= kMaxGrepFiles) {
                clipped = true;
                break;
            }
            if (it->is_symlink(ec) || !it->is_regular_file(ec)) continue;
            if (!looksLikeSource(it->path())) continue;
            if (!isInside(it->path(), sandboxRoot())) continue;
            ++scanned;
            scanOne(it->path());
        }
    }

    nlohmann::json j = {
        {"pattern", pattern},
        {"searched_under", relativeToRoot(base)},
        {"files_scanned", scanned},
        {"match_count", hits.size()},
        {"matches", hits}
    };
    if (clipped) j["truncated"] = "result limit reached; narrow the pattern or path";
    return j.dump();
}

std::string listDirTool(const nlohmann::json& args) {
    fs::path base = sandboxRoot();
    if (args.contains("path") && args.at("path").is_string() &&
        !args.at("path").get<std::string>().empty()) {
        auto resolved = resolveInSandbox(args.at("path").get<std::string>());
        if (!resolved) return refusal(args.at("path").get<std::string>());
        base = *resolved;
    }
    if (!fs::is_directory(base))
        return R"({"error":"path is not a directory."})";

    nlohmann::json entries = nlohmann::json::array();
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(
             base, fs::directory_options::skip_permission_denied, ec)) {
        if (static_cast<int>(entries.size()) >= kMaxListEntries) break;
        entries.push_back({{"name", e.path().filename().string()},
                           {"dir", e.is_directory(ec)}});
    }
    std::sort(entries.begin(), entries.end(),
              [](const nlohmann::json& a, const nlohmann::json& b) {
                  return a.at("name").get<std::string>() <
                         b.at("name").get<std::string>();
              });
    nlohmann::json j = {{"path", relativeToRoot(base)}, {"entries", entries}};
    return j.dump();
}

} // namespace

std::vector<Tool> get_source_tools() {
    return {
        {"read_file",
         "Read a slice of a source file from the analysed tree. Paths are "
         "relative to the source root; anything outside it is refused.", {
            {"path", "string", "File path relative to the source root, e.g. net/mptcp/subflow.c.", true},
            {"start_line", "number", "First line to return (1-based). Default 1.", false},
            {"max_lines", "number", "How many lines to return. Default 200, max 400.", false}
        }},
        {"grep_source",
         "Search the analysed source tree with an ECMAScript regex and get "
         "path:line:text hits. Use it to find definitions, callers, lock "
         "acquisitions or field uses.", {
            {"pattern", "string", "ECMAScript regular expression.", true},
            {"path", "string", "Restrict the search to this file or directory (relative to the source root).", false},
            {"ignore_case", "boolean", "Case-insensitive match. Default false.", false},
            {"max_results", "number", "Cap on hits returned. Default 50, max 200.", false}
        }},
        {"list_dir",
         "List entries of a directory inside the analysed source tree.", {
            {"path", "string", "Directory relative to the source root. Defaults to the root.", false}
        }},
    };
}

std::vector<Tool> get_shared_tools() {
    return {
        {"get_function", "Get the function containing a node ID.", {
            {"node_id", "number", "The ID of the node within the target function.", true}
        }},
        {"get_function_ops", "Get a function's operation nodes (CFG/CCPG nodes) with code & locations (useful when the raw function body is truncated).", {
            {"function_id", "number", "The ID of the function to inspect.", true}
        }},
        {"get_cpg_method_by_name", "Get CPG Method node(s) by exact name (returns CPG node IDs; useful when the function isn't in CCPG yet, e.g., thread entry passed as function pointer).", {
            {"name", "string", "The exact method/function name to find.", true}
        }},
        {"get_function_by_name", "Get a function by its exact name.", {
            {"name", "string", "The name of the function to find.", true}
        }},
        {"get_function_by_id", "Get a function by its ID.", {
            {"function_id", "number", "The ID of the function to find.", true}
        }},
        {"get_callees", "Get all functions called from within a given function.", {
            {"function_id", "number", "The ID of the calling function.", true}
        }},
        {"get_callers", "Get all functions that call a given function.", {
            {"function_id", "number", "The ID of the function being called.", true}
        }},
    };
}

std::optional<std::string> handle_shared_tool(
    const std::string& tool_name, 
    const nlohmann::json& arguments, 
    CCPG* ccpg) 
{
    // Source navigation needs no graph, so it stays available even when an
    // agent runs without a CCPG.
    if (tool_name == "read_file") return readFileTool(arguments);
    if (tool_name == "grep_source") return grepSourceTool(arguments);
    if (tool_name == "list_dir") return listDirTool(arguments);

    if (!ccpg) {
        return R"({"error": "CCPG context is not available."})";
    }

    if (tool_name == "get_function_ops") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* function = ccpg->getFunctionById(function_id);
        if (!function || !function->getFuncNode() || !function->getFuncNode()->getCPGNode()) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json ops = nlohmann::json::array();
        for (CCPGNode* node : function->getNodes()) {
            if (!node || !node->getCPGNode()) continue;
            const std::string& code = node->getCPGNode()->getCode();
            if (code.empty() || code == "<empty>") continue;
            ops.push_back({
                {"node_id", node->getId()},
                {"code", code},
                {"location", node->getNodeLoc().toString()}
            });
        }

        nlohmann::json result = {
            {"function_id", function->getId()},
            {"function_name", function->getFuncNode()->getCPGNode()->getName()},
            {"operations", ops}
        };
        return result.dump();
    }

    if (tool_name == "get_cpg_method_by_name") {
        std::string name = arguments.at("name").get<std::string>();
        const CPG* cpg = ccpg->getCPG();
        if (!cpg) {
            return R"({"error": "CPG context is not available."})";
        }

        std::unordered_set<Node*> nodes = cpg->findMethodsByName(name);

        nlohmann::json methods = nlohmann::json::array();
        for (Node* node : nodes) {
            if (!node) continue;
            long long id_num = -1;
            try {
                id_num = std::stoll(std::string(node->getId()));
            } catch (...) {
                continue;
            }
            std::string method_name = node->getName();
            if (method_name.empty()) {
                method_name = node->getMethodFullName();
            }
            nlohmann::json info = {
                {"cpg_node_id", id_num},
                {"method_name", method_name},
                {"method_code", node->getCode()},
                {"file", node->getProperty("FILENAME")},
                {"line", node->getLineNumber()}
            };
            methods.push_back(info);
        }
        return methods.dump();
    }

    if (tool_name == "get_function") {
        int node_id = -1;
        if (arguments.contains("node_id")) {
            node_id = arguments.at("node_id").get<int>();
        } else if (arguments.contains("function_id")) {
            node_id = arguments.at("function_id").get<int>();
        } else {
            return R"({"error": "Missing required argument 'node_id' or 'function_id'."})";
        }
        
        CCPGNode* node = ccpg->getNodeByID(node_id);
        ccpg::Function* function = node ? node->getFunction() : nullptr;
        if (function) {
            nlohmann::json result = {
                {"function_id", function->getId()},
                {"function_name", function->getFuncNode()->getCPGNode()->getName()},
                {"function_body", function->getFuncNode()->getCPGNode()->getCode()}
            };
            return result.dump();
        } else {
            return R"({"error": "Function not found for node ID: )" + std::to_string(node_id) + R"("})";
        }
    }

    if (tool_name == "get_function_by_id") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* function = ccpg->getFunctionById(function_id);
        if (function) {
            nlohmann::json result = {
                {"function_id", function->getId()},
                {"function_name", function->getFuncNode()->getCPGNode()->getName()},
                {"function_body", function->getFuncNode()->getCPGNode()->getCode()}
            };
            return result.dump();
        } else {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }
    }

    if (tool_name == "get_function_by_name") {
        std::string name = arguments.at("name").get<std::string>();
        std::unordered_set<Node*> nodes = ccpg->getCPG()->findMethodsByName(name);

        // Fallback: Fuzzy search if no exact match found
        if (nodes.empty()) {
            CPGNodeSet all_methods = ccpg->getCPG()->getNodesByType("Method");
            for (Node* methodNode : all_methods) {
                std::string methodName = methodNode->getName();
                // Check for substring match
                if (methodName.find(name) != std::string::npos && methodNode->getProperty("CODE") != "<empty>") {
                     if(methodNode->outCFGEdges.size() == 1){
                        std::unordered_set<Edge*> outEdges = methodNode->outCFGEdges;
                        Edge* edge = *outEdges.begin();
                        Node* nextNode = edge->getToNode();
                        if(nextNode->getType() == "Method_return"){
                            continue;
                        }
                    }
                    nodes.insert(methodNode);
                }
            }
        }

        nlohmann::json functions = nlohmann::json::array();
        for (Node* node : nodes) {
            CCPGNode* function_node = ccpg->getCCPGNodeByCPGNode(node);
            ccpg::Function* function = function_node ? function_node->getFunction() : nullptr;
            if (function) {
                nlohmann::json function_info = {
                    {"function_id", function->getId()},
                    {"function_name", function->getFuncNode()->getCPGNode()->getName()},
                    {"function_body", function->getFuncNode()->getCPGNode()->getCode()}
                };
                functions.push_back(function_info);
            }
        }
        return functions.dump();
    }

    if (tool_name == "get_callees") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);
        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json calleesJson = nlohmann::json::array();
        for (CCPGNode* node : func->getNodes()) {
            if (node->isCallSite()) {
                 CCPGEdge* callEdge = ccpg->hasCallEdge(node);
                if (!callEdge) continue;
                CCPGNode* calleeNode = callEdge->getDst();
                if (calleeNode) {
                     ccpg::Function* callee_func = calleeNode->getFunction();
                     if(callee_func){
                        nlohmann::json calleeInfo = {
                            {"callee_function_id", callee_func->getId()},
                            {"callee_function_name", callee_func->getFuncNode()->getCPGNode()->getName()},
                            {"callsite_code", node->getCPGNode()->getCode()},
                            {"callsite_node_id", node->getId()}
                        };
                        calleesJson.push_back(calleeInfo);
                     }
                }
            }
        }
        nlohmann::json result;
        result["callees"] = calleesJson;
        return result.dump();
    }

    if (tool_name == "get_callers") {
        int function_id = arguments.at("function_id").get<int>();
        ccpg::Function* func = ccpg->getFunctionById(function_id);
        if (!func) {
            return R"({"error": "Function not found for ID: )" + std::to_string(function_id) + R"("})";
        }

        nlohmann::json callersJson = nlohmann::json::array();
        ccpg::FunctionSet callers = func->getCallers();
        for (ccpg::Function* caller_func : callers) {
            if (caller_func) {
                nlohmann::json callerInfo = {
                    {"caller_function_id", caller_func->getId()},
                    {"caller_function_name", caller_func->getFuncNode()->getCPGNode()->getName()}
                };
                callersJson.push_back(callerInfo);
            }
        }
        nlohmann::json result;
        result["callers"] = callersJson;
        return result.dump();
    }

    // If the tool name doesn't match any shared tool, return nullopt
    return std::nullopt;
}

} // namespace SharedToolKit
} // namespace llm_client
