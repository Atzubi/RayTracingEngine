//
// Created by sebastian on 08.04.19.
//

#ifndef RAYENGINE_HBVH_H
#define RAYENGINE_HBVH_H

struct BBox {
    uint8_t x1, x2, y1, y2, z1, z2;
};

struct TreeNode {
    BBox bBox;
    uint16_t flags;         //1:child = leaf, 2:? 4:? ...
};

union HBVH {
    TreeNode node;
    uint64_t primitivePointer;
};

class HeapBVH {
private:
    HBVH * tree;

    void
    findBoundaries(Primitive *primitives, uint64_t *ids, uint64_t min, uint64_t max, double *minX, double *minY,
                   double *minZ, double *maxX, double *maxY, double *maxZ);

    uint64_t partition(uint64_t *ids, Primitive a[], uint64_t left, uint64_t right, uint64_t pivotIndex, uint8_t dim);

    void
    split(uint64_t min, uint64_t max, uint64_t midpoint, uint64_t *ids, Primitive *primitives, BBox *bBox, Vector3 *bMin,
          Vector3 *bMax);

    void
    recursiveBuild(uint64_t min, uint64_t max, Primitive *primitives, uint8_t depth, uint64_t *ids, HBVH *tree,
                   uint64_t pos,
                   Vector3 bMin, Vector3 bMax);


public:
    void build(Primitive *primitives, uint64_t size);

    HBVH* getHBVH();
};


#endif //RAYENGINE_HBVH_H
