#pragma once

#include <unordered_map>

template <class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>> class Cache
{
  public:
    bool Add(const Key& key, T object);

    const T& Get(const Key& key) const;

  private:
    std::unordered_map<Key, T, Hash, Pred, std::pmr::polymorphic_allocator<std::pair<const Key, T>>> cache_;
};
