//
// Created by sebastian on 16.06.21.
//

#ifndef DBVH_DBVHV2_H
#define DBVH_DBVHV2_H

#include <list>
#include <utility>
#include <deque>

#include "RayTraceEngine/Intersectable.h"
#include "HashMap/robin_map.h"

class DBVH : public Intersectable {
private:
    class Node {
    public:
        virtual uint8_t getType() = 0;

        virtual BoundingBox getBoundingBox() = 0;

        virtual ~Node() = default;
    };

    class InnerNode : public Node {
    public:
        Node *leftChild, *rightChild;
        BoundingBox boundingBox{};
        double surfaceArea;

        InnerNode();

        uint8_t getType() override;

        BoundingBox getBoundingBox() override;

        bool optimizeSAH();

        void refit();

        ~InnerNode() override;
    };

    class Child : public Node {
    public:
        Object *object{};

        uint8_t getType() override;

        BoundingBox getBoundingBox() override;

        ~Child() override = default;
    };

    struct TraversalContainer {
        Node *node;
        double distance;
    };

    void add(Node *currentNode, std::vector<Object *> *objects, uint8_t depth);

    void remove(Node *currentNode, Object *object);

    void remove(Node **currentNode, std::vector<Object *> *objects);

    bool traverseALl(std::vector<IntersectionInfo*> *intersectionInfo, Ray*ray);

    bool traverseFirst(IntersectionInfo *intersectionInfo, Ray *ray);

    bool traverseAny(IntersectionInfo* intersectionInfo, Ray* ray);

    Node *root;
    uint8_t maxDepth;
    bool instance;

public:

    DBVH();

    DBVH(DBVH const &clone);

    ~DBVH() override;

    void addObjects(std::vector<Object *> *objects);

    void removeObjects(std::vector<Object *> *objects);

    BoundingBox getBoundaries() override;

    bool intersectFirst(IntersectionInfo *intersectionInfo, Ray *ray) override;
    bool intersectAny(IntersectionInfo *intersectionInfo, Ray *ray) override;
    bool intersectAll(std::vector<IntersectionInfo *> *intersectionInfo, Ray *ray) override;

    double getSurfaceArea() override;

    ObjectCapsule getCapsule() override;

    bool operator==(Object *object) override;

    Object *clone() override;
};

#endif //DBVH_DBVHV2_H
