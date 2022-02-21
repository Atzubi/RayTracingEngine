//
// Created by Sebastian on 19.02.2022.
//

#include "flat_tree/FlatTree.h"

FlatTree::FlatTree(uint64_t blockSize) : blockSize(blockSize), position(0) {

}

FlatTree::FlatTree(FlatTree &&other) noexcept: blockSize(other.blockSize), flatTree(std::move(other.flatTree)),
                                               positionMap(std::move(other.positionMap)), position(other.position) {

}

FlatTree &FlatTree::operator=(FlatTree &&other) noexcept {
    blockSize = other.blockSize;
    flatTree = std::move(other.flatTree);
    positionMap = std::move(other.positionMap);
    position = other.position;
    return *this;
}

DBVHNode *FlatTree::newNode() {
    if (position % blockSize == 0) {
        flatTree.emplace_back(Array<DBVHNode>(blockSize));
    }
    auto block = position / blockSize;
    auto pos = position % blockSize;
    auto &node = flatTree.at(block).at(pos);
    node = DBVHNode();
    positionMap[&node] = position++;
    return &node;
}

void FlatTree::remove(DBVHNode &node) {
    auto index = positionMap.at(&node);
    auto block = index / blockSize;
    auto pos = index % blockSize;
    flatTree.at(block).at(pos) = flatTree.back().at(position--);
}
