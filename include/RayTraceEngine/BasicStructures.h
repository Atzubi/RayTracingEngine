//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_BASICSTRUCTURES_H
#define RAYTRACECORE_BASICSTRUCTURES_H

#include <iostream>
#include <string>
#include <limits>
#include <utility>

/**
 * Contains x, y and z coordinates representing a vector in 3 dimensions.
 */
struct Vector3D {
    double x;
    double y;
    double z;

    double operator[](int idx) const {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                std::string message = "Index " + std::to_string(idx) + " is out of range for Vector3D";
                throw std::out_of_range(message);
        }
    }

    double &operator[](int idx) {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                std::string message = "Index " + std::to_string(idx) + " is out of range for Vector3D";
                throw std::out_of_range(message);
        }
    }

    Vector3D operator+(const Vector3D &other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    void operator+=(const Vector3D &other) {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    Vector3D operator+(double scalar) const {
        return {x + scalar, y + scalar, z + scalar};
    }

    void operator+=(double scalar) {
        x += scalar;
        y += scalar;
        z += scalar;
    }

    Vector3D operator-(const Vector3D &other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    void operator-=(const Vector3D &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    Vector3D operator-(double scalar) const {
        return {x - scalar, y - scalar, z - scalar};
    }

    void operator-=(double scalar) {
        x -= scalar;
        y -= scalar;
        z -= scalar;
    }

    Vector3D operator*(const Vector3D &other) const {
        return {x * other.x, y * other.y, z * other.z};
    }

    void operator*=(const Vector3D &other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
    }

    Vector3D operator*(double scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    void operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    Vector3D operator/(const Vector3D &other) const {
        if (other.x == 0 || other.y == 0 || other.z == 0) throw std::invalid_argument("Division by 0!");
        return {x / other.x, y / other.y, z / other.z};
    }

    void operator/=(const Vector3D &other) {
        if (other.x == 0 || other.y == 0 || other.z == 0) throw std::invalid_argument("Division by 0!");
        x /= other.x;
        y /= other.y;
        z /= other.z;
    }

    Vector3D operator/(double scalar) const {
        if (scalar == 0) throw std::invalid_argument("Division by 0!");
        return {x / scalar, y / scalar, z / scalar};
    }

    void operator/=(double scalar) {
        if (scalar == 0) throw std::invalid_argument("Division by 0!");
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    [[nodiscard]] double getLength() const {
        return sqrt(x * x + y * y + z * z);
    }

    void normalize() {
        *this /= getLength();
    }

    [[nodiscard]] Vector3D getInverse() const {
        return {1.0 / x, 1.0 / y, 1.0 / z};
    }
};

/**
 * Contains x and y coordinates representing a vector in 2 dimensions.
 */
struct Vector2D {
    double x;
    double y;
};

struct GeneratorRay {
    Vector3D rayOrigin;
    Vector3D rayDirection;
};

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
        Vector3D result{};
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

/**
 * Contains additional parameters of an object that are used when constructing the data structure for rendering.
 * bounding:        a parameter used for describing the looseness of an objects bounding, higher values create
 *                  bigger boxes that cripple general rendering performance but speed up reconstructing the data
 *                  structure on an object update (animations)
 */
struct ObjectParameter {
    double bounding;
};

/**
 * Container for an axis aligned bounding box.
 * minCorner:   The corner with minimum values.
 * maxCorner:   The corner with maximum values.
 */
struct BoundingBox {
    Vector3D minCorner = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::max()};
    Vector3D maxCorner = {-std::numeric_limits<double>::max(),
                          -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};

    /**
     * Computes the surface area of the axis aligned bounding box.
     * @return  The surface area divided by two.
     */
    [[nodiscard]] double getSA() const {
        return (maxCorner.x - minCorner.x) * (maxCorner.y - minCorner.y) +
               (maxCorner.x - minCorner.x) * (maxCorner.z - minCorner.z) +
               (maxCorner.y - minCorner.y) * (maxCorner.z - minCorner.z);
    }
};

/**
 * Container for storing an image/texture.
 * name:    The name of the texture.
 * w:       The horizontal resolution of the texture.
 * h:       The vertical resolution of the texture.
 * image:   Byte sized rgb values. Every 3 chars define the color of pixel.
 */
struct Texture {
    std::string name;
    int w;
    int h;
    std::vector<unsigned char> image;
};

/**
 * Material of an object.
 */
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

/**
 * Container of a ray.
 * origin:      Origin of the ray.
 * direction:   Direction of the ray.
 * dirfrac:     1/direction of the ray. (Performance optimization)
 */
struct Ray {
    Vector3D origin, direction, dirfrac;
};

#endif //RAYTRACECORE_BASICSTRUCTURES_H
