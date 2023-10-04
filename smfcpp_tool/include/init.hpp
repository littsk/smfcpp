#ifndef INIT_HPP_
#define INIT_HPP_

#include <glog/logging.h>
#include <string>

void InitGLOG(const std::string& program_name, const std::string& log_dir, google::LogSeverity log_level);

#endif  // GLOG_UTILS_H
