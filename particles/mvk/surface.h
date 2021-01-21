#ifndef MVK_SURFACE_H
#define MVK_SURFACE_H

#include <vulkan/vulkan.h>

namespace mvk
{

    struct SurfaceInfo
    {

        SurfaceInfo() = default;
        
        SurfaceInfo(VkPhysicalDevice gpu, VkSurfaceKHR surface) noexcept : vkSurface(surface)
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
                presentModes.resize(count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, presentModes.data());
            }


            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);
        }

        bool supportedByGPU() const noexcept
        {
            return !formats.empty() && !presentModes.empty();
        }



        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
        mutable VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

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