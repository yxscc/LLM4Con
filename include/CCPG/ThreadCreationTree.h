// Thread Creation Tree

#ifndef CCPG_THREAD_CREATION_TREE_H
#define CCPG_THREAD_CREATION_TREE_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <functional>

#include "CPG/CPG.h"
#include "CCPG/CCPG.h"

/*class CCPGNode;
class CCPGEdge;
class CCPG;*/
class ThreadAPIUtil;
class Thread;

struct pair_hash {
    template <typename T1, typename T2>
    size_t operator ()(const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);  // Combine the hash values of both elements
    }
};

class ParallelLocCache {
    public:
        // 获取两个线程的并行位置结果（带缓存）
        std::pair<
            std::unordered_map<NodeLoc, Context, NodeLocHash>&,
            std::unordered_map<NodeLoc, Context, NodeLocHash>&
        > getParallelLocs(Thread* t1, Thread* t2);
    
        // 可选：清理缓存（根据需求实现）
        void clear() {
            cache.clear();
        }
    
    private:
        // 有序线程对键类型
        using OrderedThreadPair = std::pair<Thread*, Thread*>;
        
        // 生成有序的线程对（确保 first <= second）
        static OrderedThreadPair makeOrderedPair(Thread* a, Thread* b) {
            return (a <= b) ? std::make_pair(a, b) : std::make_pair(b, a);
        }
    
        // 哈希函数（组合两个指针的哈希）
        struct ThreadPairHash {
            size_t operator()(const OrderedThreadPair& p) const {
                auto hash1 = std::hash<Thread*>{}(p.first);
                auto hash2 = std::hash<Thread*>{}(p.second);
                return hash1 ^ (hash2 << 1);
            }
        };
    
        // 缓存数据结构
        std::unordered_map<
            OrderedThreadPair,
            std::pair<
                std::unordered_map<NodeLoc, Context, NodeLocHash>,
                std::unordered_map<NodeLoc, Context, NodeLocHash>
            >,
            ThreadPairHash
        > cache;
    
    };

class ThreadCreationTree {
private:
    std::unordered_set<Thread*> threads; // 存储线程指针
    CCPG* ccpg; // CCPG指针
    const CPG* cpg; // CPG指针
    std::unordered_map<std::pair<Thread*, Thread*>, std::string, pair_hash> parallelThreadPairs;
    ParallelLocCache parallelLocCache;


    // 私有构造函数
    ThreadCreationTree() {}

    // 唯一实例
    static ThreadCreationTree* instance;



public:
    // 删除复制构造函数和赋值运算符
    ThreadCreationTree(const ThreadCreationTree&) = delete;
    ThreadCreationTree& operator=(const ThreadCreationTree&) = delete;

    // 获取唯一实例的静态方法
    static ThreadCreationTree* getInstance() {
        if (instance == nullptr) {
            instance = new ThreadCreationTree();
        }
        return instance;
    }

    ~ThreadCreationTree() {}

    const CPG* getCPG() { return cpg; }
    void setCPG(const CPG* cpg) { this->cpg = cpg; }
    
    void addThread(Thread* thread); // 添加线程

    void build(); // 从dot文件构建线程创建树

    CCPG * getCCPG() { return ccpg; }
    void setCCPG(CCPG * ccpg) { this->ccpg = ccpg; }

    Node * findThreadEntryInCPG(CCPGNode* forknode); // 在CPG中查找入口节点
    Node * findThreadEntryByArg(Node * arg);
    Node * findThreadEntryByLLM(CCPGNode* forknode);

    std::unordered_set<Thread *> getThreads() {return threads;}

    void addParallelThreadPairs(Thread* thread1, Thread* thread2, const std::string& relation) {
        // 生成有序的线程对键（确保 (thread1, thread2) 和 (thread2, thread1) 被视为相同）
        auto key = (thread1 <= thread2) ? std::make_pair(thread1, thread2) : std::make_pair(thread2, thread1);
        if(parallelThreadPairs.find(key) != parallelThreadPairs.end()) return;
        parallelThreadPairs[std::make_pair(thread1, thread2)] = relation;
    }
    std::unordered_map<std::pair<Thread*, Thread*>, std::string, pair_hash> getParallelThreadPairs() { return parallelThreadPairs; }

    bool isDescendant(Thread * thread1, Thread * thread2);
    bool isSibling(Thread * thread1, Thread * thread2);
    bool isIndirectSibling(Thread * thread1, Thread * thread2);

    void countParallelThreadPairs();

    void handleJoins();

    // 同一个callNode对应线程最多出现两次（意味着要和自己比较），因为上下文信息已经在节点中
    std::unordered_set<Thread *> getDetectedThreads();

    bool sameCall(Thread * thread1, Thread * thread2);

    Thread * createThread(CCPGNode * forkNode, Thread * parent);

    void createCall(CCPGNode * callNode, Thread * thread);

    bool mayHappenInParallel(Thread * thread1, Thread * thread2);

    void dumpDot();

    void printThreadCreationTree(fs::path outputDir) const;

    std::pair<std::unordered_map<NodeLoc, Context, NodeLocHash>, std::unordered_map<NodeLoc, Context, NodeLocHash>> computeParallelLocs(Thread* t1, Thread* t2);
    std::pair<std::unordered_map<NodeLoc, Context, NodeLocHash>, std::unordered_map<NodeLoc, Context, NodeLocHash>> getParallelLocs(Thread* t1, Thread* t2);

    std::string getThreadRelationship(Thread* t1, Thread* t2) {
        // 生成有序的线程对键（确保 (t1, t2) 和 (t2, t1) 被视为相同）
        auto key = std::make_pair(t1, t2);
    
        // 查找关系字符串
        auto it = parallelThreadPairs.find(key);
        if (it != parallelThreadPairs.end()) {
            return it->second; // 找到关系字符串
        }
    
        // 如果未找到，返回默认关系
        return "No relationship found";
    }
};

class Thread {
private:
    //std::stack<Node*> conditionBlockStack = std::stack<Node*>();
    //std::unordered_map<Node*, std::unordered_set<CCPGNode*>> BlockExitMap = std::unordered_map<Node*, std::unordered_set<CCPGNode*>>();
    CCPGNodeSet nodes; // 存储节点指针
    CCPGEdgeSet edges; // 存储边指针
    CCPGNode * forkNode; // fork节点
    CCPGNode * joinNode; // join节点
    Thread* parent; // 父线程
    std::unordered_set<Thread*> children; // 子线程
    CCPGNode* exitNode; // 退出节点
    int id;
    TypeToNodeSetMap typeToNodeSet;
    FunctionSet functions;
    ccpg::Function * threadMainFunction = nullptr;

public:
    Thread() {}
    Thread(Thread* other) {
        this->nodes = other->getNodes();
        this->edges = other->getEdges();
        this->parent = other->getParent();
        this->parent->addChild(this);
        this->children = other->getChildren();
        this->typeToNodeSet = other->typeToNodeSet;
    }
    ~Thread() {}
    //void extend(CCPGNode* node); // 递归创建线程

    int getId() { return id; }
    void setId(int id) { this->id = id; }

    void addNode(CCPGNode* node){
        nodes.insert(node);
        typeToNodeSet[node->getType()].insert(node);
        
    } // 添加节点
    std::unordered_set<CCPGNode*> getNodes() { 
        return nodes; 
    } // 获取节点

    void addEdge(CCPGEdge* edge){
        edges.insert(edge);
    } // 添加边

    std::unordered_set<CCPGEdge*> getEdges() { return edges; } // 获取边

    void setParent(Thread* thread){
        parent = thread;
    } // 设置父线程
    Thread* getParent() { return parent; }

    void addChild(Thread* thread){
        children.insert(thread);
    } // 添加子线程

    std::unordered_set<Thread*> getChildren() { return children; }

    CCPGNode* getForkNode() { return forkNode; }
    void setForkNode(CCPGNode* forkNode) { this->forkNode = forkNode; }

    CCPGNode* getJoinNode() { return joinNode; }
    void setJoinNode(CCPGNode* joinNode) { this->joinNode = joinNode; }

    void addFunction(ccpg::Function* function);
    FunctionSet getFunctions() { return functions; }
    void removeFunction(ccpg::Function* function) { 
        functions.erase(function);
        for (auto it = nodes.begin(); it != nodes.end(); ) {
            CCPGNode* node = *it;
            if (node->getFunction() == function) {
                it = nodes.erase(it); // 删除并更新迭代器
            } else {
                ++it; // 继续遍历
            }
        }
    }

    void dumpDot(std::ofstream& dotFile);

    CCPGNodeSet getNodesByType(ThreadAPIUtil::TYPE type) {
        auto it = typeToNodeSet.find(type);
        if (it != typeToNodeSet.end()) {
            return it->second;
        }
        return CCPGNodeSet();
    }

    void setThreadMainFunction(ccpg::Function * function) { this->threadMainFunction = function; }
    ccpg::Function * getThreadMainFunction() { return threadMainFunction; }

    std::unordered_map<NodeLoc, Context, NodeLocHash> findLocsInScope(NodeLoc start, NodeLoc end);
    std::unordered_map<NodeLoc, Context, NodeLocHash> findAllLocs();

    void  replaceNode(CCPGNode * oldNode, CCPGNode * newNode){
        nodes.erase(oldNode);
        nodes.insert(newNode);

        for (auto it = edges.begin(); it != edges.end(); ) {
            CCPGEdge* edge = *it;
            if (edge->getSrc() == newNode || edge->getDst() == newNode) {
                it = edges.erase(it); // 删除并更新迭代器
            } else {
                ++it; // 继续遍历
            }
        }
        for(CCPGEdge * edge : oldNode->getInEdges()){
            edges.erase(edge);
        }
        for(CCPGEdge * edge : oldNode->getOutEdges()){
            edges.erase(edge);
        }
        for(CCPGEdge * edge : newNode->getInEdges()){
            edges.insert(edge);
        }
        for(CCPGEdge * edge : newNode->getOutEdges()){
            edges.insert(edge);
        }
    }

    void addThreadToDot(std::ostringstream& dot);

    std::vector<Context> getIntraThreadContext(NodeLoc loc);

};



#endif // CCPG_THREAD_CREATION_TREE_H