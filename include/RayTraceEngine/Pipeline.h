//
// Created by sebastian on 21.07.21.
//

#ifndef RAYTRACECORE_PIPELINE_H
#define RAYTRACECORE_PIPELINE_H

#include "BasicStructures.h"
#include <vector>

/**
 * Container passed to shaders, containing the basic information about the Pipeline.
 * width:           Horizontal resolution.
 * height:          Vertical resolution.
 * cameraPosition:  Position of the virtual camera.
 * cameraDirection: Direction of the virtual camera facing forwards.
 * cameraUp:        Direction of the virtual camera facing upwards.
 */
struct PipelineInfo {
    int width{}, height{};
    Vector3D cameraPosition{};
    Vector3D cameraDirection{};
    Vector3D cameraUp{};
};

/**
 * Description of a pipeline for initialization.
 * resolutionX:             Horizontal resolution.
 * resolutionY:             Vertical resolution.
 * cameraPosition:          Position of the virtual camera.
 * cameraDirection:         Direction of the virtual camera facing forwards.
 * cameraUp:                Direction of the virtual camera facing upwards.
 * objectIDs:               Ids of the objects in the engines object pool.
 * objectTransformations:   Transformation information for the objects.
 * objectParameters:        Additional parameters for the objects.
 * rayGeneratorShaderIDs:   Ids of the ray generator shaders used in this pipeline.
 * occlusionShaderIDs:      Ids of the occlusion shaders used in this pipeline.
 * hitShaderIDs:            Ids of the hit shaders used in the pipeline.
 * pierceShaderIDs:         Ids of the pierce shaders used in this pipeline.
 * missShaderIDs:           Ids of the miss shaders used in this pipeline.
 * objectInstanceIDs:       Will be filled with the ids of the resulting object instances.
 */
struct PipelineDescription {
    int resolutionX;
    int resolutionY;
    Vector3D cameraPosition;
    Vector3D cameraDirection;
    Vector3D cameraUp;
    std::vector<int> objectIDs;
    std::vector<Matrix4x4 *> objectTransformations;
    std::vector<ObjectParameter *> objectParameters;
    std::vector<int> rayGeneratorShaderIDs;
    std::vector<int> occlusionShaderIDs;
    std::vector<int> hitShaderIDs;
    std::vector<int> pierceShaderIDs;
    std::vector<int> missShaderIDs;
    std::vector<ShaderResourcePackage> shaderResources;

    std::vector<int> *objectInstanceIDs;
};

#endif //RAYTRACECORE_PIPELINE_H
