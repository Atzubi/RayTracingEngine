//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_OBJECT_H
#define RAYTRACECORE_OBJECT_H

#include <vector>
#include <cstdint>
#include "BasicStructures.h"

/**
 * Contains all the information required to construct a 3d model based on a 3d triangle mesh.
 * vertices:        a list of coordinates that are used as vertices
 * normals:         defines a normal per vertex
 * map:             defines mapping coordinates per vertex
 * ids:             a list of vertex ids that form a triangle
 */
class Object{
private:
    std::vector<double>* vertices;
    std::vector<double>* normals;
    std::vector<double>* map;
    std::vector<uint64_t>* ids;

public:
    Object(std::vector<double>* vertices, std::vector<double>* normals, std::vector<double>* map,
           std::vector<uint64_t>* ids);
    ~Object();

    /*
     * Moves an object to newPosition.
     * return:          true if success, false otherwise
     */
    bool moveObject(Vector3D newPosition);

    /*
     * Turns an object to newOrientation (euler angles).
     * return:          true if success, false otherwise
     */
    bool turnObject(Vector3D newOrientation);

    /*
     * Scales an object to newScaleFactor.
     * return:          true if success, false otherwise
     */
    bool scaleObject(double newScaleFactor);

    /*
     * Combines movement, orientation and scaling.
     * return:          true if success, false otherwise
     */
    bool manipulateObject(Vector3D newPosition, Vector3D newOrientation, double newScaleFactor);
};

#endif //RAYTRACECORE_OBJECT_H
