#ifndef RAYTRACECORE_UTILITY_H
#define RAYTRACECORE_UTILITY_H


#define DEFINE_STD_HASH_SPECIALIZATION(hashable)                               \
namespace std {                                                                \
template<>                                                                     \
struct hash<hashable> {                                                        \
    std::size_t operator()(const hashable& k) const {                          \
      return std::hash<int>()(k.id);                                           \
    }                                                                          \
  };                                                                           \
}


struct GenericId {
    int id;

    bool operator==(const GenericId &other) const {
        return id == other.id;
    }

    bool operator<(const GenericId &other) const {
        return id < other.id;
    }
};

#endif //RAYTRACECORE_UTILITY_H