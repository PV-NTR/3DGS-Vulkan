#include "UIOverlay.hpp"

#include "imgui.h"

#include "resources/vk/VkResource.hpp"
#include "resources/vk/Buffer.hpp"
#include "resources/vk/GraphicsPipeline.hpp"
#include "resources/vk/VkResourceManager.hpp"
#include "resources/vk/VkContext.hpp"

namespace X {

UIOverlay::UIOverlay()
{
    // Init ImGui
    ImGui::CreateContext();
    // Color scheme
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    // Dimensions
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale_;
}

UIOverlay::~UIOverlay()
{
    if (ImGui::GetCurrentContext()) {
        ImGui::DestroyContext();
    }
}

void UIOverlay::InitFontImage()
{
    ImGuiIO& io = ImGui::GetIO();
    // Create font texture
    unsigned char* fontData;
    int texWidth;
    int texHeight;
    const std::string filename = GetAssetPath() + "/fonts/Roboto-Medium.ttf";
    io.Fonts->AddFontFromFileTTF(filename.c_str(), 16.0f * scale_);
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
    // SRS - Set ImGui style scale factor to handle retina and other HiDPI displays (same as font scaling above)
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale_);
    resources_.fontImage_ = Backend::VkResourceManager::GetInstance().GetImageManager().RequireImage(
        { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) });
    resources_.fontImage_->Update();
}

void UIOverlay::InitResources(std::shared_ptr<Backend::RenderPass> renderPass)
{
    resources_.pipelineInfo_.name = "UIOverlay";
    resources_.pipelineInfo_.renderPass = renderPass;
    resources_.pipelineInfo_.setLayouts = {};
    Backend::BlendState blendState;
    blendState.colorOp_ = 0;    // func_add
    blendState.alphaOp_ = 0;    // func_add
    blendState.srcColor_ = static_cast<uint32_t>(vk::BlendFactor::eSrcAlpha);
    blendState.dstColor_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusSrcAlpha);
    blendState.srcAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eOneMinusSrcAlpha);
    blendState.dstAlpha_ = static_cast<uint32_t>(vk::BlendFactor::eZero);
    resources_.pipelineInfo_.blend.emplace(blendState);
    resources_.pipelineInfo_.vs = Backend::VkResourceManager::GetInstance().GetShaderManager()
        .AddShaderModule(GetShaderPath() + "/uioverlay/uioverlay.vert", ShaderType::Vertex);
    resources_.pipelineInfo_.fs = Backend::VkResourceManager::GetInstance().GetShaderManager()
        .AddShaderModule(GetShaderPath() + "/uioverlay/uioverlay.frag", ShaderType::Fragment);

    resources_.pipeline_ = Backend::VkResourceManager::GetInstance().GetPipelineTable()
        .RequireGraphicsPipeline(resources_.pipelineInfo_);
    InitFontImage();
    resources_.sampler_ = std::make_shared<Backend::Sampler>(
        Backend::VkContext::GetInstance().GetDevice().createSamplerUnique({}).value);
}

bool UIOverlay::Header(const std::string& caption)
{
    return ImGui::CollapsingHeader(caption.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
}

bool UIOverlay::CheckBox(const std::string& caption, bool& value)
{
    bool res = ImGui::Checkbox(caption.c_str(), &value);
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::CheckBox(const std::string& caption, int32_t& value)
{
    bool val = (value == 1);
    bool res = ImGui::Checkbox(caption.c_str(), &val);
    value = val;
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::RadioButton(const std::string& caption, bool value)
{
    bool res = ImGui::RadioButton(caption.c_str(), value);
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::InputFloat(const std::string& caption, std::vector<float>& value, float step, const std::string format)
{
    bool res = ImGui::InputFloat(caption.c_str(), value.data(), step, step * 10.0f, format.c_str());
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::SliderFloat(const std::string& caption, std::vector<float>& value, float min, float max)
{
    bool res = ImGui::SliderFloat(caption.c_str(), value.data(), min, max);
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::SliderInt(const std::string& caption, std::vector<int32_t>& value, int32_t min, int32_t max)
{
    bool res = ImGui::SliderInt(caption.c_str(), value.data(), min, max);
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::ComboBox(const std::string& caption, std::vector<int32_t>& itemindex, std::vector<std::string> items)
{
    if (items.empty()) {
        return false;
    }
    std::vector<const char*> charitems;
    charitems.reserve(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        charitems.push_back(items[i].c_str());
    }
    uint32_t itemCount = static_cast<uint32_t>(charitems.size());
    bool res = ImGui::Combo(caption.c_str(), itemindex.data(), &charitems[0], itemCount, itemCount);
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::Button(const std::string& caption)
{
    bool res = ImGui::Button(caption.c_str());
    if (res) {
        updated_ = true;
    }
    return res;
}

bool UIOverlay::ColorPicker(const std::string& caption, std::vector<float>& color)
{
    bool res = ImGui::ColorEdit4(caption.c_str(), color.data(), ImGuiColorEditFlags_NoInputs);
    if (res) {
        updated_ = true;
    }
    return res;
}

void UIOverlay::Text(const std::string formatstr, ...)
{
    va_list args;
    va_start(args, formatstr);
    ImGui::TextV(formatstr.c_str(), args);
    va_end(args);
}

void UIOverlay::SwitchVisibility()
{
    visible_ = !visible_;
}

void UIOverlay::Resize(uint32_t width, uint32_t height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)(width), (float)(height));
}

bool UIOverlay::Updated() const
{
    return false;
}

void UIOverlay::PrepareDrawCommands(std::shared_ptr<Backend::CommandBuffer> commandBuffer)
{
    if (!visible_) {
        return;
    }
    auto cmdBuffer = commandBuffer->get();
    ImDrawData* imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;

    if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    cmdBuffer.bindVertexBuffers(0, resources_.vbo_->GetHandle(), {});
    cmdBuffer.bindIndexBuffer(resources_.ibo_->GetHandle(), 0, vk::IndexType::eUint16);

    auto pipeline = resources_.pipeline_;
    // TODO: prepare buffer from scene
    pipeline->BindTextures({std::make_pair(resources_.fontImage_, resources_.sampler_)}, 0, 0);
    pipeline->BindDescriptorSets(commandBuffer);

    resources_.constants.scale[0] = 2.0f / io.DisplaySize.x;
    resources_.constants.scale[1] = 2.0f / io.DisplaySize.y;
    resources_.constants.translate[0] = -1.0f;
    resources_.constants.translate[1] = -1.0f;
    cmdBuffer.pushConstants(resources_.pipeline_->GetLayout(), vk::ShaderStageFlagBits::eVertex, 0,
        sizeof(resources_.constants), &resources_.constants);

    for (uint32_t i = 0; i < imDrawData->CmdListsCount; i++) {
        const ImDrawList* cmd_list = imDrawData->CmdLists[i];
        for (uint32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
            vk::Rect2D scissorRect;
            scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
            scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
            scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
            cmdBuffer.setScissor(0, 1, &scissorRect);
            cmdBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

} // namespace X
