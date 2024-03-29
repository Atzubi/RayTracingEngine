//
// Created by Sebastian on 04.12.2021.
//

#ifndef RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H
#define RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H

#include "RayTraceEngine/Shader.h"
#include "RayTraceEngine/Pipeline.h"
#include <set>
#include <unordered_map>
#include <vector>

class EngineNode;

class HitShader;

class MissShader;

class OcclusionShader;

class PierceShader;

class RayGeneratorShader;

class Object;

class Instance;

class ShaderResource;

struct DBVHNode;
struct PipelineDescription;
struct Vector3D;
struct Texture;
struct ObjectParameter;
struct Matrix4x4;

struct DeviceId{
    int deviceId;

    bool operator==(const DeviceId &other) const {
        return deviceId == other.deviceId;
    }

    bool operator<(const DeviceId &other) const {
        return deviceId < other.deviceId;
    }
};

template<>
struct std::hash<DeviceId>{
    std::size_t operator()(const DeviceId& k) const{
        return std::hash<int>()(k.deviceId);
    }
};

class DataManagementUnitV2 {
private:
    DeviceId deviceId;

    EngineNode *engineNode;

    // stored only in main DMU
    std::set<ObjectId> objectIds;
    std::set<InstanceId> objectInstanceIds;
    std::set<RayGeneratorShaderId> rayGeneratorShaderIds;
    std::set<HitShaderId> hitShaderIds;
    std::set<OcclusionShaderId> occlusionShaderIds;
    std::set<PierceShaderId> pierceShaderIds;
    std::set<MissShaderId> missShaderIds;
    std::set<ShaderResourceId> shaderResourceIds;
    std::set<PipelineId> pipelineIds;
    std::unordered_map<ObjectId, std::set<InstanceId>> objectToInstanceMap;
    std::unordered_map<PipelineId, std::set<InstanceId>> pipelineToInstanceMap;

    //std::unordered_map<int, PipelineImplement *> pipelines; // groups  pipeline information, copied to every node

    // maps ids to devices holding the data
    std::unordered_map<ObjectId, DeviceId> objectIdDeviceMap;
    std::unordered_map<InstanceId, DeviceId> objectInstanceIdDeviceMap;
    std::unordered_map<DBVHNode *, DeviceId> pipelineTreeDeviceMap;

    DeviceId getDeviceId();

public:
    DataManagementUnitV2();

    ~DataManagementUnitV2();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    PipelineId createPipeline(PipelineDescription *pipelineDescription);

    void
    updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY, Vector3D cameraPosition, Vector3D cameraDirection,
                         Vector3D cameraUp);

    Texture *getPipelineResult(PipelineId id);

    /*
     * Removes a pipeline by id.
     * return:          true if success, false otherwise
     */
    bool removePipeline(PipelineId id);

    /*
     * Binds a list of objects by id to a pipeline by id. These object will be used as geometry in the ray trace stage
     * of the pipeline on execution.
     * pipelineId:      the pipeline id the geometry gets bound to
     * objectIDs:       the object ids of the new object instances
     * position:        the relative position of the object in space
     * orientation:     the relative orientation of the object in space
     * newScaleFactor:  the relative scale of the object in space
     * objectParameter: object specific information in addition to geometry
     * return:          true if success, false otherwise, objectIDs will be overwritten with object instance ids
     */
    bool bindGeometryToPipeline(PipelineId pipelineId, std::vector<ObjectId> *objectIDs, std::vector<Matrix4x4> *transforms,
                                std::vector<ObjectParameter> *objectParameters, std::vector<InstanceId> *instanceIDs);

    /*
     * Binds a shader with its resources to a pipeline.
     * pipelineId:      the pipeline id, the shader with its resources gets bound to
     * shaderId:        the shader id
     * shaderResourceIds:   the vector of shader resource ids that are associated with the shader
     * return:          true if success, false otherwise, shaderId will be overwritten with shader instance id
     */
    bool bindShaderToPipeline(PipelineId pipelineId, RayGeneratorShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, HitShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, OcclusionShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, PierceShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, MissShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);

    /*
     * Changes existing object instance in pipeline.
     * pipelineId:      the pipeline the object instance is associated with
     * objectInstanceId:    the object instances id
     * position:        the new position of the object
     * orientation:     the new orientation of the object
     * newScaleFactor:  the new scale of the object
     * objectParameter: the new object parameters
     * return:          true if success, false otherwise
     */
    bool updatePipelineObjects(PipelineId pipelineId, std::vector<InstanceId> *objectInstanceIDs,
                               std::vector<Matrix4x4 *> *transforms,
                               std::vector<ObjectParameter *> *objectParameters);

    /*
     * Changes existing shader instance in pipeline
     * pipelineId:      the pipeline the shader instance is associated with
     * shaderInstanceId:    the shaders instance id
     * shaderResourceIds:   the shaders new resources
     * return:          true if success, false otherwise
     */
    bool updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);

    /*
     * Removes a single object instance from the specified pipeline.
     * pipelineId:      the pipeline the object instance is associated with
     * objectInstanceId:    the objects instance id
     * return:          true if success, false otherwise
     */
    bool removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId);

    /*
     * Removes a single shader instance from the specified pipeline.
     * pipelineId:      the pipeline the object instance is associated with
     * shaderInstanceId:    the shaders instance id    objects.erase(id);
     * return:          true if success, false otherwise
     */
    bool removePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, HitShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, PierceShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, MissShaderId shaderId);

    /*
     * Adds an object to the object pool.
     * object:          the basic definition of the object
     * return:          the id of the object
     */
    ObjectId addObject(Object *object);

    /*
     * Removes an object from the pool by id.
     * return:          true if success, false otherwise
     */
    bool removeObject(ObjectId id);

    /*
     * Updates an objects mesh to a new mesh given by object.
     * return:          true if success, false otherwise
     */
    bool updateObject(ObjectId id, Object *object);

    /*
     * Adds a hit shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    HitShaderId addShader(HitShader *shader);

    /*
     * Adds a miss shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    MissShaderId addShader(MissShader *shader);

    /*
     * Adds an occlusion shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    OcclusionShaderId addShader(OcclusionShader *shader);

    /*
     * Adds a pierce shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    PierceShaderId addShader(PierceShader *shader);

    /*
     * Adds a ray generator shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    RayGeneratorShaderId addShader(RayGeneratorShader *shader);

    /*
     * Removes the shader from the pool.
     * id:              the id of the shader
     * return:          true if success, false otherwise
     */
    bool removeShader(RayGeneratorShaderId id);
    bool removeShader(HitShaderId id);
    bool removeShader(OcclusionShaderId id);
    bool removeShader(PierceShaderId id);
    bool removeShader(MissShaderId id);

    /*
     * Adds shader related data to the pool.
     * resource:        the data that is used by a shader
     * return:          the id of the resource
     */
    ShaderResourceId addShaderResource(ShaderResource *resource);

    /*
     * Removes the shader resource from the pool.
     * id:              the id of the resource
     * return:          true if success, false otherwise
     */
    bool removeShaderResource(ShaderResourceId id);

    int runPipeline(PipelineId id);

    int runAllPipelines();

    Object *getBaseDataFragment(ObjectId id);

    Instance *getInstanceDataFragment(InstanceId id);
};

#endif //RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H
