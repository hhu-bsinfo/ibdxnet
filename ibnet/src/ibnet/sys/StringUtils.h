#ifndef IBNET_SYS_STRINGUTILS_H
#define IBNET_SYS_STRINGUTILS_H

#include <string>
#include <vector>

namespace ibnet {
namespace sys {

/**
 * Collection of utility functions for string related operations
 *
 * @author Stefan Nothaas, stefan.nothaas@hhu.de, 01.06.2017
 */
class StringUtils
{
public:
    /**
     * Split a string
     *
     * @param text String to split
     * @param delimiter One or multiple delimiters
     * @param ignoreEmptyTokens Filter empty tokens
     * @return Vector with tokens
     */
    static std::vector<std::string> Split(const std::string& text,
            const std::string& delimiter, bool ignoreEmptyTokens = true);

    /**
     * Convert a string to a hex string representation, e.g.
     * 0x1234
     *
     * @param value Value to convert
     * @param fillZeros Fill with leading 0s
     * @param hexNumberIdent Add hex number ident "0x"
     * @return Hex string representation of value
     */
    static std::string ToHexString(uint64_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

    /**
     * Convert a string to a hex string representation, e.g.
     * 0x1234
     *
     * @param value Value to convert
     * @param fillZeros Fill with leading 0s
     * @param hexNumberIdent Add hex number ident "0x"
     * @return Hex string representation of value
     */
    static std::string ToHexString(uint32_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

    /**
     * Convert a string to a hex string representation, e.g.
     * 0x1234
     *
     * @param value Value to convert
     * @param fillZeros Fill with leading 0s
     * @param hexNumberIdent Add hex number ident "0x"
     * @return Hex string representation of value
     */
    static std::string ToHexString(uint16_t value, bool fillZeros = true,
                                   bool hexNumberIdent = true);

    /**
     * Convert a string to a hex string representation, e.g.
     * 0x1234
     *
     * @param value Value to convert
     * @param fillZeros Fill with leading 0s
     * @param hexNumberIdent Add hex number ident "0x"
     * @return Hex string representation of value
     */
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
