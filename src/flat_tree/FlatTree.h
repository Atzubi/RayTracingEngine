//
// Created by Sebastian on 19.02.2022.
//

#ifndef RAYTRACEENGINE_FLATTREE_H
#define RAYTRACEENGINE_FLATTREE_H

#include <vector>
#include "bvh/DBVHNode.h"

class FlatTree {
private:
    std::vector<DBVHNode> flatTree;

public:
    uint64_t newNode();

    [[nodiscard]] const DBVHNode &at(uint64_t node) const;

    DBVHNode &at(uint64_t node);

    void remove(uint64_t node);
};

#endif //RAYTRACEENGINE_FLATTREE_H
