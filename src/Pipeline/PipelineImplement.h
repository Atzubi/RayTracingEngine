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
struct ShaderResourcePackage;

struct RayGeneratorShaderContainer {
    RayGeneratorShader *rayGeneratorShader;
    std::vector<ShaderResource *> shaderResources;
};

struct HitShaderContainer {
    HitShader *hitShader;
    std::vector<ShaderResource *> shaderResources;
};

struct OcclusionShaderContainer {
    OcclusionShader *occlusionShader;
    std::vector<ShaderResource *> shaderResources;
};

struct PierceShaderContainer {
    PierceShader *pierceShader;
    std::vector<ShaderResource *> shaderResources;
};

struct MissShaderContainer {
    MissShader *missShader;
    std::vector<ShaderResource *> shaderResources;
};

struct RayGeneratorShaderPackage {
    RayGeneratorShader *rayGeneratorShader;
    int id;
};

struct HitShaderPackage {
    HitShader *hitShader;
    int id;
};

struct OcclusionShaderPackage {
    OcclusionShader *occlusionShader;
    int id;
};

struct PierceShaderPackage {
    PierceShader *pierceShader;
    int id;
};

struct MissShaderPackage {
    MissShader *missShader;
    int id;
};

struct ShaderResourceContainer {
    int shaderId;
    std::vector<ShaderResource *> shaderResources;
};

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

    std::unordered_map<int, RayGeneratorShaderContainer> rayGeneratorShaders;
    std::unordered_map<int, OcclusionShaderContainer> occlusionShaders;
    std::unordered_map<int, HitShaderContainer> hitShaders;
    std::unordered_map<int, PierceShaderContainer> pierceShaders;
    std::unordered_map<int, MissShaderContainer> missShaders;


    DBVHNode *geometry;

    Texture *result;

public:
    PipelineImplement(EngineNode *engine, int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                      Vector3D *cameraUp, std::vector<RayGeneratorShaderPackage> *rayGeneratorShaders,
                      std::vector<OcclusionShaderPackage> *occlusionShaders,
                      std::vector<HitShaderPackage> *hitShaders,
                      std::vector<PierceShaderPackage> *pierceShaders, std::vector<MissShaderPackage> *missShaders,
                      std::vector<ShaderResourceContainer> *shaderResources, DBVHNode *geometry);

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
