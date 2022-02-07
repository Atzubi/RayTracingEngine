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
    struct RayContainer {
        int rayID;
        Vector3D rayOrigin;
        Vector3D rayDirection;
        RayResource *rayResource;
    };

    EngineNode *engineNode;

    PipelineInfo pipelineInfo;

    std::unordered_map<RayGeneratorShaderId, RayGeneratorShaderContainer> rayGeneratorShaders;
    std::unordered_map<OcclusionShaderId, OcclusionShaderContainer> occlusionShaders;
    std::unordered_map<HitShaderId, HitShaderContainer> hitShaders;
    std::unordered_map<PierceShaderId, PierceShaderContainer> pierceShaders;
    std::unordered_map<MissShaderId, MissShaderContainer> missShaders;


    std::unique_ptr<DBVHNode> geometry;

    std::unique_ptr<Texture> result;

    void resetResult();

    void setPixel(int id, const ShaderOutput &pixel);

    void processOcclusionShaders(int id, RayResource *&rayResource, const Ray &ray, RayGeneratorOutput &newRays);

    void processMissShaders(int id, RayResource *&rayResource, const Ray &ray, RayGeneratorOutput &newRays);

    void processHitShaders(int id, IntersectionInfo &info, RayResource *&rayResource, RayGeneratorOutput &newRays);

    void processPierceShaders(int id, RayResource *&rayResource, std::vector<IntersectionInfo> &infos,
                              RayGeneratorOutput &newRays);

    void
    generateRays(RayGeneratorShaderContainer &generator, std::vector<RayContainer> &rayContainers,
                 int rayID);

    static Ray initRay(const std::vector<RayContainer> &rayContainers);

    static IntersectionInfo getClosestIntersection(std::vector<IntersectionInfo> &infos, Ray &ray);

    static void updateRayStack(std::vector<RayContainer> &rayContainers, int id, RayGeneratorOutput &newRays);

    static IntersectionInfo initInfo(Ray &ray);

    void processShadersAnyHit(const Ray &ray, const IntersectionInfo &info, int id, RayGeneratorOutput &newRays,
                              RayResource *rayResource);

    void
    processAnyHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray, const IntersectionInfo &info);

    void processClosestHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray, IntersectionInfo &info);

    void
    processShaders(const Ray &ray, IntersectionInfo &info, int id, RayGeneratorOutput &newRays,
                   RayResource *rayResource);

    void
    processAllHitInformation(Ray &ray, std::vector<RayContainer> &rayContainers, std::vector<IntersectionInfo> &infos);

    void processShadersAllHits(Ray &ray, std::vector<IntersectionInfo> &infos, RayGeneratorOutput &newRays, int id,
                               RayResource *rayResource);

    void generatePrimaryRays(std::vector<RayContainer> &rayContainers, int rayID);

    void processRaysAnyHit(std::vector<RayContainer> &rayContainers);

    void processRaysClosestHit(std::vector<RayContainer> &rayContainers);

    void processRaysAllHits(std::vector<RayContainer> &rayContainers);

    void fullTraversal();

    void closestHitTraversal();

    void anyHitTraversal();

public:
    PipelineImplement(EngineNode *engine, int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                      Vector3D *cameraUp, std::vector<RayGeneratorShaderPackage> *rayGeneratorShaders,
                      std::vector<OcclusionShaderPackage> *occlusionShaders,
                      std::vector<HitShaderPackage> *hitShaders,
                      std::vector<PierceShaderPackage> *pierceShaders, std::vector<MissShaderPackage> *missShaders,
                      std::unique_ptr<DBVHNode> &geometry);

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

    std::unique_ptr<Object> getGeometryAsObject();

    void setEngine(EngineNode *engine);
};

#endif //RAYTRACECORE_PIPELINEIMPLEMENT_H
