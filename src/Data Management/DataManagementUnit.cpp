//
// Created by sebastian on 28.02.20.
//

#include <API/Pipeline.h>
#include <vector>
#include <API/Object.h>
#include "DataManagementUnit.h"

DataManagementUnit::DataManagementUnit(){
    objectIds.insert(0);
    shaderIds.insert(0);
    pipelineIds.insert(0);
}

DataManagementUnit::~DataManagementUnit() {

}

int DataManagementUnit::addPipeline(Pipeline &pipeline) {
    pipelines.insert(std::pair<int, Pipeline*>(*pipelineIds.begin(), &pipeline));
    int buffer = pipelineIds.extract(pipelineIds.begin()).value();
    if (pipelineIds.empty()) {
        pipelineIds.insert(buffer + 1);

        std::cout << "test " << buffer + 1 << std::endl;
    }
    return buffer;
}

bool DataManagementUnit::removePipeline(int id) {
    if(pipelines.count(id) == 0)
        return false;

    pipelines.erase(id);
    pipelineIds.insert(id);

    auto iterator =  pipelineIds.rbegin();
    int end = *iterator - 1;

    int buffer = *iterator;
    while(end-- == *++iterator){
        pipelineIds.erase(buffer);
        buffer = *iterator;
    }

    return true;
}

bool DataManagementUnit::bindGeometryToPipeline(int pipelineId, std::vector<int> *objectIds) {
    return false;
}

bool DataManagementUnit::bindShaderToPipeline(int pipelineId, int shaderId, std::vector<int> shaderResourceIds) {
    return false;
}

int DataManagementUnit::addObject(const Object &object, Vector3D position, Vector3D orientation, double newScaleFactor,
                                  ObjectParameter objectParameter) {
    return 0;
}

bool DataManagementUnit::removeObject(int id) {
    return false;
}

bool DataManagementUnit::updateObject(int id, const Object &object) {
    return false;
}

int DataManagementUnit::addShader(Shader shader) {
    return 0;
}

bool DataManagementUnit::removeShader(int id) {
    return false;
}

int DataManagementUnit::addShaderResource(Any resource) {
    return 0;
}

bool DataManagementUnit::removeShaderResource(int id) {
    return false;
}

