#ifndef IBNET_SYS_EXCEPTION_H
#define IBNET_SYS_EXCEPTION_H

#include <stdarg.h>
#include <stdio.h>

#include <backwards/backward.hpp>

#include <exception>
#include <string>

namespace ibnet {
namespace sys {

/**
 * Exception base class. Use this for all exceptions
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class Exception : public std::exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception message
     */
    explicit Exception(const std::string& msg) :
            m_msg(msg)
    {
        m_stackTrace.load_here(32);
    }

    /**
     * Constructor
     *
     * @param msg Exception message (c-string)
     */
    explicit Exception(const char* msg) :
            m_msg(msg)
    {
        m_stackTrace.load_here(32);
    }

    /**
     * Get exception message
     *
     * @return Exception message (c-string)
     */
    virtual const char* what() const throw() {
        return m_msg.c_str();
    }

    /**
     * Get the exception message
     */
    const std::string& GetMessage(void) const {
        return m_msg;
    }

    /**
     * Print the stack trace where the exception was created
     */
    void PrintStackTrace(void) {
        backward::Printer p;
        p.print(m_stackTrace);
    }

private:
    const std::string m_msg;
    backward::StackTrace m_stackTrace;
};

}
}


#endif //IBNET_SYS_EXCEPTION_H
