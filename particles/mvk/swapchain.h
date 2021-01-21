#ifndef MVK_SWAPCHAIN_H
#define MVK_SWAPCHAIN_H

#include <vulkan/vulkan.h>

#include <vector>


namespace mvk
{


    struct Swapchain
    {
        
        template<typename Device>
        void init(Device& device,
                  const VkSwapchainCreateInfoKHR& swapchainInfo) noexcept
        {
            vkSwapchain = device.createSwapchain(swapchainInfo);
            
            uint32_t count = 0;
            vkGetSwapchainImagesKHR(device, vkSwapchain, &count, nullptr);

            if(count)
            {
                images.resize(count);
                VkImage* vkImages = reinterpret_cast<VkImage*>(images.data());
                vkGetSwapchainImagesKHR(device, vkSwapchain, &count, vkImages);

                for(uint32_t i = count - 1; i > 0; --i)
                {
					VkImage image = vkImages[i];
					SwapchainImage& swapchainImage = images[i];
                    vkImages[i] = VK_NULL_HANDLE;
					swapchainImage.vkImage = image;

                }

            	for(uint32_t i = 0; i < count; ++i)
            	{
                    SwapchainImage& swapchainImage = images[i];
                    VkComponentMapping mapping = createComponentMapping();

                    VkImageSubresourceRange subresourceRange = createSubresourceRange(ImageAspect::Color);
                    
                    swapchainImage.vkImageView = device.createImageView(swapchainImage.vkImage,
                        VK_IMAGE_VIEW_TYPE_2D,
                        format.format,
                        mapping,
                        subresourceRange);
            	}
            }
        }

    	

        void release(VkDevice device, VkAllocationCallbacks* cbs) noexcept
        {
            for(auto& image : images)
            {
                vkDestroyImageView(device, image.vkImageView, cbs);

            }

            vkDestroySwapchainKHR(device, vkSwapchain, cbs);
        }

        struct SwapchainImage
        {
            VkImage vkImage;
            VkImageView vkImageView;  
        };


        VkSwapchainKHR vkSwapchain{ 0 };
        VkSurfaceFormatKHR format{};
        VkExtent2D extent{};
        VkPresentModeKHR presentMode{};
        
        std::vector<SwapchainImage> images{};
        
    };



    
    
} // namespace mvk


#endif