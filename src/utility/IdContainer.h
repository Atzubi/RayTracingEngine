//
// Created by Sebastian on 14.02.2022.
//

#ifndef RAYTRACEENGINE_IDCONTAINER_H
#define RAYTRACEENGINE_IDCONTAINER_H

#include <set>
#include "Id.h"

template<class ID> requires isShaderId<ID> || isResourceId<ID>
class IdContainer {
private:
    std::set<ID> ids;

public:
    IdContainer() {
        ids.insert({0});
    }

    ~IdContainer() = default;

    ID next() {
        auto id = ids.extract(ids.begin()).value();

        if (ids.empty()) {
            ids.insert({id.id + 1});
        }

        return id;
    }

    void remove(ID id) {
        ids.insert(id);

        auto iterator = ids.rbegin();
        unsigned long end = iterator->id - 1;

        unsigned long buffer = iterator->id;
        while (end-- == (++iterator)->id) {
            ids.erase({buffer});
            buffer = iterator->id;
        }
    }
};

#endif //RAYTRACEENGINE_IDCONTAINER_H
