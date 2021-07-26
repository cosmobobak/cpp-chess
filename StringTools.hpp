#pragma once

#include <algorithm>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace strtools {

namespace _chartools {
constexpr auto is_not_lowercase(char c) noexcept {
    return c < 'a' || c > 'z';
}
constexpr auto is_not_uppercase(char c) noexcept {
    return c < 'A' || c > 'Z';
}
constexpr auto unsafe_lower_to_upper(char c) noexcept {
    constexpr auto diff = 'A' - 'a';
    return c + diff;
}
constexpr auto unsafe_upper_to_lower(char c) noexcept {
    constexpr auto diff = 'a' - 'A';
    return c + diff;
}
}  // namespace _chartools

constexpr auto toupper(char c) noexcept -> char {
    using namespace _chartools;
    return is_not_lowercase(c) ? unsafe_lower_to_upper(c) : c;
}
constexpr auto tolower(char c) noexcept -> char {
    using namespace _chartools;
    return is_not_uppercase(c) ? unsafe_upper_to_lower(c) : c;
}
// parsing the characters '0' to '9' to the integers 0 to 9
constexpr auto to_int(char c) -> int {
    return c - '0';
}

std::string toupper(std::string s) {
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](char c){ return toupper(c); });
    return s;
}

std::string tolower(std::string s) {
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](char c){ return tolower(c); });
    return s;
}

bool isupper(const std::string& s) {
    using namespace _chartools;
    // all the characters are not lowercase letters
    return std::all_of(
        s.begin(),
        s.end(),
        is_not_lowercase);
}

bool islower(const std::string& s) {
    using namespace _chartools;
    // all the characters are not uppercase letters
    return std::all_of(
        s.begin(),
        s.end(),
        is_not_uppercase);
}

auto join(const std::vector<std::string>& ss, const std::string& calator) {
    std::stringstream collector;
    collector << ss.front();
    std::accumulate(
        ++ss.begin(),
        ss.end(),
        [calator](std::stringstream acc, std::string str) {
            acc << calator << str;
        });
    return collector.str();
}

auto strip(const std::string& s) {
    auto not_a_space = [](char c) { return c != ' '; };
    using it = std::string::const_iterator;
    it start = std::find(
        s.begin(),
        s.end(),
        not_a_space);
    it end = std::find(
                 s.rbegin(),
                 s.rend(),
                 not_a_space)
                 .base();
    std::string out;
    std::copy(start, end, std::back_inserter(out));
    return out;
}

auto contains(const std::string& str, const std::string& tgt) {
    return str.find(tgt) != std::string::npos;
}

auto contains(const std::string& str, const char tgt) {
    return std::find(
               str.begin(),
               str.end(),
               tgt) != str.end();
}

auto split(const std::string& string, const char sep) {
    // consider switching to a search-and-allocate method if push_back becomes a performace bottleneck.
    std::vector<std::string> result;

    std::stringstream ss(string);
    while (ss.good()) {
        std::string substr;
        std::getline(ss, substr, sep);
        result.push_back(substr);
    }

    return result;
}

}  // namespace strtools