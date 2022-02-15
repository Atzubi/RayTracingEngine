//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_PIPELINEIMPLEMENT_H
#define RAYTRACECORE_PIPELINEIMPLEMENT_H

#include <limits>
#include <vector>
#include "RayTraceEngine/Shader.h"
#include "bvh/DBVHv2.h"
#include "data_management/DataManagementUnitV2.h"

template<class Shader> requires isShader<Shader>
struct ShaderContainer {
    Shader *shader;
    std::vector<ShaderResource *> shaderResources;
};

template<class ID, class Shader> requires isShaderId<ID> && isShader<Shader> && correspondsTo<ID, Shader>
struct ShaderPackage {
    ShaderContainer<Shader> shaderContainer;
    ID id;
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

    DataManagementUnitV2 *dmu;

    PipelineInfo pipelineInfo;

    std::unordered_map<RayGeneratorShaderId, ShaderContainer<RayGeneratorShader>> rayGeneratorShaders;
    std::unordered_map<OcclusionShaderId, ShaderContainer<OcclusionShader>> occlusionShaders;
    std::unordered_map<HitShaderId, ShaderContainer<HitShader>> hitShaders;
    std::unordered_map<PierceShaderId, ShaderContainer<PierceShader>> pierceShaders;
    std::unordered_map<MissShaderId, ShaderContainer<MissShader>> missShaders;


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
    generateRays(const ShaderContainer<RayGeneratorShader> &generator, std::vector<RayContainer> &rayContainers,
                 int rayID, RayGeneratorOutput &rays);

    static Ray initRay(const std::vector<RayContainer> &rayContainers);

    static IntersectionInfo getFirstIntersection(std::vector<IntersectionInfo> &infos);

    static void updateRayStack(std::vector<RayContainer> &rayContainers, int id, RayGeneratorOutput &newRays);

    void processShadersAnyHit(const Ray &ray, const IntersectionInfo &info, int id, RayGeneratorOutput &newRays,
                              RayResource *rayResource);

    void
    processAnyHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray, const IntersectionInfo &info,
                             RayGeneratorOutput &newRays);

    void processFirstHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray, IntersectionInfo &info,
                                    RayGeneratorOutput &newRays);

    void
    processShaders(const Ray &ray, IntersectionInfo &info, int id, RayGeneratorOutput &newRays,
                   RayResource *rayResource);

    void
    processAllHitInformation(const Ray &ray, std::vector<RayContainer> &rayContainers,
                             std::vector<IntersectionInfo> &infos, RayGeneratorOutput &newRays);

    void
    processShadersAllHits(const Ray &ray, std::vector<IntersectionInfo> &infos, RayGeneratorOutput &newRays, int id,
                          RayResource *rayResource);

    void generatePrimaryRays(std::vector<RayContainer> &rayContainers, int rayID, RayGeneratorOutput &rays);

    void processRaysAnyHit(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays);

    void processRaysFirstHit(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays);

    void processRaysAllHits(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays);

    void fullTraversal();

    void firstHitTraversal();

    void anyHitTraversal();

public:
    PipelineImplement(DataManagementUnitV2 *dataManagement, int width, int height, const Vector3D &cameraPosition,
                      const Vector3D &cameraDirection, const Vector3D &cameraUp,
                      const std::vector<ShaderPackage<RayGeneratorShaderId, RayGeneratorShader>> &rayGeneratorShaders,
                      const std::vector<ShaderPackage<OcclusionShaderId, OcclusionShader>> &occlusionShaders,
                      const std::vector<ShaderPackage<HitShaderId, HitShader>> &hitShaders,
                      const std::vector<ShaderPackage<PierceShaderId, PierceShader>> &pierceShaders,
                      const std::vector<ShaderPackage<MissShaderId, MissShader>> &missShaders,
                      std::unique_ptr<DBVHNode> geometry);

    ~PipelineImplement();

    int run();

    Texture *getResult();

    void setResolution(int width, int height);

    void setCamera(const Vector3D &pos, const Vector3D &dir, const Vector3D &up);

    void
    addShader(RayGeneratorShaderId shaderId, const ShaderContainer<RayGeneratorShader> &rayGeneratorShaderContainer);

    void addShader(HitShaderId shaderId, const ShaderContainer<HitShader> &hitShaderContainer);

    void addShader(OcclusionShaderId shaderId, const ShaderContainer<OcclusionShader> &occlusionShaderContainer);

    void addShader(PierceShaderId shaderId, const ShaderContainer<PierceShader> &pierceShaderContainer);

    void addShader(MissShaderId shaderId, const ShaderContainer<MissShader> &missShaderContainer);

    bool removeShader(RayGeneratorShaderId shaderId);

    bool removeShader(HitShaderId shaderId);

    bool removeShader(OcclusionShaderId shaderId);

    bool removeShader(PierceShaderId shaderId);

    bool removeShader(MissShaderId shaderId);

    bool updateShader(RayGeneratorShaderId shaderId, const std::vector<ShaderResource *> &shaderResources);

    bool updateShader(HitShaderId shaderId, const std::vector<ShaderResource *> &shaderResources);

    bool updateShader(OcclusionShaderId shaderId, const std::vector<ShaderResource *> &shaderResources);

    bool updateShader(PierceShaderId shaderId, const std::vector<ShaderResource *> &shaderResources);

    bool updateShader(MissShaderId shaderId, const std::vector<ShaderResource *> &shaderResources);

    DBVHNode *getGeometry();

    std::unique_ptr<Intersectable> getGeometryAsObject();

    void setEngine(DataManagementUnitV2 *dataManagement);
};

#endif //RAYTRACECORE_PIPELINEIMPLEMENT_H
