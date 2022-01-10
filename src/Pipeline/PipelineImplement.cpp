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
                                     Vector3D *cameraDirection,
                                     Vector3D *cameraUp, std::vector<RayGeneratorShader *> *rayGeneratorShaders,
                                     std::vector<OcclusionShader *> *occlusionShaders,
                                     std::vector<HitShader *> *hitShaders, std::vector<PierceShader *> *pierceShaders,
                                     std::vector<MissShader *> *missShaders, DBVHNode *geometry) {
    this->engineNode = engine;
    this->pipelineInfo = new PipelineInfo();
    this->pipelineInfo->width = width;
    this->pipelineInfo->height = height;
    this->pipelineInfo->cameraPosition = *cameraPosition;
    this->pipelineInfo->cameraDirection = *cameraDirection;
    this->pipelineInfo->cameraUp = *cameraUp;
    this->rayGeneratorShaders = *rayGeneratorShaders;
    this->occlusionShaders = *occlusionShaders;
    this->hitShaders = *hitShaders;
    this->pierceShaders = *pierceShaders;
    this->missShaders = *missShaders;
    this->geometry = geometry;
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
    DBVHv2::deleteTree(geometry);
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
    return geometry;
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
                for (auto generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    generator->shade(rayID, pipelineInfo, nullptr, &rays);

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

                        std::vector<IntersectionInfo *> infos;
                        DBVHv2::intersectAll(geometry, &infos, &ray);

                        RayGeneratorOutput newRays;

                        for (auto pierceShader: pierceShaders) {
                            PierceShaderInput pierceShaderInput = {infos};
                            auto pixel = pierceShader->shade(id, pipelineInfo, &pierceShaderInput, nullptr,
                                                             &rayResource, &newRays);
                            result->image[id * 3] += pixel.color[0];
                            result->image[id * 3 + 1] += pixel.color[1];
                            result->image[id * 3 + 2] += pixel.color[2];
                        }

                        IntersectionInfo closest = {false, std::numeric_limits<double>::max(), ray.origin,
                                                    ray.direction, 0, 0, 0, 0, 0};
                        bool hitAny = false;
                        for (auto info: infos) {
                            if (info->hit) {
                                hitAny = true;
                                if (closest.distance > info->distance) {
                                    closest = *info;
                                }
                            }
                        }

                        if (closest.hit) {
                            for (auto hitShader: hitShaders) {
                                HitShaderInput hitShaderInput = {&closest};
                                auto pixel = hitShader->shade(id, pipelineInfo, &hitShaderInput, nullptr,
                                                              &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        if (closest.hit) {
                            for (auto occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader->shade(id, pipelineInfo, &occlusionShaderInput,
                                                                    nullptr, &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        }

                        if (!hitAny) {
                            for (auto missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader->shade(id, pipelineInfo, &missShaderInput, nullptr,
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

                        while (!infos.empty()) {
                            delete infos.back();
                            infos.pop_back();
                        }
                    }
                }
            }
        }
    } else if (!hitShaders.empty()) {
        // normal case, early out when closest found
        for (int x = 0; x < pipelineInfo->width; x++) {
            for (int y = 0; y < pipelineInfo->height; y++) {
                for (auto generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    generator->shade(rayID, pipelineInfo, nullptr, &rays);

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
                        DBVHv2::intersectFirst(geometry, &info, &ray);

                        RayGeneratorOutput newRays;

                        if (info.hit) {
                            for (auto hitShader: hitShaders) {
                                HitShaderInput hitShaderInput = {&info};
                                auto pixel = hitShader->shade(id, pipelineInfo, &hitShaderInput, nullptr,
                                                              &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }

                            for (auto occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader->shade(id, pipelineInfo, &occlusionShaderInput,
                                                                    nullptr, &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        } else {
                            for (auto missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader->shade(id, pipelineInfo, &missShaderInput, nullptr,
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
                for (auto generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    generator->shade(rayID, pipelineInfo, nullptr, &rays);
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
                        DBVHv2::intersectAny(geometry, &info, &ray);

                        RayGeneratorOutput newRays;

                        if (info.hit) {
                            for (auto occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader->shade(id, pipelineInfo, &occlusionShaderInput,
                                                                    nullptr, &rayResource, &newRays);
                                result->image[id * 3] += pixel.color[0];
                                result->image[id * 3 + 1] += pixel.color[1];
                                result->image[id * 3 + 2] += pixel.color[2];
                            }
                        } else {
                            for (auto missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader->shade(id, pipelineInfo, &missShaderInput, nullptr,
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

Texture *PipelineImplement::getResult() {
    return result;
}

void PipelineImplement::setEngine(EngineNode *engine) {
    engineNode = engine;
}


