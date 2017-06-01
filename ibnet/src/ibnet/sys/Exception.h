#ifndef IBNET_SYS_EXCEPTION_H
#define IBNET_SYS_EXCEPTION_H

#include <stdarg.h>
#include <stdio.h>

#include <backwards/backward.hpp>

#include <exception>
#include <string>

namespace ibnet {
namespace sys {

class Exception : public std::exception
{
public:
    explicit Exception(const std::string& msg) :
            m_msg(msg)
    {
        m_stackTrace.load_here(32);
    }

    explicit Exception(const char* msg) :
            m_msg(msg)
    {
        m_stackTrace.load_here(32);
    }

    virtual const char* what() const throw() {
        return m_msg.c_str();
    }

    const std::string& GetMessage(void) const {
        return m_msg;
    }

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
