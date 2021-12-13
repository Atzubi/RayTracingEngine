//
// Created by Sebastian on 28.10.2021.
//

#ifndef RAYTRACEENGINE_ENGINENODE_H
#define RAYTRACEENGINE_ENGINENODE_H

#include <unordered_map>
#include "Object/Instance.h"
#include "Acceleration Structures/DBVHv2.h"
#include "Pipeline/PipelineImplement.h"

class Instance;

class EngineNode {
private:
    class MemoryBlock {
    private:
        std::unordered_map<int, Object *> objects;
        std::unordered_map<int, Instance *> objectInstances;

        std::unordered_map<int, Object *> objectCache;
        std::unordered_map<int, Instance *> objectInstanceCache;

    public:
        void storeBaseDataFragments(Object* object, int id);

        bool deleteBaseDataFragment(int id);

        Object* getBaseDataFragment(int id);

        void storeInstanceDataFragments(Instance* instance, int id);

        bool deleteInstanceDataFragment(int id);

        Instance* getInstanceDataFragment(int id);

        void cacheBaseData(Object* object, int id);

        void cacheInstanceData(Instance* instance, int id);

        void requestTraversalData(int id);
    };

    class PipelineBlock {
    private:
        std::unordered_map<int, PipelineImplement> pipelines;
        std::unordered_map<DBVHNode*, DBVHNode*> pipelineCache;

    public:
        void storePipelineFragments();

        void requestPipelineData();
    };

    MemoryBlock memoryBlock;
    PipelineBlock pipelineBlock;

public:
    EngineNode();

    void storeBaseDataFragments(Object* object, int id);

    bool deleteBaseDataFragment(int id);

    void storeInstanceDataFragments(Instance* instance, int id);

    bool deleteInstanceDataFragment(int id);

    void cacheBaseData(Object* object, int id);

    void cacheInstanceData(Instance* instance, int id);

    void storePipelineFragments();

    Object* requestBaseData(int id);

    Instance* requestInstanceData(int id);

    void runPipelines();
};

#endif //RAYTRACEENGINE_ENGINENODE_H
