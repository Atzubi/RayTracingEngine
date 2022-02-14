template<class ID>
requires isShaderId<ID>
bool
EngineNode::bindShaderToPipeline(PipelineId pipelineId, ID shaderId, const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);
    auto shader = pipelinePool->getShader(shaderId);

    if (pipeline == nullptr || shader == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    pipeline->addShader(shaderId, {shader, shaderResources});

    return true;
}

template<class ID>
requires isShaderId<ID>
bool EngineNode::updatePipelineShader(PipelineId pipelineId, ID shaderId,
                                      const std::vector<ShaderResourceId> &resourceIds) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    std::vector<ShaderResource *> shaderResources;

    shaderResources.reserve(resourceIds.size());
    for (auto shaderResource: resourceIds) {
        shaderResources.push_back(pipelinePool->getShaderResource(shaderResource));
    }

    return pipeline->updateShader(shaderId, shaderResources);
}

template<class ID>
requires isShaderId<ID>
bool EngineNode::removePipelineShader(PipelineId pipelineId, ID shaderId) {
    auto pipeline = pipelinePool->getPipelineFragment(pipelineId);

    if (pipeline == nullptr) return false;

    return pipeline->removeShader(shaderId);
}