//
// Created by sebastian on 02.07.19.
//

#ifndef RAYTRACECORE_RAYENGINE_H
#define RAYTRACECORE_RAYENGINE_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Pipeline.h"
#include "Object.h"

/**
 * Contains all data structures
 */
class DataManagementUnit;


/**
 * The main part of the render engine. It contains all functionality for rendering.
 */
class RayEngine{
private:
    /*
     * Contains all data used by the render engine
     */
    DataManagementUnit* dataManagementUnit;

public:
    /*
     * Contains additional parameters of an object that are used when constructing the data structure for rendering.
     * bounding:        a parameter used for describing the looseness of an objects bounding, higher values create
     *                  bigger boxes that cripple general rendering performance but speed up reconstructing the data
     *                  structure on an object update (animations)
     */
    struct ObjectParameter{
        double bounding;
    };

    RayEngine();
    ~RayEngine();

    /*
     * Adds a pipeline to the pipeline pool.
     * return:          the id of the added pipeline
     */
    int addPipeline(Pipeline const &pipeline);
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
    bool bindGeometryToPipeline(int pipelineId, std::vector<int>* objectIds);

    /*
     * Adds an object to the object pool.
     * object:          the basic definition of the object
     * position:        the relative position of the object in space
     * orientation:     the relative orientation of the object in space
     * return:          the id of the object
     */
    int addObject(Object const &object, Vector3D position, Vector3D orientation, double newScaleFactor,
            ObjectParameter objectParameter);

    /*
     * Removes an object from the pool by id.
     * return           true if success, false otherwise
     */
    bool removeObject(int id);

    /*
     * Updates an objects mesh to a new mesh given by object.
     * return:          true if success, false otherwise
     */
    bool updateObject(int id, Object const &object);
};

#endif //RAYTRACECORE_RAYENGINE_H
