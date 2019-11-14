//
// Created by sebastian on 13.11.19.
//

#ifndef RAYTRACECORE_BASICSTRUCTURES_H
#define RAYTRACECORE_BASICSTRUCTURES_H

#include <iostream>

/**
 * Contains x, y and z coordinates representing a vector in 3 dimensions
 */
struct Vector3D {
    double x;
    double y;
    double z;
};

struct BoundingBox {
    Vector3D firstCorner, secondCorner;
};

struct IntersectionInfo {

};

struct Ray {
    Vector3D origin, direction;
    void *metaData;
};


class Any {
public:
    Any() : content(nullptr) {}

    ~Any() {
        delete content;
    }

    Any(const Any& rhs){
        content = rhs.content->clone();
    }
    //Any& operator=(const Any& rhs){};

    template<typename value_type>
    explicit Any(const value_type &value) : content(new holder<value_type>(value)) {}

    template<typename value_type>
    bool copy_to(value_type &value) const {
        const value_type *copyable =
                to_ptr<value_type>();
        if (copyable)
            value = *copyable;
        return copyable;
    }

private:
    [[nodiscard]] const std::type_info &type_info() const {
        return content ? content->type_info() : typeid(void);
    }

    template<typename value_type>
    const value_type *to_ptr() const {
        return type_info() == typeid(value_type) ? &static_cast< holder <value_type> *>(content)->held : 0;
    }

    class placeholder {
    public:
        virtual ~placeholder() = default;

        [[nodiscard]] virtual const std::type_info &type_info() const = 0;

        [[nodiscard]] virtual placeholder *clone() const = 0;
    };

    template<typename value_type>
    class holder : public placeholder {
    public:
        explicit holder(const value_type &value) : held(value) {}

        [[nodiscard]] const std::type_info &type_info() const override {
            return typeid(value_type);
        }

        [[nodiscard]] virtual placeholder *clone() const {
            return new holder(held);
        }

        const value_type held;
    };

    placeholder *content;
};

#endif //RAYTRACECORE_BASICSTRUCTURES_H
