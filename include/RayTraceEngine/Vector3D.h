//
// Created by Sebastian on 18.02.2022.
//

#ifndef RAYTRACEENGINE_VECTOR3D_H
#define RAYTRACEENGINE_VECTOR3D_H

#include <iostream>
#include <string>

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
        double length = getLength();
        if(length != 0)
            *this /= length;
    }

    [[nodiscard]] Vector3D getInverse() const {
        return {1.0 / x, 1.0 / y, 1.0 / z};
    }

    [[nodiscard]] Vector3D cross(const Vector3D &other) const {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }

    [[nodiscard]] double sum() const {
        return x + y + z;
    }
};

#endif //RAYTRACEENGINE_VECTOR3D_H
