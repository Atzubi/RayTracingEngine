//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "src/Pipeline/PipelineImplement.h"

PipelineImplement::PipelineImplement(int width, int height, Vector3D *cameraPosition, Vector3D *cameraDirection,
                                     Vector3D *cameraUp, std::vector<RayGeneratorShader *> *rayGeneratorShaders,
                                     std::vector<OcclusionShader *> *occlusionShaders,
                                     std::vector<HitShader *> *hitShaders, std::vector<PierceShader *> *pierceShaders,
                                     std::vector<MissShader *> *missShaders, Object *geometry) {
    this->pipelineInfo.width = width;
    this->pipelineInfo.height = height;
    this->pipelineInfo.cameraPosition = *cameraPosition;
    this->pipelineInfo.cameraDirection = *cameraDirection;
    this->pipelineInfo.cameraUp = *cameraUp;
    this->rayGeneratorShaders = *rayGeneratorShaders;
    this->occlusionShaders = *occlusionShaders;
    this->hitShaders = *hitShaders;
    this->pierceShaders = *pierceShaders;
    this->missShaders = *missShaders;
    this->geometry = geometry;
    result = {"Render", width, height, new unsigned char[width * height * 3]};

    for (int i = 0; i < width * height * 3; i++) {
        result.image[i] = 0;
    }
}

PipelineImplement::~PipelineImplement() {
    //delete geometry;
}

void PipelineImplement::setResolution(int resolutionWidth, int resolutionHeight) {
    pipelineInfo.width = resolutionWidth;
    pipelineInfo.height = resolutionHeight;
}

void PipelineImplement::setCamera(Vector3D pos, Vector3D dir, Vector3D up) {
    pipelineInfo.cameraPosition = pos;
    pipelineInfo.cameraDirection = dir;
    pipelineInfo.cameraUp = up;
}

Object *PipelineImplement::getGeometryAsObject() {
    // TODO
    return nullptr;
}

int PipelineImplement::run() {
    for (int x = 0; x < pipelineInfo.width; x++) {
        for (int y = 0; y < pipelineInfo.height; y++) {
            for (auto generator : rayGeneratorShaders) {
                int rayID = x + y * pipelineInfo.width;
                auto rays = generator->shade(rayID, &pipelineInfo, nullptr);
                rays.id = rayID;
                while (!rays.rayOrigin.empty()) {
                    Ray ray{};
                    ray.origin = rays.rayOrigin.back();
                    ray.direction = rays.rayDirection.back();
                    ray.dirfrac.x = 1.0 / ray.direction.x;
                    ray.dirfrac.y = 1.0 / ray.direction.y;
                    ray.dirfrac.z = 1.0 / ray.direction.z;

                    rays.rayOrigin.pop_back();

                    IntersectionInfo info = {false, std::numeric_limits<double>::max(), ray.origin, ray.direction, 0, 0,
                                             0, 0, 0};
                    geometry->intersect(&info, &ray);

                    RayGeneratorOutput newRays;

                    if(info.hit) {

                        for (auto hitShader : hitShaders) {
                            auto pixel = hitShader->shade(rays.id, &pipelineInfo, &info, nullptr, &newRays);
                            //std::cout << rays.id << " " << (int)pixel.color[0] << " " << (int)pixel.color[1] << " " << (int)pixel.color[2] << std::endl;
                            result.image[rays.id * 3] += pixel.color[0];
                            result.image[rays.id * 3 + 1] += pixel.color[1];
                            result.image[rays.id * 3 + 2] += pixel.color[2];
                        }
                    }

                    rays.rayOrigin.insert(rays.rayOrigin.begin(), newRays.rayOrigin.begin(), newRays.rayOrigin.end());
                    rays.rayDirection.insert(rays.rayDirection.begin(), newRays.rayDirection.begin(),
                                             newRays.rayDirection.end());
                }
            }
        }
    }
    return 0;
}

Texture PipelineImplement::getResult() {
    return result;
}


