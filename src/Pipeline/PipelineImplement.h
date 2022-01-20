//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINEIMPLEMENT_H
#define RAYTRACECORE_PIPELINEIMPLEMENT_H

#include <limits>
#include <vector>
#include "RayTraceEngine/Shader.h"

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
    RayGeneratorShaderContainer rayGeneratorShader;
    RayGeneratorShaderId id;
};

struct HitShaderPackage {
    HitShaderContainer hitShader;
    HitShaderId id;
};

struct OcclusionShaderPackage {
    OcclusionShaderContainer occlusionShader;
    OcclusionShaderId id;
};

struct PierceShaderPackage {
    PierceShaderContainer pierceShader;
    PierceShaderId id;
};

struct MissShaderPackage {
    MissShaderContainer missShader;
    MissShaderId id;
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

    std::unordered_map<RayGeneratorShaderId, RayGeneratorShaderContainer> rayGeneratorShaders;
    std::unordered_map<OcclusionShaderId, OcclusionShaderContainer> occlusionShaders;
    std::unordered_map<HitShaderId, HitShaderContainer> hitShaders;
    std::unordered_map<PierceShaderId, PierceShaderContainer> pierceShaders;
    std::unordered_map<MissShaderId, MissShaderContainer> missShaders;


    DBVHNode *geometry;

    Texture *result;

public:
    PipelineImplement(EngineNode *engine, int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                      Vector3D *cameraUp, std::vector<RayGeneratorShaderPackage> *rayGeneratorShaders,
                      std::vector<OcclusionShaderPackage> *occlusionShaders,
                      std::vector<HitShaderPackage> *hitShaders,
                      std::vector<PierceShaderPackage> *pierceShaders, std::vector<MissShaderPackage> *missShaders,
                      DBVHNode *geometry);

    ~PipelineImplement();

    int run();

    Texture *getResult();

    void setResolution(int width, int height);

    void setCamera(Vector3D pos, Vector3D dir, Vector3D up);

    void addShader(RayGeneratorShaderId shaderId, RayGeneratorShaderContainer *rayGeneratorShaderContainer);

    void addShader(HitShaderId shaderId, HitShaderContainer *hitShaderContainer);

    void addShader(OcclusionShaderId shaderId, OcclusionShaderContainer *occlusionShaderContainer);

    void addShader(PierceShaderId shaderId, PierceShaderContainer *pierceShaderContainer);

    void addShader(MissShaderId shaderId, MissShaderContainer *missShaderContainer);

    bool removeShader(RayGeneratorShaderId shaderId);

    bool removeShader(HitShaderId shaderId);

    bool removeShader(OcclusionShaderId shaderId);

    bool removeShader(PierceShaderId shaderId);

    bool removeShader(MissShaderId shaderId);

    bool updateShader(RayGeneratorShaderId shaderId, std::vector<ShaderResource *> *shaderResources);

    bool updateShader(HitShaderId shaderId, std::vector<ShaderResource *> *shaderResources);

    bool updateShader(OcclusionShaderId shaderId, std::vector<ShaderResource *> *shaderResources);

    bool updateShader(PierceShaderId shaderId, std::vector<ShaderResource *> *shaderResources);

    bool updateShader(MissShaderId shaderId, std::vector<ShaderResource *> *shaderResources);

    DBVHNode *getGeometry();

    Object *getGeometryAsObject();

    void setEngine(EngineNode *engine);
};

#endif //RAYTRACECORE_PIPELINEIMPLEMENT_H
