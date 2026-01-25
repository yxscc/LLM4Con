#include <iostream>
#include <cstdlib> // For system()
#include <filesystem>
#include <string>
#include <fstream>
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

fs::path CPGGenerator::generateCPG(std::string dir){
    
    fs::path projectDir = fs::path(PROJECT_PATH);
    fs::path cpg_dot = projectDir / fs::path("cpg_dot");
    if (!fs::exists(cpg_dot)) {
        fs::create_directory(cpg_dot);
    }
    fs::path outputDir = projectDir / fs::path("cpg_dot/" + TargetPath::getInstance()->getTargetProjectName());

    if(fs::exists(outputDir/ "export.dot")){
        return outputDir;
    }

    // 调整命令以使用新的输出目录路径,joern在该项目的joern-cli目录下
    std::string generateCPGCommand =  std::string("joern-parse") + " -J-Xmx40G " + dir ;
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
