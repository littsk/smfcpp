#include <iostream>
#include <mutex>

#include "init.hpp"

namespace 
{
std::once_flag glog_initialized_flag;
}

void InitGLOG(
    const std::string& program_name,
    const std::string& log_dir,
    google::LogSeverity log_level,
    google::LogSeverity stderr_log_level)
{
    // Use std::call_once to ensure this function is only called once
    std::call_once(glog_initialized_flag, [&]() {
        // Initialize Google's GLOG library
        google::InitGoogleLogging(program_name.c_str());

        // Configure log output destinations to write logs to the specified log file directory
        google::SetLogDestination(google::INFO, log_dir.c_str());
        google::SetLogDestination(google::WARNING, log_dir.c_str());
        google::SetLogDestination(google::ERROR, log_dir.c_str());
        google::SetLogDestination(google::FATAL, log_dir.c_str());

        // Set the log level to control which log levels are written to log files
        google::LogSeverity(log_level);

        // Set the standard error output level to control which log levels are also output to the terminal
        google::SetStderrLogging(stderr_log_level);
    });
}

