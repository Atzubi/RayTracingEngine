//
// Created by Sebastian on 18.02.2022.
//

#ifndef RAYTRACEENGINE_MATRIX4X4_H
#define RAYTRACEENGINE_MATRIX4X4_H

#include "RayTraceEngine/Vector3D.h"

/**
 * Contains a 4 by 4 matrix.
 * elements:    The matrix.
 */
struct Matrix4x4 {
    double elements[4][4];

    /**
     * Multiplies another matrix with this matrix. Stores the result in the current matrix.
     * @param matrix    Another matrix.
     */
    void multiplyBy(const Matrix4x4 &matrix) {
        double e[4][4];
        for (int row = 0; row < 4; row++) {
            for (int column = 0; column < 4; column++) {
                e[row][column] = 0;
                for (int index = 0; index < 4; index++) {
                    e[row][column] += elements[row][index] * matrix.elements[index][column];
                }
            }
        }
        for (int row = 0; row < 4; row++) {
            for (int column = 0; column < 4; column++) {
                elements[row][column] = e[row][column];
            }
        }
    }

    Vector3D operator*(const Vector3D &vector) const {
        Vector3D result;
        for (int line = 0; line < 3; line++) {
            result[line] = elements[line][0] * vector.x +
                           elements[line][1] * vector.y +
                           elements[line][2] * vector.z +
                           elements[line][3];
        }
        return result;
    }

    /**
     * Computes the inverse of this matrix.
     * @return  The inverse of this matrix.
     */
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

    static Matrix4x4 getIdentity() {
        Matrix4x4 identity{};

        identity.elements[0][0] = 1;
        identity.elements[0][1] = 0;
        identity.elements[0][2] = 0;
        identity.elements[0][3] = 0;
        identity.elements[1][0] = 0;
        identity.elements[1][1] = 1;
        identity.elements[1][2] = 0;
        identity.elements[1][3] = 0;
        identity.elements[2][0] = 0;
        identity.elements[2][1] = 0;
        identity.elements[2][2] = 1;
        identity.elements[2][3] = 0;
        identity.elements[3][0] = 0;
        identity.elements[3][1] = 0;
        identity.elements[3][2] = 0;
        identity.elements[3][3] = 1;

        return identity;
    }
};

#endif //RAYTRACEENGINE_MATRIX4X4_H
