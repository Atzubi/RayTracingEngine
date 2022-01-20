//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYENGINE_H
#define RAYTRACECORE_RAYENGINE_H

#include <cstdint>
#include "Object.h"
#include "Shader.h"
#include "Pipeline.h"
#include "BasicStructures.h"

/**
 * Manages data movement within the engine.
 */
class DataManagementUnitV2;


/**
 * The interface of the ray tracing engine.
 * dataManagementUnit:  Manages data used by the engine.
 */
class RayEngine {
private:
    DataManagementUnitV2 *dataManagementUnit;

public:
    /**
     * Constructor. Creates a new ray tracing engine.
     */
    RayEngine();

    /**
     * Destructor
     */
    ~RayEngine();

    /**
     * Adds a pipeline to the pool of pipelines managed by the engine.
     * @param pipelineDescription   Describes the pipeline to be created.
     * @return                      An id for referencing the pipeline after creation.
     */
    PipelineId createPipeline(PipelineDescription *pipelineDescription);

    /**
     * Updates a pipelines virtual camera description.
     * @param id                The id of the pipeline to be updated.
     * @param resolutionX       Horizontal resolution of the virtual camera.
     * @param resolutionY       Vertical resolution of the virtual camera.
     * @param cameraPosition    Position of the virtual camera.
     * @param cameraDirection   Direction of the virtual camera.
     * @param cameraUp          Direction of the virtual camera that is facing upwards.
     */
    void
    updatePipelineCamera(PipelineId id, int resolutionX, int resolutionY, Vector3D cameraPosition, Vector3D cameraDirection,
                         Vector3D cameraUp);

    /**
     * Waits on pipeline execution to finish, then returns with the result.
     * @param id    Id of the pipeline.
     * @return      Returns an image containing the render result.
     */
    Texture *getPipelineResult(PipelineId id);

    /**
     * Deletes a pipeline from the engines pipeline pool.
     * @param id    Id of the pipeline to be removed.
     * @return      True if the pipeline could successfully be removed, false otherwise.
     */
    bool deletePipeline(PipelineId id);

    /**
     * Executes a pipeline.
     * @param id    Id of the pipeline to be executed.
     * @return      Status identifier including error codes.
     */
    int runPipeline(PipelineId id);

    /**
     * Executes all pipelines in the pool.
     * @return      Status identifier including error codes.
     */
    int runAll();

    /**
     * Binds a list of objects by id to a pipeline by id. These object will be instanced and used as geometry in the ray
     * trace stage of the pipeline on execution.
     * @param pipelineId        Id of the pipeline the objects will be bound to.
     * @param objectIDs         Ids of objects that are added.
     * @param transforms        Transforms for the objects.
     * @param objectParameters  Other object parameters.
     * @param instanceIDs       Contains the resulting object instance ids.
     * @return                  True if the objects could successfully be bound to the pipeline, false otherwise.
     */
    bool bindGeometryToPipeline(PipelineId pipelineId, std::vector<ObjectId> *objectIDs, std::vector<Matrix4x4> *transforms,
                                std::vector<ObjectParameter> *objectParameters, std::vector<InstanceId> *instanceIDs);

    /**
     * Binds a shader with its resources to a pipeline.
     * @param pipelineId        Id of the pipeline the shader will be bound to.
     * @param shaderId          Id of the shader, will be changed to the shaders instance id.
     * @param shaderResourceIds Vector of resource ids.
     * @return                  True if the shader could successfully be bound to the pipeline, false otherwise.
     */
    bool bindShaderToPipeline(PipelineId pipelineId, RayGeneratorShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, HitShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, OcclusionShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, PierceShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool bindShaderToPipeline(PipelineId pipelineId, MissShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);

    /**
     * Updates object instances within a pipeline.
     * @param pipelineId        Id of the pipeline.
     * @param objectInstanceIDs Object instance ids of the objects that will be updated.
     * @param transforms        New transforms for the object instances.
     * @param objectParameters  New additional parameters for the object instances.
     * @return                  True if the object instances could successfully be updated, false otherwise.
     */
    bool updatePipelineObjects(PipelineId pipelineId, std::vector<InstanceId> *objectInstanceIDs,
                               std::vector<Matrix4x4 *> *transforms,
                               std::vector<ObjectParameter *> *objectParameters);

    /**
     * Updates Shaders within a pipeline.
     * @param pipelineId        Id of the pipeline.
     * @param shaderId  Shader instance id of the shader that will be updated.
     * @param shaderResourceIds New resource ids.
     * @return                  True if the shader could successfully be updated, false otherwise.
     */
    bool updatePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, HitShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, PierceShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);
    bool updatePipelineShader(PipelineId pipelineId, MissShaderId shaderId, std::vector<ShaderResourceId> *shaderResourceIds);

    /**
     * Removes an object instance from a pipeline.
     * @param pipelineId        Id of the pipeline.
     * @param objectInstanceId  Id of the object instance.
     * @return                  True if the object instance could be removed, false otherwise.
     */
    bool removePipelineObject(PipelineId pipelineId, InstanceId objectInstanceId);

    /**
     * Removes a shader from a pipeline.
     * @param pipelineId        Id of the pipeline.
     * @param shaderId  Id of the shader instance.
     * @return                  True if the shader instance could be removed, false otherwise.
     */
    bool removePipelineShader(PipelineId pipelineId, RayGeneratorShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, HitShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, OcclusionShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, PierceShaderId shaderId);
    bool removePipelineShader(PipelineId pipelineId, MissShaderId shaderId);

    /**
     * Adds an object to the engines object pool.
     * @param object    Pointer to the object that is being added.
     * @return          Returns the object id.
     */
    ObjectId addObject(Object *object);

    /**
     * Removes and object from the engines object pool.
     * @param id    Id of the object.
     * @return      True if the object could be removed, false otherwise.
     */
    bool removeObject(ObjectId id);

    /**
     * Updates an existing object with a new one.
     * @param id        Id of the old object.
     * @param object    New object.
     * @return          True if the object could be updated, false otherwise.
     */
    bool updateObject(ObjectId id, Object *object);

    /**
     * Adds a shader to the engines shader pool.
     * @param shader    Shader that will be added.
     * @return          Id of the shader for referencing it within the engine.
     */
    HitShaderId addShader(HitShader *shader);

    /**
     * Adds a shader to the engines shader pool.
     * @param shader    Shader that will be added.
     * @return          Id of the shader for referencing it within the engine.
     */
    MissShaderId addShader(MissShader *shader);

    /**
     * Adds a shader to the engines shader pool.
     * @param shader    Shader that will be added.
     * @return          Id of the shader for referencing it within the engine.
     */
    OcclusionShaderId addShader(OcclusionShader *shader);

    /**
     * Adds a shader to the engines shader pool.
     * @param shader    Shader that will be added.
     * @return          Id of the shader for referencing it within the engine.
     */
    PierceShaderId addShader(PierceShader *shader);

    /**
     * Adds a shader to the engines shader pool.
     * @param shader    Shader that will be added.
     * @return          Id of the shader for referencing it within the engine.
     */
    RayGeneratorShaderId addShader(RayGeneratorShader *shader);

    /**
     * Removes a shader from the engines shader pool.
     * @param id    Id of the shader.
     * @return      True if the shader could be removed, false otherwise.
     */
    bool removeShader(RayGeneratorShaderId id);
    bool removeShader(HitShaderId id);
    bool removeShader(OcclusionShaderId id);
    bool removeShader(PierceShaderId id);
    bool removeShader(MissShaderId id);

    /**
     * Adds shader resources to the engines shader resource pool.
     * @param resource  Shader resource.
     * @return          Id of the shader resource for referencing it within the engine.
     */
    ShaderResourceId addShaderResource(ShaderResource *resource);

    /**
     * Removes a shader resource from the engines pool.
     * @param id    Id of the shader resource.
     * @return      True if the shader resource could be removed, false otherwise.
     */
    bool removeShaderResource(ShaderResourceId id);
};

#endif //RAYTRACECORE_RAYENGINE_H
