#ifndef INIT_HPP_
#define INIT_HPP_

#include <glog/logging.h>
#include <string>

/**
 * @brief Initialize Google's GLOG library and configure log output parameters.
 *
 * This function is used to initialize Google's GLOG library once and configure
 * log output parameters such as the log file path, log level, and standard error
 * output level. If this function has been called before, calling it again will
 * have no effect.
 *
 * @param program_name The name of the program, used to identify log outputs.
 * @param log_dir The path to the directory where log files should be written.
 * @param log_level The log level, controlling which log levels are written to log files.
 * @param stderr_log_level The standard error output level, controlling which log levels
 *                         are simultaneously output to the standard error (terminal).
 */
void InitGLOG(
    const std::string& program_name, 
    const std::string& log_dir, 
    google::LogSeverity log_level, 
    google::LogSeverity stderr_log_level);

#endif  // GLOG_UTILS_H
