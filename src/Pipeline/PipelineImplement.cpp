//
// Created by sebastian on 02.07.19.
//

#include <iostream>

#include "Data Management/DataManagementUnitV2.h"
#include "Pipeline/PipelineImplement.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Shader.h"
#include "Acceleration Structures/DBVHv2.h"
#include "Engine Node/EngineNode.h"

PipelineImplement::PipelineImplement(EngineNode *engine, int width, int height, const Vector3D &cameraPosition,
                                     const Vector3D &cameraDirection, const Vector3D &cameraUp,
                                     const std::vector<RayGeneratorShaderPackage> &rayGeneratorShaders,
                                     const std::vector<OcclusionShaderPackage> &occlusionShaders,
                                     const std::vector<HitShaderPackage> &hitShaders,
                                     const std::vector<PierceShaderPackage> &pierceShaders,
                                     const std::vector<MissShaderPackage> &missShaders,
                                     std::unique_ptr<DBVHNode> geometry) {
    this->engineNode = engine;
    this->pipelineInfo.width = width;
    this->pipelineInfo.height = height;
    this->pipelineInfo.cameraPosition = cameraPosition;
    this->pipelineInfo.cameraDirection = cameraDirection;
    this->pipelineInfo.cameraUp = cameraUp;

    for (auto &shader: rayGeneratorShaders) {
        this->rayGeneratorShaders[shader.id] = shader.rayGeneratorShader;
    }
    for (auto &shader: hitShaders) {
        this->hitShaders[shader.id] = shader.hitShader;
    }
    for (auto &shader: occlusionShaders) {
        this->occlusionShaders[shader.id] = shader.occlusionShader;
    }
    for (auto &shader: pierceShaders) {
        this->pierceShaders[shader.id] = shader.pierceShader;
    }
    for (auto &shader: missShaders) {
        this->missShaders[shader.id] = shader.missShader;
    }

    this->geometry = std::move(geometry);
    result = std::make_unique<Texture>();
    result->name = "Render";
    result->w = width;
    result->h = height;
    result->image = std::vector<unsigned char>(width * height * 3);

    for (int i = 0; i < width * height * 3; i++) {
        result->image[i] = 0;
    }
}

PipelineImplement::~PipelineImplement() = default;

void PipelineImplement::setResolution(int resolutionWidth, int resolutionHeight) {
    pipelineInfo.width = resolutionWidth;
    pipelineInfo.height = resolutionHeight;
}

void PipelineImplement::setCamera(const Vector3D &pos, const Vector3D &dir, const Vector3D &up) {
    pipelineInfo.cameraPosition = pos;
    pipelineInfo.cameraDirection = dir;
    pipelineInfo.cameraUp = up;
}

DBVHNode *PipelineImplement::getGeometry() {
    return geometry.get();
}

std::unique_ptr<Object> PipelineImplement::getGeometryAsObject() {
    // TODO
    return nullptr;
}

int PipelineImplement::run() {
    resetResult();

    if (!pierceShaders.empty()) {
        fullTraversal();
    } else if (!hitShaders.empty()) {
        closestHitTraversal();
    } else {
        anyHitTraversal();
    }

    return 0;
}

void PipelineImplement::resetResult() {
    for (int i = 0; i < pipelineInfo.width * pipelineInfo.height * 3; i++) {
        result->image[i] = 0;
    }
}

void PipelineImplement::setPixel(int id, const ShaderOutput &pixel) {
    result->image[id * 3] += pixel.color[0];
    result->image[id * 3 + 1] += pixel.color[1];
    result->image[id * 3 + 2] += pixel.color[2];
}

void
PipelineImplement::processMissShaders(int id, RayResource *&rayResource, const Ray &ray, RayGeneratorOutput &newRays) {
    for (auto &missShader: missShaders) {
        MissShaderInput missShaderInput = {ray.origin, ray.direction};
        auto pixel = missShader.second.missShader->shade(id, pipelineInfo, missShaderInput,
                                                         missShader.second.shaderResources,
                                                         rayResource, newRays);
        setPixel(id, pixel);
    }
}

void PipelineImplement::processOcclusionShaders(int id, RayResource *&rayResource, const Ray &ray,
                                                RayGeneratorOutput &newRays) {
    for (auto &occlusionShader: occlusionShaders) {
        OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
        auto pixel = occlusionShader.second.occlusionShader->shade(id, pipelineInfo,
                                                                   occlusionShaderInput,
                                                                   occlusionShader.second.shaderResources,
                                                                   rayResource,
                                                                   newRays);
        setPixel(id, pixel);
    }
}

void PipelineImplement::processHitShaders(int id, IntersectionInfo &info, RayResource *&rayResource,
                                          RayGeneratorOutput &newRays) {
    for (auto &hitShader: hitShaders) {
        HitShaderInput hitShaderInput = {&info};
        auto pixel = hitShader.second.hitShader->shade(id, pipelineInfo, hitShaderInput,
                                                       hitShader.second.shaderResources,
                                                       rayResource, newRays);
        setPixel(id, pixel);
    }
}

void PipelineImplement::processPierceShaders(int id, RayResource *&rayResource, std::vector<IntersectionInfo> &infos,
                                             RayGeneratorOutput &newRays) {
    for (auto &pierceShader: pierceShaders) {
        PierceShaderInput pierceShaderInput = {infos};
        auto pixel = pierceShader.second.pierceShader->shade(id, pipelineInfo, pierceShaderInput,
                                                             pierceShader.second.shaderResources,
                                                             rayResource, newRays);
        setPixel(id, pixel);
    }
}

void
PipelineImplement::generateRays(const RayGeneratorShaderContainer &generator, std::vector<RayContainer> &rayContainers,
                                int rayID, RayGeneratorOutput &rays) {
    generator.rayGeneratorShader->shade(rayID, pipelineInfo, generator.shaderResources, rays);
    for (auto &ray: rays.rays) {
        RayContainer rayContainer = {rayID, ray.rayOrigin, ray.rayDirection, nullptr};
        rayContainers.push_back(rayContainer);
    }
    rays.rays.clear();
}

Ray PipelineImplement::initRay(const std::vector<RayContainer> &rayContainers) {
    Ray ray{};
    ray.origin = rayContainers.back().rayOrigin;
    ray.direction = rayContainers.back().rayDirection;
    ray.dirfrac = ray.direction.getInverse();
    return ray;
}

IntersectionInfo PipelineImplement::initInfo(const Ray &ray) {
    return {false, std::numeric_limits<double>::max(), ray.origin,
            ray.direction, {0, 0, 0}, {0, 0, 0}, {0, 0}, nullptr};
}

IntersectionInfo PipelineImplement::getClosestIntersection(std::vector<IntersectionInfo> &infos, const Ray &ray) {
    IntersectionInfo closest = initInfo(ray);
    for (auto info: infos) {
        if (info.hit && closest.distance > info.distance) {
            closest = info;
        }
    }
    return closest;
}

void
PipelineImplement::updateRayStack(std::vector<RayContainer> &rayContainers, int id, const RayGeneratorOutput &newRays) {
    rayContainers.pop_back();

    for (auto &r: newRays.rays) {
        RayContainer rayContainer = {id, r.rayDirection, r.rayOrigin,
                                     nullptr};
        rayContainers.push_back(rayContainer);
    }
}

void PipelineImplement::generatePrimaryRays(std::vector<RayContainer> &rayContainers, int rayID, RayGeneratorOutput &rays) {
    for (auto &generator: rayGeneratorShaders) {
        generateRays(generator.second, rayContainers, rayID, rays);
    }
}

void PipelineImplement::processShaders(const Ray &ray, IntersectionInfo &info, int id, RayGeneratorOutput &newRays,
                                       RayResource *rayResource) {
    if (info.hit) {
        processHitShaders(id, info, rayResource, newRays);

        processOcclusionShaders(id, rayResource, ray, newRays);
    } else {
        processMissShaders(id, rayResource, ray, newRays);
    }
}

void PipelineImplement::processShadersAnyHit(const Ray &ray, const IntersectionInfo &info, int id,
                                             RayGeneratorOutput &newRays, RayResource *rayResource) {
    if (info.hit) {
        processOcclusionShaders(id, rayResource, ray, newRays);
    } else {
        processMissShaders(id, rayResource, ray, newRays);
    }
}

void
PipelineImplement::processShadersAllHits(const Ray &ray, std::vector<IntersectionInfo> &infos, RayGeneratorOutput &newRays,
                                         int id, RayResource *rayResource) {
    processPierceShaders(id, rayResource, infos, newRays);

    auto closest = getClosestIntersection(infos, ray);

    processShaders(ray, closest, id, newRays, rayResource);
}

void
PipelineImplement::processAnyHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray,
                                            const IntersectionInfo &info, RayGeneratorOutput &newRays) {
    int id = rayContainers.back().rayID;
    auto rayResource = rayContainers.back().rayResource;
    processShadersAnyHit(ray, info, id, newRays, rayResource);

    updateRayStack(rayContainers, id, newRays);
}

void PipelineImplement::processClosestHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray,
                                                     IntersectionInfo &info, RayGeneratorOutput &newRays) {
    int id = rayContainers.back().rayID;
    auto rayResource = rayContainers.back().rayResource;
    processShaders(ray, info, id, newRays, rayResource);

    updateRayStack(rayContainers, id, newRays);
}

void PipelineImplement::processAllHitInformation(const Ray &ray, std::vector<RayContainer> &rayContainers,
                                                 std::vector<IntersectionInfo> &infos, RayGeneratorOutput &newRays) {
    int id = rayContainers.back().rayID;
    auto rayResource = rayContainers.back().rayResource;

    processShadersAllHits(ray, infos, newRays, id, rayResource);

    updateRayStack(rayContainers, id, newRays);
}

void PipelineImplement::processRaysAnyHit(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays) {
    while (!rayContainers.empty()) {
        Ray ray = initRay(rayContainers);
        IntersectionInfo info = initInfo(ray);

        DBVHv2::intersectAny(*geometry, info, ray);

        processAnyHitInformation(rayContainers, ray, info, newRays);

    }
}

void PipelineImplement::processRaysClosestHit(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays) {
    while (!rayContainers.empty()) {
        Ray ray = initRay(rayContainers);
        IntersectionInfo info = initInfo(ray);
        DBVHv2::intersectFirst(*geometry, info, ray);

        processClosestHitInformation(rayContainers, ray, info, newRays);
    }
}

void PipelineImplement::processRaysAllHits(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays) {
    while (!rayContainers.empty()) {
        Ray ray = initRay(rayContainers);
        std::vector<IntersectionInfo> infos;

        DBVHv2::intersectAll(*geometry, infos, ray);

        processAllHitInformation(ray, rayContainers, infos, newRays);

    }
}

void PipelineImplement::anyHitTraversal() {
    std::vector<RayContainer> rayContainers;
    RayGeneratorOutput rays;
    RayGeneratorOutput newRays;

    for (int rayID = 0; rayID < pipelineInfo.width * pipelineInfo.height; rayID++) {
        generatePrimaryRays(rayContainers, rayID, rays);

        processRaysAnyHit(rayContainers, newRays);
    }
}

void PipelineImplement::closestHitTraversal() {
    std::vector<RayContainer> rayContainers;
    RayGeneratorOutput rays;
    RayGeneratorOutput newRays;

    for (int rayID = 0; rayID < pipelineInfo.width * pipelineInfo.height; rayID++) {
        generatePrimaryRays(rayContainers, rayID, rays);

        processRaysClosestHit(rayContainers, newRays);
    }
}

void PipelineImplement::fullTraversal() {
    std::vector<RayContainer> rayContainers;
    RayGeneratorOutput rays;
    RayGeneratorOutput newRays;

    for (int rayID = 0; rayID < pipelineInfo.width * pipelineInfo.height; rayID++) {
        generatePrimaryRays(rayContainers, rayID, rays);

        processRaysAllHits(rayContainers, newRays);
    }
}

void
PipelineImplement::addShader(RayGeneratorShaderId shaderId,
                             const RayGeneratorShaderContainer &rayGeneratorShaderContainer) {
    rayGeneratorShaders[shaderId] = rayGeneratorShaderContainer;
}

void PipelineImplement::addShader(HitShaderId shaderId, const HitShaderContainer &hitShaderContainer) {
    hitShaders[shaderId] = hitShaderContainer;
}

void
PipelineImplement::addShader(OcclusionShaderId shaderId, const OcclusionShaderContainer &occlusionShaderContainer) {
    occlusionShaders[shaderId] = occlusionShaderContainer;
}

void PipelineImplement::addShader(PierceShaderId shaderId, const PierceShaderContainer &pierceShaderContainer) {
    pierceShaders[shaderId] = pierceShaderContainer;
}

void PipelineImplement::addShader(MissShaderId shaderId, const MissShaderContainer &missShaderContainer) {
    missShaders[shaderId] = missShaderContainer;
}

bool PipelineImplement::removeShader(RayGeneratorShaderId shaderId) {
    if (rayGeneratorShaders.count(shaderId) != 0) {
        rayGeneratorShaders.erase(shaderId);
        return true;
    }
    return false;
}

bool PipelineImplement::removeShader(HitShaderId shaderId) {
    if (hitShaders.count(shaderId) != 0) {
        hitShaders.erase(shaderId);
        return true;
    }
    return false;
}

bool PipelineImplement::removeShader(OcclusionShaderId shaderId) {
    if (occlusionShaders.count(shaderId) != 0) {
        occlusionShaders.erase(shaderId);
        return true;
    }
    return false;
}

bool PipelineImplement::removeShader(PierceShaderId shaderId) {
    if (pierceShaders.count(shaderId) != 0) {
        pierceShaders.erase(shaderId);
        return true;
    }
    return false;
}

bool PipelineImplement::removeShader(MissShaderId shaderId) {
    if (missShaders.count(shaderId) != 0) {
        missShaders.erase(shaderId);
        return true;
    }
    return false;
}

bool
PipelineImplement::updateShader(RayGeneratorShaderId shaderId, const std::vector<ShaderResource *> &shaderResources) {
    if (rayGeneratorShaders.count(shaderId) != 0) {
        rayGeneratorShaders[shaderId].shaderResources = shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(HitShaderId shaderId, const std::vector<ShaderResource *> &shaderResources) {
    if (hitShaders.count(shaderId) != 0) {
        hitShaders[shaderId].shaderResources = shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(OcclusionShaderId shaderId, const std::vector<ShaderResource *> &shaderResources) {
    if (occlusionShaders.count(shaderId) != 0) {
        occlusionShaders[shaderId].shaderResources = shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(PierceShaderId shaderId, const std::vector<ShaderResource *> &shaderResources) {
    if (pierceShaders.count(shaderId) != 0) {
        pierceShaders[shaderId].shaderResources = shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(MissShaderId shaderId, const std::vector<ShaderResource *> &shaderResources) {
    if (missShaders.count(shaderId) != 0) {
        missShaders[shaderId].shaderResources = shaderResources;
        return true;
    }
    return false;
}

Texture *PipelineImplement::getResult() {
    return result.get();
}

void PipelineImplement::setEngine(EngineNode *engine) {
    engineNode = engine;
}


