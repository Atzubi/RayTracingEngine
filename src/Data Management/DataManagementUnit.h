//
// Created by sebastian on 05.07.19.
//

#ifndef RAYTRACECORE_DATAMANAGEMENTUNIT_H
#define RAYTRACECORE_DATAMANAGEMENTUNIT_H

#include <set>
#include <unordered_map>

#include "RayTraceEngine/Pipeline.h"
#include "RayTraceEngine/Shader.h"
#include "Pipeline/PipelineImplement.h"


class DataManagementUnit {
private:
    std::unordered_map<int, Object *> objects;
    std::set<int> objectIds;
    std::unordered_map<int, int> objectInstances;
    std::set<int> objectInstanceIds;
    std::unordered_map<int, HitShader *> hitShaders;
    std::unordered_map<int, MissShader *> missShaders;
    std::unordered_map<int, OcclusionShader *> occlusionShaders;
    std::unordered_map<int, PierceShader *> pierceShaders;
    std::unordered_map<int, RayGeneratorShader *> rayGeneratorShaders;
    std::set<int> shaderIds;
    std::unordered_map<int, Any *> shadersResources;
    std::set<int> shaderResourceIds;
    std::unordered_map<int, PipelineImplement *> pipelines;
    std::set<int> pipelineIds;

public:
    DataManagementUnit();

    ~DataManagementUnit();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    int addPipeline(PipelineDescription *pipelineDescription);

    void
    updatePipelineCamera(int id, int resolutionX, int resolutionY, Vector3D cameraPosition, Vector3D cameraDirection,
                         Vector3D cameraUp);

    Texture getPipelineResult(int id);

    /*
     * Removes a pipeline by id.
     * return:          true if success, false otherwise
     */
    bool removePipeline(int id);

    /*
     * Binds a list of objects by id to a pipeline by id. These object will be used as geometry in the ray trace stage
     * of the pipeline on execution.
     * pipelineId:      the pipeline id the geometry gets bound to
     * objectIds:       the object ids of the new object instances
     * position:        the relative position of the object in space
     * orientation:     the relative orientation of the object in space
     * newScaleFactor:  the relative scale of the object in space
     * objectParameter: object specific information in addition to geometry
     * return:          true if success, false otherwise, objectIds will be overwritten with object instance ids
     */
    bool bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds, std::vector<Matrix4x4> *transforms,
                                std::vector<ObjectParameter> *objectParameters, std::vector<int> *instanceIDs);

    /*
     * Binds a shader with its resources to a pipeline.
     * pipelineId:      the pipeline id, the shader with its resources gets bound to
     * shaderId:        the shader id
     * shaderResourceIds:   the vector of shader resource ids that are associated with the shader
     * return:          true if success, false otherwise, shaderId will be overwritten with shader instance id
     */
    bool bindShaderToPipeline(int pipelineId, int *shaderId, std::vector<int> *shaderResourceIds);

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
    bool updatePipelineObjects(int pipelineId, std::vector<int> *objectInstanceIDs,
                               std::vector<Matrix4x4 *> *transforms,
                               std::vector<ObjectParameter *> *objectParameters);

    /*
     * Changes existing shader instance in pipeline
     * pipelineId:      the pipeline the shader instance is associated with
     * shaderInstanceId:    the shaders instance id
     * shaderResourceIds:   the shaders new resources
     * return:          true if success, false otherwise
     */
    bool updatePipelineShader(int pipelineId, int shaderInstanceId, std::vector<int> *shaderResourceIds);

    /*
     * Removes a single object instance from the specified pipeline.
     * pipelineId:      the pipeline the object instance is associated with
     * objectInstanceId:    the objects instance id
     * return:          true if success, false otherwise
     */
    bool removePipelineObject(int pipelineId, int objectInstanceId);

    /*
     * Removes a single shader instance from the specified pipeline.
     * pipelineId:      the pipeline the object instance is associated with
     * shaderInstanceId:    the shaders instance id    objects.erase(id);
     * return:          true if success, false otherwise
     */
    bool removePipelineShader(int pipelineId, int shaderInstanceId);

    /*
     * Adds an object to the object pool.
     * object:          the basic definition of the object
     * return:          the id of the object
     */
    int addObject(Object *object);

    /*
     * Removes an object from the pool by id.
     * return:          true if success, false otherwise
     */
    bool removeObject(int id);

    /*
     * Updates an objects mesh to a new mesh given by object.
     * return:          true if success, false otherwise
     */
    bool updateObject(int id, Object *object);

    /*
     * Adds a hit shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(HitShader *shader);

    /*
     * Adds a miss shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(MissShader *shader);

    /*
     * Adds an occlusion shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(OcclusionShader *shader);

    /*
     * Adds a pierce shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(PierceShader *shader);

    /*
     * Adds a ray generator shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(RayGeneratorShader *shader);

    /*
     * Removes the shader from the pool.
     * id:              the id of the shader
     * return:          true if success, false otherwise
     */
    bool removeShader(int id);

    /*
     * Adds shader related data to the pool.
     * resource:        the data that is used by a shader
     * return:          the id of the resource
     */
    int addShaderResource(Any *resource);

    /*
     * Removes the shader resource from the pool.
     * id:              the id of the resource
     * return:          true if success, false otherwise
     */
    bool removeShaderResource(int id);

    int runPipeline(int id);

    int runAllPipelines();
};

#endif //RAYTRACECORE_DATAMANAGEMENTUNIT_H
