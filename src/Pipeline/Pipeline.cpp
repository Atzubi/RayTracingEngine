//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "../../API/RayEngine.h"

Pipeline::Pipeline() {
    width = 0;
    height = 0;
    missShader = nullptr;
    hitShader = nullptr;
    pierceShader = nullptr;
    occlusionShader = nullptr;
    rayGeneratorShader = nullptr;
    controlShader = nullptr;
}

Pipeline::~Pipeline() = default;

void Pipeline::setResolution(int resolutionWidth, int resolutionHeight) {
    width = resolutionWidth;
    height = resolutionHeight;
}

bool Pipeline::setRayGeneratorShader(RayGeneratorShader *rayGeneratorShader) {
    if(this->rayGeneratorShader != nullptr)
        return false;
    else{
        this->rayGeneratorShader = rayGeneratorShader;
    }
    return true;
}

bool Pipeline::setOcclusionShader(OcclusionShader *occlusionShader) {
    if(this->occlusionShader != nullptr)
        return false;
    else{
        this->occlusionShader = occlusionShader;
    }
    return true;
}

bool Pipeline::setPierceShader(PierceShader *pierceShader) {
    if(this->pierceShader != nullptr)
        return false;
    else{
        this->pierceShader = pierceShader;
    }
    return true;
}

bool Pipeline::setHitShader(HitShader *hitShader) {
    if(this->hitShader != nullptr)
        return false;
    else{
        this->hitShader = hitShader;
    }
    return true;
}

bool Pipeline::setMissShader(MissShader *missShader) {
    if(this->missShader != nullptr)
        return false;
    else{
        this->missShader = missShader;
    }
    return true;
}

bool Pipeline::setControlShader(ControlShader *controlShader) {
    if(this->controlShader != nullptr)
        return false;
    else{
        this->controlShader = controlShader;
    }
    return true;
}


