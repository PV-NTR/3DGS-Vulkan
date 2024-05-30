#pragma once

#include "utils/Singleton.hpp"
// #include "managers/commandbuffermanager.hpp"
#include "managers/BufferManager.hpp"
#include "managers/ImageManager.hpp"
// #include "managers/pipelinemanager.hpp"
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

private:
    BufferManager bufferMgr_ {};
    ImageManager imageMgr_ {};
};

} // namespace X::Backend