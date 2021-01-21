#ifndef FORMATS_H
#define FORMATS_H

#include <initializer_list>

#include <vulkan/vulkan.h>

VkFormat findSupportedFormat(std::initializer_list<VkFormat> candidates,
                             VkPhysicalDevice gpu,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features) noexcept
{
    for(VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }

    }

    return VkFormat::VK_FORMAT_MAX_ENUM;

}

bool hasStencilComponent(VkFormat format) noexcept
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


#endif