#include <iostream>
#include <mutex>

#include "init.hpp"

namespace 
{
std::once_flag glog_initialized_flag;
}

void InitGLOG(const std::string& program_name, const std::string& log_dir, google::LogSeverity log_level) {
    std::call_once(glog_initialized_flag, [&]() {
        google::InitGoogleLogging(program_name.c_str());
        google::SetLogDestination(google::INFO, log_dir.c_str());
        google::SetLogDestination(google::WARNING, log_dir.c_str());
        google::SetLogDestination(google::ERROR, log_dir.c_str());
        google::LogSeverity(log_level);
    });
}
