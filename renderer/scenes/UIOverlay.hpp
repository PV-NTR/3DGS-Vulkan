#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include "resources/vk/VkResource.hpp"
#include "resources/vk/GraphicsPipeline.hpp"

namespace X {

namespace Backend {
class Buffer;
class Image;
class RenderPass;
} // namespace Backend

class UIOverlay {
public:
    UIOverlay();
    ~UIOverlay();

    void InitResources(std::shared_ptr<Backend::RenderPass> renderPass);

    void Resize(uint32_t width, uint32_t height);
    void SwitchVisibility();
    void PrepareDrawCommands(std::shared_ptr<Backend::CommandBuffer> commandBuffer);

    bool IsVisible() const { return visible_; }
    bool Updated() const;

    bool Header(const std::string& caption);
    bool CheckBox(const std::string& caption, bool& value);
    bool CheckBox(const std::string& caption, int32_t& value);
    bool RadioButton(const std::string& caption, bool value);
    bool InputFloat(const std::string& caption, std::vector<float>& value, float step,
        const std::string format = "%.3f");
    bool SliderFloat(const std::string& caption, std::vector<float>& value, float min, float max);
    bool SliderInt(const std::string& caption, std::vector<int32_t>& value, int32_t min, int32_t max);
    bool ComboBox(const std::string& caption, std::vector<int32_t>& itemindex, std::vector<std::string> items);
    bool Button(const std::string& caption);
    bool ColorPicker(const std::string& caption, std::vector<float>& color);
    void Text(const std::string formatstr, ...);

protected:
    void InitFontImage();

private:
    bool visible_ = true;
    bool updated_ = false;
    float scale_ = 1.0f;

    struct {
        struct {
            float scale[2];
            float translate[2];
        } constants;

        Backend::GraphicsPipelineInfo pipelineInfo_ {};
        std::shared_ptr<Backend::GraphicsPipeline> pipeline_;
        std::shared_ptr<Backend::Buffer> vbo_;
        std::shared_ptr<Backend::Buffer> ibo_;
        std::shared_ptr<Backend::Image> fontImage_;
        std::shared_ptr<Backend::Sampler> sampler_;
    } resources_;
};

} // namespace X