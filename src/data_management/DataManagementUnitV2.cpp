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
    if (objects.count(id))
        return objects[id].get();
    if (objectCache.count(id))
        return objectCache[id].get();
    // TODO request object from network and store it in cache
    return nullptr;
}

Instance *DataManagementUnitV2::getInstanceDataFragment(InstanceId id) {
    if (objectInstances.count(id))
        return objectInstances[id].get();
    if (objectInstanceCache.count(id))
        return objectInstanceCache[id].get();
    // TODO request object from network and store it in cache
    return nullptr;

}
