//
// Created by sebastian on 08.04.19.
//

#include <thread>
#include <limits>
#include "HBVH.h"

void HeapBVH::findBoundaries(Primitive *primitives, uint64_t *ids, uint64_t min, uint64_t max, double_t *minX,
                             double_t *minY, double_t *minZ, double_t *maxX, double_t *maxY, double_t *maxZ) {
    for (uint64_t i = min; i <= max; i++) {
        if (primitives[ids[i]].v1.x < *minX) {
            *minX = primitives[ids[i]].v1.x;
        }
        if (primitives[ids[i]].v2.x < *minX) {
            *minX = primitives[ids[i]].v2.x;
        }
        if (primitives[ids[i]].v3.x < *minX) {
            *minX = primitives[ids[i]].v3.x;
        }
        if (primitives[ids[i]].v1.y < *minY) {
            *minY = primitives[ids[i]].v1.y;
        }
        if (primitives[ids[i]].v2.y < *minY) {
            *minY = primitives[ids[i]].v2.y;
        }
        if (primitives[ids[i]].v3.y < *minY) {
            *minY = primitives[ids[i]].v3.y;
        }
        if (primitives[ids[i]].v1.z < *minZ) {
            *minZ = primitives[ids[i]].v1.z;
        }
        if (primitives[ids[i]].v2.z < *minZ) {
            *minZ = primitives[ids[i]].v2.z;
        }
        if (primitives[ids[i]].v3.z < *minZ) {
            *minZ = primitives[ids[i]].v3.z;
        }

        if (primitives[ids[i]].v1.x > *maxX) {
            *maxX = primitives[ids[i]].v1.x;
        }
        if (primitives[ids[i]].v2.x > *maxX) {
            *maxX = primitives[ids[i]].v2.x;
        }
        if (primitives[ids[i]].v3.x > *maxX) {
            *maxX = primitives[ids[i]].v3.x;
        }
        if (primitives[ids[i]].v1.y > *maxY) {
            *maxY = primitives[ids[i]].v1.y;
        }
        if (primitives[ids[i]].v2.y > *maxY) {
            *maxY = primitives[ids[i]].v2.y;
        }
        if (primitives[ids[i]].v3.y > *maxY) {
            *maxY = primitives[ids[i]].v3.y;
        }
        if (primitives[ids[i]].v1.z > *maxZ) {
            *maxZ = primitives[ids[i]].v1.z;
        }
        if (primitives[ids[i]].v2.z > *maxZ) {
            *maxZ = primitives[ids[i]].v2.z;
        }
        if (primitives[ids[i]].v3.z > *maxZ) {
            *maxZ = primitives[ids[i]].v3.z;
        }
    }
}

uint64_t HeapBVH::partition(uint64_t *ids, Primitive *a, uint64_t left, uint64_t right, uint64_t pivotIndex,
                            uint8_t dim) {
    // Pick pivotIndex as pivot from the array
    switch (dim) {
        case 0: {
            double_t pivot =
                    (a[ids[pivotIndex]].v1.x + a[ids[pivotIndex]].v2.x + a[ids[pivotIndex]].v3.x) / 3;

            // Move pivot to end
            swap(&ids[pivotIndex], &ids[right]);

            // Elements less than pivot will be pushed to the left of pIndex
            // Elements more than pivot will be pushed to the right of pIndex
            // Equal elements can go either way
            uint64_t pIndex = left;
            uint64_t i;

            // Each time we find an element less than or equal to pivot, pIndex
            // Is incremented and that element would be placed before the pivot.
            for (i = left; i < right; i++) {
                if ((a[ids[i]].v1.x + a[ids[i]].v2.x + a[ids[i]].v3.x) / 3 <= pivot) {
                    swap(&ids[i], &ids[pIndex]);
                    pIndex++;
                }
            }

            // Move pivot to its final place
            swap(&ids[pIndex], &ids[right]
            );

            // Return pIndex (index of pivot element)
            return
                    pIndex;
        }
        case 1: {
            double_t pivot =
                    (a[ids[pivotIndex]].v1.y + a[ids[pivotIndex]].v2.y + a[ids[pivotIndex]].v3.y) / 3;

            // Move pivot to end
            swap(&ids[pivotIndex], &ids[right]);

            // Elements less than pivot will be pushed to the left of pIndex
            // Elements more than pivot will be pushed to the right of pIndex
            // Equal elements can go either way
            uint64_t pIndex = left;
            uint64_t i;

            // Each time we find an element less than or equal to pivot, pIndex
            // Is incremented and that element would be placed before the pivot.
            for (i = left; i < right; i++) {
                if ((a[ids[i]].v1.y + a[ids[i]].v2.y + a[ids[i]].v3.y) / 3 <= pivot) {
                    swap(&ids[i], &ids[pIndex]);
                    pIndex++;
                }
            }

            // Move pivot to its final place
            swap(&ids[pIndex], &ids[right]
            );

            // Return pIndex (index of pivot element)
            return
                    pIndex;
        }
        case 2: {
            double_t pivot =
                    (a[ids[pivotIndex]].v1.z + a[ids[pivotIndex]].v2.z + a[ids[pivotIndex]].v3.z) / 3;

            // Move pivot to end
            swap(&ids[pivotIndex], &ids[right]);

            // Elements less than pivot will be pushed to the left of pIndex
            // Elements more than pivot will be pushed to the right of pIndex
            // Equal elements can go either way
            uint64_t pIndex = left;
            uint64_t i;

            // Each time we find an element less than or equal to pivot, pIndex
            // Is incremented and that element would be placed before the pivot.
            for (i = left; i < right; i++) {
                if ((a[ids[i]].v1.z + a[ids[i]].v2.z + a[ids[i]].v3.z) / 3 <= pivot) {
                    swap(&ids[i], &ids[pIndex]);
                    pIndex++;
                }
            }

            // Move pivot to its final place
            swap(&ids[pIndex], &ids[right]
            );

            // Return pIndex (index of pivot element)
            return
                    pIndex;
        }
        default:
            return -1;
    }
}

void HeapBVH::split(uint64_t min, uint64_t max, uint64_t midpoint, uint64_t *ids, Primitive *primitives,
                    BBox *bBox, Vector3 *bMin, Vector3 *bMax) {
    uint64_t left = min;
    uint64_t right = max;

    //------------------determine best splitting plane-----------------------------------------------------
    double_t deltaX = 0;
    double_t deltaY = 0;
    double_t deltaZ = 0;

    double_t minX = std::numeric_limits<double_t>::max();
    double_t minY = std::numeric_limits<double_t>::max();
    double_t minZ = std::numeric_limits<double_t>::max();
    double_t maxX = -std::numeric_limits<double_t>::max();
    double_t maxY = -std::numeric_limits<double_t>::max();
    double_t maxZ = -std::numeric_limits<double_t>::max();

    // Finds the corner points for a bounding box corresponding to a given dataset
    findBoundaries(primitives, ids, min, max, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);

    double_t boxX = (bMax->x - bMin->x) / (256);
    double_t boxY = (bMax->y - bMin->y) / (256);
    double_t boxZ = (bMax->z - bMin->z) / (256);

    bBox->x1 = (uint8_t) (-(bMin->x - minX) / boxX);
    bBox->x2 = (uint8_t) ((bMax->x - maxX) / boxX);
    bBox->y1 = (uint8_t) (-(bMin->y - minY) / boxY);
    bBox->y2 = (uint8_t) ((bMax->y - maxY) / boxY);
    bBox->z1 = (uint8_t) (-(bMin->z - minZ) / boxZ);
    bBox->z2 = (uint8_t) ((bMax->z - maxZ) / boxZ);

    Vector3 minBuffer = *bMin, maxBuffer = *bMax;

    bMin->x = minBuffer.x + bBox->x1 * ((maxBuffer.x - minBuffer.x) / (256));
    bMin->y = minBuffer.y + bBox->y1 * ((maxBuffer.y - minBuffer.y) / (256));
    bMin->z = minBuffer.z + bBox->z1 * ((maxBuffer.z - minBuffer.z) / (256));
    bMax->x = maxBuffer.x - bBox->x2 * ((maxBuffer.x - minBuffer.x) / (256));
    bMax->y = maxBuffer.y - bBox->y2 * ((maxBuffer.y - minBuffer.y) / (256));
    bMax->z = maxBuffer.z - bBox->z2 * ((maxBuffer.z - minBuffer.z) / (256));

    deltaX = maxX - minX;
    deltaY = maxY - minY;
    deltaZ = maxZ - minZ;
    uint8_t dim = 0;

    if (deltaX >= deltaY && deltaX >= deltaZ) {
        dim = 0;
    } else if (deltaY >= deltaZ) {
        dim = 1;
    } else {
        dim = 2;
    }

    //----------------------------quickselect--------------------------------------------------------------

    while (true) {
        // If the array contains only one element, return that element
        if (left == right)
            break;

        // Select a pivotIndex between left and right
        uint64_t pivotIndex = (left + right) / 2;

        pivotIndex = partition(ids, primitives, left, right, pivotIndex, dim);

        // The pivot is in its final sorted position
        if (min + midpoint == pivotIndex)
            break;

            // If k is less than the pivot index
        else if (min + midpoint < pivotIndex)
            right = pivotIndex - 1;

            // If k is more than the pivot index
        else
            left = pivotIndex + 1;
    }
}

void HeapBVH::recursiveBuild(uint64_t min, uint64_t max, Primitive *primitives, uint8_t depth, uint64_t *ids,
                             HBVH *tree, uint64_t pos, Vector3 bMin, Vector3 bMax) {
    uint64_t delta = max - min + 1;
    if (delta <= 1) {
        // Write leaf data
        tree[pos + 6].primitivePointer = ids[min];
        return;
    }

    // Determine midpoint for this layer
    uint64_t normalized = (uint64_t) 2 << (log2_64(delta) - 1);
    uint64_t midpoint = delta - normalized / 2 < normalized ? delta - normalized / 2 : normalized;

    BBox bBox;

    // Partitions the data in a way that a good space partitioning is gained
    split(min, max, midpoint, ids, primitives, &bBox, &bMin, &bMax);

    // Create bounding volumes
    uint16_t flags = delta == 2 ? (uint16_t) 1 : (uint16_t) 0;
    TreeNode node = {bBox, flags};
    tree[pos + 6].node = node;

    // Spawns multiple threads to work on the sub problems up to a certain amount
    if (depth < 4) {
        std::thread left, right;
        left = std::thread(&HeapBVH::recursiveBuild, this, min, min + midpoint - 1, primitives, depth + 1, ids, tree,
                           pos * 2 + 1, bMin, bMax);
        right = std::thread(&HeapBVH::recursiveBuild, this, min + midpoint, max, primitives, depth + 1, ids, tree,
                            pos * 2 + 2, bMin, bMax);
        left.join();
        right.join();
    } else {
        recursiveBuild(min, min + midpoint - 1, primitives, depth, ids, tree, pos * 2 + 1, bMin, bMax);
        recursiveBuild(min + midpoint, max, primitives, depth, ids, tree, pos * 2 + 2, bMin, bMax);
    }
}

void HeapBVH::build(Primitive *primitives, uint64_t size) {
    tree = new HBVH[size * 2 - 1 + 6];
    uint64_t *ids = new uint64_t[size];
    Vector3 bMin, bMax;

    //------------------setup----------------------------------------------------------------------

    for (uint64_t i = 0; i < size; i++) {
        ids[i] = i;
    }

    double_t minX = std::numeric_limits<double_t>::max();
    double_t minY = std::numeric_limits<double_t>::max();
    double_t minZ = std::numeric_limits<double_t>::max();
    double_t maxX = std::numeric_limits<double_t>::min();
    double_t maxY = std::numeric_limits<double_t>::min();
    double_t maxZ = std::numeric_limits<double_t>::min();

    // Finds the corner points for a bounding box corresponding to a given dataset
    findBoundaries(primitives, ids, 0, size - 1, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);

    tree[0].primitivePointer = *((uint64_t *) &minX);
    tree[1].primitivePointer = *((uint64_t *) &minY);
    tree[2].primitivePointer = *((uint64_t *) &minZ);
    tree[3].primitivePointer = *((uint64_t *) &maxX);
    tree[4].primitivePointer = *((uint64_t *) &maxY);
    tree[5].primitivePointer = *((uint64_t *) &maxZ);

    bMin.x = minX;
    bMin.y = minY;
    bMin.z = minZ;
    bMax.x = maxX;
    bMax.y = maxY;
    bMax.z = maxZ;

    //-------------building the tree--------------------------------------------------------------

    recursiveBuild(0, size - 1, primitives, 0, ids, tree, 0, bMin, bMax);

    //-------------done building the tree---------------------------------------------------------

    delete (ids);
}

HBVH *HeapBVH::getHBVH() {
    return tree;
}
