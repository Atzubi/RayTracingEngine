//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYENGINE_H
#define RAYTRACECORE_RAYENGINE_H

#include <cstdint>
#include <vector>
#include <unordered_map>

class DataManagementUnit;

class Scene;

struct Vector3D{
    double x;
    double y;
    double z;
};

struct RayGeneratorOutput{
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

struct ShaderOutput{
    uint8_t color[3];
};

struct RayTracerOutput{
    Vector3D intersectionPoint;
    Vector3D normal;
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
    virtual void* getAssociatedData() = 0;
    virtual ShaderOutput shade(int id, RayTracerOutput shaderInput, void* dataInput) = 0;
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

class Object{
private:
    std::vector<double> vertices;
    std::vector<double> normals;
    std::vector<double> map;
    std::vector<uint64_t> ids;

public:
    Object(std::vector<double> const &vertices, std::vector<double> const &normals, std::vector<double> const &map,
           std::vector<uint64_t> const &ids);
    ~Object();
};

class Geometry{
private:
    Scene* scene;

public:
    Geometry();
    ~Geometry();

    int addStaticObject(Object object, Vector3D position, Vector3D orientation);
    int addAnimatedObject(Object object, Vector3D position, Vector3D orientation);
    bool removeObject(int id);
    bool moveObject(int id, Vector3D newPosition);
    bool turnObject(int id, Vector3D newOrientation);
    bool moveAndTurnObject(int id, Vector3D newPosition, Vector3D newOrientation);
    bool updateAnimatedObject(int id, Object object);
};

class RayEngine{
private:
    DataManagementUnit* dataManagementUnit;
    std::unordered_map<int, Pipeline> pipelines;

public:
    RayEngine();
    ~RayEngine();

    int addPipeline(Pipeline const &pipeline);
    bool removePipeline(int id);

    int runPipeline(int id);
    int runAll();

    void addGeometry(Geometry geometry);
};

#endif //RAYTRACECORE_RAYENGINE_H
