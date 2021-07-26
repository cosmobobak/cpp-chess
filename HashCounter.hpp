#pragma once

#include <unordered_map>

namespace hscnt {
template <typename Hashable>
class HashCounter {
    std::unordered_map<Hashable, int> hash_map;
   public:
    HashCounter() = default;

    void add(Hashable key) {
        if (hash_map.contains(key)) {
            hash_map[key]++;
        } else {
            hash_map[key] = 0;
        }
    }

    auto count(Hashable key) -> int {
        if (hash_map.contains(key)) {
            return hash_map[key];
        } else {
            return 0;
        }
    }
};
}