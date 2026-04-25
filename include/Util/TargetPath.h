#ifndef TARGETPATH_H
#define TARGETPATH_H

#include <filesystem>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace fs = std::filesystem;

class TargetPath
{
    private:
        static TargetPath * instance;
        TargetPath() {}

        fs::path TargetAbsolutePath;
        std::string targetProjectName;
        fs::path outputDir;

    public:
        static TargetPath * getInstance(){
            if (instance == nullptr){
                instance = new TargetPath();
            }
            return instance;
        }

        void setTargetAbsolutePath(const std::string& TargetAbsolutePath){
            fs::path targetAbsolutePath = fs::path(TargetAbsolutePath);
            if (targetAbsolutePath.is_relative()) {
                targetAbsolutePath = fs::absolute(targetAbsolutePath);
            }
            if (fs::exists(targetAbsolutePath)) {
                targetAbsolutePath = fs::canonical(targetAbsolutePath);
            }
            if(fs::is_directory(targetAbsolutePath)){
                this->TargetAbsolutePath = targetAbsolutePath;
                targetProjectName = targetAbsolutePath.filename();
                if(targetProjectName == ""){
                    targetProjectName = targetAbsolutePath.parent_path().filename();
                }

                // Heuristic: if user passes ".../src" as input directory, use parent folder
                // as the "project name" to avoid collisions like multiple projects named "src".
                if (targetProjectName == "src") {
                    targetProjectName = targetAbsolutePath.parent_path().filename();
                }

                // Heuristic: make vuln_targets projects unique vs top-level clones.
                // Example: ".../vuln_targets/memcached/src" -> "vuln_targets_memcached"
                if (targetAbsolutePath.parent_path().parent_path().filename() == "vuln_targets") {
                    if (!targetProjectName.empty()) {
                        targetProjectName = std::string("vuln_targets_") + targetProjectName;
                    }
                }
            }
            else{
                this->TargetAbsolutePath = targetAbsolutePath.parent_path();
                // targetProjectName为文件名去掉后缀
                targetProjectName = targetAbsolutePath.stem();
            }
        }
        fs::path getTargetAbsolutePath() const{
            return TargetAbsolutePath;
        }

        std::string getTargetProjectName() const{
            return targetProjectName;
        }



        fs::path getOutputDir(){
            if(!outputDir.empty()){
                return outputDir;
            }

            fs::path ccpg_dump = fs::path(PROJECT_PATH) / "LLM_dump";
            if (!fs::exists(ccpg_dump)) {
                fs::create_directory(ccpg_dump);
            }

            std::string targetProjectName = getTargetProjectName();

            std::stringstream ss;
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);

            ss << targetProjectName << "_";
            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
            // create output dir with system time as name
            fs::path outputDir0 = fs::path(PROJECT_PATH) / "LLM_dump" / ss.str();
            outputDir = outputDir0;
            
            if (!fs::exists(outputDir)) {
                fs::create_directory(outputDir);
            }

            return outputDir0;

        }

};

#endif // TARGETPATH_H
