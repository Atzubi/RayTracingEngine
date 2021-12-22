//
// Created by sebastian on 02.07.19.
//

#include <iostream>

#include <Data Management/DataManagementUnitV2.h>
#include "Pipeline/PipelineImplement.h"
#include "RayTraceEngine/Pipeline.h"
#include "RayTraceEngine/BasicStructures.h"
#include "RayTraceEngine/Shader.h"
#include "Acceleration Structures/DBVHv2.h"
#include "Engine Node/EngineNode.h"

PipelineImplement::PipelineImplement(DataManagementUnitV2* dmu,int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                                     Vector3D *cameraUp, std::vector<RayGeneratorShader *> *rayGeneratorShaders,
                                     std::vector<OcclusionShader *> *occlusionShaders,
                                     std::vector<HitShader *> *hitShaders, std::vector<PierceShader *> *pierceShaders,
                                     std::vector<MissShader *> *missShaders, DBVHNode *geometry) {
    this->engineNode = new EngineNode(dmu);
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
    delete result;
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
    for (int i = 0; i < pipelineInfo->width * pipelineInfo->height * 3; i++) {
        result->image[i] = 0;
    }

    if (!pierceShaders.empty()) {
        // worst case, full traversal
        for (int x = 0; x < pipelineInfo->width; x++) {
            for (int y = 0; y < pipelineInfo->height; y++) {
                for (auto generator: rayGeneratorShaders) {
                    int rayID = x + y * pipelineInfo->width;
                    auto rays = generator->shade(rayID, pipelineInfo, nullptr);
                    rays.id = rayID;
                    while (!rays.rayOrigin.empty()) {
                        Ray ray{};
                        ray.origin = rays.rayOrigin.back();
                        ray.direction = rays.rayDirection.back();
                        ray.dirfrac.x = 1.0 / ray.direction.x;
                        ray.dirfrac.y = 1.0 / ray.direction.y;
                        ray.dirfrac.z = 1.0 / ray.direction.z;

                        rays.rayOrigin.pop_back();

                        std::vector<IntersectionInfo *> infos;
                        DBVHv2::intersectAll(geometry, &infos, &ray);

                        RayGeneratorOutput newRays;

                        for (auto pierceShader: pierceShaders) {
                            PierceShaderInput pierceShaderInput = {infos};
                            auto pixel = pierceShader->shade(rays.id, pipelineInfo, &pierceShaderInput, nullptr,
                                                             &newRays);
                            result->image[rays.id * 3] += pixel.color[0];
                            result->image[rays.id * 3 + 1] += pixel.color[1];
                            result->image[rays.id * 3 + 2] += pixel.color[2];
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
                                auto pixel = hitShader->shade(rays.id, pipelineInfo, &hitShaderInput, nullptr,
                                                              &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        }

                        if (closest.hit) {
                            for (auto occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader->shade(rays.id, pipelineInfo, &occlusionShaderInput,
                                                                    nullptr, &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        }

                        if (!hitAny) {
                            for (auto missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader->shade(rays.id, pipelineInfo, &missShaderInput, nullptr,
                                                               &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        }

                        rays.rayOrigin.insert(rays.rayOrigin.begin(), newRays.rayOrigin.begin(),
                                              newRays.rayOrigin.end());
                        rays.rayDirection.insert(rays.rayDirection.begin(), newRays.rayDirection.begin(),
                                                 newRays.rayDirection.end());

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
                    auto rays = generator->shade(rayID, pipelineInfo, nullptr);
                    rays.id = rayID;
                    while (!rays.rayOrigin.empty()) {
                        Ray ray{};
                        ray.origin = rays.rayOrigin.back();
                        ray.direction = rays.rayDirection.back();
                        ray.dirfrac.x = 1.0 / ray.direction.x;
                        ray.dirfrac.y = 1.0 / ray.direction.y;
                        ray.dirfrac.z = 1.0 / ray.direction.z;

                        rays.rayOrigin.pop_back();

                        IntersectionInfo info = {false, std::numeric_limits<double>::max(), ray.origin, ray.direction,
                                                 0, 0, 0, 0, 0};
                        DBVHv2::intersectFirst(geometry, &info, &ray);

                        RayGeneratorOutput newRays;

                        if (info.hit) {
                            for (auto hitShader: hitShaders) {
                                HitShaderInput hitShaderInput = {&info};
                                auto pixel = hitShader->shade(rays.id, pipelineInfo, &hitShaderInput, nullptr,
                                                              &newRays);
                                //std::cout << rays.id << " " << (int)pixel.color[0] << " " << (int)pixel.color[1] << " " << (int)pixel.color[2] << std::endl;
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }

                            for (auto occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader->shade(rays.id, pipelineInfo, &occlusionShaderInput,
                                                                    nullptr, &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        } else {
                            for (auto missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader->shade(rays.id, pipelineInfo, &missShaderInput, nullptr,
                                                               &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        }

                        rays.rayOrigin.insert(rays.rayOrigin.begin(), newRays.rayOrigin.begin(),
                                              newRays.rayOrigin.end());
                        rays.rayDirection.insert(rays.rayDirection.begin(), newRays.rayDirection.begin(),
                                                 newRays.rayDirection.end());
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
                    auto rays = generator->shade(rayID, pipelineInfo, nullptr);
                    rays.id = rayID;
                    while (!rays.rayOrigin.empty()) {
                        Ray ray{};
                        ray.origin = rays.rayOrigin.back();
                        ray.direction = rays.rayDirection.back();
                        ray.dirfrac.x = 1.0 / ray.direction.x;
                        ray.dirfrac.y = 1.0 / ray.direction.y;
                        ray.dirfrac.z = 1.0 / ray.direction.z;

                        rays.rayOrigin.pop_back();

                        IntersectionInfo info = {false, std::numeric_limits<double>::max(), ray.origin, ray.direction,
                                                 0, 0, 0, 0, 0};
                        DBVHv2::intersectAny(geometry, &info, &ray);

                        RayGeneratorOutput newRays;

                        if (info.hit) {
                            for (auto occlusionShader: occlusionShaders) {
                                OcclusionShaderInput occlusionShaderInput = {ray.origin, ray.direction};
                                auto pixel = occlusionShader->shade(rays.id, pipelineInfo, &occlusionShaderInput,
                                                                    nullptr, &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        } else {
                            for (auto missShader: missShaders) {
                                MissShaderInput missShaderInput = {ray.origin, ray.direction};
                                auto pixel = missShader->shade(rays.id, pipelineInfo, &missShaderInput, nullptr,
                                                               &newRays);
                                result->image[rays.id * 3] += pixel.color[0];
                                result->image[rays.id * 3 + 1] += pixel.color[1];
                                result->image[rays.id * 3 + 2] += pixel.color[2];
                            }
                        }

                        rays.rayOrigin.insert(rays.rayOrigin.begin(), newRays.rayOrigin.begin(),
                                              newRays.rayOrigin.end());
                        rays.rayDirection.insert(rays.rayDirection.begin(), newRays.rayDirection.begin(),
                                                 newRays.rayDirection.end());
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


