#ifndef IBNET_SYS_LOGGER_H
#define IBNET_SYS_LOGGER_H

#include <spdlog/spdlog.h>

namespace ibnet {
namespace sys {

class Logger
{
public:
    static void Setup(void);

    static void Shutdown(void);

    static std::shared_ptr<spdlog::logger>& GetLogger() {
        return m_logger;
    }

private:
    Logger(void) {};
    ~Logger(void) {};

    static std::shared_ptr<spdlog::logger> m_logger;
};

}
}

#endif //IBNET_SYS_LOGGER_H
