//
// Created by Sebastian on 28.10.2021.
//

#ifndef RAYTRACEENGINE_ENGINENODE_H
#define RAYTRACEENGINE_ENGINENODE_H

#include "RayTraceEngine/Object.h"
#include "RayTraceEngine/Shader.h"
#include "RayTraceEngine/Pipeline.h"
#include <unordered_map>

class Instance;

class PipelineImplement;

class DataManagementUnitV2;

class ShaderResource;

struct DBVHNode;

class EngineNode {
private:
    class MemoryBlock {
    private:
        std::unordered_map<ObjectId, std::unique_ptr<Object>> objects;
        std::unordered_map<InstanceId, std::unique_ptr<Instance>> objectInstances;

        std::unordered_map<ObjectId, std::unique_ptr<Object>> objectCache;
        std::unordered_map<InstanceId, std::unique_ptr<Instance>> objectInstanceCache;

        std::unordered_map<ShaderResourceId, ShaderResource *> shaderResources;

    public:
        MemoryBlock();

        ~MemoryBlock();

        void storeBaseDataFragments(std::unique_ptr<Object> &object, ObjectId id);

        bool deleteBaseDataFragment(ObjectId id);

        Object *getBaseDataFragment(ObjectId id);

        void storeInstanceDataFragments(std::unique_ptr<Instance> &instance, InstanceId id);

        bool deleteInstanceDataFragment(InstanceId id);

        Instance *getInstanceDataFragment(InstanceId id);

        void cacheBaseData(std::unique_ptr<Object> &object, ObjectId id);

        void cacheInstanceData(std::unique_ptr<Instance> &instance, InstanceId id);

        void storeShaderResource(ShaderResource *shaderResource, ShaderResourceId id);

        ShaderResource *getShaderResource(ShaderResourceId id);

        bool deleteShaderResource(ShaderResourceId id);
    };

    class PipelineBlock {
    private:
        std::unordered_map<PipelineId, PipelineImplement *> pipelines;
        std::unordered_map<DBVHNode *, DBVHNode *> pipelineCache;

        std::unordered_map<HitShaderId, HitShader *> hitShaders;
        std::unordered_map<MissShaderId, MissShader *> missShaders;
        std::unordered_map<OcclusionShaderId, OcclusionShader *> occlusionShaders;
        std::unordered_map<PierceShaderId, PierceShader *> pierceShaders;
        std::unordered_map<RayGeneratorShaderId, RayGeneratorShader *> rayGeneratorShaders;

    public:
        PipelineBlock();

        ~PipelineBlock();

        void storePipelineFragments(PipelineImplement *pipeline, PipelineId id);

        bool deletePipelineFragment(PipelineId id);

        PipelineImplement *getPipelineFragment(PipelineId id);

        void addShader(RayGeneratorShaderId id, RayGeneratorShader *shader);

        void addShader(HitShaderId id, HitShader *shader);

        void addShader(OcclusionShaderId id, OcclusionShader *shader);

        void addShader(PierceShaderId id, PierceShader *shader);

        void addShader(MissShaderId id, MissShader *shader);

        RayGeneratorShader *getShader(RayGeneratorShaderId id);

        HitShader *getShader(HitShaderId id);

        OcclusionShader *getShader(OcclusionShaderId id);

        PierceShader *getShader(PierceShaderId id);

        MissShader *getShader(MissShaderId id);

        bool deleteShader(RayGeneratorShaderId id);

        bool deleteShader(HitShaderId id);

        bool deleteShader(OcclusionShaderId id);

        bool deleteShader(PierceShaderId id);

        bool deleteShader(MissShaderId id);

        void runPipeline(PipelineId id);

        void runPipelines();
    };

    DataManagementUnitV2 *dataManagementUnit;

    MemoryBlock *memoryBlock;
    PipelineBlock *pipelineBlock;

public:
    explicit EngineNode(DataManagementUnitV2 *DMU);

    ~EngineNode();

    void storeBaseDataFragments(std::unique_ptr<Object> &object, ObjectId id);

    bool deleteBaseDataFragment(ObjectId id);

    void storeInstanceDataFragments(std::unique_ptr<Instance> &instance, InstanceId id);

    bool deleteInstanceDataFragment(InstanceId id);

    void cacheBaseData(std::unique_ptr<Object> &, ObjectId id);

    void cacheInstanceData(std::unique_ptr<Instance> &instance, InstanceId id);

    void storeShaderResource(ShaderResource *shaderResource, ShaderResourceId id);

    bool deleteShaderResource(ShaderResourceId id);

    ShaderResource *getShaderResource(ShaderResourceId id);

    void storePipelineFragments(PipelineImplement *pipeline, PipelineId id);

    bool deletePipelineFragment(PipelineId id);

    Object *requestBaseData(ObjectId id);

    Instance *requestInstanceData(InstanceId id);

    PipelineImplement *requestPipelineFragment(PipelineId id);

    void addShader(RayGeneratorShaderId id, RayGeneratorShader *shader);

    void addShader(HitShaderId id, HitShader *shader);

    void addShader(OcclusionShaderId id, OcclusionShader *shader);

    void addShader(PierceShaderId id, PierceShader *shader);

    void addShader(MissShaderId id, MissShader *shader);

    RayGeneratorShader *getShader(RayGeneratorShaderId id);

    HitShader *getShader(HitShaderId id);

    OcclusionShader *getShader(OcclusionShaderId id);

    PierceShader *getShader(PierceShaderId id);

    MissShader *getShader(MissShaderId id);

    bool deleteShader(RayGeneratorShaderId id);

    bool deleteShader(HitShaderId id);

    bool deleteShader(OcclusionShaderId id);

    bool deleteShader(PierceShaderId id);

    bool deleteShader(MissShaderId id);

    void runPipeline(PipelineId id);

    void runPipelines();
};

#endif //RAYTRACEENGINE_ENGINENODE_H
