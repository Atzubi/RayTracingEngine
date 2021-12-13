//
// Created by Sebastian on 13.12.2021.
//

#include "DBVHv2.h"

void DBVHv2::addObjects(DBVHNode *root, std::vector<Object *> *objects) {

}

void DBVHv2::removeObjects(DBVHNode *root, std::vector<Object *> *objects) {

}

bool DBVHv2::intersectFirst(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray) {
    return false;
}

bool DBVHv2::intersectAny(DBVHNode *root, IntersectionInfo *intersectionInfo, Ray *ray) {
    return false;
}

bool DBVHv2::intersectAll(DBVHNode *root, std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) {
    return false;
}
