#include "LLMUtil/MechanismKnowledgeBase.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace llm_client {

namespace {

std::string lowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
    return s;
}

std::vector<std::string> stringVec(const nlohmann::json& j,
                                   const char* key) {
    std::vector<std::string> out;
    if (!j.contains(key) || !j[key].is_array()) return out;
    for (const auto& item : j[key]) {
        if (item.is_string()) out.push_back(item.get<std::string>());
    }
    return out;
}

bool flagValue(const query::SharedObject& obj, const std::string& flag) {
    if (flag == "uaf_risk") return obj.has_free_operation;
    if (flag == "unprotected_write") return obj.has_unprotected_write;
    if (flag == "cross_thread_rw") return obj.has_cross_thread_rw;
    if (flag == "inconsistent_lock") return obj.has_inconsistent_locking;
    if (flag == "scalar_torn_access") return obj.has_scalar_torn_access;
    if (flag == "read_dominated_lone_writer")
        return obj.has_read_dominated_lone_writer;
    if (flag == "missing_atomic_annotation")
        return obj.has_missing_atomic_annotation;
    if (flag == "list_mutation_race") return obj.has_list_mutation;
    return false;
}

void addUnique(std::vector<std::string>& out, const std::string& value) {
    if (std::find(out.begin(), out.end(), value) == out.end()) {
        out.push_back(value);
    }
}

std::string join(const std::vector<std::string>& items,
                 const std::string& sep) {
    std::ostringstream ss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i) ss << sep;
        ss << items[i];
    }
    return ss.str();
}

std::string defaultKbPath() {
    if (const char* env = std::getenv("LACE_MECHANISM_KB")) {
        if (env[0] != '\0') return env;
    }
#ifdef PROJECT_PATH
    return (std::filesystem::path(PROJECT_PATH) /
            "kernel_experiment" / "knowledge_base" /
            "mechanism_patterns.json").string();
#else
    return "kernel_experiment/knowledge_base/mechanism_patterns.json";
#endif
}

} // namespace

const MechanismKnowledgeBase& MechanismKnowledgeBase::instance() {
    static MechanismKnowledgeBase kb;
    return kb;
}

MechanismKnowledgeBase::MechanismKnowledgeBase() {
    load();
}

void MechanismKnowledgeBase::load() {
    load_path_ = defaultKbPath();
    std::ifstream in(load_path_);
    if (!in.is_open()) {
        load_error_ = "cannot open KB file: " + load_path_;
        return;
    }

    try {
        nlohmann::json root;
        in >> root;
        if (!root.contains("patterns") || !root["patterns"].is_array()) {
            load_error_ = "KB file has no patterns array: " + load_path_;
            return;
        }

        for (const auto& pj : root["patterns"]) {
            Pattern p;
            p.id = pj.value("id", "");
            p.mechanism = pj.value("mechanism", "");
            p.description = pj.value("description", "");
            p.template_hint = pj.value("template", "");
            p.bug_category_hint = pj.value("bug_category_hint", "");
            p.object_name_regex = stringVec(pj, "object_name_regex");
            p.code_terms_any = stringVec(pj, "code_terms_any");
            p.function_terms_any = stringVec(pj, "function_terms_any");
            p.risk_flags_required = stringVec(pj, "risk_flags_required");
            p.risk_flags_any = stringVec(pj, "risk_flags_any");
            p.positive_examples = stringVec(pj, "positive_examples");
            p.negative_hints = stringVec(pj, "negative_hints");
            p.priority_boost = pj.value("priority_boost", 0);
            p.minimum_score = pj.value("minimum_score", 1);
            p.emit_interaction = pj.value("emit_interaction", false);
            p.interaction_strategy =
                pj.value("interaction_strategy", std::string("none"));
            if (!p.id.empty()) patterns_.push_back(std::move(p));
        }
        loaded_ = true;
        loadCaseCards();
    } catch (const std::exception& e) {
        load_error_ = e.what();
        patterns_.clear();
        loaded_ = false;
    }
}

void MechanismKnowledgeBase::loadCaseCards() {
    std::filesystem::path casesPath =
        std::filesystem::path(load_path_).parent_path() / "bug_cases.jsonl";
    std::ifstream in(casesPath);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        try {
            nlohmann::json j = nlohmann::json::parse(line);
            CaseCard c;
            c.case_id = j.value("case_id", "");
            if (c.case_id.empty()) continue;
            c.source = j.value("source", "");
            c.mechanism_guess = j.value("mechanism_guess", "");
            c.affected_files = stringVec(j, "affected_files");
            c.patch_actions = stringVec(j, "patch_actions");
            c.patched_functions = stringVec(j, "patched_functions");
            c.patched_objects = stringVec(j, "patched_objects");
            c.summary = j.value("summary", "");
            if (c.summary.size() > 600) c.summary.resize(600);
            case_index_[c.case_id] = case_cards_.size();
            case_cards_.push_back(std::move(c));
        } catch (const std::exception&) {
            // A stale or partially written JSONL row should not disable KB use.
        }
    }
}

const MechanismKnowledgeBase::CaseCard*
MechanismKnowledgeBase::caseCard(const std::string& case_id) const {
    auto it = case_index_.find(case_id);
    if (it == case_index_.end()) return nullptr;
    if (it->second >= case_cards_.size()) return nullptr;
    return &case_cards_[it->second];
}

std::vector<MechanismKnowledgeBase::Match>
MechanismKnowledgeBase::matchObject(const query::SharedObject& obj) const {
    std::vector<Match> matches;
    if (!loaded_) return matches;

    std::string objectText = lowerCopy(obj.name + " " + obj.type);
    std::string codeText;
    std::string functionText;
    for (const auto& a : obj.accesses) {
        codeText += " ";
        codeText += lowerCopy(a.code_snippet);
        functionText += " ";
        functionText += lowerCopy(a.function_name);
        functionText += " ";
        functionText += lowerCopy(a.containing_function);
    }

    for (const auto& p : patterns_) {
        bool requiredOk = true;
        std::vector<std::string> matchedFlags;
        for (const auto& flag : p.risk_flags_required) {
            if (!flagValue(obj, flag)) {
                requiredOk = false;
                break;
            }
            addUnique(matchedFlags, flag);
        }
        if (!requiredOk) continue;

        int score = p.priority_boost;
        bool strongEvidence = false;
        std::vector<std::string> matchedTerms;

        for (const auto& rxText : p.object_name_regex) {
            if (rxText == ".*") continue;
            try {
                std::regex rx(rxText, std::regex_constants::icase);
                if (std::regex_search(obj.name, rx)) {
                    score += 35;
                    strongEvidence = true;
                    addUnique(matchedTerms, "object:" + rxText);
                }
            } catch (const std::regex_error&) {
                // Bad KB regexes should not take down detection.
            }
        }

        for (const auto& rawTerm : p.code_terms_any) {
            std::string term = lowerCopy(rawTerm);
            if (!term.empty() && codeText.find(term) != std::string::npos) {
                score += 30;
                strongEvidence = true;
                addUnique(matchedTerms, "code:" + rawTerm);
            }
        }

        for (const auto& rawTerm : p.function_terms_any) {
            std::string term = lowerCopy(rawTerm);
            if (!term.empty() &&
                functionText.find(term) != std::string::npos) {
                score += 12;
                addUnique(matchedTerms, "function:" + rawTerm);
            }
        }

        for (const auto& flag : p.risk_flags_any) {
            if (flagValue(obj, flag)) {
                score += 18;
                strongEvidence = true;
                addUnique(matchedFlags, flag);
            }
        }

        if (!strongEvidence) continue;
        if (score < p.minimum_score) continue;

        Match m;
        m.pattern = &p;
        m.score = score;
        m.matched_terms = std::move(matchedTerms);
        m.matched_flags = std::move(matchedFlags);
        std::vector<std::string> reasonParts;
        if (!m.matched_terms.empty()) {
            reasonParts.push_back("terms=" + join(m.matched_terms, ","));
        }
        if (!m.matched_flags.empty()) {
            reasonParts.push_back("flags=" + join(m.matched_flags, ","));
        }
        m.reason = join(reasonParts, " ");
        matches.push_back(std::move(m));
    }

    std::stable_sort(matches.begin(), matches.end(),
        [](const Match& a, const Match& b) {
            return a.score > b.score;
        });
    return matches;
}

} // namespace llm_client
