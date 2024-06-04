#include "BlendState.hpp"

#include "common/VkCommon.hpp"

namespace X::Backend {

const std::array<vk::BlendOp, BlendState::BLEND_OP_NUM> BlendState::blendOpMap = {
    vk::BlendOp::eAdd,
    vk::BlendOp::eSubtract,
    vk::BlendOp::eReverseSubtract,
    vk::BlendOp::eMin,
    vk::BlendOp::eMax,
    vk::BlendOp::eZeroEXT,
    vk::BlendOp::eSrcEXT,
    vk::BlendOp::eDstEXT,
    vk::BlendOp::eSrcOverEXT,
    vk::BlendOp::eDstOverEXT,
    vk::BlendOp::eSrcInEXT,
    vk::BlendOp::eDstInEXT,
    vk::BlendOp::eSrcOutEXT,
    vk::BlendOp::eDstOutEXT,
    vk::BlendOp::eSrcAtopEXT,
    vk::BlendOp::eDstAtopEXT,
    vk::BlendOp::eXorEXT,
    vk::BlendOp::eMultiplyEXT,
    vk::BlendOp::eScreenEXT,
    vk::BlendOp::eOverlayEXT,
    vk::BlendOp::eDarkenEXT,
    vk::BlendOp::eLightenEXT,
    vk::BlendOp::eColordodgeEXT,
    vk::BlendOp::eColorburnEXT,
    vk::BlendOp::eHardlightEXT,
    vk::BlendOp::eSoftlightEXT,
    vk::BlendOp::eDifferenceEXT,
    vk::BlendOp::eExclusionEXT,
    vk::BlendOp::eInvertEXT,
    vk::BlendOp::eInvertRgbEXT,
    vk::BlendOp::eLineardodgeEXT,
    vk::BlendOp::eLinearburnEXT,
    vk::BlendOp::eVividlightEXT,
    vk::BlendOp::eLinearlightEXT,
    vk::BlendOp::ePinlightEXT,
    vk::BlendOp::eHardmixEXT,
    vk::BlendOp::eHslHueEXT,
    vk::BlendOp::eHslSaturationEXT,
    vk::BlendOp::eHslColorEXT,
    vk::BlendOp::eHslLuminosityEXT,
    vk::BlendOp::ePlusEXT,
    vk::BlendOp::ePlusClampedEXT,
    vk::BlendOp::ePlusClampedAlphaEXT,
    vk::BlendOp::ePlusDarkerEXT,
    vk::BlendOp::eMinusEXT,
    vk::BlendOp::eMinusClampedEXT,
    vk::BlendOp::eContrastEXT,
    vk::BlendOp::eInvertOvgEXT,
    vk::BlendOp::eRedEXT,
    vk::BlendOp::eGreenEXT,
    vk::BlendOp::eBlueEXT,
};
   
vk::BlendOp BlendState::GetColorBlendOp() const
{
    return blendOpMap[colorOp_];
}

vk::BlendOp BlendState::GetAlphaBlendOp() const
{
    return blendOpMap[alphaOp_];
}


} // namespace X::Backend
