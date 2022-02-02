//
// Created by sebastian on 02.07.19.
//

#include <iostream>

#include "Data Management/DataManagementUnitV2.h"
#include "Pipeline/PipelineImplement.h"
#include "RayTraceEngine/Pipeline.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Shader.h"
#include "Acceleration Structures/DBVHv2.h"
#include "Engine Node/EngineNode.h"

struct RayContainer {
    int rayID;
    Vector3D rayOrigin;
    Vector3D rayDirection;
    RayResource *rayResource;
};

PipelineImplement::PipelineImplement(EngineNode *engine, int width, int height, Vector3D *cameraPosition,
                                     Vector3D *cameraDirection, Vector3D *cameraUp,
                                     std::vector<RayGeneratorShaderPackage> *rayGeneratorShaders,
                                     std::vector<OcclusionShaderPackage> *occlusionShaders,
                                     std::vector<HitShaderPackage> *hitShaders,
                                     std::vector<PierceShaderPackage> *pierceShaders,
                                     std::vector<MissShaderPackage> *missShaders, std::unique_ptr<DBVHNode> &geometry) {
    this->engineNode = engine;
    this->pipelineInfo = new PipelineInfo();
    this->pipelineInfo->width = width;
    this->pipelineInfo->height = height;
    this->pipelineInfo->cameraPosition = *cameraPosition;
    this->pipelineInfo->cameraDirection = *cameraDirection;
    this->pipelineInfo->cameraUp = *cameraUp;

    for (auto &shader: *rayGeneratorShaders) {
        this->rayGeneratorShaders[shader.id] = shader.rayGeneratorShader;
    }
    for (auto &shader: *hitShaders) {
        this->hitShaders[shader.id] = shader.hitShader;
    }
    for (auto &shader: *occlusionShaders) {
        this->occlusionShaders[shader.id] = shader.occlusionShader;
    }
    for (auto &shader: *pierceShaders) {
        this->pierceShaders[shader.id] = shader.pierceShader;
    }
    for (auto &shader: *missShaders) {
        this->missShaders[shader.id] = shader.missShader;
    }

    this->geometry = std::move(geometry);
    result = new Texture{"Render", width, height, new unsigned char[width * height * 3]};

    for (int i = 0; i < width * height * 3; i++) {
        result->image[i] = 0;
    }
}

PipelineImplement::~PipelineImplement() {
    //delete geometry;
    delete[] result->image;
    delete result;
    delete pipelineInfo;
}

void PipelineImplement::setResolution(int resolutionWidth, int resolutionHeight) {
    pipelineInfo->width = resolutionWidth;
    pipelineInfo->height = resolutionHeight;
}

void PipelineImplement::setCamera(Vector3D pos, Vector3D dir, Vector3D up) {
    pipelineInfo->cameraPosition = pos;
    pipelineInfo->cameraDirection = dir;
    pipelineInfo->cameraUp = up;
}

DBVHNode *PipelineImplement::getGeometry() {
    return geometry.get();
}

Object *PipelineImplement::getGeometryAsObject() {
    // TODO
    return nullptr;
}

int PipelineImplement::run() {
    RayGeneratorOutput rays;

    std::vector<RayContainer> rayContainers;

    for (int i = 0; i < pipelineInfo->width * pipelineInfo->height * 3; i++) {
        result->image[i] = 0;
    }

    if (!pierceShaders.empty()) {
        // worst case, full traversal
        for (int x = 0; x < pipelineInfo->width; x++) {
            for (int y = 0; y < pipelineInfo->height; y++) {
                for (auto &generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    generator.second.rayGeneratorShader->shade(rayID, pipelineInfo, &generator.second.shaderResources,
                                                               &rays);

                    for (auto &ray: rays.rays) {
                        RayContainer rayContainer = {rayID, ray.rayOrigin, ray.rayDirection, nullptr};
                        rayContainers.push_back(rayContainer);
                    }

                    rays.rays.clear();

                    while (!rayContainers.empty()) {
                        int id = rayContainers.back().rayID;
                        auto rayResource = rayContainers.back().rayResource;
                        Ray ray{};
                        ray.origin = rayContainers.back().rayOrigin;
                        ray.direction = rayContainers.back().rayDirection;
                        ray.dirfrac.x = 1.0 / ray.direction.x;
                        ray.dirfrac.y = 1.0 / ray.direction.y;
                        ray.dirfrac.z = 1.0 / ray.direction.z;

                        std::vector<IntersectionInfo> infos;
                        DBVHv2::intersectAll(*geometry, infos, ray);

                        RayGeneratorOutput newRays;

                        for (auto &pierceShader: pierceShaders) {
                            PierceShaderInput pierceShaderInput = {infos};
                            auto pixel = pierceShader.second.pierceShader->shade(id, pipelineInfo, &pierceShaderInput,
                                                                                 &pierceShader.second.shaderResources,
                                                                                 &rayResource, &newRays);
                            result->image[id * 3] += pixel.color[0];
                            result->image[id * 3 + 1] += pixel.color[1];
                            result->image[id * 3 + 2] += pixel.color[2];
                        }

                        IntersectionInfo closest = {false, std::numeric_limits<double>::max(), ray.origin,
                                                    ray.direction, 0, 0, 0, 0, 0};
                        bool hitAny = false;
                        for (auto info: infos) {
                            if (info.hit) {
                                hitAny = true;
                                if (closest.distance > info.distance) {
                                    closest = info;
                                }
                            }
                        }

                        if (closest.hit) {
                            for (auto &hitShader: hitShaders) {
                                HitShaderInput hitShaderInput = {&closest};
                                auto pixel = hitShader.second.hitShader->shade(id, pipelineInfo, &hitShaderInput,
                                                                               &hitShader.second.shaderResources,
                                                                               &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        if (closest.hit) {
                            for (auto &occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader.second.occlusionShader->shade(id, pipelineInfo,
                                                                                           &occlusionShaderInput,
                                                                                           &occlusionShader.second.shaderResources,
                                                                                           &rayResource,
                                                                                           &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        if (!hitAny) {
                            for (auto &missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader.second.missShader->shade(id, pipelineInfo, &missShaderInput,
                                                                                 &missShader.second.shaderResources,
                                                                                 &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        rayContainers.pop_back();

                        for (auto &r: newRays.rays) {
                            RayContainer rayContainer = {id, r.rayDirection, r.rayOrigin,
                                                         rayResource == nullptr ? nullptr : rayResource->clone()};
                            rayContainers.push_back(rayContainer);
                        }
                    }
                }
            }
        }
    } else if (!hitShaders.empty()) {
        // normal case, early out when closest found
        for (int x = 0; x < pipelineInfo->width; x++) {
            for (int y = 0; y < pipelineInfo->height; y++) {
                for (auto &generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    generator.second.rayGeneratorShader->shade(rayID, pipelineInfo, &generator.second.shaderResources,
                                                               &rays);

                    for (auto &ray: rays.rays) {
                        RayContainer rayContainer = {rayID, ray.rayOrigin, ray.rayDirection, nullptr};
                        rayContainers.push_back(rayContainer);
                    }

                    rays.rays.clear();

                    while (!rayContainers.empty()) {
                        int id = rayContainers.back().rayID;
                        auto rayResource = rayContainers.back().rayResource;
                        Ray ray{};
                        ray.origin = rayContainers.back().rayOrigin;
                        ray.direction = rayContainers.back().rayDirection;
                        ray.dirfrac.x = 1.0 / ray.direction.x;
                        ray.dirfrac.y = 1.0 / ray.direction.y;
                        ray.dirfrac.z = 1.0 / ray.direction.z;

                        IntersectionInfo info = {false, std::numeric_limits<double>::max(), ray.origin, ray.direction,
                                                 0, 0, 0, 0, 0};
                        DBVHv2::intersectFirst(*geometry, info, ray);

                        RayGeneratorOutput newRays;

                        if (info.hit) {
                            for (auto &hitShader: hitShaders) {
                                HitShaderInput hitShaderInput = {&info};
                                auto pixel = hitShader.second.hitShader->shade(id, pipelineInfo, &hitShaderInput,
                                                                               &hitShader.second.shaderResources,
                                                                               &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }

                            for (auto &occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader.second.occlusionShader->shade(id, pipelineInfo,
                                                                                           &occlusionShaderInput,
                                                                                           &occlusionShader.second.shaderResources,
                                                                                           &rayResource,
                                                                                           &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        } else {
                            for (auto &missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader.second.missShader->shade(id, pipelineInfo, &missShaderInput,
                                                                                 &missShader.second.shaderResources,
                                                                                 &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        rayContainers.pop_back();

                        for (auto &r: newRays.rays) {
                            RayContainer rayContainer = {id, r.rayDirection, r.rayOrigin,
                                                         rayResource == nullptr ? nullptr : rayResource->clone()};
                            rayContainers.push_back(rayContainer);
                        }
                    }
                }
            }
        }
    } else {
        // best case, early out when any found
        for (int x = 0; x < pipelineInfo->width; x++) {
            for (int y = 0; y < pipelineInfo->height; y++) {
                for (auto &generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    generator.second.rayGeneratorShader->shade(rayID, pipelineInfo, &generator.second.shaderResources,
                                                               &rays);
                    for (auto &ray: rays.rays) {
                        RayContainer rayContainer = {rayID, ray.rayOrigin, ray.rayDirection, nullptr};
                        rayContainers.push_back(rayContainer);
                    }

                    rays.rays.clear();

                    while (!rayContainers.empty()) {
                        int id = rayContainers.back().rayID;
                        auto rayResource = rayContainers.back().rayResource;
                        Ray ray{};
                        ray.origin = rayContainers.back().rayOrigin;
                        ray.direction = rayContainers.back().rayDirection;
                        ray.dirfrac.x = 1.0 / ray.direction.x;
                        ray.dirfrac.y = 1.0 / ray.direction.y;
                        ray.dirfrac.z = 1.0 / ray.direction.z;

                        IntersectionInfo info = {false, std::numeric_limits<double>::max(), ray.origin, ray.direction,
                                                 0, 0, 0, 0, 0};
                        DBVHv2::intersectAny(*geometry, info, ray);

                        RayGeneratorOutput newRays;

                        if (info.hit) {
                            for (auto &occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader.second.occlusionShader->shade(id, pipelineInfo,
                                                                                           &occlusionShaderInput,
                                                                                           &occlusionShader.second.shaderResources,
                                                                                           &rayResource,
                                                                                           &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        } else {
                            for (auto &missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader.second.missShader->shade(id, pipelineInfo, &missShaderInput,
                                                                                 &missShader.second.shaderResources,
                                                                                 &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        rayContainers.pop_back();

                        for (auto &r: newRays.rays) {
                            RayContainer rayContainer = {id, r.rayDirection, r.rayOrigin,
                                                         rayResource == nullptr ? nullptr : rayResource->clone()};
                            rayContainers.push_back(rayContainer);
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void
PipelineImplement::addShader(RayGeneratorShaderId shaderId, RayGeneratorShaderContainer *rayGeneratorShaderContainer) {
    rayGeneratorShaders[shaderId] = *rayGeneratorShaderContainer;
}

void PipelineImplement::addShader(HitShaderId shaderId, HitShaderContainer *hitShaderContainer) {
    hitShaders[shaderId] = *hitShaderContainer;
}

void PipelineImplement::addShader(OcclusionShaderId shaderId, OcclusionShaderContainer *occlusionShaderContainer) {
    occlusionShaders[shaderId] = *occlusionShaderContainer;
}

void PipelineImplement::addShader(PierceShaderId shaderId, PierceShaderContainer *pierceShaderContainer) {
    pierceShaders[shaderId] = *pierceShaderContainer;
}

void PipelineImplement::addShader(MissShaderId shaderId, MissShaderContainer *missShaderContainer) {
    missShaders[shaderId] = *missShaderContainer;
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

bool PipelineImplement::updateShader(RayGeneratorShaderId shaderId, std::vector<ShaderResource *> *shaderResources) {
    if (rayGeneratorShaders.count(shaderId) != 0) {
        rayGeneratorShaders[shaderId].shaderResources = *shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(HitShaderId shaderId, std::vector<ShaderResource *> *shaderResources) {
    if (hitShaders.count(shaderId) != 0) {
        hitShaders[shaderId].shaderResources = *shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(OcclusionShaderId shaderId, std::vector<ShaderResource *> *shaderResources) {
    if (occlusionShaders.count(shaderId) != 0) {
        occlusionShaders[shaderId].shaderResources = *shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(PierceShaderId shaderId, std::vector<ShaderResource *> *shaderResources) {
    if (pierceShaders.count(shaderId) != 0) {
        pierceShaders[shaderId].shaderResources = *shaderResources;
        return true;
    }
    return false;
}

bool PipelineImplement::updateShader(MissShaderId shaderId, std::vector<ShaderResource *> *shaderResources) {
    if (missShaders.count(shaderId) != 0) {
        missShaders[shaderId].shaderResources = *shaderResources;
        return true;
    }
    return false;
}

Texture *PipelineImplement::getResult() {
    return result;
}

void PipelineImplement::setEngine(EngineNode *engine) {
    engineNode = engine;
}


