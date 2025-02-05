#pragma once
#include "../external/header_only/Array/Array.h"
#include "bvh/DBVHNode.h"
#include <unordered_map>
#include <vector>

class FlatTree
{
  public:
    uint64_t                                blockSize;
    std::vector<Array<DBVHNode>>            flatTree;
    std::unordered_map<DBVHNode*, uint64_t> positionMap;
    uint64_t                                position;

  public:
    explicit FlatTree(uint64_t blockSize);

    FlatTree(FlatTree&& other) noexcept;

    FlatTree& operator=(FlatTree&& other) noexcept;

    DBVHNode* newNode();

    void remove(DBVHNode& node);
};
