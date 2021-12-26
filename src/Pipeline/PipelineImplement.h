//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINEIMPLEMENT_H
#define RAYTRACECORE_PIPELINEIMPLEMENT_H

#include <limits>
#include <vector>

class DataManagementUnitV2;

class EngineNode;

class RayGeneratorShader;

class OcclusionShader;

class HitShader;

class PierceShader;

class MissShader;

class Object;

struct PipelineInfo;
struct DBVHNode;
struct Texture;
struct Vector3D;

/**
 * Contains all the information needed that defines a pipeline.
 * PipelineImplement Model:
 *                                      OcclusionShader
 * RayGeneratorShader -> Ray Tracer ->  HitShader
 *                                      PierceShader
 *                                      MissShader
 */
class PipelineImplement {
private:
    EngineNode *engineNode;

    PipelineInfo *pipelineInfo;

    std::vector<RayGeneratorShader *> rayGeneratorShaders;
    std::vector<OcclusionShader *> occlusionShaders;
    std::vector<HitShader *> hitShaders;
    std::vector<PierceShader *> pierceShaders;
    std::vector<MissShader *> missShaders;

    DBVHNode *geometry;

    Texture *result;

public:
    PipelineImplement(EngineNode* engine,int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                      Vector3D *cameraUp, std::vector<RayGeneratorShader *> *rayGeneratorShaders,
                      std::vector<OcclusionShader *> *occlusionShaders,
                      std::vector<HitShader *> *hitShaders, std::vector<PierceShader *> *pierceShaders,
                      std::vector<MissShader *> *missShaders, DBVHNode *geometry);

    ~PipelineImplement();

    int run();

    Texture *getResult();

    void setResolution(int width, int height);

    void setCamera(Vector3D pos, Vector3D dir, Vector3D up);

    DBVHNode *getGeometry();

    Object *getGeometryAsObject();

    void setEngine(EngineNode *engine);
};

#endif //RAYTRACECORE_PIPELINEIMPLEMENT_H
