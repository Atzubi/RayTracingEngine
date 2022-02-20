//
// Created by sebastian on 02.07.19.
//

#include "pipeline/PipelineImplement.h"

PipelineImplement::PipelineImplement(PipelineInit &pipelineInit) {
    dmu = pipelineInit.dataManagement;
    pipelineInfo = pipelineInit.pipelineInfo;
    setShaders(pipelineInit);
    geometry = pipelineInit.geometry;
    createResultBuffer();
}

void PipelineImplement::setShaders(const PipelineInit &pipelineInit) {
    for (auto &shader: pipelineInit.rayGeneratorShaders) {
        rayGeneratorShaders[shader.id] = shader.shaderContainer;
    }
    for (auto &shader: pipelineInit.hitShaders) {
        hitShaders[shader.id] = shader.shaderContainer;
    }
    for (auto &shader: pipelineInit.occlusionShaders) {
        occlusionShaders[shader.id] = shader.shaderContainer;
    }
    for (auto &shader: pipelineInit.pierceShaders) {
        pierceShaders[shader.id] = shader.shaderContainer;
    }
    for (auto &shader: pipelineInit.missShaders) {
        missShaders[shader.id] = shader.shaderContainer;
    }
}

void PipelineImplement::createResultBuffer() {
    result = std::make_unique<Texture>();
    result->name = "Render";
    result->w = pipelineInfo.width;
    result->h = pipelineInfo.height;
    result->image = std::vector<unsigned char>(result->w * result->h * 3);

    resetResult();
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

DBVHv2 &PipelineImplement::getGeometry() {
    return geometry;
}

std::unique_ptr<Intersectable> PipelineImplement::getGeometryAsObject() {
    // TODO
    return nullptr;
}

int PipelineImplement::run() {
    resetResult();

    if (!pierceShaders.empty()) {
        fullTraversal();
    } else if (!hitShaders.empty()) {
        firstHitTraversal();
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

inline void PipelineImplement::setPixel(int id, const ShaderOutput &pixel) {
    result->image[id * 3] += pixel.color[0];
    result->image[id * 3 + 1] += pixel.color[1];
    result->image[id * 3 + 2] += pixel.color[2];
}

void
PipelineImplement::processMissShaders(int id, RayResource *&rayResource, const Ray &ray, RayGeneratorOutput &newRays) {
    for (auto &missShader: missShaders) {
        MissShaderInput missShaderInput = {ray.origin, ray.direction};
        auto pixel = missShader.second.shader->shade(id, pipelineInfo, missShaderInput,
                                                     missShader.second.shaderResources,
                                                     rayResource, newRays);
        setPixel(id, pixel);
    }
}

void PipelineImplement::processOcclusionShaders(int id, RayResource *&rayResource, const Ray &ray,
                                                RayGeneratorOutput &newRays) {
    for (auto &occlusionShader: occlusionShaders) {
        OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
        auto pixel = occlusionShader.second.shader->shade(id, pipelineInfo,
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
        auto pixel = hitShader.second.shader->shade(id, pipelineInfo, hitShaderInput,
                                                    hitShader.second.shaderResources,
                                                    rayResource, newRays);
        setPixel(id, pixel);
    }
}

void PipelineImplement::processPierceShaders(int id, RayResource *&rayResource, std::vector<IntersectionInfo> &infos,
                                             RayGeneratorOutput &newRays) {
    for (auto &pierceShader: pierceShaders) {
        PierceShaderInput pierceShaderInput = {infos};
        auto pixel = pierceShader.second.shader->shade(id, pipelineInfo, pierceShaderInput,
                                                       pierceShader.second.shaderResources,
                                                       rayResource, newRays);
        setPixel(id, pixel);
    }
}

void
PipelineImplement::generateRays(const ShaderContainer<RayGeneratorShader> &generator,
                                std::vector<RayContainer> &rayContainers,
                                int rayID, RayGeneratorOutput &rays) {
    generator.shader->shade(rayID, pipelineInfo, generator.shaderResources, rays);
    for (auto &ray: rays.rays) {
        RayContainer rayContainer = {rayID, ray.rayOrigin, ray.rayDirection, nullptr};
        rayContainers.push_back(rayContainer);
    }
    rays.rays.clear();
}

inline Ray PipelineImplement::initRay(const std::vector<RayContainer> &rayContainers) {
    auto r = rayContainers.back();
    return {r.rayOrigin, r.rayDirection, r.rayOrigin.getInverse()};
}

IntersectionInfo PipelineImplement::getFirstIntersection(std::vector<IntersectionInfo> &infos) {
    IntersectionInfo closest{false, std::numeric_limits<double>::max()};
    for (auto info: infos) {
        if (info.hit && closest.distance > info.distance) {
            closest = info;
        }
    }
    return closest;
}

void
PipelineImplement::updateRayStack(std::vector<RayContainer> &rayContainers, int id, RayGeneratorOutput &newRays) {
    rayContainers.pop_back();

    for (auto &r: newRays.rays) {
        RayContainer rayContainer = {id, r.rayDirection, r.rayOrigin,
                                     nullptr};
        rayContainers.push_back(rayContainer);
    }
    newRays.rays.clear();
}

void
PipelineImplement::generatePrimaryRays(std::vector<RayContainer> &rayContainers, int rayID, RayGeneratorOutput &rays) {
    for (auto &generator: rayGeneratorShaders) {
        generateRays(generator.second, rayContainers, rayID, rays);
    }
}

inline void
PipelineImplement::processShaders(const Ray &ray, IntersectionInfo &info, int id, RayGeneratorOutput &newRays,
                                  RayResource *rayResource) {
    if (info.hit) {
        processHitShaders(id, info, rayResource, newRays);

        processOcclusionShaders(id, rayResource, ray, newRays);
    } else {
        processMissShaders(id, rayResource, ray, newRays);
    }
}

inline void PipelineImplement::processShadersAnyHit(const Ray &ray, const IntersectionInfo &info, int id,
                                                    RayGeneratorOutput &newRays, RayResource *rayResource) {
    if (info.hit) {
        processOcclusionShaders(id, rayResource, ray, newRays);
    } else {
        processMissShaders(id, rayResource, ray, newRays);
    }
}

inline void
PipelineImplement::processShadersAllHits(const Ray &ray, std::vector<IntersectionInfo> &infos,
                                         RayGeneratorOutput &newRays,
                                         int id, RayResource *rayResource) {
    processPierceShaders(id, rayResource, infos, newRays);

    auto closest = getFirstIntersection(infos);

    processShaders(ray, closest, id, newRays, rayResource);
}

inline void
PipelineImplement::processAnyHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray,
                                            const IntersectionInfo &info, RayGeneratorOutput &newRays) {
    int id = rayContainers.back().rayID;
    auto rayResource = rayContainers.back().rayResource;
    processShadersAnyHit(ray, info, id, newRays, rayResource);

    updateRayStack(rayContainers, id, newRays);
}

inline void PipelineImplement::processFirstHitInformation(std::vector<RayContainer> &rayContainers, const Ray &ray,
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
    for (auto &info: infos) {
        info.rayOrigin = rayContainers.back().rayOrigin;
        info.rayDirection = rayContainers.back().rayDirection;
    }
    processShadersAllHits(ray, infos, newRays, id, rayResource);

    updateRayStack(rayContainers, id, newRays);
}

void PipelineImplement::processRaysAnyHit(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays) {
    while (!rayContainers.empty()) {
        Ray ray = initRay(rayContainers);
        IntersectionInfo info{false, std::numeric_limits<double>::max()};
        if (geometry.intersectAny(info, ray)) {
            info.rayOrigin = rayContainers.back().rayOrigin;
            info.rayDirection = rayContainers.back().rayDirection;
        }

        processAnyHitInformation(rayContainers, ray, info, newRays);
    }
}

void PipelineImplement::processRaysFirstHit(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays) {
    while (!rayContainers.empty()) {
        Ray ray = initRay(rayContainers);
        IntersectionInfo info{false, std::numeric_limits<double>::max()};
        if (geometry.intersectFirst(info, ray)) {
            info.rayOrigin = rayContainers.back().rayOrigin;
            info.rayDirection = rayContainers.back().rayDirection;
        }

        processFirstHitInformation(rayContainers, ray, info, newRays);
    }
}

void PipelineImplement::processRaysAllHits(std::vector<RayContainer> &rayContainers, RayGeneratorOutput &newRays) {
    while (!rayContainers.empty()) {
        Ray ray = initRay(rayContainers);
        std::vector<IntersectionInfo> infos;
        geometry.intersectAll(infos, ray);

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

void PipelineImplement::firstHitTraversal() {
    std::vector<RayContainer> rayContainers;
    RayGeneratorOutput rays;
    RayGeneratorOutput newRays;

    for (int rayID = 0; rayID < pipelineInfo.width * pipelineInfo.height; rayID++) {
        generatePrimaryRays(rayContainers, rayID, rays);

        processRaysFirstHit(rayContainers, newRays);
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
                             const ShaderContainer<RayGeneratorShader> &rayGeneratorShaderContainer) {
    rayGeneratorShaders[shaderId] = rayGeneratorShaderContainer;
}

void PipelineImplement::addShader(HitShaderId shaderId, const ShaderContainer<HitShader> &hitShaderContainer) {
    hitShaders[shaderId] = hitShaderContainer;
}

void
PipelineImplement::addShader(OcclusionShaderId shaderId,
                             const ShaderContainer<OcclusionShader> &occlusionShaderContainer) {
    occlusionShaders[shaderId] = occlusionShaderContainer;
}

void PipelineImplement::addShader(PierceShaderId shaderId, const ShaderContainer<PierceShader> &pierceShaderContainer) {
    pierceShaders[shaderId] = pierceShaderContainer;
}

void PipelineImplement::addShader(MissShaderId shaderId, const ShaderContainer<MissShader> &missShaderContainer) {
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


