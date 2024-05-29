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

private:
    std::shared_ptr<BufferManager> bufferMgr_;
    std::shared_ptr<ImageManager> imageMgr_;

};

} // namespace X::Backend