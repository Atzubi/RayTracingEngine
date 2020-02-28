//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_SHADER_H
#define RAYTRACECORE_SHADER_H

#include <cstdint>
#include "BasicStructures.h"

/**
 * RayGeneratorOutput contains the data generated by the RayGeneratorShader. It is used in the ray tracer.
 * rayOrigin:           the point in space the ray is shot from
 * rayDirection:        the direction the ray is shot in
 */
struct RayGeneratorOutput{
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

/**
 * Contains the color represented as rgb generated by the pipeline
 */
struct ShaderOutput{
    uint8_t color[3];
};

/**
 * RayTracerOutput contains the data that is generated by the ray tracer.
 * intersectionPoint:   the intersection point of the ray generated by the RayGeneratorShader and the geometry bound to
 *                      this pipeline
 * normal:              the (interpolated) normal of the triangle hit at intersectionPoint
 * map:                 the (interpolated) mapping coordinates of the triangle hit at intersectionPoint
 * objectId:            the object id of the object hit
 */
struct RayTracerOutput{
    Vector3D intersectionPoint;
    Vector3D normal;
    Vector3D map;
    long objectId;
};

class Shader{

};

/**
 * Template for the Ray Generator Shader to be implemented. It generates the ray used for ray tracing.
 * getAssociatedData:   returns a pointer to the data that is fed to this RayGeneratorShader on execution
 * shade:               the template for the shader that is used in the pipeline
 */
class RayGeneratorShader : public Shader{
public:
    virtual RayGeneratorOutput shade(int id, void* dataInput) = 0;
    virtual ~RayGeneratorShader() = default;
};

/**
 * Template for the Occlusion Shader to be implemented. It is called when a ray hits anything.
 * getAssociatedData:   returns a pointer to the data that is fed to this RayGeneratorShader on execution
 * shade:               the template for the shader that is used in the pipeline
 */
class OcclusionShader : public Shader{
public:
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~OcclusionShader() = default;
};

/**
 * Template for the Pierce Shader to be implemented. It is called on every object that is hit by a ray.
 * getAssociatedData:   returns a pointer to the data that is fed to this RayGeneratorShader on execution
 * shade:               the template for the shader that is used in the pipeline
 */
class PierceShader : public Shader{
public:
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~PierceShader() = default;
};

/**
 * Template for the Hit Shader to be implemented. It is called on the closest object hit.
 * getAssociatedData:   returns a pointer to the data that is fed to this RayGeneratorShader on execution
 * shade:               the template for the shader that is used in the pipeline
 */
class HitShader : public Shader{
public:
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~HitShader() = default;
};

/**
 * Template for the Miss Shader to be implemented. It is called whenever a ray hits no geometry.
 * getAssociatedData:   returns a pointer to the data that is fed to this RayGeneratorShader on execution
 * shade:               the template for the shader that is used in the pipeline
 */
class MissShader : public Shader{
public:
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~MissShader() = default;
};

/**
 * Template for the Control Shader to be implemented. It is called after a ray has been computed. It is used for data
 * write back and has the ability to call another RayGeneratorShader.
 * getAssociatedData:   returns a pointer to the data that is fed to this RayGeneratorShader on execution
 * shade:               the template for the shader that is used in the pipeline
 */
class ControlShader : public Shader{
public:
    virtual int shade(int id, ShaderOutput shaderInput, void* dataInput) = 0;
    virtual ~ControlShader() = default;
};

#endif //RAYTRACECORE_SHADER_H