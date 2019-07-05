//
// Created by sebastian on 08.04.19.
//

#ifndef RAYENGINE_SCENEDATA_H
#define RAYENGINE_SCENEDATA_H


#include "src/OBJ_Loader.h"
#include "API/DataManagementUnit.h"

class SceneData {
private:
    uint64_t *indices;
    Vector3 *vertices;


public:
    int loadFromFile(std::string source);
    void setGeometry();
    void addGeometry();
    void removeGeometry();
    void setTextues();
    void addTexture();
    void removeTexture();
    void addMap();
    void removeMap();
};


#endif //RAYENGINE_SCENEDATA_H
