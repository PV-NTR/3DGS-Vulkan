#pragma once

#include "Renderer.hpp"
#include "resources/vk/RenderPass.hpp"
#include "resources/vk/Buffer.hpp"
#include "resources/vk/GraphicsPipeline.hpp"
#include "resources/vk/ComputePipeline.hpp"

namespace X {

class GaussianRenderer : public Renderer {
public:
    GaussianRenderer();

protected:
    void InitAuxiliaryBuffers(Scene* scene) override;
    void RecordComputeCommands(Scene* scene) override;

    void SubmitGraphicsCommands() override;
    void SubmitComputeCommands();

    bool OnInit(Backend::DisplaySurface* surface) override;
    void OnRecordGraphicsCommands(Scene* scene, vk::CommandBuffer cmdBuffer) override;
    void OnDrawFrame() override;

    void SetDescriptorSetLayouts();
    void SetBlendState();
    void CreateGraphicsPipeline(std::shared_ptr<Backend::RenderPass> renderPass);
    void CreateComputePipeline();

private:
    static const std::vector<float> vboData_;
    static const std::vector<uint16_t> iboData_;

    // static const std::vector<uint32_t> iboData_;
    Backend::GraphicsPipelineInfo graphicsPipelineInfo_;
    Backend::ComputePipelineInfo computePipelineInfo_;

    std::shared_ptr<Backend::Buffer> vbo_;
    std::shared_ptr<Backend::Buffer> ibo_;

    std::shared_ptr<Backend::Buffer> preComputed_;

    std::shared_ptr<Backend::GraphicsPipeline> pipeline_;
    std::shared_ptr<Backend::ComputePipeline> computePipeline_;
    std::vector<std::shared_ptr<Backend::DescriptorSetLayout>> descriptorSetLayouts_;
    std::optional<Backend::BlendState> blend_;
    vk::Semaphore preprocessComplete_;
};
    
} // namespace X
