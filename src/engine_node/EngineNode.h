//
// Created by Sebastian on 28.10.2021.
//

#ifndef RAYTRACEENGINE_ENGINENODE_H
#define RAYTRACEENGINE_ENGINENODE_H

#include "pipeline/PipelinePool.h"
#include "data_management/DataManagementUnitV2.h"
#include "bvh/DBVHv2.h"
#include "utility/Id.h"
#include "RayTraceEngine/Pipeline.h"
#include <unordered_map>
#include <set>

class EngineNode {
private:
    std::unique_ptr<DataManagementUnitV2> dmu;
    std::unique_ptr<PipelinePool> pipelinePool;

    DeviceId deviceId;

    // stored only in main node
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

    // maps ids to devices holding the data
    std::unordered_map<ObjectId, DeviceId> objectIdDeviceMap;
    std::unordered_map<InstanceId, DeviceId> objectInstanceIdDeviceMap;
    std::unordered_map<DBVHNode *, DeviceId> pipelineTreeDeviceMap;

    DeviceId getDeviceId();

public:
    EngineNode();

    ~EngineNode();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    PipelineId createPipeline(PipelineDescription &pipelineDescription);

    void updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY, const Vector3D &cameraPosition,
                              const Vector3D &cameraDirection, const Vector3D &cameraUp);

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
    bool bindGeometryToPipeline(PipelineId pipelineId, const std::vector<ObjectId> &objectIDs,
                                const std::vector<Matrix4x4> &transforms,
                                const std::vector<ObjectParameter> &objectParameters,
                                std::vector<InstanceId> &instanceIDs);

    /*
     * Binds a shader with its resources to a pipeline.
     * pipelineId:      the pipeline id, the shader with its resources gets bound to
     * shaderId:        the shader id
     * shaderResourceIds:   the vector of shader resource ids that are associated with the shader
     * return:          true if success, false otherwise, shaderId will be overwritten with shader instance id
     */
    template<class ID>
    requires isShaderId<ID>
    bool bindShaderToPipeline(PipelineId pipelineId, ID shaderId,
                              const std::vector<ShaderResourceId> &shaderResourceIds);

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
    bool updatePipelineObjects(PipelineId pipelineId, const std::vector<InstanceId> &objectInstanceIDs,
                               const std::vector<Matrix4x4> &transforms,
                               const std::vector<ObjectParameter> &objectParameters);

    /*
     * Changes existing shader instance in pipeline
     * pipelineId:      the pipeline the shader instance is associated with
     * shaderInstanceId:    the shaders instance id
     * shaderResourceIds:   the shaders new resources
     * return:          true if success, false otherwise
     */
    bool updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId,
                              const std::vector<ShaderResourceId> &shaderResourceIds);

    bool updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId,
                              const std::vector<ShaderResourceId> &shaderResourceIds);

    bool updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId,
                              const std::vector<ShaderResourceId> &shaderResourceIds);

    bool updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId,
                              const std::vector<ShaderResourceId> &shaderResourceIds);

    bool updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId,
                              const std::vector<ShaderResourceId> &shaderResourceIds);

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
    ObjectId addObject(const Intersectable &object);

    /*
     * Removes an object from the pool by id.
     * return:          true if success, false otherwise
     */
    bool removeObject(ObjectId id);

    /*
     * Updates an objects mesh to a new mesh given by object.
     * return:          true if success, false otherwise
     */
    bool updateObject(ObjectId id, const Intersectable &object);

    /*
     * Adds a hit shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    HitShaderId addShader(const HitShader &shader);

    /*
     * Adds a miss shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    MissShaderId addShader(const MissShader &shader);

    /*
     * Adds an occlusion shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    OcclusionShaderId addShader(const OcclusionShader &shader);

    /*
     * Adds a pierce shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    PierceShaderId addShader(const PierceShader &shader);

    /*
     * Adds a ray generator shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    RayGeneratorShaderId addShader(const RayGeneratorShader &shader);

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
    ShaderResourceId addShaderResource(const ShaderResource &resource);

    /*
     * Removes the shader resource from the pool.
     * id:              the id of the resource
     * return:          true if success, false otherwise
     */
    bool removeShaderResource(ShaderResourceId id);

    int runPipeline(PipelineId id);

    int runAllPipelines();
};

#include "engine_node/EngineNode.tpp"

#endif //RAYTRACEENGINE_ENGINENODE_H
