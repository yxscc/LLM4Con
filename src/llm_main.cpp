#include "CPG/CPGGenerator.h"
#include "CCPG/CCPG.h"
#include "PhasarUtil/AnalysisManager.h"
#include "LLMUtil/ModuleAnalysis.h"
#include "Util/ExecutionTimer.h"
#include "llvm/Support/CommandLine.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>
TargetPath* TargetPath::instance = nullptr;
ExecutionTimer* ExecutionTimer::instance = nullptr;
namespace {
namespace cl = llvm::cl;
cl::opt<std::string> source("input-src",cl::desc("Complete module source directory"),cl::init(""));
cl::opt<std::string> bitcode("input-bc",cl::desc("Matching LLVM .ll/.bc file"),cl::init(""));
cl::opt<std::string> modelOpt("llm-model",cl::desc("Explicit model name (or LLM_MODEL)"),cl::init(""));
cl::opt<bool> staticOnly("static-only",cl::desc("Build complete input-module graph only; no model calls"),cl::init(false));
cl::opt<bool> noReview("no-review",cl::desc("Extract contracts and run E; disable final Agent review"),cl::init(false));
std::string env(const char* a,const char* b = nullptr) {
    const char* v = std::getenv(a);
    if ((!v || !*v) && b) v = std::getenv(b);
    return v ? v : "";
}
void checkpoint(const std::string& phase,const std::string& status) {
    std::ofstream(TargetPath::getInstance()->getOutputDir()/"analysis_checkpoint.txt")
        << "Phase: " << phase << "\nStatus: " << status << '\n';
    std::cout << "[Checkpoint] " << phase << " - " << status << std::endl;
}
}
int main(int argc,char** argv) {
    llvm::cl::ParseCommandLineOptions(argc,argv,"LACE complete-module concurrency analysis\n");
    std::string phase = "inputs";
    try {
        if (!std::filesystem::is_directory(source.getValue()) || !std::filesystem::is_regular_file(bitcode.getValue()))
            throw std::invalid_argument("--input-src must be a directory and --input-bc an existing matching IR file");
        const bool noModel = staticOnly;
        const auto model = modelOpt.empty() ? env("LLM_MODEL") : modelOpt.getValue();
        const auto key = env("LLM_API_KEY","OPENAI_API_KEY");
        if (!noModel && (model.empty() || key.empty())) throw std::invalid_argument("Set LLM_MODEL and LLM_API_KEY (or OPENAI_API_KEY)");
        TargetPath::getInstance()->setTargetAbsolutePath(source);
        const auto started = std::chrono::steady_clock::now();
        phase = "static"; checkpoint(phase,"IN_PROGRESS");
        CPGGenerator generator;
        std::unique_ptr<CPG> cpg(generator.buildCPGByDot(source));
        auto pointer = std::make_unique<PhasarPointerAnalysis>(bitcode);
        AnalysisManager::getInstance()->initialize(std::move(pointer));
        auto ccpg = std::make_unique<CCPG>(cpg.get()); ccpg->build();
        const auto output = TargetPath::getInstance()->getOutputDir();
        checkpoint(phase,"COMPLETED");
        std::shared_ptr<llm_client::LLMClient> client;
        if (!noModel) {
            auto url = env("LLM_BASE_URL","OPENAI_BASE_URL");
            if (url.empty()) url = "https://api.openai.com/v1";
            auto timeout = env("LACE_LLM_TIMEOUT");
            client = std::make_shared<llm_client::LLMClient>(url,key,model,timeout.empty() ? 300 : std::stol(timeout));
        }
        phase = staticOnly ? "module_graph" : "module_agent"; checkpoint(phase,"IN_PROGRESS");
        llm_client::runModuleAnalysis(ccpg.get(),client,staticOnly,noReview);
        checkpoint(phase,"COMPLETED");
        const auto stats = client ? client->get_token_stats() : llm_client::LLMClient::TokenStats{};
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-started).count();
        std::ostringstream report;
        report << "Total Time: " << seconds << " seconds\nLLM API Requests: " << stats.total_requests
            << "\nTotal Prompt Tokens: " << stats.total_prompt_tokens << "\nTotal Completion Tokens: " << stats.total_completion_tokens
            << "\nTotal Tokens: " << stats.total_prompt_tokens + stats.total_completion_tokens << '\n';
        std::ofstream(output/"analysis_stats.txt") << report.str(); std::cout << report.str();
        return 0;
    } catch (const std::exception& e) {
        if (phase != "inputs") checkpoint(phase,"FAILED");
        std::cerr << "Analysis incomplete: " << e.what() << std::endl;
        return 2;
    }
}
