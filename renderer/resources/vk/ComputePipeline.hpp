#pragma once

#include "Pipeline.hpp"
#include "ShaderModule.hpp"

namespace X::Backend {

struct ComputePipelineInfo {
    std::string name;
    std::vector<std::shared_ptr<DescriptorSetLayout>> setLayouts;
    std::shared_ptr<ShaderModule> cs;
};

class ComputePipeline : public Pipeline {
private:
    friend class PipelineTable;

public:
    ~ComputePipeline() override = default;

protected:
    ComputePipeline(const ComputePipelineInfo& info, vk::PipelineCache cache) noexcept;
};

} // namespace X::Backend