//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_OBJECT_H
#define RAYTRACECORE_OBJECT_H

#include <vector>
#include <cstdint>
#include "BasicStructures.h"

class Object {
public:
    /*
     * Default destructor
     */
    virtual ~Object() = default;

    /*
     * Returns the bounding box of the objects geometry.
     * return:          the bounding box of the object
     */
    virtual BoundingBox getBoundaries() = 0;

    /*
     * Computes the intersection with ray and this object.
     * return:          information about the intersection
     */
    virtual IntersectionInfo intersect(Ray ray) = 0;
};

#endif //RAYTRACECORE_OBJECT_H
