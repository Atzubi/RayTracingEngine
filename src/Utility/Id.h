#ifndef RAYTRACECORE_UTILITY_H
#define RAYTRACECORE_UTILITY_H


#define DEFINE_STD_HASH_SPECIALIZATION(hashable)       \
template<>                                             \
struct std::hash<hashable> {                           \
    std::size_t operator()(const hashable& k) const {  \
        return std::hash<int>()(k.id);                 \
    }                                                  \
};


struct GenericId {
    int id;

    bool operator==(const GenericId &other) const {
        return id == other.id;
    }

    bool operator<(const GenericId &other) const {
        return id < other.id;
    }
};



struct ShaderId: public GenericId {};
struct RayGeneratorShaderId: public ShaderId {};
struct HitShaderId: public ShaderId {};
struct OcclusionShaderId: public ShaderId {};
struct PierceShaderId: public ShaderId {};
struct MissShaderId: public ShaderId {};


struct ResourceId: public GenericId {};
struct ShaderResourceId: public ResourceId {};
struct PipelineId: public ResourceId {};
struct DeviceId: public ResourceId {};

// Compiler macros
DEFINE_STD_HASH_SPECIALIZATION(GenericId);
DEFINE_STD_HASH_SPECIALIZATION(RayGeneratorShaderId);
DEFINE_STD_HASH_SPECIALIZATION(HitShaderId);
DEFINE_STD_HASH_SPECIALIZATION(OcclusionShaderId);
DEFINE_STD_HASH_SPECIALIZATION(PierceShaderId);
DEFINE_STD_HASH_SPECIALIZATION(MissShaderId);

DEFINE_STD_HASH_SPECIALIZATION(ResourceId);
DEFINE_STD_HASH_SPECIALIZATION(ShaderResourceId);
DEFINE_STD_HASH_SPECIALIZATION(PipelineId);
DEFINE_STD_HASH_SPECIALIZATION(DeviceId);


#endif //RAYTRACECORE_UTILITY_H