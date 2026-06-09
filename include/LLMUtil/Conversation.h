#pragma once

#include "LLMClient.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

class CCPGNode; // Forward declaration

namespace llm_client {

class Conversation {
public:
    // 构造函数
    Conversation(std::shared_ptr<LLMClient> client, 
                const std::string& system_prompt = "",
                size_t max_history = 20);

    virtual ~Conversation() = default;

    const std::vector<ChatMessage>& get_history() const {
        return history_;
    }
    
    // 发送消息并获取回复
    std::string send_message(const std::string& user_message, void* context_for_tools = nullptr);
    
    void set_system_prompt(const std::string& prompt);

    // 重置对话（保留系统提示）
    void reset();
    
    // 获取客户端引用
    LLMClient& get_client();

    void* get_context_for_tools() const {
        return context_for_tools_;
    }

    // ---- P0: context-management knobs (default OFF; behavior-preserving) ----
    // When max_tokens > 0, prune_history switches from the legacy
    // message-count window to a token-budget window that keeps the system
    // prompt + all pinned messages and drops the oldest prunable rounds.
    // When max_tokens == 0 (default), the legacy message-count pruning is
    // used unchanged.
    void set_token_budget(size_t max_tokens) { max_tokens_ = max_tokens; }
    size_t get_token_budget() const { return max_tokens_; }

    // Optional compaction hook. When set, prune_history replaces a dropped
    // round-span with a single summary message produced by this callback
    // instead of deleting it outright. Default: null (drop). The callback
    // receives the messages about to be removed and returns the summary text.
    using Compactor = std::function<std::string(const std::vector<ChatMessage>&)>;
    void set_compactor(Compactor c) { compactor_ = std::move(c); }

    // Pin helpers: pinned messages are never dropped by prune_history (only
    // relevant under a token budget). Use to keep task setup / contracts.
    void pin_next_user_message() { pin_next_user_ = true; }
    void pin_message_at(size_t idx) { if (idx < history_.size()) history_[idx].pinned = true; }

    // Rough token estimate of a single message (content + serialized tool
    // calls). Heuristic: ~4 chars/token. Public so agents can budget.
    static size_t estimate_tokens(const ChatMessage& m) {
        size_t chars = m.content.size();
        if (m.tool_calls) {
            for (const auto& tc : *m.tool_calls) {
                chars += tc.toolname.size() + tc.arguments.dump().size() + 8;
            }
        }
        return (chars / 4) + 4;  // +4: per-message role/framing overhead
    }

private:
    std::shared_ptr<LLMClient> client_;
    std::string base_system_prompt_;
    std::vector<ChatMessage> history_;
    void* context_for_tools_ = nullptr;
    size_t max_history_messages_;
    std::ofstream simplified_log_file_;
    // P0 context-management state (see public knobs above).
    size_t max_tokens_ = 0;       // 0 => token budgeting disabled
    Compactor compactor_ = nullptr;
    bool pin_next_user_ = false;
    
    void prune_history() {
        if (history_.empty()) return;
        // P0: when a token budget is configured, use token-aware pruning;
        // otherwise fall back to the unchanged legacy message-count window.
        if (max_tokens_ > 0) {
            prune_history_by_tokens();
        } else {
            prune_history_by_count();
        }
    }

    // Legacy behavior: keep system prompt + last (max_history_messages_ - 1)
    // messages, dropping the oldest. Unchanged from the original design.
    void prune_history_by_count() {
        size_t current_size = history_.size();
        size_t limit = max_history_messages_;

        if (current_size <= limit) return;

        std::vector<ChatMessage>::iterator erase_start;
        std::vector<ChatMessage>::iterator erase_end;

        if (history_[0].role == MessageRole::SYSTEM) {
            // Keep system prompt + last (limit - 1) messages
            // We erase from index 1 to (1 + count)
            erase_start = history_.begin() + 1;
            size_t num_to_remove = current_size - limit;
            
            // Safety clamp
            if (num_to_remove > current_size - 1) num_to_remove = current_size - 1;

            erase_end = history_.begin() + 1 + num_to_remove;
        } else {
            // No system prompt, just keep last 'limit' messages
            erase_start = history_.begin();
            size_t num_to_remove = current_size - limit;
             // Safety clamp
            if (num_to_remove > current_size) num_to_remove = current_size;
            
            erase_end = history_.begin() + num_to_remove;
        }

        // CRITICAL FIX: Prevent "Orphaned Tool Response" error.
        // The OpenAI API requires that every message with role 'tool' must be immediately 
        // preceded by a message with role 'assistant' containing 'tool_calls'.
        // Since we prune from the oldest messages, we might delete an ASSISTANT message 
        // but leave its subsequent TOOL response as the new first message.
        // To fix this, if the first message we plan to KEEP (erase_end) is a TOOL message,
        // we must continue erasing until we find a non-TOOL message (or empty the history).
        while (erase_end != history_.end() && erase_end->role == MessageRole::TOOL) {
            erase_end++;
        }

        if (erase_start < erase_end) {
            history_.erase(erase_start, erase_end);
        }
    }

    // P0: token-budget window. Keeps the system prompt + all pinned messages,
    // and drops (or compacts) the oldest prunable "rounds" until the estimated
    // total token count is within budget. A round groups an ASSISTANT message
    // that carries tool_calls with its following TOOL responses, so we never
    // strand an orphaned tool response (same invariant the legacy path guards).
    void prune_history_by_tokens() {
        size_t total = 0;
        for (const auto& m : history_) total += estimate_tokens(m);
        if (total <= max_tokens_) return;

        size_t startIdx = (history_[0].role == MessageRole::SYSTEM) ? 1 : 0;
        size_t i = startIdx;

        while (total > max_tokens_ && i < history_.size()) {
            // Never drop the most recent message (needed for the next call).
            if (i + 1 >= history_.size()) break;

            if (history_[i].pinned) { ++i; continue; }

            // Determine the round span [i, spanEnd).
            size_t spanEnd = i + 1;
            if (history_[i].role == MessageRole::ASSISTANT &&
                history_[i].tool_calls && !history_[i].tool_calls->empty()) {
                while (spanEnd < history_.size() &&
                       history_[spanEnd].role == MessageRole::TOOL) {
                    ++spanEnd;
                }
            }

            // Keep the whole span if any message in it is pinned.
            bool spanPinned = false;
            for (size_t k = i; k < spanEnd; ++k) {
                if (history_[k].pinned) { spanPinned = true; break; }
            }
            if (spanPinned) { i = spanEnd; continue; }

            // Don't let the span swallow the most recent message.
            if (spanEnd >= history_.size()) break;

            size_t spanTokens = 0;
            for (size_t k = i; k < spanEnd; ++k) spanTokens += estimate_tokens(history_[k]);

            if (compactor_) {
                std::vector<ChatMessage> dropped(history_.begin() + i,
                                                 history_.begin() + spanEnd);
                std::string summary = compactor_(dropped);
                history_.erase(history_.begin() + i, history_.begin() + spanEnd);
                ChatMessage note;
                note.role = MessageRole::USER;
                note.content = "[context compacted] " + summary;
                history_.insert(history_.begin() + i, note);
                total = total - spanTokens + estimate_tokens(note);
                ++i;  // step past the inserted summary note
            } else {
                history_.erase(history_.begin() + i, history_.begin() + spanEnd);
                total -= spanTokens;
                // i now indexes the following message; do not advance.
            }
        }

        // Orphan guard: first non-system message must not be a TOOL response.
        while (startIdx < history_.size() &&
               history_[startIdx].role == MessageRole::TOOL &&
               !history_[startIdx].pinned) {
            history_.erase(history_.begin() + startIdx);
        }
    }

    // To be overridden by Agent subclasses to provide their tools
    virtual std::vector<Tool> get_available_tools() const {
        return {};
    }

    // Agent-specific tool choice for OpenAI-compatible APIs.
    // Typical values: "auto" (default) or "required" (force tool calls when tools are provided).
    virtual std::string get_tool_choice() const {
        return "auto";
    }

    virtual std::string parseResult(const std::vector<ChatMessage>& history) {
        // Default implementation just returns the last assistant message
        if (!history.empty() && history.back().role == MessageRole::ASSISTANT) {
            return history.back().content;
        }
        return "No valid assistant response found.";
    }

    // To be overridden by Agent subclasses to execute their specific tools
    // The `tool_execution_context_` member is available for use here.
    virtual std::string execute_tool(const std::string& tool_name, const nlohmann::json& arguments) {
        nlohmann::json error_resp;
        error_resp["error"] = "Tool '" + tool_name + "' not found or not implemented by this agent.";
        error_resp["tool_name"] = tool_name;
        error_resp["arguments_received"] = arguments;
        return error_resp.dump();
    }

    // Helper to build the full system prompt including tool descriptions
    virtual std::string build_effective_system_prompt() {
        std::string effective_prompt = base_system_prompt_;
        return effective_prompt;
    }
};

} // namespace llm_client