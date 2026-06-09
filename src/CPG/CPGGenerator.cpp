#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <graphviz/cgraph.h>

#include "CPG/CPGGenerator.h"
#include "Util/TargetPath.h"
#include "Util/ExecutionTimer.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
//using namespace rapidjson;
//using namespace tinyxml2;
namespace fs = std::filesystem;

namespace {

bool isCPGSourceFile(const fs::path& p) {
    std::string ext = p.extension().string();
    return ext == ".c" || ext == ".cc" || ext == ".cpp" ||
           ext == ".h" || ext == ".hh" || ext == ".hpp";
}

std::string buildSourceManifest(const fs::path& sourceDir) {
    std::vector<std::string> rows;
    if (!fs::exists(sourceDir)) return "";

    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file() || !isCPGSourceFile(entry.path())) {
            continue;
        }
        std::error_code ec;
        fs::path rel = fs::relative(entry.path(), sourceDir, ec);
        if (ec) rel = entry.path().filename();
        auto size = fs::file_size(entry.path(), ec);
        if (ec) size = 0;
        auto mtime = fs::last_write_time(entry.path(), ec);
        auto ticks = ec ? 0 : mtime.time_since_epoch().count();

        std::ostringstream row;
        row << rel.generic_string() << "\t" << size << "\t" << ticks;
        rows.push_back(row.str());
    }

    std::sort(rows.begin(), rows.end());
    std::ostringstream manifest;
    manifest << "source_dir=" << fs::absolute(sourceDir).generic_string() << "\n";
    for (const auto& row : rows) manifest << row << "\n";
    return manifest.str();
}

bool fileContentsEqual(const fs::path& path, const std::string& expected) {
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str() == expected;
}

void writeTextFile(const fs::path& path, const std::string& content) {
    std::ofstream out(path);
    if (out.is_open()) out << content;
}

std::unordered_set<std::string> collectReferencedConfigMacros(
    const fs::path& sourceDir) {
    std::unordered_set<std::string> refs;
    if (!fs::exists(sourceDir)) return refs;

    static const std::regex configRe(R"(CONFIG_[A-Za-z0-9_]+)");
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file() || !isCPGSourceFile(entry.path())) {
            continue;
        }
        std::ifstream in(entry.path());
        if (!in.is_open()) continue;
        std::string text((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
        for (std::sregex_iterator it(text.begin(), text.end(), configRe), end;
             it != end; ++it) {
            refs.insert((*it)[0].str());
        }
    }
    return refs;
}

std::string normalizeConfigValue(std::string value) {
    auto trim = [](std::string& s) {
        const char* ws = " \t\r\n";
        size_t first = s.find_first_not_of(ws);
        if (first == std::string::npos) {
            s.clear();
            return;
        }
        size_t last = s.find_last_not_of(ws);
        s = s.substr(first, last - first + 1);
    };
    trim(value);
    if (value.empty()) return "1";

    // Joern --define only needs truthiness for #ifdef/#if defined. Preserve
    // simple numeric values for #if CONFIG_FOO > N, otherwise use 1 to avoid
    // shell quoting problems with strings or compound expressions.
    bool numeric = true;
    for (char c : value) {
        if (!std::isdigit(static_cast<unsigned char>(c)) &&
            c != 'x' && c != 'X' &&
            (c < 'a' || c > 'f') &&
            (c < 'A' || c > 'F')) {
            numeric = false;
            break;
        }
    }
    return numeric ? value : "1";
}

void loadAutoconfDefines(const fs::path& autoconf,
                         const std::unordered_set<std::string>& refs,
                         std::unordered_map<std::string, std::string>& out) {
    std::ifstream in(autoconf);
    if (!in.is_open()) return;

    static const std::regex defineRe(
        R"(^\s*#\s*define\s+(CONFIG_[A-Za-z0-9_]+)(?:\s+(.+))?\s*$)");
    std::string line;
    while (std::getline(in, line)) {
        std::smatch m;
        if (!std::regex_match(line, m, defineRe)) continue;
        std::string name = m[1].str();
        if (!refs.count(name)) continue;
        out[name] = normalizeConfigValue(m.size() > 2 ? m[2].str() : "1");
    }
}

std::unordered_map<std::string, std::string> collectCompileConfigDefines(
    const fs::path& sourceDir,
    const std::unordered_set<std::string>& refs) {
    std::unordered_map<std::string, std::string> defines;
    if (refs.empty()) return defines;

    fs::path absSourceDir = fs::absolute(sourceDir);
    fs::path caseDir = absSourceDir.filename() == "src"
        ? absSourceDir.parent_path()
        : absSourceDir;
    if (!fs::exists(caseDir)) return defines;

    static const std::regex dFlagRe(R"(-D(CONFIG_[A-Za-z0-9_]+)(?:=([^\s'"]+))?)");
    static const std::regex autoconfRe(R"(-include\s+['"]?([^\s'"]*autoconf\.h)['"]?)");
    for (const auto& entry : fs::directory_iterator(caseDir)) {
        if (!entry.is_regular_file()) continue;
        std::string filename = entry.path().filename().string();
        if (filename.find("_compile.log") == std::string::npos &&
            filename != "compile.log") {
            continue;
        }

        std::ifstream in(entry.path());
        if (!in.is_open()) continue;
        std::string text((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());

        for (std::sregex_iterator it(text.begin(), text.end(), dFlagRe), end;
             it != end; ++it) {
            std::string name = (*it)[1].str();
            if (!refs.count(name)) continue;
            defines[name] = normalizeConfigValue(
                (*it).size() > 2 ? (*it)[2].str() : "1");
        }

        for (std::sregex_iterator it(text.begin(), text.end(), autoconfRe), end;
             it != end; ++it) {
            loadAutoconfDefines(fs::path((*it)[1].str()), refs, defines);
        }
    }
    return defines;
}

std::string renderConfigManifest(
    const std::unordered_map<std::string, std::string>& defines) {
    std::vector<std::string> rows;
    for (const auto& [k, v] : defines) rows.push_back(k + "=" + v);
    std::sort(rows.begin(), rows.end());
    std::ostringstream out;
    out << "config_defines=" << rows.size() << "\n";
    for (const auto& row : rows) out << row << "\n";
    return out.str();
}

} // namespace

fs::path CPGGenerator::generateCPG(std::string dir){
    
    fs::path projectDir = fs::path(PROJECT_PATH);
    fs::path cpg_dot = projectDir / fs::path("cpg_dot");
    if (!fs::exists(cpg_dot)) {
        fs::create_directory(cpg_dot);
    }
    fs::path outputDir = projectDir / fs::path("cpg_dot/" + TargetPath::getInstance()->getTargetProjectName());

    const fs::path sourceDir = fs::path(dir);
    const auto referencedConfigs = collectReferencedConfigMacros(sourceDir);
    const auto compileConfigDefines =
        collectCompileConfigDefines(sourceDir, referencedConfigs);
    const fs::path manifestPath = outputDir / ".lace_cpg_manifest.txt";
    // CPG frontend trade-off (measured on the kernel dataset):
    //
    //  * WITH the kernel `--define` flags (default): c2cpg/CDT runs strict
    //    preprocessing and builds COMPLETE CFGs (needed for thread-body
    //    expansion + access collection), but on an incomplete kernel header
    //    tree it silently DROPS ~half the function bodies from the CPG
    //    (shmem src: 2654 -> 1390 methods; shmem_getattr disappears entirely).
    //
    //  * WITHOUT any `--define` (LACE_CPG_FUZZY=1): the fuzzy parser keeps the
    //    full method set (shmem_getattr is present) BUT only emits the AST —
    //    methods come back with a degenerate single-edge CFG, which the
    //    findMethod stub-filter (correctly) rejects, and which regresses cases
    //    that currently map fine (e.g. keyctl 27 -> 16 threads).
    //
    // Neither mode is a clean win; both are symptoms of c2cpg not being able to
    // fully parse a partial kernel source tree. Keep the COMPLETE-CFG define
    // mode as the default so we never regress working cases. The fuzzy mode is
    // retained behind LACE_CPG_FUZZY for experiments. The chosen mode is part
    // of the cache key so flipping it forces a clean regeneration.
    const bool useKernelDefines =
        std::getenv("LACE_CPG_FUZZY") == nullptr;
    // Keep the default (define) manifest BYTE-IDENTICAL to the historical format
    // so existing caches stay valid; only the opt-in fuzzy mode tags the
    // manifest, which forces a clean regeneration when toggled.
    const std::string modeTag =
        useKernelDefines ? std::string("") : std::string("cpg_define_mode=fuzzy\n");
    const std::string sourceManifest =
        modeTag + buildSourceManifest(sourceDir) + renderConfigManifest(compileConfigDefines);
    if(fs::exists(outputDir / "export.dot")){
        if (fileContentsEqual(manifestPath, sourceManifest)) {
            return outputDir;
        }
        std::cout << "[CPG] Source tree changed or cache manifest missing; "
                  << "regenerating CPG for current src." << std::endl;
        fs::remove_all(outputDir);
    }

    // Joern's C frontend (c2cpg) does not know kernel-specific GCC attributes.
    // We define them away via --frontend-args --define so the parser treats them
    // as empty tokens.  This does NOT modify the source files on disk.
    static const std::vector<std::string> kernelDefines = {
        "__user", "__kernel", "__iomem", "__rcu", "__percpu",
        "__force", "__cold", "__read_mostly", "__ro_after_init",
        "__init", "__exit", "__initdata", "__exitdata", "__initconst",
        "__net_init", "__net_exit", "__net_initdata",
        "__devinit", "__devexit", "__devinitdata",
        "__acquires", "__releases", "__must_hold",
        "__maybe_unused", "__always_inline",
        "asmlinkage", "notrace", "noinline",
        "__bitwise", "__randomize_layout", "__aligned",
        "__cacheline_aligned", "__cacheline_aligned_in_smp",
        "__packed", "__weak", "__visible",
    };
    // Function-like macros that expand into a real C function body so that
    // Joern's c2cpg creates a proper Method node with a predictable name.
    // SYSCALL_DEFINEn(name, t1, a1, ...)     -> long sys_##name(t1 a1, ...)
    // COMPAT_SYSCALL_DEFINEn(name, t1, a1, ...) -> long compat_sys_##name(t1 a1, ...)
    // Without these, the kernel uses multi-level macros that Joern won't
    // resolve, leaving the syscall bodies invisible to CPG::findMethod.
    static const std::vector<std::string> kernelFuncMacros = {
        "SYSCALL_DEFINE0(name)=long sys_##name(void)",
        "SYSCALL_DEFINE1(name,t1,a1)=long sys_##name(t1 a1)",
        "SYSCALL_DEFINE2(name,t1,a1,t2,a2)=long sys_##name(t1 a1, t2 a2)",
        "SYSCALL_DEFINE3(name,t1,a1,t2,a2,t3,a3)=long sys_##name(t1 a1, t2 a2, t3 a3)",
        "SYSCALL_DEFINE4(name,t1,a1,t2,a2,t3,a3,t4,a4)=long sys_##name(t1 a1, t2 a2, t3 a3, t4 a4)",
        "SYSCALL_DEFINE5(name,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5)=long sys_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)",
        "SYSCALL_DEFINE6(name,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5,t6,a6)=long sys_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)",
        "COMPAT_SYSCALL_DEFINE0(name)=long compat_sys_##name(void)",
        "COMPAT_SYSCALL_DEFINE1(name,t1,a1)=long compat_sys_##name(t1 a1)",
        "COMPAT_SYSCALL_DEFINE2(name,t1,a1,t2,a2)=long compat_sys_##name(t1 a1, t2 a2)",
        "COMPAT_SYSCALL_DEFINE3(name,t1,a1,t2,a2,t3,a3)=long compat_sys_##name(t1 a1, t2 a2, t3 a3)",
        "COMPAT_SYSCALL_DEFINE4(name,t1,a1,t2,a2,t3,a3,t4,a4)=long compat_sys_##name(t1 a1, t2 a2, t3 a3, t4 a4)",
        "COMPAT_SYSCALL_DEFINE5(name,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5)=long compat_sys_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)",
        "COMPAT_SYSCALL_DEFINE6(name,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5,t6,a6)=long compat_sys_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)",
    };
    std::string defineArgs;
    if (useKernelDefines) {
        for (const auto& def : kernelDefines) {
            defineArgs += " --define " + def;
        }
        for (const auto& [name, value] : compileConfigDefines) {
            defineArgs += " --define " + name + "=" + value;
        }
        if (!compileConfigDefines.empty()) {
            std::cout << "[CPG] Synced " << compileConfigDefines.size()
                      << " referenced CONFIG define(s) from compile logs/autoconf."
                      << std::endl;
        }
        for (const auto& m : kernelFuncMacros) {
            // Each macro must be quoted because it contains parentheses and
            // commas which would otherwise be chewed up by the shell.
            defineArgs += " --define \"" + m + "\"";
        }
    } else {
        std::cout << "[CPG] Fuzzy mode (no --define): full method coverage; "
                     "syscalls mapped via __x64_sys_ prefix fallback. "
                     "Set LACE_CPG_KERNEL_DEFINES=1 to restore define mode."
                  << std::endl;
    }
    // In fuzzy mode we pass NO frontend-args at all, otherwise c2cpg enables
    // strict CDT preprocessing and drops bodies it cannot fully resolve.
    std::string generateCPGCommand = std::string("joern-parse") + " -J-Xmx40G " + dir;
    if (!defineArgs.empty()) {
        generateCPGCommand += " --frontend-args" + defineArgs;
    }
    // 生成dot格式的CPG
    std::string drawCPGCommand = std::string("joern-export") + " -J-Xmx40G cpg.bin --repr=all --format=dot --out " + outputDir.string();
    
    printf("generateCPGCommand: %s\n", generateCPGCommand.c_str());
    printf("drawCPGCommand: %s\n", drawCPGCommand.c_str());
    //printf("drawNeo4jCPGCommand: %s\n", drawNeo4jCPGCommand.c_str());
    
    // 使用 std::chrono 记录开始时间
    ExecutionTimer::getInstance()->start("Joern CPG generation (generate cpg.bin)");

    // 调用joern生成CPG.bin
    int result = system(generateCPGCommand.c_str());
    if (result != 0) {
        cerr << "Failed to generate CPG with Joern." << endl;
        exit(1);
    }

    // 记录结束时间
    ExecutionTimer::getInstance()->stop("Joern CPG generation (generate cpg.bin)");

    // 检查输出文件夹是否已存在，如果存在则删除文件夹
    if (fs::exists(outputDir)) {
        fs::remove_all(outputDir);
    }

    // 创建输出文件夹
    //fs::create_directory(outputDir);

    // 使用 std::chrono 记录开始时间
    ExecutionTimer::getInstance()->start("Joern CPG drawing");

    // 画cpg图
    result = system(drawCPGCommand.c_str());
    if (result != 0) {
        cerr << "Failed to draw CPG with Joern." << endl;
        exit(1);
    }

    // 记录结束时间
    ExecutionTimer::getInstance()->stop("Joern CPG drawing");

    // 进一步处理Joern生成的数据（直接使用原始的 export.dot）
    std::cout << "CPG data generated at: " << outputDir.string() << endl;
    writeTextFile(manifestPath, sourceManifest);
    return outputDir;
}


CPG * CPGGenerator::buildCPGByDot(std::string dir) {
    // 生成CPG
    fs::path outputDir = generateCPG(dir);
    fs::path dotFile = outputDir / "export.dot";
    if (!fs::exists(dotFile)) {
        throw std::runtime_error("DOT file does not exist at the specified location: " + dotFile.string());
    }

    //DotGraph graph = DotParser::ParseFromFile(dotFile.string());

    // 打开并读取 DOT 文件
    FILE* dotFileStream = fopen(dotFile.c_str(), "r");
    if (!dotFileStream) {
        throw std::runtime_error("Failed to open DOT file: " + dotFile.string());
    }

    // 使用 std::chrono 记录开始时间
    ExecutionTimer::getInstance()->start("Graphviz parse dot");

    // 解析 DOT 文件
    Agraph_t* g = agread(dotFileStream, nullptr);
    fclose(dotFileStream);

    // 记录结束时间
    ExecutionTimer::getInstance()->stop("Graphviz parse dot");

    if (!g) {
        throw std::runtime_error("Failed to parse DOT file: " + dotFile.string());
    }

    ExecutionTimer::getInstance()->start("CPG building");

    // 临时map，用于通过ID快速查找Node指针
    std::unordered_map<std::string, Node*> nodeMap;
    nodeMap.reserve(agnnodes(g)); 

    // 遍历图中的所有节点
    for (Agnode_t *node = agfstnode(g); node; node = agnxtnode(g, node)) {
        const char* id = agnameof(node);
        const char *label = agget(node, "label");

        // 获取节点属性
        std::unordered_map<std::string, std::string> properties;
        for (Agsym_t *sym = agnxtattr(g, AGNODE, nullptr); sym; sym = agnxtattr(g, AGNODE, sym)) {
            char* key = sym->name;
            char* value = agget(node, key);
            properties[key] = value;
        }

        auto node_ptr = std::make_unique<Node>(std::string(id), label, properties);
        Node* raw_node_ptr = node_ptr.get();
        nodeMap[id] = raw_node_ptr;
        cpg->addNode(std::move(node_ptr));

    }

    // 步骤2: 遍历所有边，创建Edge对象并直接链接
    for (auto const& [id, fromNode] : nodeMap) {
        // 从 graphviz 图中找到对应的原始节点
        Agnode_t* g_node = agnode(g, const_cast<char*>(id.c_str()), 0);
        if (!g_node) continue;

        // 遍历这个节点的出边
        for (Agedge_t* g_edge = agfstout(g, g_node); g_edge; g_edge = agnxtout(g, g_edge)) {
            Agnode_t* g_head = aghead(g_edge); // head是边的指向节点
            // Agnode_t* g_tail = agtail(g_edge); // tail是边的起始节点 (就是 fromNode)

            // 利用map直接获取指针，无需findNode
            Node* toNode = nodeMap[agnameof(g_head)];
            
            const char* label = agget(g_edge, "label");

            auto edge_ptr = std::make_unique<Edge>(fromNode->getId(), toNode->getId(), label);
            Edge* raw_edge_ptr = edge_ptr.get();
            raw_edge_ptr->setFromNode(fromNode);
            raw_edge_ptr->setToNode(toNode);
            fromNode->outEdges.insert(raw_edge_ptr);
            toNode->inEdges.insert(raw_edge_ptr);
            
            // 根据类型更新具体的边集合
            if (raw_edge_ptr->getType() == "Cfg") {
                fromNode->outCFGEdges.insert(raw_edge_ptr);
                toNode->inCFGEdges.insert(raw_edge_ptr);
            }
            else if(raw_edge_ptr->getType() == "Argument"){
                fromNode->argumentEdges.insert(raw_edge_ptr);
            }
            else if(raw_edge_ptr->getType() == "Condition"){
                fromNode->conditionEdges.insert(raw_edge_ptr);
            }

            cpg->addEdge(std::move(edge_ptr));
        }

    }

    // 释放 Graphviz 图对象资源
    agclose(g);
    ExecutionTimer::getInstance()->stop("CPG building");

    return cpg;
}
