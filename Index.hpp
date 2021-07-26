#pragma once

#include <algorithm>

template <typename Container, typename T>
constexpr auto index(Container c, T val) {
    return std::distance(
        c.begin(),
        std::find(
            c.begin(),
            c.end(),
            val));
}