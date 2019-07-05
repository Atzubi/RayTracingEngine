//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYENGINE_H
#define RAYTRACECORE_RAYENGINE_H

#include <cstdint>
#include <vector>

class DataManagementUnit;

struct RayGeneratorOutput{
    double rayOrigin[3];
    double rayDirection[3];
};

struct ShaderOutput{
    uint8_t color[3];
};

struct RayTracerOutput{
    double intersectionPoint[3];
    double normal[3];
    long objectId;
};

class RayGeneratorShader{
public:
    virtual void* getAssociatedData() = 0;
    virtual RayGeneratorOutput shade(int id, void* dataInput) = 0;
    virtual ~RayGeneratorShader() = default;
};

class OcclusionShader{
public:
    virtual void* getAssociatedData() = 0;
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~OcclusionShader() = default;
};

class PierceShader{
public:
    virtual void* getAssociatedData() = 0;
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~PierceShader() = default;
};

class HitShader{
public:
    virtual void* getAssociatedData() = 0;
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
    virtual ~HitShader() = default;
};

class MissShader{
public:
    virtual void* getAssociatedData();
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput);
    virtual ~MissShader() = default;
};

class ControlShader{
public:
    virtual void* getAssociatedData() = 0;
    virtual int shade(int id, ShaderOutput shaderInput, void* dataInput) = 0;
    virtual ~ControlShader() = default;
};

class Pipeline{
private:
    RayGeneratorShader* rayGeneratorShader;
    OcclusionShader* occlusionShader;
    PierceShader* pierceShader;
    HitShader* hitShader;
    MissShader* missShader;
    ControlShader* controlShader;
    int width, height;

public:
    Pipeline();
    ~Pipeline();

    void setResolution(int width, int height);

    bool addRayGeneratorShader(RayGeneratorShader *rayGeneratorShader);
    bool addOcclusionShader(OcclusionShader *occlusionShader);
    bool addPierceShader(PierceShader *pierceShader);
    bool addHitShader(HitShader *hitShader);
    bool addMissShader(MissShader *missShader);
    bool addControlShader(ControlShader *controlShader);
};

class Object {
private:
    std::vector<double> vertices;
    std::vector<double> normals;
    std::vector<double> map;
    std::vector<uint64_t> ids;

public:
    Object(std::vector<double> &vertices, std::vector<double> &normals, std::vector<double> &map,
           std::vector<uint64_t> &ids);
    ~Object();
};

class Geometry{
private:

public:
    Geometry();
    ~Geometry();

    int addObject(Object object);
};

class RayEngine{
private:
    DataManagementUnit* dataManagementUnit;
    std::vector<Pipeline> pipelines;

public:
    RayEngine();
    ~RayEngine();

    bool addPipeline(Pipeline const &pipeline);
    bool removePipeline(Pipeline const &pipeline);

    int runPipeline(Pipeline const &pipeline);
    int runAll();

    void addGeometry();
};

#endif //RAYTRACECORE_RAYENGINE_H
