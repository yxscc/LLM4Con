#include "LLMUtil/SDKBridge.h"
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <spawn.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <vector>
extern char** environ;

namespace llm_client {
nlohmann::json runSDKWorker(const nlohmann::json& request, const SDKToolExecutor& execute) {
    int channel[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, channel))
        throw std::runtime_error("SDK IPC: socketpair failed");
    const char* configured = std::getenv("LACE_PYTHON");
    std::string python = configured && *configured ? configured : "python3";
    std::string worker = std::string(PROJECT_PATH) + "/harness/sdk_worker.py";
    std::vector<char*> argv{python.data(), worker.data(), nullptr};
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, channel[1], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, channel[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, channel[0]);
    posix_spawn_file_actions_addclose(&actions, channel[1]);
    pid_t pid;
    int err = posix_spawnp(&pid, python.c_str(), &actions, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    close(channel[1]);
    if (err) { close(channel[0]); throw std::runtime_error("Cannot start official SDK worker; set LACE_PYTHON and install requirements-harness.txt"); }
    auto send = [&](const nlohmann::json& j) {
        std::string data = j.dump() + "\n";
        size_t offset = 0;
        while (offset < data.size()) {
            ssize_t n = ::send(channel[0], data.data() + offset, data.size() - offset, MSG_NOSIGNAL);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) throw std::runtime_error("SDK worker closed its input");
            offset += static_cast<size_t>(n);
        }
    };
    nlohmann::json result;
    std::string pending;
    try {
        send(request);
        char buf[8192];
        bool done = false;
        while (!done) {
            ssize_t n = recv(channel[0], buf, sizeof(buf), 0);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) throw std::runtime_error("SDK worker exited without a result (analysis incomplete)");
            pending.append(buf, static_cast<size_t>(n));
            size_t end;
            while ((end = pending.find('\n')) != std::string::npos) {
                auto msg = nlohmann::json::parse(pending.substr(0, end));
                pending.erase(0, end + 1);
                std::string event = msg.value("event", "");
                if (event == "tool_call") {
                    if (!execute) throw std::runtime_error("SDK requested a tool without an executor");
                    // Tool failures are observations; transport failures remain fatal.
                    std::string output;
                    try { output = execute(msg.at("name"), msg.at("arguments")); }
                    catch (const std::exception& e) { output = nlohmann::json({{"error", e.what()}}).dump(); }
                    send({{"output", output}});
                } else if (event == "result") { result = std::move(msg); done = true; break; }
                else if (event == "error") {
                    throw std::runtime_error("Official SDK: " + msg.value("error_type", "error") + ": " + msg.value("message", "analysis incomplete"));
                } else throw std::runtime_error("Unexpected SDK IPC event");
            }
        }
    } catch (...) {
        close(channel[0]);
        kill(pid, SIGTERM);
        while (waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {}
        throw;
    }
    close(channel[0]);
    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
    if (!WIFEXITED(status) || WEXITSTATUS(status)) throw std::runtime_error("SDK worker failed after returning output");
    return result;
}
}
