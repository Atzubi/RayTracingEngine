//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYENGINE_H
#define RAYTRACECORE_RAYENGINE_H

#include <cstdint>
#include <vector>
#include "Pipeline.h"
#include "Object.h"
#include "Shader.h"
#include "BasicStructures.h"

/**
 * Contains all data structures
 */
class DataManagementUnit;


/**
 * The interface of the render engine. It contains all functionality for rendering.
 */
class RayEngine {
private:
    /*
     * Contains all data used by the render engine
     */
    DataManagementUnit *dataManagementUnit;

public:
    RayEngine();

    ~RayEngine();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    int addPipeline(Pipeline *pipeline);

    /*
     * Removes a pipeline by id.
     * return:          true if success, false otherwise
     */
    bool removePipeline(int id);

    /*
     * Executes a pipeline by id.
     * return:          status id (including error codes)
     */
    int runPipeline(int id);

    /*
     * Executes all pipelines in the pool.
     * return:          status id (including error codes)
     */
    int runAll();

    /*
     * Binds a list of objects by id to a pipeline by id. These object will be used as geometry in the ray trace stage
     * of the pipeline on execution.
     * return:          true if success, false otherwise
     */
    bool bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds, std::vector<Vector3D> *position,
                                std::vector<Vector3D> *orientation, std::vector<double> *newScaleFactor,
                                std::vector<ObjectParameter> *objectParameter);

    /*
     * Binds a shader with its resources to a pipeline.
     * pipelineId:      the pipeline id, the shader with its resources gets bound to
     * shaderId:        the shader id
     * shaderResourceIds:   the vector of shader resource ids that are associated with the shader
     * return:          true if success, false otherwise
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
    bool updatePipelineObject(int pipelineId, int objectInstanceId, Vector3D position, Vector3D orientation,
                              double newScaleFactor, ObjectParameter objectParameter);

    /*
     * Changes existing shader instance in pipeline
     * pipelineId:      the pipeline the shader instance is associated with
     * shaderInstanceId:    the shaders instance id
     * shaderResourceIds:   the shaders new resources
     * return:          true if success, false otherwise
     */
    bool updatePipelineShader(int pipelineId, int shaderInstanceId, std::vector<int> *shaderResourceIds);

    /*
     * Adds a single object to the pipeline
     * pipelineId:      the pipeline the new object instance is added to
     * objectId:        the object id of the new object instance
     * position:        the new position of the object
     * orientation:     the new orientation of the object
     * newScaleFactor:  the new scale of the object
     * objectParameter: the new object parameters
     * return:          true if success, false otherwise, objectId will be overwritten with the object instance id
     */
    bool bindObjectToPipeline(int pipelineId, int *objectId, Vector3D position, Vector3D orientation,
                              double newScaleFactor, ObjectParameter objectParameter);

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
     * shaderInstanceId:    the shaders instance id
     * return:          true if success, false otherwise
     */
    bool removePipelineShader(int pipelineId, int shaderInstanceId);

    /*
     * Adds an object to the object pool.
     * object:          the basic definition of the object
     * position:        the relative position of the object in space
     * orientation:     the relative orientation of the object in space
     * return:          the id of the object
     */
    int addObject(Object *object, Vector3D position, Vector3D orientation, double newScaleFactor,
                  ObjectParameter objectParameter);

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
     * Adds a shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(ControlShader *shader);

    int addShader(HitShader *shader);

    int addShader(MissShader *shader);

    int addShader(OcclusionShader *shader);

    int addShader(PierceShader *shader);

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
};

#endif //RAYTRACECORE_RAYENGINE_H
