//
// Created by Sebastian on 28.10.2021.
//

#ifndef RAYTRACEENGINE_ENGINENODE_H
#define RAYTRACEENGINE_ENGINENODE_H

#include "RayTraceEngine/Object.h"
#include "RayTraceEngine/Pipeline.h"

class EngineNode {
private:
    class MemoryBlock {
    private:

    public:
        void storeBaseDataFragments();

        void storeInstanceDataFragments();

        void cacheBaseData();

        void cacheInstanceData();

        Object *getTraversalData();
    };

    class PipelineBlock {
    private:

    public:
        void storePipelineFragments();

        PipelineDescription* getPipelineData();
    };

    MemoryBlock memoryBlock;
    PipelineBlock pipelineBlock;

public:
    EngineNode();

    void storeBaseDataFragments();

    void storeInstanceDataFragments();

    void cacheBaseData();

    void cacheInstanceData();

    void storePipelineFragments();

    void runPipelines();
};

#endif //RAYTRACEENGINE_ENGINENODE_H
