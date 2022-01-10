//
// Created by Sebastian on 28.10.2021.
//

#ifndef RAYTRACEENGINE_ENGINENODE_H
#define RAYTRACEENGINE_ENGINENODE_H

#include <unordered_map>

class Object;

class Instance;

class PipelineImplement;

class DataManagementUnitV2;

class ShaderResource;

class Shader;

class HitShader;

class OcclusionShader;

class PierceShader;

class MissShader;

class RayGeneratorShader;

struct DBVHNode;

class EngineNode {
private:
    class MemoryBlock {
    private:
        std::unordered_map<int, Object *> objects;
        std::unordered_map<int, Instance *> objectInstances;

        std::unordered_map<int, Object *> objectCache;
        std::unordered_map<int, Instance *> objectInstanceCache;

        std::unordered_map<int, ShaderResource *> shaderResources;

    public:
        MemoryBlock();
        ~MemoryBlock();

        void storeBaseDataFragments(Object *object, int id);

        bool deleteBaseDataFragment(int id);

        Object *getBaseDataFragment(int id);

        void storeInstanceDataFragments(Instance *instance, int id);

        bool deleteInstanceDataFragment(int id);

        Instance *getInstanceDataFragment(int id);

        void cacheBaseData(Object *object, int id);

        void cacheInstanceData(Instance *instance, int id);

        void storeShaderResource(ShaderResource *shaderResource, int id);

        bool deleteShaderResource(int id);
    };

    class PipelineBlock {
    private:
        std::unordered_map<int, PipelineImplement *> pipelines;
        std::unordered_map<DBVHNode *, DBVHNode *> pipelineCache;

        std::unordered_map<int, HitShader *> hitShaders;
        std::unordered_map<int, MissShader *> missShaders;
        std::unordered_map<int, OcclusionShader *> occlusionShaders;
        std::unordered_map<int, PierceShader *> pierceShaders;
        std::unordered_map<int, RayGeneratorShader *> rayGeneratorShaders;

    public:
        PipelineBlock();
        ~PipelineBlock();

        void storePipelineFragments(PipelineImplement *pipeline, int id);

        bool deletePipelineFragment(int id);

        PipelineImplement *getPipelineFragment(int id);

        void addShader(int id, RayGeneratorShader *shader);

        void addShader(int id, HitShader *shader);

        void addShader(int id, OcclusionShader *shader);

        void addShader(int id, PierceShader *shader);

        void addShader(int id, MissShader *shader);

        Shader* getShader(int id);

        bool deleteShader(int id);

        void runPipeline(int id);

        void runPipelines();
    };

    DataManagementUnitV2 *dataManagementUnit;

    MemoryBlock *memoryBlock;
    PipelineBlock *pipelineBlock;

public:
    explicit EngineNode(DataManagementUnitV2 *DMU);

    ~EngineNode();

    void storeBaseDataFragments(Object *object, int id);

    bool deleteBaseDataFragment(int id);

    void storeInstanceDataFragments(Instance *instance, int id);

    bool deleteInstanceDataFragment(int id);

    void cacheBaseData(Object *object, int id);

    void cacheInstanceData(Instance *instance, int id);

    void storeShaderResource(ShaderResource *shaderResource, int id);

    bool deleteShaderResource(int id);

    void storePipelineFragments(PipelineImplement *pipeline, int id);

    bool deletePipelineFragment(int id);

    Object *requestBaseData(int id);

    Instance *requestInstanceData(int id);

    PipelineImplement *requestPipelineFragment(int id);

    void addShader(int id, RayGeneratorShader *shader);

    void addShader(int id, HitShader *shader);

    void addShader(int id, OcclusionShader *shader);

    void addShader(int id, PierceShader *shader);

    void addShader(int id, MissShader *shader);

    Shader* getShader(int id);

    bool deleteShader(int id);

    void runPipeline(int id);

    void runPipelines();
};

#endif //RAYTRACEENGINE_ENGINENODE_H
