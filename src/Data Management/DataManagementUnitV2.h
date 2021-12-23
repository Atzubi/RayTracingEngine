//
// Created by Sebastian on 04.12.2021.
//

#ifndef RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H
#define RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H

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

class Any;

struct DBVHNode;
struct PipelineDescription;
struct Vector3D;
struct Texture;
struct ObjectParameter;
struct Matrix4x4;

class DataManagementUnitV2 {
private:
    int deviceId;

    EngineNode *engineNode;

    // stored only in main DMU
    std::set<int> objectIds;
    std::set<int> objectInstanceIds;
    std::set<int> shaderIds;
    std::set<int> shaderResourceIds;
    std::set<int> pipelineIds;

    // copied to every node
    std::unordered_map<int, HitShader *> hitShaders;
    std::unordered_map<int, MissShader *> missShaders;
    std::unordered_map<int, OcclusionShader *> occlusionShaders;
    std::unordered_map<int, PierceShader *> pierceShaders;
    std::unordered_map<int, RayGeneratorShader *> rayGeneratorShaders;

    //std::unordered_map<int, PipelineImplement *> pipelines; // groups  pipeline information, copied to every node

    // maps ids to devices holding the data
    std::unordered_map<int, int> objectIdDeviceMap;
    std::unordered_map<int, int> objectInstanceIdDeviceMap;
    std::unordered_map<DBVHNode *, int> pipelineTreeDeviceMap;

    int getDeviceId();

public:
    DataManagementUnitV2();

    ~DataManagementUnitV2();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    int addPipeline(PipelineDescription *pipelineDescription);

    void
    updatePipelineCamera(int id, int resolutionX, int resolutionY, Vector3D cameraPosition, Vector3D cameraDirection,
                         Vector3D cameraUp);

    Texture *getPipelineResult(int id);

    /*
     * Removes a pipeline by id.
     * return:          true if success, false otherwise
     */
    bool removePipeline(int id);

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
    bool bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIDs, std::vector<Matrix4x4> *transforms,
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

    Object *getBaseDataFragment(int id);

    Instance *getInstanceDataFragment(int id);
};

#endif //RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H
