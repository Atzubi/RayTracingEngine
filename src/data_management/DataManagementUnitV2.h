//
// Created by Sebastian on 04.12.2021.
//

#ifndef RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H
#define RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H


#include <unordered_map>
#include <memory>
#include "utility/Id.h"
#include "RayTraceEngine/Intersectable.h"
#include "intersectable/Instance.h"

class Instance;

class DataManagementUnitV2 {
private:
    std::unordered_map<ObjectId, std::unique_ptr<Intersectable>> objects;
    std::unordered_map<InstanceId, std::unique_ptr<Instance>> objectInstances;

    std::unordered_map<ObjectId, std::unique_ptr<Intersectable>> objectCache;
    std::unordered_map<InstanceId, std::unique_ptr<Instance>> objectInstanceCache;

public:
    DataManagementUnitV2();

    ~DataManagementUnitV2();

    void storeBaseDataFragments(std::unique_ptr<Intersectable> object, ObjectId id);

    bool deleteBaseDataFragment(ObjectId id);

    Intersectable *getBaseDataFragment(ObjectId id);

    void storeInstanceDataFragments(std::unique_ptr<Instance> instance, InstanceId id);

    bool deleteInstanceDataFragment(InstanceId id);

    Instance *getInstanceDataFragment(InstanceId id);

    void cacheBaseData(std::unique_ptr<Intersectable> object, ObjectId id);

    void cacheInstanceData(std::unique_ptr<Instance> instance, InstanceId id);
};

#endif //RAYTRACEENGINE_DATAMANAGEMENTUNITV2_H
