#ifndef UTILS_INCLUDE_STRINGUTILS_HPP
#define UTILS_INCLUDE_STRINGUTILS_HPP

#include <algorithm>
#include <string>
#include <string_view>

#define SPACES " \t\n\r\f\v"

// Inplace with string
static inline void ltrim(std::string& s, const char* delim = SPACES) { s.erase(0, s.find_first_not_of(delim)); }

static inline void rtrim(std::string& s, const char* delim = SPACES) { s.erase(s.find_last_not_of(delim) + 1); }

static inline void trim(std::string& s, const char* delim = SPACES) {
    ltrim(s, delim);
    rtrim(s, delim);
}

// Remove multiple adjacent chars listed in delim, leaving only delim[0] as delimiter char
static inline void remove_multiple_adj(std::string& s, const char* delim = SPACES) {

    const char* delim_end = delim;
    while (*delim_end != '\0') ++delim_end;

    auto s_wit = s.begin();
    bool prev_is_delim = false;
    for (char& c : s) {
        if (std::find(delim, delim_end, c)) {
            if (!prev_is_delim) {
                *s_wit = delim[0];
                ++s_wit;
                prev_is_delim = true;
            }
        } else {
            *s_wit = c;
            ++s_wit;
            prev_is_delim = false;
        }
    }
    s.erase(s_wit, s.end());
}

// Return "new" object with string_view (which are only a proxy on the same memory)
static inline std::string_view ltrim(std::string_view s, const char* delim = SPACES) { return s.substr(s.find_first_not_of(delim)); }

static inline std::string_view rtrim(std::string_view s, const char* delim = SPACES) { return s.substr(s.find_last_not_of(delim) + 1); }

static inline std::string_view trim(std::string_view s, const char* delim = SPACES) { return rtrim(ltrim(s, delim), delim); }

static inline std::string remove_multiple_adj(std::string_view& s, const char* delim = SPACES) {
    std::string s_new(s);
    remove_multiple_adj(s_new, delim);
    return s_new;
}


#endif