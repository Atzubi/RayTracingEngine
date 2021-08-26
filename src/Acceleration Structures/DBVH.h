//
// Created by sebastian on 16.06.21.
//

#ifndef DBVH_DBVHV2_H
#define DBVH_DBVHV2_H

#include <list>
#include <utility>
#include <deque>
#include "Utils/HashMap/robin_map.h"

namespace Atzubi {
    struct Vector3D {
        double x, y, z;
    };

    struct Vector2D {
        double x, y;
    };

    struct Matrix4x4 {
        double elements[4][4];

        void multiplyBy(Matrix4x4 *matrix) {
            double e[4][4];
            for (int row = 0; row < 4; row++) {
                for (int column = 0; column < 4; column++) {
                    e[row][column] = 0;
                    for (int index = 0; index < 4; index++) {
                        e[row][column] += elements[row][index] * matrix->elements[index][column];
                    }
                }
            }
            for (int row = 0; row < 4; row++) {
                for (int column = 0; column < 4; column++) {
                    elements[row][column] = e[row][column];
                }
            }
        }

        Matrix4x4 getInverse() {
            double det = elements[0][0] * elements[1][1] * elements[2][2] *
                         elements[3][3] -
                         elements[0][0] * elements[1][2] * elements[2][1] *
                         elements[3][3] -
                         elements[0][1] * elements[1][0] * elements[2][2] *
                         elements[3][3] +
                         elements[0][2] * elements[1][0] * elements[2][1] *
                         elements[3][3] +
                         elements[0][1] * elements[1][2] * elements[2][0] *
                         elements[3][3] -
                         elements[0][2] * elements[1][1] * elements[2][0] *
                         elements[3][3];

            Matrix4x4 inverse{};

            inverse.elements[0][0] = elements[1][1] * elements[2][2] * elements[3][3] -
                                     elements[1][2] * elements[2][1] * elements[3][3];
            inverse.elements[0][1] =
                    -elements[0][1] * elements[2][2] * elements[3][3] +
                    elements[0][2] * elements[2][1] * elements[3][3];
            inverse.elements[0][2] = elements[0][1] * elements[1][2] * elements[3][3] -
                                     elements[0][2] * elements[1][1] * elements[3][3];
            inverse.elements[0][3] =
                    -elements[0][1] * elements[1][2] * elements[2][3] -
                    elements[0][2] * elements[1][3] * elements[2][1] -
                    elements[0][3] * elements[1][1] * elements[2][2] +
                    elements[0][3] * elements[1][2] * elements[2][1] +
                    elements[0][2] * elements[1][1] * elements[2][3] +
                    elements[0][1] * elements[1][3] * elements[2][2];
            inverse.elements[1][0] =
                    -elements[1][0] * elements[2][2] * elements[3][3] +
                    elements[1][2] * elements[2][0] * elements[3][3];
            inverse.elements[1][1] = elements[0][0] * elements[2][2] * elements[3][3] -
                                     elements[0][2] * elements[2][0] * elements[3][3];
            inverse.elements[1][2] =
                    -elements[0][0] * elements[1][2] * elements[3][3] +
                    elements[0][2] * elements[1][0] * elements[3][3];
            inverse.elements[1][3] = elements[0][0] * elements[1][2] * elements[2][3] +
                                     elements[0][2] * elements[1][3] * elements[2][0] +
                                     elements[0][3] * elements[1][0] * elements[2][2] -
                                     elements[0][3] * elements[1][2] * elements[2][0] -
                                     elements[0][2] * elements[1][0] * elements[2][3] -
                                     elements[0][0] * elements[1][3] * elements[2][2];
            inverse.elements[2][0] = elements[1][0] * elements[2][1] * elements[3][3] -
                                     elements[1][1] * elements[2][0] * elements[3][3];
            inverse.elements[2][1] =
                    -elements[0][0] * elements[2][1] * elements[3][3] +
                    elements[0][1] * elements[2][0] * elements[3][3];
            inverse.elements[2][2] = elements[0][0] * elements[1][1] * elements[3][3] -
                                     elements[0][1] * elements[1][0] * elements[3][3];
            inverse.elements[2][3] =
                    -elements[0][0] * elements[1][1] * elements[2][3] -
                    elements[0][1] * elements[1][3] * elements[2][0] -
                    elements[0][3] * elements[1][0] * elements[2][1] +
                    elements[0][3] * elements[1][1] * elements[1][0] +
                    elements[0][1] * elements[2][0] * elements[2][3] +
                    elements[0][0] * elements[1][3] * elements[2][1];
            inverse.elements[3][3] = elements[0][0] * elements[1][1] * elements[2][2] +
                                     elements[0][1] * elements[1][2] * elements[2][0] +
                                     elements[0][2] * elements[1][0] * elements[2][1] -
                                     elements[0][2] * elements[1][1] * elements[2][0] -
                                     elements[0][1] * elements[1][0] * elements[2][2] -
                                     elements[0][0] * elements[1][2] * elements[2][1];

            inverse.elements[0][0] = (1 / det) * inverse.elements[0][0];
            inverse.elements[0][1] = (1 / det) * inverse.elements[0][1];
            inverse.elements[0][2] = (1 / det) * inverse.elements[0][2];
            inverse.elements[0][3] = (1 / det) * inverse.elements[0][3];
            inverse.elements[1][0] = (1 / det) * inverse.elements[1][0];
            inverse.elements[1][1] = (1 / det) * inverse.elements[1][1];
            inverse.elements[1][2] = (1 / det) * inverse.elements[1][2];
            inverse.elements[1][3] = (1 / det) * inverse.elements[1][3];
            inverse.elements[2][0] = (1 / det) * inverse.elements[2][0];
            inverse.elements[2][1] = (1 / det) * inverse.elements[2][1];
            inverse.elements[2][2] = (1 / det) * inverse.elements[2][2];
            inverse.elements[2][3] = (1 / det) * inverse.elements[2][3];
            inverse.elements[3][0] = 0;
            inverse.elements[3][1] = 0;
            inverse.elements[3][2] = 0;
            inverse.elements[3][3] = (1 / det) * inverse.elements[3][3];

            return inverse;
        }
    };

    struct AABB {
        Vector3D frontBottomLeft, backTopRight;

        [[nodiscard]] double getSA() const {
            return (backTopRight.x - frontBottomLeft.x) * (backTopRight.y - frontBottomLeft.y) +
                   (backTopRight.x - frontBottomLeft.x) * (backTopRight.z - frontBottomLeft.z) +
                   (backTopRight.y - frontBottomLeft.y) * (backTopRight.z - frontBottomLeft.z);
        }
    };

    struct Ray {
        Vector3D origin, direction, dirfrac;
    };

    struct Texture {
        std::string name;
        int w;
        int h;
        unsigned char *image;
    };

    struct Material {
        // Material Name
        std::string name;
        // Ambient Color
        Vector3D Ka;
        // Diffuse Color
        Vector3D Kd;
        // Specular Color
        Vector3D Ks;
        // Specular Exponent
        float Ns;
        // Optical Density
        float Ni;
        // Dissolve
        float d;
        // Illumination
        int illum;
        // Ambient Texture Map
        Texture map_Ka;
        // Diffuse Texture Map
        Texture map_Kd;
        // Specular Texture Map
        Texture map_Ks;
        // Specular Hightlight Map
        Texture map_Ns;
        // Alpha Texture Map
        Texture map_d;
        // Bump Map
        Texture map_bump;
    };

    struct HitInformation {
        bool hit;
        double distance;
        Vector3D normal;
        Vector3D position;
        Vector2D texture;
        Material *material;
    };

    class Object {
    public:
        virtual AABB getBoundingBox() = 0;

        virtual bool intersect(HitInformation *hitInformation, Ray *ray) = 0;

        virtual double getSurfaceArea() = 0;

        virtual bool operator==(Object *object) = 0;

        virtual ~Object() = default;;
    };

    struct Vertex {
        Vector3D position;
        Vector3D normal;
        Vector2D texture;
    };

    class Triangle;

    struct TriangleMesh {
        uint64_t triangleCount{};
        uint64_t *indices{};
        Vertex *vertices{};
        Material material;

        void toTriangles(std::vector<Object *> *triangles);
    };

    class Triangle : public Object {
    public:
        TriangleMesh *mesh{};
        uint64_t pos{};

        Triangle() = default;

        AABB getBoundingBox() override {
            Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
            Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
            Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

            Vector3D front = vertex1, back = vertex1;
            if (front.x > vertex2.x) {
                front.x = vertex2.x;
            }
            if (front.x > vertex3.x) {
                front.x = vertex3.x;
            }
            if (front.y > vertex2.y) {
                front.y = vertex2.y;
            }
            if (front.y > vertex3.y) {
                front.y = vertex3.y;
            }
            if (front.z > vertex2.z) {
                front.z = vertex2.z;
            }
            if (front.z > vertex3.z) {
                front.z = vertex3.z;
            }
            if (back.x < vertex2.x) {
                back.x = vertex2.x;
            }
            if (back.x < vertex3.x) {
                back.x = vertex3.x;
            }
            if (back.y < vertex2.y) {
                back.y = vertex2.y;
            }
            if (back.y < vertex3.y) {
                back.y = vertex3.y;
            }
            if (back.z < vertex2.z) {
                back.z = vertex2.z;
            }
            if (back.z < vertex3.z) {
                back.z = vertex3.z;
            }
            return {front, back};
        }

        bool intersect(HitInformation *hitInformation, Ray *ray) override {
            Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
            Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
            Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

            Vector3D e1{}, e2{}, pvec{}, qvec{}, tvec{};
            double_t epsilon = 0.000001f;

            e1.x = vertex2.x - vertex1.x;
            e1.y = vertex2.y - vertex1.y;
            e1.z = vertex2.z - vertex1.z;

            e2.x = vertex3.x - vertex1.x;
            e2.y = vertex3.y - vertex1.y;
            e2.z = vertex3.z - vertex1.z;

            pvec.x = ray->direction.y * e2.z - ray->direction.z * e2.y;
            pvec.y = ray->direction.z * e2.x - ray->direction.x * e2.z;
            pvec.z = ray->direction.x * e2.y - ray->direction.y * e2.x;

            //NORMALIZE(pvec);
            double_t det = pvec.x * e1.x + pvec.y * e1.y + pvec.z * e1.z;

            if (det < epsilon && det > -epsilon) {
                hitInformation->hit = false;
                return false;
            }

            double_t invDet = 1.0f / det;

            tvec.x = ray->origin.x - vertex1.x;
            tvec.y = ray->origin.y - vertex1.y;
            tvec.z = ray->origin.z - vertex1.z;

            // NORMALIZE(tvec);
            double_t u = invDet * (tvec.x * pvec.x + tvec.y * pvec.y + tvec.z * pvec.z);

            if (u < 0.0f || u > 1.0f) {
                hitInformation->hit = false;
                return false;
            }

            qvec.x = tvec.y * e1.z - tvec.z * e1.y;
            qvec.y = tvec.z * e1.x - tvec.x * e1.z;
            qvec.z = tvec.x * e1.y - tvec.y * e1.x;

            // NORMALIZE(qvec);
            double_t v =
                    invDet * (qvec.x * ray->direction.x + qvec.y * ray->direction.y + qvec.z * ray->direction.z);

            if (v < 0.0f || u + v > 1.0f) {
                hitInformation->hit = false;
                return false;
            }

            double t = invDet * (e2.x * qvec.x + e2.y * qvec.y + e2.z * qvec.z);

            if (t <= epsilon) {
                hitInformation->hit = false;
                return false;
            }

            double_t w = 1 - u - v;

            hitInformation->position.x = ray->origin.x + ray->direction.x * t;
            hitInformation->position.y = ray->origin.y + ray->direction.y * t;
            hitInformation->position.z = ray->origin.z + ray->direction.z * t;

            hitInformation->distance = sqrt(
                    (ray->origin.x - hitInformation->position.x) * (ray->origin.x - hitInformation->position.x) +
                    (ray->origin.y - hitInformation->position.y) * (ray->origin.y - hitInformation->position.y) +
                    (ray->origin.z - hitInformation->position.z) * (ray->origin.z - hitInformation->position.z));

            Vector3D normal1 = mesh->vertices[mesh->indices[pos]].normal;
            Vector3D normal2 = mesh->vertices[mesh->indices[pos + 1]].normal;
            Vector3D normal3 = mesh->vertices[mesh->indices[pos + 2]].normal;

            hitInformation->normal.x = w * normal1.x + u * normal2.x + v * normal3.x;
            hitInformation->normal.y = w * normal1.y + u * normal2.y + v * normal3.y;
            hitInformation->normal.z = w * normal1.z + u * normal2.z + v * normal3.z;

            double_t length = sqrt(hitInformation->normal.x * hitInformation->normal.x +
                                   hitInformation->normal.y * hitInformation->normal.y +
                                   hitInformation->normal.z * hitInformation->normal.z);

            hitInformation->normal.x /= length;
            hitInformation->normal.y /= length;
            hitInformation->normal.z /= length;

            Vector2D texture1 = mesh->vertices[mesh->indices[pos]].texture;
            Vector2D texture2 = mesh->vertices[mesh->indices[pos + 1]].texture;
            Vector2D texture3 = mesh->vertices[mesh->indices[pos + 2]].texture;

            hitInformation->texture.x = w * texture1.x + u * texture2.x + v * texture3.x;
            hitInformation->texture.y = w * texture1.y + u * texture2.y + v * texture3.y;

            hitInformation->material = &mesh->material;

            hitInformation->hit = true;
            return true;
        }

        double getSurfaceArea() override {
            return getBoundingBox().getSA();
        }

        bool operator==(Object *object) override {
            auto *triangle = dynamic_cast<Triangle *>(object);
            if (triangle == nullptr) {
                return false;
            } else {
                Vector3D vertex1 = mesh->vertices[mesh->indices[pos]].position;
                Vector3D vertex2 = mesh->vertices[mesh->indices[pos + 1]].position;
                Vector3D vertex3 = mesh->vertices[mesh->indices[pos + 2]].position;

                Vector3D otherVertex1 = triangle->mesh->vertices[triangle->mesh->indices[pos]].position;
                Vector3D otherVertex2 = triangle->mesh->vertices[triangle->mesh->indices[pos + 1]].position;
                Vector3D otherVertex3 = triangle->mesh->vertices[triangle->mesh->indices[pos + 2]].position;

                return otherVertex1.x == vertex1.x && otherVertex1.y == vertex1.y &&
                       otherVertex1.z == vertex1.z && otherVertex2.x == vertex2.x &&
                       otherVertex2.y == vertex2.y && otherVertex2.z == vertex2.z &&
                       otherVertex3.x == vertex3.x && otherVertex3.y == vertex3.y &&
                       otherVertex3.z == vertex3.z;
            }
        }

        ~Triangle() override = default;
    };

    void TriangleMesh::toTriangles(std::vector<Object *> *triangles) {
        for (int i = 0; i < triangleCount; i++) {
            auto *triangle = new Triangle();
            triangle->mesh = this;
            triangle->pos = i * 3;
            triangles->push_back(triangle);
        }
    }

    class DBVHv2 : public Object {
    private:
        class Node {
        public:
            virtual uint8_t getType() = 0;

            virtual AABB getBoundingBox() = 0;

            virtual ~Node() = default;
        };

        class InnerNode : public Node {
        public:
            Node *leftChild, *rightChild;
            AABB boundingBox{};
            double surfaceArea;

            InnerNode() {
                surfaceArea = 0;
                leftChild = nullptr;
                rightChild = nullptr;
                boundingBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                               -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
            }

            uint8_t getType() override {
                return 0;
            }

            AABB getBoundingBox() override {
                return boundingBox;
            }

            bool optimizeSAH() {
                int bestSAH = 0;
                double SAHs[5];

                AABB leftBox{}, rightBox{}, leftLeftBox{}, leftRightBox{}, rightLeftBox{}, rightRightBox{};
                double leftSA = 0, rightSA = 0, leftLeftSA = 0, leftRightSA = 0, rightLeftSA = 0, rightRightSA = 0;

                AABB swapLeftLeftToRight{}, swapLeftRightToRight{}, swapRightLeftToLeft{}, swapRightRightToLeft{};

                SAHs[0] = surfaceArea;

                if (leftChild->getType() == 0) {
                    auto *leftNode = (InnerNode *) leftChild;
                    leftBox = leftNode->boundingBox;
                    leftSA = leftNode->surfaceArea;
                    if (leftNode->leftChild->getType() == 0) {
                        leftLeftBox = ((InnerNode *) (leftNode->leftChild))->boundingBox;
                        leftLeftSA = ((InnerNode *) (leftNode->leftChild))->surfaceArea;
                    } else {
                        leftLeftBox = ((Child * )(leftNode->leftChild))->object->getBoundingBox();
                        leftLeftSA = ((Child * )(leftNode->leftChild))->object->getSurfaceArea();
                    }
                    if (leftNode->rightChild->getType() == 0) {
                        leftRightBox = ((InnerNode *) (leftNode->rightChild))->boundingBox;
                        leftRightSA = ((InnerNode *) (leftNode->rightChild))->surfaceArea;
                    } else {
                        leftRightBox = ((Child * )(leftNode->rightChild))->object->getBoundingBox();
                        leftRightSA = ((Child * )(leftNode->rightChild))->object->getSurfaceArea();
                    }

                    if (rightChild->getType() == 0) {
                        auto *rightNode = (InnerNode *) rightChild;
                        rightBox = rightNode->boundingBox;
                        rightSA = rightNode->surfaceArea;
                        if (rightNode->leftChild->getType() == 0) {
                            rightLeftBox = ((InnerNode *) (rightNode->leftChild))->boundingBox;
                            rightLeftSA = ((InnerNode *) (rightNode->leftChild))->surfaceArea;
                        } else {
                            rightLeftBox = ((Child * )(rightNode->leftChild))->object->getBoundingBox();
                            rightLeftSA = ((Child * )(rightNode->leftChild))->object->getSurfaceArea();
                        }
                        if (rightNode->rightChild->getType() == 0) {
                            rightRightBox = ((InnerNode *) (rightNode->rightChild))->boundingBox;
                            rightRightSA = ((InnerNode *) (rightNode->rightChild))->surfaceArea;
                        } else {
                            rightRightBox = ((Child * )(rightNode->rightChild))->object->getBoundingBox();
                            rightRightSA = ((Child * )(rightNode->rightChild))->object->getSurfaceArea();
                        }

                        swapLeftLeftToRight = {std::min(rightBox.frontBottomLeft.x, leftRightBox.frontBottomLeft.x),
                                               std::min(rightBox.frontBottomLeft.y, leftRightBox.frontBottomLeft.y),
                                               std::min(rightBox.frontBottomLeft.z, leftRightBox.frontBottomLeft.z),
                                               std::max(rightBox.backTopRight.x, leftRightBox.backTopRight.x),
                                               std::max(rightBox.backTopRight.y, leftRightBox.backTopRight.y),
                                               std::max(rightBox.backTopRight.z, leftRightBox.backTopRight.z)};
                        swapLeftRightToRight = {std::min(rightBox.frontBottomLeft.x, leftLeftBox.frontBottomLeft.x),
                                                std::min(rightBox.frontBottomLeft.y, leftLeftBox.frontBottomLeft.y),
                                                std::min(rightBox.frontBottomLeft.z, leftLeftBox.frontBottomLeft.z),
                                                std::max(rightBox.backTopRight.x, leftLeftBox.backTopRight.x),
                                                std::max(rightBox.backTopRight.y, leftLeftBox.backTopRight.y),
                                                std::max(rightBox.backTopRight.z, leftLeftBox.backTopRight.z)};
                        swapRightLeftToLeft = {std::min(leftBox.frontBottomLeft.x, rightRightBox.frontBottomLeft.x),
                                               std::min(leftBox.frontBottomLeft.y, rightRightBox.frontBottomLeft.y),
                                               std::min(leftBox.frontBottomLeft.z, rightRightBox.frontBottomLeft.z),
                                               std::max(leftBox.backTopRight.x, rightRightBox.backTopRight.x),
                                               std::max(leftBox.backTopRight.y, rightRightBox.backTopRight.y),
                                               std::max(leftBox.backTopRight.z, rightRightBox.backTopRight.z)};
                        swapRightRightToLeft = {std::min(leftBox.frontBottomLeft.x, rightLeftBox.frontBottomLeft.x),
                                                std::min(leftBox.frontBottomLeft.y, rightLeftBox.frontBottomLeft.y),
                                                std::min(leftBox.frontBottomLeft.z, rightLeftBox.frontBottomLeft.z),
                                                std::max(leftBox.backTopRight.x, rightLeftBox.backTopRight.x),
                                                std::max(leftBox.backTopRight.y, rightLeftBox.backTopRight.y),
                                                std::max(leftBox.backTopRight.z, rightLeftBox.backTopRight.z)};

                        SAHs[1] =
                                boundingBox.getSA() + leftLeftSA + rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                        SAHs[2] =
                                boundingBox.getSA() + leftRightSA + rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                        SAHs[3] =
                                boundingBox.getSA() + rightLeftSA + leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                        SAHs[4] = boundingBox.getSA() + rightRightSA + leftSA + rightLeftSA +
                                  swapRightRightToLeft.getSA();
                    } else {
                        auto *rightNode = (Child *) rightChild;
                        rightBox = rightNode->object->getBoundingBox();
                        rightSA = rightNode->object->getSurfaceArea();

                        swapLeftLeftToRight = {std::min(rightBox.frontBottomLeft.x, leftRightBox.frontBottomLeft.x),
                                               std::min(rightBox.frontBottomLeft.y, leftRightBox.frontBottomLeft.y),
                                               std::min(rightBox.frontBottomLeft.z, leftRightBox.frontBottomLeft.z),
                                               std::max(rightBox.backTopRight.x, leftRightBox.backTopRight.x),
                                               std::max(rightBox.backTopRight.y, leftRightBox.backTopRight.y),
                                               std::max(rightBox.backTopRight.z, leftRightBox.backTopRight.z)};
                        swapLeftRightToRight = {std::min(rightBox.frontBottomLeft.x, leftLeftBox.frontBottomLeft.x),
                                                std::min(rightBox.frontBottomLeft.y, leftLeftBox.frontBottomLeft.y),
                                                std::min(rightBox.frontBottomLeft.z, leftLeftBox.frontBottomLeft.z),
                                                std::max(rightBox.backTopRight.x, leftLeftBox.backTopRight.x),
                                                std::max(rightBox.backTopRight.y, leftLeftBox.backTopRight.y),
                                                std::max(rightBox.backTopRight.z, leftLeftBox.backTopRight.z)};

                        SAHs[1] =
                                boundingBox.getSA() + leftLeftSA + rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                        SAHs[2] =
                                boundingBox.getSA() + leftRightSA + rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                        SAHs[3] = std::numeric_limits<double>::max();
                        SAHs[4] = std::numeric_limits<double>::max();
                    }
                } else {
                    auto *leftNode = (Child *) leftChild;
                    leftBox = leftNode->object->getBoundingBox();
                    leftSA = leftNode->object->getSurfaceArea();

                    if (rightChild->getType() == 0) {
                        auto *rightNode = (InnerNode *) rightChild;
                        rightBox = rightNode->boundingBox;
                        rightSA = rightNode->surfaceArea;
                        if (rightNode->leftChild->getType() == 0) {
                            rightLeftBox = ((InnerNode *) (rightNode->leftChild))->boundingBox;
                            rightLeftSA = ((InnerNode *) (rightNode->leftChild))->surfaceArea;
                        } else {
                            rightLeftBox = ((Child * )(rightNode->leftChild))->object->getBoundingBox();
                            rightLeftSA = ((Child * )(rightNode->leftChild))->object->getSurfaceArea();
                        }
                        if (rightNode->rightChild->getType() == 0) {
                            rightRightBox = ((InnerNode *) (rightNode->rightChild))->boundingBox;
                            rightRightSA = ((InnerNode *) (rightNode->rightChild))->surfaceArea;
                        } else {
                            rightRightBox = ((Child * )(rightNode->rightChild))->object->getBoundingBox();
                            rightRightSA = ((Child * )(rightNode->rightChild))->object->getSurfaceArea();
                        }

                        swapRightLeftToLeft = {std::min(leftBox.frontBottomLeft.x, rightRightBox.frontBottomLeft.x),
                                               std::min(leftBox.frontBottomLeft.y, rightRightBox.frontBottomLeft.y),
                                               std::min(leftBox.frontBottomLeft.z, rightRightBox.frontBottomLeft.z),
                                               std::max(leftBox.backTopRight.x, rightRightBox.backTopRight.x),
                                               std::max(leftBox.backTopRight.y, rightRightBox.backTopRight.y),
                                               std::max(leftBox.backTopRight.z, rightRightBox.backTopRight.z)};
                        swapRightRightToLeft = {std::min(leftBox.frontBottomLeft.x, rightLeftBox.frontBottomLeft.x),
                                                std::min(leftBox.frontBottomLeft.y, rightLeftBox.frontBottomLeft.y),
                                                std::min(leftBox.frontBottomLeft.z, rightLeftBox.frontBottomLeft.z),
                                                std::max(leftBox.backTopRight.x, rightLeftBox.backTopRight.x),
                                                std::max(leftBox.backTopRight.y, rightLeftBox.backTopRight.y),
                                                std::max(leftBox.backTopRight.z, rightLeftBox.backTopRight.z)};

                        SAHs[1] = std::numeric_limits<double>::max();
                        SAHs[2] = std::numeric_limits<double>::max();
                        SAHs[3] =
                                boundingBox.getSA() + rightLeftSA + leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                        SAHs[4] = boundingBox.getSA() + rightRightSA + leftSA + rightLeftSA +
                                  swapRightRightToLeft.getSA();
                    } else {
                        return false;
                    }
                }


                if (SAHs[1] < SAHs[0]) {
                    bestSAH = 1;
                }
                if (SAHs[2] < SAHs[bestSAH]) {
                    bestSAH = 2;
                }
                if (SAHs[3] < SAHs[bestSAH]) {
                    bestSAH = 3;
                }
                if (SAHs[4] < SAHs[bestSAH]) {
                    bestSAH = 4;
                }

                switch (bestSAH) {
                    case 1: {
                        auto buffer = rightChild;
                        rightChild = ((InnerNode *) leftChild)->leftChild;
                        ((InnerNode *) leftChild)->boundingBox = swapLeftLeftToRight;
                        ((InnerNode *) leftChild)->leftChild = buffer;

                        surfaceArea = SAHs[1];
                        ((InnerNode *) leftChild)->surfaceArea = rightSA + leftRightSA + swapLeftLeftToRight.getSA();
                        return true;
                    }
                    case 2: {
                        auto buffer = rightChild;
                        rightChild = ((InnerNode *) leftChild)->rightChild;
                        ((InnerNode *) leftChild)->boundingBox = swapLeftRightToRight;
                        ((InnerNode *) leftChild)->rightChild = buffer;

                        surfaceArea = SAHs[2];
                        ((InnerNode *) leftChild)->surfaceArea = rightSA + leftLeftSA + swapLeftRightToRight.getSA();
                        return true;
                    }
                    case 3: {
                        auto buffer = leftChild;
                        leftChild = ((InnerNode *) rightChild)->leftChild;
                        ((InnerNode *) rightChild)->boundingBox = swapRightLeftToLeft;
                        ((InnerNode *) rightChild)->leftChild = buffer;

                        surfaceArea = SAHs[3];
                        ((InnerNode *) rightChild)->surfaceArea = leftSA + rightRightSA + swapRightLeftToLeft.getSA();
                        return true;
                    }
                    case 4: {
                        auto buffer = leftChild;
                        leftChild = ((InnerNode *) rightChild)->rightChild;
                        ((InnerNode *) rightChild)->boundingBox = swapRightRightToLeft;
                        ((InnerNode *) rightChild)->rightChild = buffer;

                        surfaceArea = SAHs[4];
                        ((InnerNode *) rightChild)->surfaceArea = leftSA + rightLeftSA + swapRightRightToLeft.getSA();
                        return true;
                    }
                    default:
                        surfaceArea = SAHs[0];
                        return false;
                }
            }

            ~InnerNode() override {
                delete leftChild;
                delete rightChild;
            }
        };

        class Child : public Node {
        public:
            Object *object{};

            uint8_t getType() override {
                return 1;
            }

            AABB getBoundingBox() override {
                return object->getBoundingBox();
            }

            ~Child() override = default;
        };

        static double
        evaluateBucket(AABB *leftChildBox, AABB *rightChildBox, double leftSAH, double rightSAH,
                       std::vector<Object *> *objects, Vector3D splittingPlane,
                       uint8_t *newParent) {
            // initialize both bucket boxes
            AABB aabbLeft = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                             -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
            AABB aabbRight = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                              std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                              -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
            /*AABB aabbLeft = *leftChildBox;
            AABB aabbRight = *rightChildBox;*/

            double leftCount = 0;
            double rightCount = 0;

            // sort all objects into their bucket, then update the buckets bounding box
            if (splittingPlane.x != 0) {
                for (auto &object : *objects) {
                    if ((object->getBoundingBox().backTopRight.x + object->getBoundingBox().frontBottomLeft.x) / 2 <
                        splittingPlane.x) {
                        refit(aabbLeft, object);
                        leftCount += 1;
                    } else {
                        refit(aabbRight, object);
                        rightCount += 1;
                    }
                }
            } else if (splittingPlane.y != 0) {
                for (auto &object : *objects) {
                    if ((object->getBoundingBox().backTopRight.y + object->getBoundingBox().frontBottomLeft.y) / 2 <
                        splittingPlane.y) {
                        refit(aabbLeft, object);
                        leftCount += 1;
                    } else {
                        refit(aabbRight, object);
                        rightCount += 1;
                    }
                }
            } else {
                for (auto &object : *objects) {
                    if ((object->getBoundingBox().backTopRight.z + object->getBoundingBox().frontBottomLeft.z) / 2 <
                        splittingPlane.z) {
                        refit(aabbLeft, object);
                        leftCount += 1;
                    } else {
                        refit(aabbRight, object);
                        rightCount += 1;
                    }
                }
            }

            if (leftChildBox != nullptr && rightChildBox != nullptr) {
                AABB oldLeft = *leftChildBox;
                AABB oldLeftNewLeft = *leftChildBox;
                AABB oldLeftNewRight = *leftChildBox;
                AABB oldLeftNewLeftNewRight = *leftChildBox;
                AABB oldLeftOldRight = *leftChildBox;
                AABB oldLeftOldRightNewLeft = *leftChildBox;
                AABB oldLeftOldRightNewRight = *leftChildBox;
                AABB oldRightNewLeftNewRight = *rightChildBox;
                AABB oldRightNewRight = *rightChildBox;
                AABB oldRightNewLeft = *rightChildBox;
                AABB oldRight = *rightChildBox;
                AABB newLeftNewRight = aabbLeft;
                AABB newRight = aabbRight;
                AABB newLeft = aabbLeft;

                double SAHs[7];
                double SAH = std::numeric_limits<double>::max();
                int bestSAH = 0;

                refit(&newLeftNewRight, newRight);
                refit(&oldLeftNewLeft, newLeft);
                refit(&oldLeftNewRight, newRight);
                refit(&oldLeftNewLeftNewRight, newLeftNewRight);
                refit(&oldLeftOldRight, oldRight);
                refit(&oldRightNewLeft, newLeft);
                refit(&oldRightNewRight, newRight);
                refit(&oldLeftOldRightNewLeft, oldRightNewLeft);
                refit(&oldLeftOldRightNewRight, oldRightNewRight);
                refit(&oldRightNewLeftNewRight, newLeftNewRight);

                SAHs[0] = oldLeftNewLeft.getSA() * (leftCount + leftSAH) +
                          oldRightNewRight.getSA() * (rightCount + rightSAH);
                SAHs[1] = oldLeftNewRight.getSA() * (rightCount + leftSAH) +
                          oldRightNewLeft.getSA() * (leftCount + rightSAH);
                SAHs[2] = oldLeft.getSA() * leftSAH +
                          oldRightNewLeftNewRight.getSA() * (leftCount + rightCount + rightSAH);
                SAHs[3] = oldLeftNewLeftNewRight.getSA() * (leftCount + rightCount + leftSAH) +
                          oldRight.getSA() * rightSAH;
                SAHs[4] = oldLeftOldRight.getSA() * (leftSAH + rightSAH) +
                          newLeftNewRight.getSA() * (leftCount + rightCount);
                SAHs[5] = oldLeftOldRightNewLeft.getSA() * (leftCount + leftSAH + rightSAH) +
                          newRight.getSA() * rightCount;
                SAHs[6] = oldLeftOldRightNewRight.getSA() * (rightCount + leftSAH + rightSAH) +
                          newLeft.getSA() * leftCount;

                for (int i = 0; i < 7; i++) {
                    if (SAHs[i] < SAH) {
                        SAH = SAHs[i];
                        bestSAH = i;
                    }
                }

                *newParent = bestSAH + 1;
                return SAHs[bestSAH];
            } else {
                // return the combined surface area of both boxes
                return aabbLeft.getSA() * leftCount + aabbRight.getSA() * rightCount;
            }
        }

        static void
        split(std::vector<Object *> *leftChild, std::vector<Object *> *rightChild, std::vector<Object *> *objects,
              Vector3D splittingPlane) {
            if (splittingPlane.x != 0) {
                for (auto &object : *objects) {
                    if ((object->getBoundingBox().backTopRight.x + object->getBoundingBox().frontBottomLeft.x) / 2 <
                        splittingPlane.x) {
                        leftChild->push_back(object);
                    } else {
                        rightChild->push_back(object);
                    }
                }
            } else if (splittingPlane.y != 0) {
                for (auto &object : *objects) {
                    if ((object->getBoundingBox().backTopRight.y + object->getBoundingBox().frontBottomLeft.y) / 2 <
                        splittingPlane.y) {
                        leftChild->push_back(object);
                    } else {
                        rightChild->push_back(object);
                    }
                }
            } else {
                for (auto &object : *objects) {
                    if ((object->getBoundingBox().backTopRight.z + object->getBoundingBox().frontBottomLeft.z) / 2 <
                        splittingPlane.z) {
                        leftChild->push_back(object);
                    } else {
                        rightChild->push_back(object);
                    }
                }
            }
        }

        static void refit(AABB *target, AABB resizeBy) {
            target->frontBottomLeft.x = std::min(target->frontBottomLeft.x, resizeBy.frontBottomLeft.x);
            target->frontBottomLeft.y = std::min(target->frontBottomLeft.y, resizeBy.frontBottomLeft.y);
            target->frontBottomLeft.z = std::min(target->frontBottomLeft.z, resizeBy.frontBottomLeft.z);
            target->backTopRight.x = std::max(target->backTopRight.x, resizeBy.backTopRight.x);
            target->backTopRight.y = std::max(target->backTopRight.y, resizeBy.backTopRight.y);
            target->backTopRight.z = std::max(target->backTopRight.z, resizeBy.backTopRight.z);
        }

        static void refit(AABB &aabb, Object *object) {
            refit(&aabb, object->getBoundingBox());
        }

        static void refit(AABB &aabb, std::vector<Object *> *objects, double looseness) {
            // refit box to fit all objects
            for (auto &object : *objects) {
                refit(aabb, object);
            }

            // increase box size by looseness factor
            if (looseness > 0) {
                double scaleX = aabb.backTopRight.x - aabb.frontBottomLeft.x;
                double scaleY = aabb.backTopRight.y - aabb.frontBottomLeft.y;
                double scaleZ = aabb.backTopRight.z - aabb.frontBottomLeft.z;
                aabb.frontBottomLeft.x -= scaleX * looseness;
                aabb.frontBottomLeft.y -= scaleY * looseness;
                aabb.frontBottomLeft.z -= scaleZ * looseness;
                aabb.backTopRight.x += scaleX * looseness;
                aabb.backTopRight.y += scaleY * looseness;
                aabb.backTopRight.z += scaleZ * looseness;
            }
        }

        void add(Node *currentNode, std::vector<Object *> *objects, uint8_t depth) {
            if (depth > maxDepth) maxDepth = depth;

            // refit current node to objects
            auto *node = (InnerNode *) currentNode;
            refit(node->boundingBox, objects, 0);

            // create split buckets
            Vector3D splittingPlanes[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0};
            uint8_t newParent[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

            double x = (node->boundingBox.backTopRight.x - node->boundingBox.frontBottomLeft.x) / 4.0;
            splittingPlanes[0].x = node->boundingBox.frontBottomLeft.x + x;
            splittingPlanes[1].x = node->boundingBox.frontBottomLeft.x + x * 2.0;
            splittingPlanes[2].x = node->boundingBox.frontBottomLeft.x + x * 3.0;

            double y = (node->boundingBox.backTopRight.y - node->boundingBox.frontBottomLeft.y) / 4.0;
            splittingPlanes[3].y = node->boundingBox.frontBottomLeft.y + y;
            splittingPlanes[4].y = node->boundingBox.frontBottomLeft.y + y * 2.0;
            splittingPlanes[5].y = node->boundingBox.frontBottomLeft.y + y * 3.0;

            double z = (node->boundingBox.backTopRight.z - node->boundingBox.frontBottomLeft.z) / 4.0;
            splittingPlanes[6].z = node->boundingBox.frontBottomLeft.z + z;
            splittingPlanes[7].z = node->boundingBox.frontBottomLeft.z + z * 2.0;
            splittingPlanes[8].z = node->boundingBox.frontBottomLeft.z + z * 3.0;

            // evaluate split buckets
            double SAH[9];
            AABB leftBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                            -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
            AABB rightBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                             -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
            double leftSA = 0, rightSA = 0;

            if (node->leftChild != nullptr) {
                if (node->leftChild->getType() == 0) {
                    leftBox = ((InnerNode *) (node->leftChild))->boundingBox;
                    leftSA = ((InnerNode *) (node->leftChild))->surfaceArea / leftBox.getSA();
                } else {
                    leftBox = ((Child *) (node->leftChild))->object->getBoundingBox();
                    leftSA = ((Child *) (node->leftChild))->object->getSurfaceArea() / leftBox.getSA();
                }
                if (node->rightChild != nullptr) {
                    if (node->rightChild->getType() == 0) {
                        rightBox = ((InnerNode *) (node->rightChild))->boundingBox;
                        rightSA = ((InnerNode *) (node->rightChild))->surfaceArea / rightBox.getSA();
                    } else {
                        rightBox = ((Child *) (node->rightChild))->object->getBoundingBox();
                        rightSA = ((Child *) (node->rightChild))->object->getSurfaceArea() / rightBox.getSA();
                    }
                    for (int i = 0; i < 9; i++) {
                        SAH[i] = evaluateBucket(&leftBox, &rightBox, leftSA, rightSA, objects, splittingPlanes[i],
                                                &(newParent[i]));
                    }
                } else {
                    for (int i = 0; i < 9; i++) {
                        SAH[i] = evaluateBucket(nullptr, nullptr, 0, 0, objects, splittingPlanes[i], &(newParent[i]));
                    }
                }
            } else {
                for (int i = 0; i < 9; i++) {
                    SAH[i] = evaluateBucket(nullptr, nullptr, 0, 0, objects, splittingPlanes[i], &(newParent[i]));
                }
            }

            // choose best split bucket and split node accordingly
            auto *leftObjects = new std::vector<Object *>();
            auto *rightObjects = new std::vector<Object *>();

            double bestSAH = std::numeric_limits<double>::max();
            int bestSplittingPlane = -1;

            for (int i = 0; i < 9; i++) {
                if (SAH[i] < bestSAH) {
                    bestSAH = SAH[i];
                    bestSplittingPlane = i;
                }
            }

            if (bestSplittingPlane == -1) {
                for (uint64_t i = 0; i < objects->size() / 2; i++) {
                    leftObjects->push_back(objects->at(i));
                }
                for (uint64_t i = objects->size() / 2; i < objects->size(); i++) {
                    rightObjects->push_back(objects->at(i));
                }
            } else {
                split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                /*switch (newParent[bestSplittingPlane]) {
                    case 0:
                        // default
                    case 1:
                        // existing boxes with correct order
                        split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                        break;
                    case 2: {
                        // existing boxes with wrong order
                        split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                        auto buffer = leftObjects;
                        leftObjects = rightObjects;
                        rightObjects = buffer;
                        break;
                    }
                    case 3: {
                        // existing box all new left
                        *leftObjects = *objects;
                        break;
                    }
                    case 4: {
                        // existing box all new right
                        *rightObjects = *objects;
                        break;
                    }
                    case 5: {
                        // new box split
                        auto *newNode = new InnerNode();
                        newNode->rightChild = node->rightChild;
                        newNode->leftChild = node->leftChild;
                        refit(&newNode->boundingBox, node->rightChild->getBoundingBox());
                        refit(&newNode->boundingBox, node->leftChild->getBoundingBox());
                        newNode->surfaceArea = node->surfaceArea;
                        node->leftChild = newNode;
                        node->rightChild = nullptr;
                        *rightObjects = *objects;
                        break;
                    }
                    case 6: {
                        // new box correct order
                        split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                        auto *newNode = new InnerNode();
                        newNode->rightChild = node->rightChild;
                        newNode->leftChild = node->leftChild;
                        refit(&newNode->boundingBox, node->rightChild->getBoundingBox());
                        refit(&newNode->boundingBox, node->leftChild->getBoundingBox());
                        newNode->surfaceArea = node->surfaceArea;
                        node->leftChild = newNode;
                        node->rightChild = nullptr;
                        break;
                    }
                    case 7: {
                        // new box wrong order
                        split(leftObjects, rightObjects, objects, splittingPlanes[bestSplittingPlane]);
                        auto buffer = leftObjects;
                        leftObjects = rightObjects;
                        rightObjects = buffer;
                        auto *newNode = new InnerNode();
                        newNode->rightChild = node->rightChild;
                        newNode->leftChild = node->leftChild;
                        refit(&newNode->boundingBox, node->rightChild->getBoundingBox());
                        refit(&newNode->boundingBox, node->leftChild->getBoundingBox());
                        newNode->surfaceArea = node->surfaceArea;
                        node->leftChild = newNode;
                        node->rightChild = nullptr;
                        break;
                    }
                    default:
                        std::cout << "wtf\n";
                        throw (std::exception());
                }*/
            }

            // pass objects to children
            if (leftObjects->size() == 1) {
                if (node->leftChild == nullptr) {
                    // create new child
                    auto child = new Child();
                    child->object = leftObjects->at(0);
                    node->leftChild = child;
                } else if (node->leftChild->getType() == 1) {
                    // create new parent for both children
                    auto buffer = node->leftChild;
                    auto child = new Child();
                    auto parent = new InnerNode();
                    child->object = leftObjects->at(0);
                    parent->boundingBox = buffer->getBoundingBox();
                    refit(parent->boundingBox, leftObjects, 0);
                    /*double SA = parent->boundingBox.getSA();
                    SA += child->object->getSurfaceArea();
                    SA += ((Child *) buffer)->object->getSurfaceArea();
                    parent->surfaceArea = SA;*/
                    parent->leftChild = buffer;
                    parent->rightChild = child;
                    node->leftChild = parent;
                } else {
                    add(node->leftChild, leftObjects, depth + 1);
                }
            } else if (!leftObjects->empty()) {
                if (node->leftChild == nullptr) {
                    // create new child
                    auto child = new InnerNode();
                    node->leftChild = child;
                    add(node->leftChild, leftObjects, depth + 1);
                } else if (node->leftChild->getType() == 1) {
                    // create new parent for both children
                    auto buffer = node->leftChild;
                    auto parent = new InnerNode();
                    parent->leftChild = buffer;
                    parent->boundingBox = buffer->getBoundingBox();
                    parent->surfaceArea = parent->boundingBox.getSA() * 2;
                    node->leftChild = parent;
                    add(node->leftChild, leftObjects, depth + 1);
                } else {
                    add(node->leftChild, leftObjects, depth + 1);
                }
            }

            if (rightObjects->size() == 1) {
                if (node->rightChild == nullptr) {
                    // create new child
                    auto child = new Child();
                    child->object = rightObjects->at(0);
                    node->rightChild = child;
                } else if (node->rightChild->getType() == 1) {
                    // create new parent for both children
                    auto buffer = node->rightChild;
                    auto child = new Child();
                    auto parent = new InnerNode();
                    child->object = rightObjects->at(0);
                    parent->boundingBox = buffer->getBoundingBox();
                    refit(parent->boundingBox, rightObjects, 0);
                    /*double SA = parent->boundingBox.getSA();
                    SA += child->object->getSurfaceArea();
                    SA += ((Child *) buffer)->object->getSurfaceArea();
                    parent->surfaceArea = SA;*/
                    parent->leftChild = buffer;
                    parent->rightChild = child;
                    node->rightChild = parent;
                } else {
                    add(node->rightChild, rightObjects, depth + 1);
                }
            } else if (!rightObjects->empty()) {
                if (node->rightChild == nullptr) {
                    // create new child
                    auto child = new InnerNode();
                    node->rightChild = child;
                    add(node->rightChild, rightObjects, depth + 1);
                } else if (node->rightChild->getType() == 1) {
                    // create new parent for both children
                    auto buffer = node->rightChild;
                    auto parent = new InnerNode();
                    parent->rightChild = buffer;
                    parent->boundingBox = buffer->getBoundingBox();
                    parent->surfaceArea = parent->boundingBox.getSA() * 2;
                    node->rightChild = parent;
                    add(node->rightChild, rightObjects, depth + 1);
                } else {
                    add(node->rightChild, rightObjects, depth + 1);
                }
            }

            delete leftObjects;
            delete rightObjects;

            node->surfaceArea = node->boundingBox.getSA();
            if (node->leftChild->getType() == 0) {
                node->surfaceArea += ((InnerNode *) node->leftChild)->surfaceArea;
            } else {
                node->surfaceArea += ((Child *) node->leftChild)->object->getSurfaceArea();
            }
            if (node->rightChild->getType() == 0) {
                node->surfaceArea += ((InnerNode *) node->rightChild)->surfaceArea;
            } else {
                node->surfaceArea += ((Child *) node->rightChild)->object->getSurfaceArea();
            }

            // use tree rotations going the tree back up to optimize SAH
            node->optimizeSAH();
        }

        static void refit(InnerNode *node) {
            node->boundingBox = {std::numeric_limits<double>::max(),
                                 std::numeric_limits<double>::max(),
                                 std::numeric_limits<double>::max(),
                                 -std::numeric_limits<double>::max(),
                                 -std::numeric_limits<double>::max(),
                                 -std::numeric_limits<double>::max()};
            if (node->rightChild->getType() == 0) {
                refit(&node->boundingBox, ((InnerNode *) node->rightChild)->boundingBox);
                node->surfaceArea = ((InnerNode *) node->rightChild)->surfaceArea;
            } else {
                refit(&node->boundingBox, ((Child *) node->rightChild)->object->getBoundingBox());
                node->surfaceArea = ((Child *) node->rightChild)->object->getSurfaceArea();
            }
            if (node->leftChild->getType() == 0) {
                refit(&node->boundingBox, ((InnerNode *) node->leftChild)->boundingBox);
                node->surfaceArea += ((InnerNode *) node->leftChild)->surfaceArea;
                node->surfaceArea += node->boundingBox.getSA();
            } else {
                refit(&node->boundingBox, ((Child *) node->leftChild)->object->getBoundingBox());
                node->surfaceArea += ((Child *) node->leftChild)->object->getSurfaceArea();
                node->surfaceArea += node->boundingBox.getSA();
            }
        }

        static bool contains(AABB aabb1, AABB aabb2) {
            return aabb1.frontBottomLeft.x <= aabb2.frontBottomLeft.x &&
                   aabb1.backTopRight.x >= aabb2.backTopRight.x &&
                   aabb1.frontBottomLeft.y <= aabb2.frontBottomLeft.y &&
                   aabb1.backTopRight.y >= aabb2.backTopRight.y &&
                   aabb1.frontBottomLeft.z <= aabb2.frontBottomLeft.z &&
                   aabb1.backTopRight.z >= aabb2.backTopRight.z;
        }

        void remove(Node *currentNode, Object *object) {
            if (currentNode->getType() == 0) {
                auto node = (InnerNode *) currentNode;
                if (contains(node->boundingBox, object->getBoundingBox())) {
                    if (node->leftChild->getType() == 0) {
                        auto child = (InnerNode *) (node->leftChild);
                        if (contains(child->boundingBox, object->getBoundingBox())) {
                            if (child->leftChild->getType() == 1) {
                                auto grandChild = (Child *) (child->leftChild);
                                if (grandChild->object->operator==(object)) {
                                    node->leftChild = child->rightChild;
                                    delete grandChild;
                                    refit(node);
                                    return;
                                }
                            }
                            if (child->rightChild->getType() == 1) {
                                auto grandChild = (Child *) (child->rightChild);
                                if (grandChild->object->operator==(object)) {
                                    node->leftChild = child->leftChild;
                                    delete grandChild;
                                    refit(node);
                                    return;
                                }
                            }
                        }
                    }
                    if (node->rightChild->getType() == 0) {
                        auto child = (InnerNode *) (node->rightChild);
                        if (contains(child->boundingBox, object->getBoundingBox())) {
                            if (child->leftChild->getType() == 1) {
                                auto grandChild = (Child *) (child->leftChild);
                                if (grandChild->object->operator==(object)) {
                                    node->rightChild = child->rightChild;
                                    delete grandChild;
                                    refit(node);
                                    return;
                                }
                            }
                            if (child->rightChild->getType() == 1) {
                                auto grandChild = (Child *) (child->rightChild);
                                if (grandChild->object->operator==(object)) {
                                    node->rightChild = child->leftChild;
                                    delete grandChild;
                                    refit(node);
                                    return;
                                }
                            }
                        }
                    }
                    remove(node->leftChild, object);
                    remove(node->rightChild, object);

                    refit(node);

                    node->optimizeSAH();
                }
            }
        }

        void remove(Node **currentNode, std::vector<Object *> *objects) {
            if (*currentNode == nullptr) return;
            // find object in tree by insertion
            // remove object and refit nodes going the tree back up
            for (auto &object : *objects) {
                if ((*currentNode)->getType() == 0) {
                    auto node = (InnerNode *) *currentNode;
                    if (node->leftChild->getType() == 1) {
                        auto child = (Child *) (node->leftChild);
                        if (child->object->operator==(object)) {
                            delete child;
                            *currentNode = node->rightChild;
                            continue;
                        }
                    }
                    if (node->rightChild->getType() == 1) {
                        auto child = (Child *) (node->rightChild);
                        if (child->object->operator==(object)) {
                            delete child;
                            *currentNode = node->leftChild;
                            continue;
                        }
                    }
                } else {
                    if (((Child *) *currentNode)->object->operator==(object)) {
                        delete root;
                        root = nullptr;
                        return;
                    }
                }
                remove(*currentNode, object);
            }
        }

        static bool rayBoxIntersection(Vector3D *min, Vector3D *max, Ray *ray, double *distance) {
            double t1 = (min->x - ray->origin.x) * ray->dirfrac.x;
            double t2 = (max->x - ray->origin.x) * ray->dirfrac.x;
            double t3 = (min->y - ray->origin.y) * ray->dirfrac.y;
            double t4 = (max->y - ray->origin.y) * ray->dirfrac.y;
            double t5 = (min->z - ray->origin.z) * ray->dirfrac.z;
            double t6 = (max->z - ray->origin.z) * ray->dirfrac.z;

            double tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
            double tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

            *distance = tmin;
            return tmax >= 0 && tmin <= tmax;
        }

        struct TraversalContainer {
            Node *node;
            double distance;
        };

        static bool traverse(HitInformation *hitInformation, Node *currentNode, Ray *ray, uint8_t maxDepth) {
            bool hit = false;

            TraversalContainer stack[maxDepth];
            uint64_t stackPointer = 0;
            stack[0] = {currentNode, 0};
            stackPointer++;

            while (stackPointer != 0) {
                if (stack[stackPointer - 1].distance >= hitInformation->distance) {
                    stackPointer--;
                    continue;
                }
                auto *node = (InnerNode *) (stack[stackPointer - 1].node);
                stackPointer--;

                double distanceRight = 0;
                double distanceLeft = 0;
                bool right = false;
                bool left = false;

                if (node->rightChild->getType() == 0) {
                    auto *rightChild = (InnerNode *) node->rightChild;
                    right = rayBoxIntersection(&(rightChild->boundingBox.frontBottomLeft),
                                               &(rightChild->boundingBox.backTopRight), ray, &distanceRight);
                } else {
                    HitInformation hitInformationBuffer{};
                    hitInformationBuffer.hit = false;
                    hitInformationBuffer.distance = std::numeric_limits<double>::max();
                    hitInformationBuffer.position = {0, 0, 0};
                    auto *rightChild = (Child *) (node->rightChild);
                    rightChild->object->intersect(&hitInformationBuffer, ray);
                    if (hitInformationBuffer.hit) {
                        if (hitInformationBuffer.distance < hitInformation->distance) {
                            *hitInformation = hitInformationBuffer;
                            hit = true;
                        }
                    }
                }
                if (node->leftChild->getType() == 0) {
                    auto *leftChild = (InnerNode *) node->leftChild;
                    left = rayBoxIntersection(&(leftChild->boundingBox.frontBottomLeft),
                                              &(leftChild->boundingBox.backTopRight), ray, &distanceLeft);
                } else {
                    HitInformation hitInformationBuffer{};
                    hitInformationBuffer.hit = false;
                    hitInformationBuffer.distance = std::numeric_limits<double>::max();
                    hitInformationBuffer.position = {0, 0, 0};
                    auto *leftChild = (Child *) (node->leftChild);
                    leftChild->object->intersect(&hitInformationBuffer, ray);
                    if (hitInformationBuffer.hit) {
                        if (hitInformationBuffer.distance < hitInformation->distance) {
                            *hitInformation = hitInformationBuffer;
                            hit = true;
                        }
                    }
                }

                if (right && left) {
                    if (distanceRight < distanceLeft) {
                        stack[stackPointer++] = {node->leftChild, distanceLeft};
                        stack[stackPointer++] = {node->rightChild, distanceRight};
                    } else {
                        stack[stackPointer++] = {node->rightChild, distanceRight};
                        stack[stackPointer++] = {node->leftChild, distanceLeft};
                    }
                } else if (right) {
                    stack[stackPointer++] = {node->rightChild, distanceRight};
                } else if (left) {
                    stack[stackPointer++] = {node->leftChild, distanceLeft};
                }
            }

            return hit;
        }


        static void createAABB(AABB *aabb, Matrix4x4 *transform) {
            // apply transformation to box
            Vector3D mid = {(aabb->backTopRight.x + aabb->frontBottomLeft.x) / 2,
                            (aabb->backTopRight.y + aabb->frontBottomLeft.y) / 2,
                            (aabb->backTopRight.z + aabb->frontBottomLeft.z) / 2};

            aabb->frontBottomLeft.x -= mid.x;
            aabb->frontBottomLeft.y -= mid.y;
            aabb->frontBottomLeft.z -= mid.z;
            aabb->backTopRight.x -= mid.x;
            aabb->backTopRight.y -= mid.y;
            aabb->backTopRight.z -= mid.z;

            Vector3D frontBottomLeft = aabb->frontBottomLeft;
            Vector3D frontBottomRight = {aabb->backTopRight.x, aabb->frontBottomLeft.y, aabb->frontBottomLeft.z};
            Vector3D frontTopLeft = {aabb->frontBottomLeft.x, aabb->backTopRight.y, aabb->frontBottomLeft.z};
            Vector3D frontTopRight = {aabb->backTopRight.x, aabb->backTopRight.y, aabb->frontBottomLeft.z};
            Vector3D backBottomLeft = {aabb->frontBottomLeft.x, aabb->frontBottomLeft.y, aabb->backTopRight.z};
            Vector3D backBottomRight = {aabb->backTopRight.x, aabb->frontBottomLeft.y, aabb->backTopRight.z};
            Vector3D backTopLeft = {aabb->frontBottomLeft.x, aabb->backTopRight.y, aabb->backTopRight.z};
            Vector3D backTopRight = aabb->backTopRight;

            Vector3D buffer{};

            buffer = frontBottomLeft;
            frontBottomLeft.x = transform->elements[0][0] * buffer.x +
                                transform->elements[0][1] * buffer.y +
                                transform->elements[0][2] * buffer.z +
                                transform->elements[0][3];
            frontBottomLeft.y = transform->elements[1][0] * buffer.x +
                                transform->elements[1][1] * buffer.y +
                                transform->elements[1][2] * buffer.z +
                                transform->elements[1][3];
            frontBottomLeft.z = transform->elements[2][0] * buffer.x +
                                transform->elements[2][1] * buffer.y +
                                transform->elements[2][2] * buffer.z +
                                transform->elements[2][3];
            buffer = frontBottomRight;
            frontBottomRight.x = transform->elements[0][0] * buffer.x +
                                 transform->elements[0][1] * buffer.y +
                                 transform->elements[0][2] * buffer.z +
                                 transform->elements[0][3];
            frontBottomRight.y = transform->elements[1][0] * buffer.x +
                                 transform->elements[1][1] * buffer.y +
                                 transform->elements[1][2] * buffer.z +
                                 transform->elements[1][3];
            frontBottomRight.z = transform->elements[2][0] * buffer.x +
                                 transform->elements[2][1] * buffer.y +
                                 transform->elements[2][2] * buffer.z +
                                 transform->elements[2][3];
            buffer = frontTopLeft;
            frontTopLeft.x = transform->elements[0][0] * buffer.x +
                             transform->elements[0][1] * buffer.y +
                             transform->elements[0][2] * buffer.z +
                             transform->elements[0][3];
            frontTopLeft.y = transform->elements[1][0] * buffer.x +
                             transform->elements[1][1] * buffer.y +
                             transform->elements[1][2] * buffer.z +
                             transform->elements[1][3];
            frontTopLeft.z = transform->elements[2][0] * buffer.x +
                             transform->elements[2][1] * buffer.y +
                             transform->elements[2][2] * buffer.z +
                             transform->elements[2][3];
            buffer = frontTopRight;
            frontTopRight.x = transform->elements[0][0] * buffer.x +
                              transform->elements[0][1] * buffer.y +
                              transform->elements[0][2] * buffer.z +
                              transform->elements[0][3];
            frontTopRight.y = transform->elements[1][0] * buffer.x +
                              transform->elements[1][1] * buffer.y +
                              transform->elements[1][2] * buffer.z +
                              transform->elements[1][3];
            frontTopRight.z = transform->elements[2][0] * buffer.x +
                              transform->elements[2][1] * buffer.y +
                              transform->elements[2][2] * buffer.z +
                              transform->elements[2][3];
            buffer = backBottomLeft;
            backBottomLeft.x = transform->elements[0][0] * buffer.x +
                               transform->elements[0][1] * buffer.y +
                               transform->elements[0][2] * buffer.z +
                               transform->elements[0][3];
            backBottomLeft.y = transform->elements[1][0] * buffer.x +
                               transform->elements[1][1] * buffer.y +
                               transform->elements[1][2] * buffer.z +
                               transform->elements[1][3];
            backBottomLeft.z = transform->elements[2][0] * buffer.x +
                               transform->elements[2][1] * buffer.y +
                               transform->elements[2][2] * buffer.z +
                               transform->elements[2][3];
            buffer = backBottomRight;
            backBottomRight.x = transform->elements[0][0] * buffer.x +
                                transform->elements[0][1] * buffer.y +
                                transform->elements[0][2] * buffer.z +
                                transform->elements[0][3];
            backBottomRight.y = transform->elements[1][0] * buffer.x +
                                transform->elements[1][1] * buffer.y +
                                transform->elements[1][2] * buffer.z +
                                transform->elements[1][3];
            backBottomRight.z = transform->elements[2][0] * buffer.x +
                                transform->elements[2][1] * buffer.y +
                                transform->elements[2][2] * buffer.z +
                                transform->elements[2][3];
            buffer = backTopLeft;
            backTopLeft.x = transform->elements[0][0] * buffer.x +
                            transform->elements[0][1] * buffer.y +
                            transform->elements[0][2] * buffer.z +
                            transform->elements[0][3];
            backTopLeft.y = transform->elements[1][0] * buffer.x +
                            transform->elements[1][1] * buffer.y +
                            transform->elements[1][2] * buffer.z +
                            transform->elements[1][3];
            backTopLeft.z = transform->elements[2][0] * buffer.x +
                            transform->elements[2][1] * buffer.y +
                            transform->elements[2][2] * buffer.z +
                            transform->elements[2][3];
            buffer = backTopRight;
            backTopRight.x = transform->elements[0][0] * buffer.x +
                             transform->elements[0][1] * buffer.y +
                             transform->elements[0][2] * buffer.z +
                             transform->elements[0][3];
            backTopRight.y = transform->elements[1][0] * buffer.x +
                             transform->elements[1][1] * buffer.y +
                             transform->elements[1][2] * buffer.z +
                             transform->elements[1][3];
            backTopRight.z = transform->elements[2][0] * buffer.x +
                             transform->elements[2][1] * buffer.y +
                             transform->elements[2][2] * buffer.z +
                             transform->elements[2][3];

            aabb->frontBottomLeft.x = std::min(std::min(std::min(std::min(
                    std::min(std::min(std::min(backTopRight.x, backTopLeft.x), backBottomRight.x), backBottomLeft.x),
                    frontTopRight.x), frontTopLeft.x), frontBottomRight.x), frontBottomLeft.x);
            aabb->frontBottomLeft.y = std::min(std::min(std::min(std::min(
                    std::min(std::min(std::min(backTopRight.y, backTopLeft.y), backBottomRight.y), backBottomLeft.y),
                    frontTopRight.y), frontTopLeft.y), frontBottomRight.y), frontBottomLeft.y);
            aabb->frontBottomLeft.z = std::min(std::min(std::min(std::min(
                    std::min(std::min(std::min(backTopRight.z, backTopLeft.z), backBottomRight.z), backBottomLeft.z),
                    frontTopRight.z), frontTopLeft.z), frontBottomRight.z), frontBottomLeft.z);
            aabb->backTopRight.x = std::max(std::max(std::max(std::max(
                    std::max(std::max(std::max(backTopRight.x, backTopLeft.x), backBottomRight.x), backBottomLeft.x),
                    frontTopRight.x), frontTopLeft.x), frontBottomRight.x), frontBottomLeft.x);
            aabb->backTopRight.y = std::max(std::max(std::max(std::max(
                    std::max(std::max(std::max(backTopRight.y, backTopLeft.y), backBottomRight.y), backBottomLeft.y),
                    frontTopRight.y), frontTopLeft.y), frontBottomRight.y), frontBottomLeft.y);
            aabb->backTopRight.z = std::max(std::max(std::max(std::max(
                    std::max(std::max(std::max(backTopRight.z, backTopLeft.z), backBottomRight.z), backBottomLeft.z),
                    frontTopRight.z), frontTopLeft.z), frontBottomRight.z), frontBottomLeft.z);

            aabb->frontBottomLeft.x += mid.x;
            aabb->frontBottomLeft.y += mid.y;
            aabb->frontBottomLeft.z += mid.z;
            aabb->backTopRight.x += mid.x;
            aabb->backTopRight.y += mid.y;
            aabb->backTopRight.z += mid.z;
        }

        Node *root;
        AABB boundingBox{};
        Matrix4x4 transform{};
        Matrix4x4 inverseTransform{};
        uint8_t maxDepth;
        bool instance;

    public:

        DBVHv2() {
            boundingBox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                           -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
            transform.elements[0][0] = 1;
            transform.elements[0][1] = 0;
            transform.elements[0][2] = 0;
            transform.elements[0][3] = 0;
            transform.elements[1][0] = 0;
            transform.elements[1][1] = 1;
            transform.elements[1][2] = 0;
            transform.elements[1][3] = 0;
            transform.elements[2][0] = 0;
            transform.elements[2][1] = 0;
            transform.elements[2][2] = 1;
            transform.elements[2][3] = 0;
            transform.elements[3][0] = 0;
            transform.elements[3][1] = 0;
            transform.elements[3][2] = 0;
            transform.elements[3][3] = 1;
            inverseTransform.elements[0][0] = 1;
            inverseTransform.elements[0][1] = 0;
            inverseTransform.elements[0][2] = 0;
            inverseTransform.elements[0][3] = 0;
            inverseTransform.elements[1][0] = 0;
            inverseTransform.elements[1][1] = 1;
            inverseTransform.elements[1][2] = 0;
            inverseTransform.elements[1][3] = 0;
            inverseTransform.elements[2][0] = 0;
            inverseTransform.elements[2][1] = 0;
            inverseTransform.elements[2][2] = 1;
            inverseTransform.elements[2][3] = 0;
            inverseTransform.elements[3][0] = 0;
            inverseTransform.elements[3][1] = 0;
            inverseTransform.elements[3][2] = 0;
            inverseTransform.elements[3][3] = 1;
            root = nullptr;
            maxDepth = 0;
            instance = false;
        };

        DBVHv2(DBVHv2 const &clone) {
            root = clone.root;
            transform = clone.transform;
            inverseTransform = clone.inverseTransform;
            boundingBox = clone.boundingBox;
            maxDepth = clone.maxDepth;
            instance = true;
        }

        ~        DBVHv2() override {
            if (!instance) {
                delete root;
            }
        }

        void applyTransform(Matrix4x4 *newTransform) {
            createAABB(&boundingBox, newTransform);
            transform.multiplyBy(newTransform);
            inverseTransform = transform.getInverse();
        }

        void addObjects(std::vector<Object *> *objects) {
            if (objects->empty()) return;
            if (root == nullptr) {
                if (objects->size() == 1) {
                    root = new Child();
                    ((Child *) root)->object = objects->at(0);

                    AABB aabb = root->getBoundingBox();
                    createAABB(&aabb, &transform);
                    boundingBox = aabb;
                    return;
                } else {
                    root = new InnerNode();
                }
            } else if (root->getType() == 1) {
                objects->push_back(((Child *) (root))->object);
                ((Child *) root)->object = nullptr;
                delete root;
                root = new InnerNode();
            }
            add(root, objects, 1);

            AABB aabb = root->getBoundingBox();
            createAABB(&aabb, &transform);
            boundingBox = aabb;

            std::cout << ((InnerNode *) root)->surfaceArea << std::endl;
        }

        void removeObjects(std::vector<Object *> *objects) {
            remove(&root, objects);

            AABB aabb = root->getBoundingBox();
            createAABB(&aabb, &transform);
            boundingBox = aabb;
        }

        AABB getBoundingBox() override {
            return boundingBox;
        }

        bool intersect(HitInformation *hitInformation, Ray *ray) override {
            Ray newRay = *ray;

            AABB originalAABB{};
            if (root->getType() == 0) {
                originalAABB = ((InnerNode *) this->root)->boundingBox;
            } else {
                originalAABB = ((Child *) this->root)->object->getBoundingBox();
            }

            Vector3D originalMid = {(originalAABB.backTopRight.x + originalAABB.frontBottomLeft.x) / 2,
                                    (originalAABB.backTopRight.y + originalAABB.frontBottomLeft.y) / 2,
                                    (originalAABB.backTopRight.z + originalAABB.frontBottomLeft.z) / 2};

            newRay.origin.x -= originalMid.x;
            newRay.origin.y -= originalMid.y;
            newRay.origin.z -= originalMid.z;

            Vector3D directionBuffer{};
            directionBuffer.x = newRay.origin.x + newRay.direction.x;
            directionBuffer.y = newRay.origin.y + newRay.direction.y;
            directionBuffer.z = newRay.origin.z + newRay.direction.z;

            Vector3D originBuffer = newRay.origin;
            newRay.origin.x = inverseTransform.elements[0][0] * originBuffer.x +
                              inverseTransform.elements[0][1] * originBuffer.y +
                              inverseTransform.elements[0][2] * originBuffer.z +
                              inverseTransform.elements[0][3];
            newRay.origin.y = inverseTransform.elements[1][0] * originBuffer.x +
                              inverseTransform.elements[1][1] * originBuffer.y +
                              inverseTransform.elements[1][2] * originBuffer.z +
                              inverseTransform.elements[1][3];
            newRay.origin.z = inverseTransform.elements[2][0] * originBuffer.x +
                              inverseTransform.elements[2][1] * originBuffer.y +
                              inverseTransform.elements[2][2] * originBuffer.z +
                              inverseTransform.elements[2][3];

            newRay.direction.x = inverseTransform.elements[0][0] * directionBuffer.x +
                                 inverseTransform.elements[0][1] * directionBuffer.y +
                                 inverseTransform.elements[0][2] * directionBuffer.z +
                                 inverseTransform.elements[0][3];
            newRay.direction.y = inverseTransform.elements[1][0] * directionBuffer.x +
                                 inverseTransform.elements[1][1] * directionBuffer.y +
                                 inverseTransform.elements[1][2] * directionBuffer.z +
                                 inverseTransform.elements[1][3];
            newRay.direction.z = inverseTransform.elements[2][0] * directionBuffer.x +
                                 inverseTransform.elements[2][1] * directionBuffer.y +
                                 inverseTransform.elements[2][2] * directionBuffer.z +
                                 inverseTransform.elements[2][3];

            newRay.direction.x = newRay.direction.x - newRay.origin.x;
            newRay.direction.y = newRay.direction.y - newRay.origin.y;
            newRay.direction.z = newRay.direction.z - newRay.origin.z;

            double length = sqrt(newRay.direction.x * newRay.direction.x + newRay.direction.y * newRay.direction.y +
                                 newRay.direction.z * newRay.direction.z);

            newRay.direction.x /= length;
            newRay.direction.y /= length;
            newRay.direction.z /= length;

            newRay.dirfrac.x = 1.0 / newRay.direction.x;
            newRay.dirfrac.y = 1.0 / newRay.direction.y;
            newRay.dirfrac.z = 1.0 / newRay.direction.z;

            newRay.origin.x += originalMid.x;
            newRay.origin.y += originalMid.y;
            newRay.origin.z += originalMid.z;

            double distance;
            bool hit = false;

            if (root == nullptr) return false;

            if (root->getType() == 1) {
                HitInformation hitInformationBuffer{};
                hitInformationBuffer.hit = false;
                hitInformationBuffer.distance = std::numeric_limits<double>::max();
                hitInformationBuffer.position = {0, 0, 0};
                auto *node = (Child *) (root);
                node->object->intersect(&hitInformationBuffer, ray);
                if (hitInformationBuffer.hit) {
                    if (hitInformationBuffer.distance < hitInformation->distance) {
                        *hitInformation = hitInformationBuffer;
                        hit = true;
                    }
                }
            } else {
                if (rayBoxIntersection(&boundingBox.frontBottomLeft, &boundingBox.backTopRight, ray, &distance)) {
                    hit = traverse(hitInformation, root, &newRay, maxDepth);
                }
            }

            if (hit) {
                Vector3D pos = hitInformation->position;

                hitInformation->position.x = transform.elements[0][0] * pos.x +
                                             transform.elements[0][1] * pos.y +
                                             transform.elements[0][2] * pos.z +
                                             transform.elements[0][3];
                hitInformation->position.y = transform.elements[1][0] * pos.x +
                                             transform.elements[1][1] * pos.y +
                                             transform.elements[1][2] * pos.z +
                                             transform.elements[1][3];
                hitInformation->position.z = transform.elements[2][0] * pos.x +
                                             transform.elements[2][1] * pos.y +
                                             transform.elements[2][2] * pos.z +
                                             transform.elements[2][3];

                Vector3D normal = {hitInformation->normal.x + pos.x, hitInformation->normal.y + pos.y,
                                   hitInformation->normal.z + pos.z};

                hitInformation->normal.x = transform.elements[0][0] * normal.x +
                                           transform.elements[0][1] * normal.y +
                                           transform.elements[0][2] * normal.z +
                                           transform.elements[0][3];
                hitInformation->normal.y = transform.elements[1][0] * normal.x +
                                           transform.elements[1][1] * normal.y +
                                           transform.elements[1][2] * normal.z +
                                           transform.elements[1][3];
                hitInformation->normal.z = transform.elements[2][0] * normal.x +
                                           transform.elements[2][1] * normal.y +
                                           transform.elements[2][2] * normal.z +
                                           transform.elements[2][3];

                hitInformation->normal.x = hitInformation->normal.x - hitInformation->position.x;
                hitInformation->normal.y = hitInformation->normal.y - hitInformation->position.y;
                hitInformation->normal.z = hitInformation->normal.z - hitInformation->position.z;

                length = sqrt(hitInformation->normal.x * hitInformation->normal.x +
                              hitInformation->normal.y * hitInformation->normal.y +
                              hitInformation->normal.z * hitInformation->normal.z);

                hitInformation->normal.x /= length;
                hitInformation->normal.y /= length;
                hitInformation->normal.z /= length;

                hitInformation->
                        distance = sqrt(
                        (ray->origin.x - hitInformation->position.x) *
                        (ray->origin.x - hitInformation->position.x) +
                        (ray->origin.y - hitInformation->position.y) *
                        (ray->origin.y - hitInformation->position.y) +
                        (ray->origin.z - hitInformation->position.z) *
                        (ray->origin.z - hitInformation->position.z));
            }
            return hit;
        }

        double getSurfaceArea() override {
            if (root->getType() == 0) {
                return ((InnerNode *) root)->surfaceArea;
            } else {
                return ((Child *) root)->object->getSurfaceArea();
            }
        }

        bool operator==(Object *object) override {
            auto obj = dynamic_cast<DBVHv2 *>(object);
            if (obj == nullptr) {
                return false;
            } else {
                if (obj->root == root) {
                    return obj->transform.elements[0][0] == transform.elements[0][0] &&
                           obj->transform.elements[0][1] == transform.elements[0][1] &&
                           obj->transform.elements[0][2] == transform.elements[0][2] &&
                           obj->transform.elements[0][3] == transform.elements[0][3] &&
                           obj->transform.elements[1][0] == transform.elements[1][0] &&
                           obj->transform.elements[1][1] == transform.elements[1][1] &&
                           obj->transform.elements[1][2] == transform.elements[1][2] &&
                           obj->transform.elements[1][3] == transform.elements[1][3] &&
                           obj->transform.elements[2][0] == transform.elements[2][0] &&
                           obj->transform.elements[2][1] == transform.elements[2][1] &&
                           obj->transform.elements[2][2] == transform.elements[2][2] &&
                           obj->transform.elements[2][3] == transform.elements[2][3] &&
                           obj->transform.elements[3][0] == transform.elements[3][0] &&
                           obj->transform.elements[3][1] == transform.elements[3][1] &&
                           obj->transform.elements[3][2] == transform.elements[3][2] &&
                           obj->transform.elements[3][3] == transform.elements[3][3];
                } else {
                    return false;
                }
            }
        }
    };
}
#endif //DBVH_DBVHV2_H
