#pragma once

#include <cstddef>
#include <utility>
#include <array>

namespace cag {
// from https://stackoverflow.com/a/19023500/13243460
template <class Function, std::size_t... Indices>
constexpr auto make_array_helper(Function f, std::index_sequence<Indices...>)
    -> std::array<typename std::result_of<Function(std::size_t)>::type, sizeof...(Indices)> {
    return {{f(Indices)...}};
}

template <int N, class Function>
constexpr auto make_array(Function f)
    -> std::array<typename std::result_of<Function(std::size_t)>::type, N> {
    return make_array_helper(f, std::make_index_sequence<N>{});
}
}