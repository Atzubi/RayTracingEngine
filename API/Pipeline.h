//
// Created by sebastian on 21.07.21.
//

#ifndef RAYTRACECORE_PIPELINE_H
#define RAYTRACECORE_PIPELINE_H

#include "BasicStructures.h"
#include <vector>

struct PipelineInfo{
    int width{}, height{};
    Vector3D cameraPosition{};
    Vector3D cameraDirection{};
    Vector3D cameraUp{};
};

struct PipelineDescription{
    int resolutionX;
    int resolutionY;
    Vector3D cameraPosition;
    Vector3D cameraDirection;
    Vector3D cameraUp;
    std::vector<int> objectIDs;
    std::vector<Matrix4x4*> objectTransformations;
    std::vector<ObjectParameter*> objectParameters;
    std::vector<int> rayGeneratorShaderIDs;
    std::vector<int> occlusionShaderIDs;
    std::vector<int> hitShaderIDs;
    std::vector<int> pierceShaderIDs;
    std::vector<int> missShaderIDs;

    std::vector<int>* objectInstanceIDs;
};

#endif //RAYTRACECORE_PIPELINE_H
