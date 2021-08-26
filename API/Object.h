//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_OBJECT_H
#define RAYTRACECORE_OBJECT_H

#include <vector>
#include <cstdint>
#include "Shader.h"

class Object {
public:
    /*
     * Default destructor
     */
    virtual ~Object() = default;

    /*
     * Clones the object
     * return:          pointer to a new copy
     */
    virtual Object *clone() = 0;

    /*
     * Returns the bounding box of the objects geometry.
     * return:          the bounding box of the object
     */
    virtual BoundingBox getBoundaries() = 0;

    /*
     * Computes the intersection with ray and this object.
     * return:          information about the intersection
     */
    virtual bool intersect(IntersectionInfo *intersectionInfo, Ray *ray) = 0;

    virtual double getSurfaceArea() = 0;

    virtual bool operator==(Object *object) = 0;
};

#endif //RAYTRACECORE_OBJECT_H
