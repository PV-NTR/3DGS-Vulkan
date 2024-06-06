#include "PipelineTable.hpp"

#ifdef HOST_ANDROID
#elif defined HOST_WIN32
#include <UserEnv.h>
#endif // HOST_WIN32

#include <fstream>

#include "VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

std::string PipelineTable::CACHE_FILE = "";

PipelineTable::PipelineTable()
{
#ifdef HOST_ANDROID
    // TODO: get app data path as cache path
#elif defined HOST_WIN32
    char buf[MAX_PATH] = { '\0' };
    auto ret = GetEnvironmentVariable("HOME", buf, MAX_PATH);
    if (!ret) {
        CACHE_FILE = std::string(buf) + "/.3dgs/pipeline.cache";
    }
#endif
    CreatePipelineCache();
}

PipelineTable::~PipelineTable()
{
    SavePipelineCache();
}

std::shared_ptr<GraphicsPipeline> PipelineTable::RequireGraphicsPipeline(const GraphicsPipelineInfo& info)
{
    if (pipelines_.find(info.name) == pipelines_.end()) {
        if (pipelines_.size() >= TABLE_SIZE) {
            std::string name = pipelineQueue_.front();
            pipelineQueue_.pop();
            pipelines_.erase(name);
        }
        pipelines_[info.name] = std::shared_ptr<GraphicsPipeline>(new GraphicsPipeline(info, *cache_));
    }
    assert(pipelines_[info.name] != nullptr);
    return std::static_pointer_cast<GraphicsPipeline>(pipelines_[info.name]);
}

std::shared_ptr<ComputePipeline> PipelineTable::RequireComputePipeline(const ComputePipelineInfo& info)
{
    if (pipelines_.find(info.name) == pipelines_.end()) {
        if (pipelines_.size() >= TABLE_SIZE) {
            std::string name = pipelineQueue_.front();
            pipelineQueue_.pop();
            pipelines_.erase(name);
        }
        pipelines_[info.name] = std::shared_ptr<ComputePipeline>(new ComputePipeline(info, *cache_));
    }
    assert(pipelines_[info.name] != nullptr);
    return std::static_pointer_cast<ComputePipeline>(pipelines_[info.name]);
}

void PipelineTable::CreatePipelineCache()
{
    std::ifstream is(CACHE_FILE, std::ios::binary | std::ios::in | std::ios::ate);
    if (!is.is_open()) {
        XLOGW("Cachefile not exist!");
        return;
    }

    size_t size = is.tellg();
    assert(size > 0);
    std::vector<char> cacheData(size);
    is.seekg(0, std::ios::beg);
    is.read(cacheData.data(), size);
    is.close();
    vk::PipelineCacheCreateInfo cacheCI {};
    
    cacheCI.setInitialDataSize(size).setPInitialData(cacheData.data());
    auto [ret, cache] = VkContext::GetInstance().GetDevice().createPipelineCacheUnique(cacheCI);
    if (ret != vk::Result::eSuccess) {
        XLOGW("CreatePipelineCache failed, errCode: %d", ret);
        return;
    }
    cache_.swap(cache);
}

void PipelineTable::SavePipelineCache()
{
    if (!cache_) {
        XLOGW("No pipeline cache to save!");
        return;
    }
    size_t cacheSize = 0;
    auto ret = VkContext::GetInstance().GetDevice().getPipelineCacheData(*cache_, &cacheSize, nullptr);
    if (ret != vk::Result::eSuccess) {
        XLOGW("GetPipelineCacheData size failed, errCode: %d", ret);
        return;
    }

    std::vector<char> cacheData(cacheSize);
    ret = VkContext::GetInstance().GetDevice().getPipelineCacheData(*cache_, &cacheSize, cacheData.data());
    if (ret != vk::Result::eSuccess) {
        XLOGW("GetPipelineCacheData size failed, errCode: %d", ret);
        return;
    }

    std::ofstream os(CACHE_FILE, std::ios::out | std::ios::trunc);
    os.write(cacheData.data(), cacheSize);
    os.close();
}

} // namespace X::Backend
