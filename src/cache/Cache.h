//
// Created by Sebastian on 19.02.2022.
//

#ifndef RAYTRACEENGINE_CACHE_H
#define RAYTRACEENGINE_CACHE_H

#include <unordered_map>

template<class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
class Cache {
private:
    std::unordered_map<Key, T, Hash, Pred, std::pmr::polymorphic_allocator<std::pair<const Key, T>>> cache;

public:
    bool add(const Key &key, T &&object);

    bool get(const Key &key);
};

#endif //RAYTRACEENGINE_CACHE_H
