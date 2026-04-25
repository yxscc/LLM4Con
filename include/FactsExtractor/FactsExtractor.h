// include/FactsExtractor/FactsExtractor.h

#ifndef FACTS_EXTRACTOR_H
#define FACTS_EXTRACTOR_H

#include <string>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

class CCPG;
class CCPGNode;
class Thread;
class PhasarPointerAnalysis;

namespace llvm {
    class Value;
}

namespace facts {

struct SharedVariable {
    std::string var_name;
    std::vector<std::string> threads;
    std::string evidence;
};

struct MemoryAccess {
    int line;
    std::string var;
    std::string type;  // "READ" or "WRITE" or "FREE"
    std::string thread;
    std::string code;
};

struct AliasPair {
    std::string var_a;
    std::string var_b;
    bool is_alias;
};

struct LockOperation {
    int line;
    std::string lock;
    std::string op;  // "acquire" or "release"
    std::string thread;
    std::string code;
};

struct ProtectedRegion {
    int start;
    int end;
    std::string lock;
    std::string thread;
};

struct ThreadContext {
    std::string function;
    int start_line;
    int end_line;
    int thread_id;
};

struct ParallelPair {
    int line_a;
    int line_b;
    std::string thread_a;
    std::string thread_b;
    bool can_parallel;
};

struct HappensBefore {
    int line_a;
    int line_b;
    std::string relation;  // "before", "after", "no_order"
};

struct GroundTruth {
    std::string bug_type;
    std::vector<int> bug_lines;
    std::string description;
};

class FactsExtractor {
public:
    FactsExtractor(CCPG* ccpg, PhasarPointerAnalysis* pta);
    ~FactsExtractor() = default;

    nlohmann::json extractFacts(const std::string& cve_id, const std::string& code_file);
    
    void setGroundTruth(const GroundTruth& gt) { ground_truth_ = gt; }

private:
    nlohmann::json extractSharedVariables();
    nlohmann::json extractMemoryAccesses();
    nlohmann::json extractAliasPairs();
    nlohmann::json extractLockOperations();
    nlohmann::json extractProtectedRegions();
    nlohmann::json extractThreadContexts();
    nlohmann::json extractParallelPairs();
    nlohmann::json extractHappensBefore();
    nlohmann::json extractUnprotectedAccesses();

    std::string getThreadNameForNode(CCPGNode* node);
    std::string getLockNameFromNode(CCPGNode* node);
    std::string getVariableNameFromNode(CCPGNode* node);
    
    std::vector<CCPGNode*> findNodesInProtectedRegion(CCPGNode* lockAcquire, CCPGNode* lockRelease);
    bool isNodeInProtectedRegion(CCPGNode* node, const std::string& lock);

    CCPG* ccpg_;
    PhasarPointerAnalysis* pta_;
    GroundTruth ground_truth_;
    
    std::vector<SharedVariable> shared_vars_;
    std::vector<MemoryAccess> memory_accesses_;
    std::vector<AliasPair> alias_pairs_;
    std::vector<LockOperation> lock_ops_;
    std::vector<ProtectedRegion> protected_regions_;
    std::vector<ThreadContext> thread_contexts_;
    std::vector<ParallelPair> parallel_pairs_;
    std::vector<HappensBefore> hb_relations_;
};

GroundTruth parseGroundTruthFromReadme(const std::string& readme_path);

} // namespace facts

#endif // FACTS_EXTRACTOR_H
