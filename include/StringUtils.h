// String utility functions for RGLite
// This file contains string manipulation utilities

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace rglite {

/**
 * @brief String utility functions
 */
class StringUtils {
public:
    /**
     * @brief Remove whitespace from both ends of a string
     * @param str The string to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);
    
    /**
     * @brief Convert string to lowercase
     * @param str The string to convert
     * @return Lowercase string
     */
    static std::string toLower(const std::string& str);
    
    /**
     * @brief Convert string to uppercase
     * @param str The string to convert
     * @return Uppercase string
     */
    static std::string toUpper(const std::string& str);
    
    /**
     * @brief Check if string starts with prefix
     * @param str The string to check
     * @param prefix The prefix to check for
     * @return True if string starts with prefix
     */
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    /**
     * @brief Check if string ends with suffix
     * @param str The string to check
     * @param suffix The suffix to check for
     * @return True if string ends with suffix
     */
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    /**
     * @brief Split string by delimiter
     * @param str The string to split
     * @param delimiter The delimiter character
     * @return Vector of tokens
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    /**
     * @brief Split string by delimiter string
     * @param str The string to split
     * @param delimiter The delimiter string
     * @return Vector of tokens
     */
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    
    /**
     * @brief Join strings with delimiter
     * @param strings The strings to join
     * @param delimiter The delimiter to use
     * @return Joined string
     */
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
    
    /**
     * @brief Check if character is whitespace
     * @param c The character to check
     * @return True if character is whitespace
     */
    static bool isWhitespace(char c);
    
    /**
     * @brief Check if character is digit
     * @param c The character to check
     * @return True if character is digit
     */
    static bool isDigit(char c);
    
    /**
     * @brief Check if character is alphabetic
     * @param c The character to check
     * @return True if character is alphabetic
     */
    static bool isAlpha(char c);
    
    /**
     * @brief Check if character is alphanumeric
     * @param c The character to check
     * @return True if character is alphanumeric
     */
    static bool isAlphaNumeric(char c);
    
    /**
     * @brief Check if character can start an identifier
     * @param c The character to check
     * @return True if character can start identifier
     */
    static bool isIdentifierStart(char c);
    
    /**
     * @brief Check if character can be in an identifier
     * @param c The character to check
     * @return True if character can be in identifier
     */
    static bool isIdentifierChar(char c);
    
    /**
     * @brief Escape special characters in string
     * @param str The string to escape
     * @return Escaped string
     */
    static std::string escapeString(const std::string& str);
    
    /**
     * @brief Unescape special characters in string
     * @param str The string to unescape
     * @return Unescaped string
     */
    static std::string unescapeString(const std::string& str);
};

} // namespace rglite

#endif // STRING_UTILS_H