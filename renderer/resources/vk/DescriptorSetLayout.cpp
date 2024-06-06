#include "DescriptorSetLayout.hpp"

#include "common/LogCommon.hpp"
#include "VkContext.hpp"

namespace X::Backend {

const std::array<vk::DescriptorType, 4> DescriptorSetLayout::descTypeMap_ = {
	vk::DescriptorType::eCombinedImageSampler,
	vk::DescriptorType::eUniformBuffer,
	vk::DescriptorType::eStorageBuffer,
	vk::DescriptorType::eInputAttachment
};

const std::array<vk::ShaderStageFlagBits, 14> DescriptorSetLayout::shaderStageMap_ = {
    vk::ShaderStageFlagBits::eVertex,
    vk::ShaderStageFlagBits::eTessellationControl,
    vk::ShaderStageFlagBits::eTessellationEvaluation,
    vk::ShaderStageFlagBits::eGeometry,
    vk::ShaderStageFlagBits::eFragment,
    vk::ShaderStageFlagBits::eCompute,
    vk::ShaderStageFlagBits::eRaygenKHR,
    vk::ShaderStageFlagBits::eAnyHitKHR,
    vk::ShaderStageFlagBits::eClosestHitKHR,
    vk::ShaderStageFlagBits::eMissKHR,
    vk::ShaderStageFlagBits::eIntersectionKHR,
    vk::ShaderStageFlagBits::eCallableKHR,
    vk::ShaderStageFlagBits::eTaskEXT,
    vk::ShaderStageFlagBits::eMeshEXT,
};

DescriptorSetLayout::DescriptorSetLayout(const LayoutBindingInfo& bindingInfo) : bindingInfo_(bindingInfo)
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	for (const auto& binding : bindingInfo) {
		if (binding.second != 0) {
			bindings.emplace_back();
			vk::ShaderStageFlags flags {};
			for (uint32_t i = 0; i < shaderStageMap_.size(); i++) {
				if (i & binding.first.stage) {
					flags |= shaderStageMap_[i];
				}
			}
			bindings.back().setDescriptorType(descTypeMap_[binding.first.stage])
				.setDescriptorCount(binding.second).setStageFlags(flags);
		}
	}
	vk::DescriptorSetLayoutCreateInfo layoutCI {};
	layoutCI.setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool).setBindings(bindings);
	auto [ret, uniqueDescLayout] = VkContext::GetInstance().GetDevice().createDescriptorSetLayoutUnique(layoutCI);
	if (ret != vk::Result::eSuccess) {
		XLOGE("CreateDescriptorSetLayout failed, errCode: %d", ret);
		return;
	}
	uniqueDescLayout_.swap(uniqueDescLayout);
	descLayout_ = *uniqueDescLayout_;
}

DescriptorSetLayout::~DescriptorSetLayout()
{

}

} // namespace X::Backend
