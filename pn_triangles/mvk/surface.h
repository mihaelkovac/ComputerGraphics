#ifndef MVK_SURFACE_H
#define MVK_SURFACE_H

#include <vulkan/vulkan.h>

namespace mvk
{

    struct SurfaceInfo
    {

        SurfaceInfo() = default;
        
        SurfaceInfo(VkPhysicalDevice gpu, VkSurfaceKHR surface) noexcept : vk_surface(surface)
        {
            uint32_t count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, nullptr);
            if(count)
            {
                formats.resize(count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &count, formats.data());
            }

            count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, nullptr);
            if(count)
            {
                present_modes.resize(count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, present_modes.data());
            }


            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);
        }

        bool SupportedByGPU() const noexcept
        {
            return !formats.empty() && !present_modes.empty();
        }



        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};
        mutable VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

    };

    template<typename Swapchain>
    struct Surface
    {
        

    private:
        Swapchain swapchain;
        VkSurfaceKHR surface;
    };


} // namespace mvk


#endif