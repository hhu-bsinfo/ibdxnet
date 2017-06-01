#ifndef IBNET_SYS_STRINGUTILS_H
#define IBNET_SYS_STRINGUTILS_H

#include <string>
#include <vector>

namespace ibnet {
namespace sys {

class StringUtils
{
public:
    static std::vector<std::string> Split(const std::string& text,
            const std::string& delimiter, bool ignoreEmptyTokens = true);

    static std::string ToHexString(uint64_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

    static std::string ToHexString(uint32_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

    static std::string ToHexString(uint16_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

    static std::string ToHexString(uint8_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

private:
    StringUtils(void) {};
    ~StringUtils(void) {};

    static std::string __ToHexString(uint64_t value, uint32_t fillZerosCount, bool hexNumberIdent);
};

}
}

#endif //IBNET_SYS_STRINGUTILS_H
