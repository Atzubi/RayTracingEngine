//
// Created by Sebastian on 04.12.2021.
//

#include "DataManagementUnitV2.h"

DataManagementUnitV2::DataManagementUnitV2() = default;

DataManagementUnitV2::~DataManagementUnitV2() = default;

void DataManagementUnitV2::storeBaseDataFragments(std::unique_ptr<Intersectable> object, ObjectId id) {
    objects[id] = std::move(object);
}

void DataManagementUnitV2::storeInstanceDataFragments(std::unique_ptr<Instance> instance, InstanceId id) {
    objectInstances[id] = std::move(instance);
}

void DataManagementUnitV2::cacheBaseData(std::unique_ptr<Intersectable> object, ObjectId id) {
    // TODO: implement cache eviction
    objectCache[id] = std::move(object);
}

void DataManagementUnitV2::cacheInstanceData(std::unique_ptr<Instance> instance, InstanceId id) {
    // TODO: implement cache eviction
    objectInstanceCache[id] = std::move(instance);
}

bool DataManagementUnitV2::deleteBaseDataFragment(ObjectId id) {
    const bool objectRemoved = objects.erase(id) != 0;
    return objectRemoved;
}

bool DataManagementUnitV2::deleteInstanceDataFragment(InstanceId id) {
    const bool objectRemoved = objectInstances.erase(id) != 0;
    return objectRemoved;
}

Intersectable *DataManagementUnitV2::getBaseDataFragment(ObjectId id) {
    if (objects.count(id) == 0) {
        // object was not originally stored on this node, check cache
        if (objectCache.count(id) == 0) {
            // object is not currently in the cache
            return nullptr;
        } else {
            // object was found in cache
            return objectCache[id].get();
        }
    } else {
        // object was found in node
        return objects[id].get();
    }
}

Instance *DataManagementUnitV2::getInstanceDataFragment(InstanceId id) {
    if (objectInstances.count(id) == 0) {
        // object was not originally stored on this node, check cache
        if (objectInstanceCache.count(id) == 0) {
            // object is not currently in the cache
            return nullptr;
        } else {
            // object was found in cache
            return objectInstanceCache[id].get();
        }
    } else {
        // object was found in node
        return objectInstances[id].get();
    }
}
