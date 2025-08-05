#ifndef TARGETPATH_H
#define TARGETPATH_H

#include "Util/config.h"
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
            if(fs::is_directory(targetAbsolutePath)){
                this->TargetAbsolutePath = targetAbsolutePath;
                targetProjectName = targetAbsolutePath.filename();
                if(targetProjectName == ""){
                    targetProjectName = targetAbsolutePath.parent_path().filename();
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
