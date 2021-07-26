#include <cassert>
#include <vector>
#include <iostream>

#include "SquareIterator.hpp"
#include "target.hpp"

auto test_scan_forward() {
    std::vector<int> squares;
    auto range = sqgen::Range<
        Chess::Square, 
        sqgen::Forward>(7);
    std::copy(range.begin(), range.end(), std::back_inserter(squares));
    std::vector expected = {0, 1, 2};

    auto result = (squares == expected);
    if (!result)
        for (auto i : squares) std::cout << i << " ";
    return result;
}

auto test_scan_reverse() {
    std::vector<int> squares;
    auto range = sqgen::Range<
        Chess::Square, 
        sqgen::Reverse>(7);
    std::copy(range.begin(), range.end(), std::back_inserter(squares));
    std::vector expected = {2, 1, 0};

    auto result = (squares == expected);
    if (!result)
        for (auto i : squares) std::cout << i << " ";
    return result;
}

auto test_carry_rippler() {
    std::vector<int> subsets;
    auto range = sqgen::CRRange(7);
    std::copy(range.begin(), range.end(), std::back_inserter(subsets));
    std::vector expected = {0, 1, 2, 3, 4, 5, 6, 7};
    
    return (subsets == expected);
}

int main() {
    std::cout << "test_scan_forward:  " << (test_scan_forward() ? "PASS ✅" : "FAIL ❌") << '\n';
    std::cout << "test_scan_reverse:  " << (test_scan_reverse() ? "PASS ✅" : "FAIL ❌") << '\n';
    std::cout << "test_carry_rippler: " << (test_carry_rippler() ? "PASS ✅" : "FAIL ❌") << '\n';
    return 0;
}
