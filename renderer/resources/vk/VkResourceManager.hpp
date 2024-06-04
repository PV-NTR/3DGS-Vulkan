#pragma once

#include "utils/Singleton.hpp"
#include "managers/BufferManager.hpp"
#include "managers/ImageManager.hpp"
#include "managers/ShaderManager.hpp"
#include "managers/PipelineManager.hpp"
// #include "managers/renderpassmanager.hpp"

namespace X::Backend {

class VkResourceManager : public Singleton<VkResourceManager> {
public:
    BufferManager& GetBufferManager()
    {
        return bufferMgr_;
    }

    ImageManager& GetImageManager()
    {
        return imageMgr_;
    }

    ShaderManager& GetShaderManager()
    {
        return shaderMgr_;
    }

    PipelineManager& GetPipelineManager()
    {
        return pipelineMgr_;
    }

private:
    BufferManager bufferMgr_ {};
    ImageManager imageMgr_ {};
    ShaderManager shaderMgr_ {};
    PipelineManager pipelineMgr_ {};
};

} // namespace X::Backend