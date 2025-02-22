#include "cache/Cache.h"

template <class Key, class T, class Hash, class Pred> bool Cache<Key, T, Hash, Pred>::Add(const Key& key, T object)
{
    // TODO
    return false;
}

template <class Key, class T, class Hash, class Pred> const T& Cache<Key, T, Hash, Pred>::Get(const Key& key) const
{
    // TODO
    return cache_.at(0);
}
