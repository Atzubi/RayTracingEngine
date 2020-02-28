//
// Created by sebastian on 05.07.19.
//

#ifndef RAYTRACECORE_DATAMANAGEMENTUNIT_H
#define RAYTRACECORE_DATAMANAGEMENTUNIT_H

#include <set>
#include <unordered_map>

class DataManagementUnit{
private:
    struct ObjectMeta{
        Object* object;
        Vector3D position;
        Vector3D orientation;
        double newScaleFactor;
        ObjectParameter objectParameter;
    };

    std::unordered_map<int, ObjectMeta> objects;
    std::set<int> objectIds;
    std::unordered_map<int, Shader*> shaders;
    std::set<int> shaderIds;
    std::unordered_map<int, Pipeline> pipelines;
    std::set<int> pipelineIds;

public:
    DataManagementUnit();
    ~DataManagementUnit();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    int addPipeline(Pipeline* pipeline);

    /*
     * Removes a pipeline by id.
     * return:          true if success, false otherwise
     */
    bool removePipeline(int id);

    /*
     * Binds a list of objects by id to a pipeline by id. These object will be used as geometry in the ray trace stage
     * of the pipeline on execution.
     * return:          true if success, false otherwise
     */
    bool bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds);

    /*
     * Binds a shader with its resources to a pipeline.
     * pipelineId:      the pipeline id, the shader with its resources gets bound to
     * shaderId:        the shader id
     * shaderResourceIds:   the vector of shader resource ids that are associated with the shader
     * return:          true if success, false otherwise
     */
    bool bindShaderToPipeline(int pipelineId, int shaderId, std::vector<int> shaderResourceIds);

    /*
     * Adds an object to the object pool.
     * object:          the basic definition of the object
     * position:        the relative position of the object in space
     * orientation:     the relative orientation of the object in space
     * return:          the id of the object
     */
    int addObject(Object* object, Vector3D position, Vector3D orientation, double newScaleFactor,
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
    bool updateObject(int id, Object* object);

    /*
     * Adds a shader to the shader pool.
     * shader:          the added shader
     * return:          the id of the shader
     */
    int addShader(ControlShader* shader);
    int addShader(HitShader* shader);
    int addShader(MissShader* shader);
    int addShader(OcclusionShader* shader);
    int addShader(PierceShader* shader);
    int addShader(RayGeneratorShader* shader);

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
    int addShaderResource(Any resource);

    /*
     * Removes the shader resource from the pool.
     * id:              the id of the resource
     * return:          true if success, false otherwise
     */
    bool removeShaderResource(int id);
};

#endif //RAYTRACECORE_DATAMANAGEMENTUNIT_H
