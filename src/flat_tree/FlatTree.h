//
// Created by Sebastian on 19.02.2022.
//

#ifndef RAYTRACEENGINE_FLATTREE_H
#define RAYTRACEENGINE_FLATTREE_H

#include <vector>
#include <unordered_map>
#include "bvh/DBVHNode.h"
#include "../external/header_only/Array/Array.h"

class FlatTree {
public:
    uint64_t blockSize;
    std::vector<Array<DBVHNode>> flatTree;
    std::unordered_map<DBVHNode *, uint64_t> positionMap;
    uint64_t position;

public:
    explicit FlatTree(uint64_t blockSize);

    FlatTree(FlatTree &&other) noexcept;

    FlatTree &operator=(FlatTree &&other) noexcept;

    DBVHNode *newNode();

    void remove(DBVHNode &node);
};

#endif //RAYTRACEENGINE_FLATTREE_H
