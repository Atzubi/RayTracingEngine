//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "../../API/RayEngine.h"

Pipeline::Pipeline(int resolutionWidth, int resolutionHeight) {
    this->width = resolutionWidth;
    this->height = resolutionHeight;
}

Pipeline::~Pipeline() = default;

void Pipeline::setResolution(int resolutionWidth, int resolutionHeight) {
    width = resolutionWidth;
    height = resolutionHeight;
}


