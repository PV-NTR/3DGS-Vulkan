#pragma once

#include <queue>

#include "GraphicsPipeline.hpp"
#include "ComputePipeline.hpp"

namespace X::Backend {

class PipelineTable {
public:
    PipelineTable();
    ~PipelineTable();
    std::shared_ptr<GraphicsPipeline> RequireGraphicsPipeline(const GraphicsPipelineInfo& info);
    std::shared_ptr<ComputePipeline> RequireComputePipeline(const ComputePipelineInfo& info);

protected:
    void CreatePipelineCache();
    void SavePipelineCache();

private:
    static inline constexpr uint32_t TABLE_SIZE = 256;
    static std::string CACHE_FILE;

    std::unordered_map<std::string, std::shared_ptr<Pipeline>> pipelines_;
    std::queue<std::string> pipelineQueue_;
    vk::UniquePipelineCache cache_;
};
    
} // namespace X::Backend
