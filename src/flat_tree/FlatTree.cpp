//
// Created by Sebastian on 19.02.2022.
//

#include "flat_tree/FlatTree.h"

uint64_t FlatTree::newNode() {
    DBVHNode node;
    flatTree.push_back(node);
    return flatTree.size() - 1;
}

const DBVHNode & FlatTree::at(uint64_t node) const {
    return flatTree.at(node);
}

DBVHNode &FlatTree::at(uint64_t node) {
    return flatTree.at(node);
}

void FlatTree::remove(uint64_t node) {
    flatTree[node] = flatTree.back();
    flatTree.pop_back();
}
