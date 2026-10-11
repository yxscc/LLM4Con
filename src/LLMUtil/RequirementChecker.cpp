#include "LLMUtil/RequirementChecker.h"
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <vector>
namespace llm_client {
namespace {
using J = nlohmann::json;
using Nodes = std::vector<int>;
// C identifiers are case sensitive; dereferencing and annotations must not
// silently identify different synchronization objects.
std::string token(std::string s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
    if (!s.empty() && s.front() == '&') s.erase(s.begin());
    return s;
}
bool contains(const Nodes& nodes, int n) { return std::find(nodes.begin(), nodes.end(), n) != nodes.end(); }
struct Evidence {
    const CheckerRelations& relations;
    std::vector<std::pair<int,int>> edges;
    std::map<std::string, Nodes> exclusive, shared, atomic;
    Evidence(const CheckerRelations& r, const J& guarantees) : relations(r) {
        for (const auto& g : guarantees) {
            if (!g.value("unconditional", false)) continue;
            const auto form = g.at("form").get<std::string>();
            const auto a = g.at("a_nodes").get<Nodes>(), b = g.at("b_nodes").get<Nodes>();
            if (form == "Order" || form == "Wait") {
                for (int x : a) for (int y : b) edges.emplace_back(x,y);
            } else {
                const auto t = token(g.value("token", std::string()));
                if (t.empty()) continue;
                auto& dst = form == "AtomicOp" ? atomic[t] : g.value("mode", std::string()) == "shared" ? shared[t] : exclusive[t];
                dst.insert(dst.end(), a.begin(), a.end());
            }
        }
    }
    bool control(int a, int b) const { return a == b || (relations.control_reachable && relations.control_reachable(a,b)); }
    bool order(int a, int b) const {
        // One code node can denote two concurrently running activations.
        if (a == b) return false;
        if (relations.synchronized_order && relations.synchronized_order(a,b)) return true;
        std::set<int> visited; Nodes work;
        for (const auto& e : edges) if (control(a,e.first) && visited.insert(e.second).second) work.push_back(e.second);
        while (!work.empty()) {
            int x = work.back(); work.pop_back();
            if (control(x,b)) return true;
            for (const auto& e : edges) if (control(x,e.first) && visited.insert(e.second).second) work.push_back(e.second);
        }
        return false;
    }
    // All sequence operations and the interferer must share ONE exclusion
    // identity. Protecting one step, or different steps under different locks,
    // cannot discharge a compound atomicity requirement.
    bool excludes(const Nodes& sequence, int other) const {
        for (const auto& bucket : exclusive) {
            const auto& ex = bucket.second;
            auto it = shared.find(bucket.first);
            auto covered = [&](int n) { return contains(ex,n) || (it != shared.end() && contains(it->second,n)); };
            if (!covered(other)) continue;
            if (std::all_of(sequence.begin(), sequence.end(), [&](int n) {
                return covered(n) && (contains(ex,n) || contains(ex,other)); })) return true;
        }
        return false;
    }
    bool atomicPair(int a, int b) const {
        for (const auto& [t, ns] : atomic) if (contains(ns,a) && contains(ns,b)) return true;
        return false;
    }
};
bool lockCovers(const J& object, const Nodes& sequence, int other) {
    std::set<std::string> common;
    auto locks = [&](int n) {
        std::set<std::string> out;
        for (const auto& a : object.at("accesses")) if (a.at("node_id") == n && a.value("lock_protected",false)) {
            auto t = token(a.value("lock",std::string())); if (!t.empty()) out.insert(t);
        }
        return out;
    };
    common = locks(other);
    for (int n : sequence) {
        auto ns = locks(n); std::set<std::string> next;
        std::set_intersection(common.begin(), common.end(), ns.begin(), ns.end(), std::inserter(next,next.end()));
        common = std::move(next);
    }
    return !sequence.empty() && !common.empty();
}
}
J checkModuleRequirements(const J& requirements, const J& guarantees, const J& surface, const CheckerRelations& relations) {
    J result = J::object(); Evidence evidence(relations,guarantees);
    std::set<int> reentrant;
    for (const auto& t : surface.value("threads",J::array())) if (t.value("reentrant_entry",false)) reentrant.insert(t.at("thread_id").get<int>());
    for (const auto& r : requirements) {
        const std::string id = r.at("id"), form = r.at("form");
        auto& check = result[id]; check = {{"status","requires_semantic_review"},{"proof",false}};
        const int oid = r.at("object_id");
        if (oid < 0 || static_cast<size_t>(oid) >= surface.at("shared_objects").size() || r.value("requires_semantic_review",false)) {
            check["reason"] = "Resource or activation/source operands lack a static mapping"; continue;
        }
        const auto& object = surface.at("shared_objects").at(oid);
        const auto a = r.at("a_nodes").get<Nodes>(), b = r.at("b_nodes").get<Nodes>();
        if (a.empty() || (form != "MustBeAtomic" && b.empty())) { check["reason"] = "Empty static operand set"; continue; }
        auto fail = [&](int x,int y,const std::string& why) {
            check = {{"status","undischarged"},{"proof",false},{"first_witness",{{"a",x},{"b",y},{"reason",why}}}};
        };
        bool failed = false;
        if (form == "MustPrecede" || form == "MustBeMediated") {
            for (int x : a) {
                for (int y : b) {
                    bool covered = evidence.order(x,y);
                    if (form == "MustBeMediated") covered = covered || evidence.order(y,x) || evidence.excludes({x},y) || evidence.atomicPair(x,y) || lockCovers(object,{x},y);
                    if (!covered) { fail(x,y,"No applicable ordering or synchronization witness for " + form); failed=true; break; }
                }
                if (failed) break;
            }
        } else if (form == "MustBeAtomic") {
            const int owner = r.at("owner_thread"); bool any = false;
            for (const auto& access : object.at("accesses")) {
                int tid = access.at("thread_id"), n = access.at("node_id");
                if (tid == owner && !reentrant.count(tid)) continue;
                const auto kind = access.at("access_type").get<std::string>();
                // Both reads and writes may observe/break a compound invariant.
                if (n < 0 || (kind != "Read" && kind != "Write" && kind != "ReadWrite" && kind != "Free")) continue;
                any = true;
                bool outside = evidence.order(n,a.front()) || evidence.order(a.back(),n);
                // Individual atomic accesses do not make a multi-step sequence atomic.
                if (outside || evidence.excludes(a,n) || lockCovers(object,a,n)) continue;
                fail(a.front(),n,"An access may interleave the compound region; no whole-region exclusion witness"); failed=true; break;
            }
            if (!any) { check["reason"] = "No statically modeled interference; semantic applicability still needs review"; continue; }
        } else { check["reason"] = "Unsupported requirement form"; continue; }
        if (!failed) check = {{"status","checker_suggests_satisfied"},{"proof",false},
            {"reason","Retained operands have static/declared synchronization hints; path, identity and protocol scope require review"}};
    }
    return result;
}
}
