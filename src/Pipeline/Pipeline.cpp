//
// Created by sebastian on 02.07.19.
//

#include <iostream>
#include "../../API/RayEngine.h"

Pipeline::Pipeline() {
    width = 0;
    height = 0;
}

Pipeline::~Pipeline() = default;

void Pipeline::setResolution(int resolutionWidth, int resolutionHeight) {
    width = resolutionWidth;
    height = resolutionHeight;
}


