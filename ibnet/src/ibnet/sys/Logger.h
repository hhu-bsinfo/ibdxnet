#ifndef IBNET_SYS_LOGGER_H
#define IBNET_SYS_LOGGER_H

#include <spdlog/spdlog.h>

namespace ibnet {
namespace sys {

/**
 * Logger class to log errors, warnings, debug messages etc
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Logger
{
public:
    /**
     * Setup the logger. Make sure to call this to enable the logger
     */
    static void Setup(void);

    /**
     * Shutdown and cleanup everything. Call this before your application
     * exits to properly close and flush everything
     */
    static void Shutdown(void);

    /**
     * Get the logger
     */
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
