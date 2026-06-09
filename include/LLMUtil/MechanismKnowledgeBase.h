#pragma once

#include "Query/VulnerabilitySurfaceGenerator.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace llm_client {

class MechanismKnowledgeBase {
public:
    struct Pattern {
        std::string id;
        std::string mechanism;
        std::string description;
        std::string template_hint;
        std::string bug_category_hint;
        std::vector<std::string> object_name_regex;
        std::vector<std::string> code_terms_any;
        std::vector<std::string> function_terms_any;
        std::vector<std::string> risk_flags_required;
        std::vector<std::string> risk_flags_any;
        std::vector<std::string> positive_examples;
        std::vector<std::string> negative_hints;
        int priority_boost = 0;
        int minimum_score = 1;
        bool emit_interaction = false;
        std::string interaction_strategy = "none";
    };

    struct Match {
        const Pattern* pattern = nullptr;
        int score = 0;
        std::vector<std::string> matched_terms;
        std::vector<std::string> matched_flags;
        std::string reason;
    };

    struct CaseCard {
        std::string case_id;
        std::string source;
        std::string mechanism_guess;
        std::vector<std::string> affected_files;
        std::vector<std::string> patch_actions;
        std::vector<std::string> patched_functions;
        std::vector<std::string> patched_objects;
        std::string summary;
    };

    static const MechanismKnowledgeBase& instance();

    bool loaded() const { return loaded_; }
    const std::string& loadPath() const { return load_path_; }
    const std::string& loadError() const { return load_error_; }
    const std::vector<Pattern>& patterns() const { return patterns_; }
    const CaseCard* caseCard(const std::string& case_id) const;

    std::vector<Match> matchObject(const query::SharedObject& obj) const;

private:
    MechanismKnowledgeBase();

    void load();
    void loadCaseCards();

    bool loaded_ = false;
    std::string load_path_;
    std::string load_error_;
    std::vector<Pattern> patterns_;
    std::vector<CaseCard> case_cards_;
    std::unordered_map<std::string, size_t> case_index_;
};

} // namespace llm_client
