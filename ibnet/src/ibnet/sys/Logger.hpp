#ifndef IBNET_SYS_LOGGER_HPP
#define IBNET_SYS_LOGGER_HPP

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ibnet/sys/Logger.h"

#ifndef LOG_MODULE
#define S1(x) #x
#define S2(x) S1(x)
#define LOG_MODULE strrchr(__FILE__ ":" S2(__LINE__), '/') ? \
    strrchr(__FILE__ ":" S2(__LINE__), '/') + 1 : __FILE__ ":" S2(__LINE__)
#endif

#define IBNET_LOG_PANIC(fmt, ...) ibnet::sys::Logger::GetLogger()->critical(("[{}] " + std::string(fmt)).c_str(), LOG_MODULE, ##__VA_ARGS__)
#define IBNET_LOG_ERROR(fmt, ...) ibnet::sys::Logger::GetLogger()->error(("[{}] " + std::string(fmt)).c_str(), LOG_MODULE, ##__VA_ARGS__)
#define IBNET_LOG_WARN(fmt, ...) ibnet::sys::Logger::GetLogger()->warn(("[{}] " + std::string(fmt)).c_str(), LOG_MODULE, ##__VA_ARGS__)
#define IBNET_LOG_INFO(fmt, ...) ibnet::sys::Logger::GetLogger()->info(("[{}] " + std::string(fmt)).c_str(), LOG_MODULE, ##__VA_ARGS__)
#define IBNET_LOG_DEBUG(fmt, ...) ibnet::sys::Logger::GetLogger()->debug(("[{}] " + std::string(fmt)).c_str(), LOG_MODULE, ##__VA_ARGS__)
#define IBNET_LOG_TRACE(fmt, ...) ibnet::sys::Logger::GetLogger()->trace(("[{}] " + std::string(fmt)).c_str(), LOG_MODULE, ##__VA_ARGS__)

#define IBNET_LOG_TRACE_FUNC IBNET_LOG_TRACE(__PRETTY_FUNCTION__)

#endif