//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_BASICSTRUCTURES_H
#define RAYTRACECORE_BASICSTRUCTURES_H

/**
 * Contains x, y and z coordinates representing a vector in 3 dimensions
 */
struct Vector3D{
    double x;
    double y;
    double z;
};

struct BoundingBox{
    Vector3D firstCorner, secondCorner;
};

struct IntersectionInfo{

};

struct Ray{
    Vector3D origin, direction;
    void* metaData;
};

#endif //RAYTRACECORE_BASICSTRUCTURES_H
