#ifndef MVK_SWAPCHAIN_H
#define MVK_SWAPCHAIN_H

#include <vulkan/vulkan.h>

#include <vector>

#include "utils.h"


namespace mvk
{


    struct Swapchain
    {
        
        /*template<typename Device>
        void Init(Device& device,
                  const VkSwapchainCreateInfoKHR& swapchain_info) noexcept
        {
            vk_swapchain = device.CreateSwapchain(swapchain_info);
            
            uint32_t count = 0;
            vkGetSwapchainImagesKHR(device, vk_swapchain, &count, nullptr);

            if(count)
            {
                images.resize(count);
                VkImage* vkImages = reinterpret_cast<VkImage*>(images.data());
                vkGetSwapchainImagesKHR(device, vk_swapchain, &count, vkImages);

                for(uint32_t i = count - 1; i > 0; --i)
                {
					VkImage image = vkImages[i];
					SwapchainImage& swapchain_image = images[i];
                    vkImages[i] = VK_NULL_HANDLE;
					swapchain_image.vk_image = image;

                }

            	for(uint32_t i = 0; i < count; ++i)
            	{
                    SwapchainImage& swapchain_image = images[i];
                    VkComponentMapping mapping = NewComponentMapping();

                    VkImageSubresourceRange subresource_range = NewSubresourceRange(ImageAspect::Color);
                    
                    swapchain_image.vk_image_view = device.CreateImageView(swapchain_image.vk_image,
                        VK_IMAGE_VIEW_TYPE_2D,
                        format.format,
                        mapping,
                        subresource_range);
            	}
            }
        }*/

        void Release(VkDevice device, VkAllocationCallbacks* cbs) noexcept
        {
            for(auto& image : images)
            {
                vkDestroyImageView(device, image.vk_image_view, cbs);

            }

            vkDestroySwapchainKHR(device, vk_swapchain, cbs);
        }

    	operator VkSwapchainKHR() const noexcept
        {
            return vk_swapchain;
        }

    	VkSwapchainKHR GetVkSwapchain() const noexcept
        {
            return vk_swapchain;
        }

        size_t GetImageCount() const noexcept
        {
            return images.size();
        }

        struct SwapchainImage
        {
            VkImage vk_image;
            VkImageView vk_image_view;  
        };


        VkSwapchainKHR vk_swapchain{ 0 };
        VkSurfaceFormatKHR format{};
        VkExtent2D extent{};
        VkPresentModeKHR present_mode{};
        
        std::vector<SwapchainImage> images{};
        
    };


    enum class SwapchainCreateFlag
    {
		DontCare = 0,
        SplitInstanceBindRegions = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR,
        Protected = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR,
        MutableFormat = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR
	};

    using SwapchainCreateFlags = util::EnumFlags<SwapchainCreateFlag, VkSwapchainCreateFlagsKHR>;

	enum class CompositeAlpha
	{
		DontCare = 0,
        Opaque = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        PreMultiplied = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        PostMultiplied = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        Inherit = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
	};
	
	struct SwapchainBuilder
	{

		SwapchainBuilder(VkSurfaceKHR surface, VkPhysicalDevice gpu) noexcept : surface_info_(gpu, surface), gpu_(gpu)
		{
            swapchain_create_info_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchain_create_info_.surface = surface;
            swapchain_create_info_.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            swapchain_create_info_.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swapchain_create_info_.imageArrayLayers = 1;
            swapchain_create_info_.clipped = VK_TRUE;
		}


        SwapchainBuilder& SetPreferedPresentMode(const VkPresentModeKHR present_mode) noexcept
        {
            prefered_present_mode_ = present_mode;
            return *this;
        }

		SwapchainBuilder& SetPreferredFormatAndColorspace(const VkFormat format, const VkColorSpaceKHR color_space) noexcept
        {
            format_ = format;
            color_space_ = color_space;
            return *this;
        }
		
        SwapchainBuilder& SetRequiredFormatAndColorspace(const VkFormat format, const VkColorSpaceKHR color_space) noexcept
        {
            required_format_ = format;
            required_color_space_ = color_space;
            return *this;
        }
		
		SwapchainBuilder& SetSurfaceExtent(int width, int height) noexcept
        {
            width_ = width;
            height_ = height;
            return *this;
        }

		SwapchainBuilder& SetSurface(VkSurfaceKHR surface) noexcept
		{
            surface_info_queried = surface_info_queried && swapchain_create_info_.surface == surface;
			swapchain_create_info_.surface = surface;
            return *this;
		}

		SwapchainBuilder& SetGPU(VkPhysicalDevice gpu) noexcept
		{
            surface_info_queried = surface_info_queried && gpu_ == gpu;
            gpu_ = gpu;
            return *this;
		}

		SwapchainBuilder& SetCreationFlags(const SwapchainCreateFlags flags) noexcept
		{
            swapchain_create_info_.flags = flags;
            return *this;
		}

		SwapchainBuilder& SetCompositeAlpha(const CompositeAlpha composite_alpha) noexcept
		{
            swapchain_create_info_.compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(composite_alpha);
			return *this;
		}

		SwapchainBuilder& SetClipped(const bool clipped = true) noexcept
		{
            swapchain_create_info_.clipped = static_cast<VkBool32>(clipped);
            return *this;
		}

		SwapchainBuilder& SetImageArrayLayers(const uint32_t layers) noexcept
		{
            swapchain_create_info_.imageArrayLayers = layers;
            return *this;
		}

		SwapchainBuilder& SetImageUsage(const ImageUsageFlags usage) noexcept
		{
            swapchain_create_info_.imageUsage = usage;
            return *this;
		}

        SwapchainBuilder& SetPreTransform(const VkSurfaceTransformFlagBitsKHR transform) noexcept
		{
            pre_transform_ = transform;
            return *this;
		}

		SwapchainBuilder& SetMinImageCount(const uint32_t image_count) noexcept
		{
            swapchain_create_info_.minImageCount = image_count;
            return *this;
		}

		SwapchainBuilder& SetOldSwapchain(VkSwapchainKHR swapchain) noexcept
		{
            swapchain_create_info_.oldSwapchain = swapchain;
            return *this;
		}

        SwapchainBuilder& SetImageSharingMode(const SharingMode sharing_mode) noexcept
		{
            swapchain_create_info_.imageSharingMode = static_cast<VkSharingMode>(util::Underlying(sharing_mode));
            return *this;
		}
		
		SwapchainBuilder& SetQueueFamilyIndices(const uint32_t* indices, const uint32_t indices_count) noexcept
		{
            swapchain_create_info_.pQueueFamilyIndices = indices;
            swapchain_create_info_.queueFamilyIndexCount = indices_count;
            return *this;
		}

		SwapchainBuilder& SetComponentMapping(VkComponentMapping mapping) noexcept
		{
            mapping_ = mapping;
            return *this;
		}

		SwapchainBuilder& SetImageSubresourceRange(VkImageSubresourceRange range) noexcept
		{
            range_ = range;
            return *this;
		}

		const VkSurfaceCapabilitiesKHR& GetCapabilities() const noexcept
		{
            return surface_info_.capabilities;
		}

		
        template<typename Device>
		void BuildSwapchain(Swapchain& swapchain, Device& device) noexcept
        {
            if(!surface_info_queried)
            {
                surface_info_ = SurfaceInfo{ gpu_, swapchain_create_info_.surface };
            }

            MVK_CHECK_FATAL(surface_info_.SupportedByGPU(),
							"SwapchainBuilder::BuildSwapchain - The specified physical device(VkPhysicalDevice) does not support the given surface (VkSurface)");
			
            const VkExtent2D& capable_extent = surface_info_.capabilities.currentExtent;
            const VkExtent2D& max_extent = surface_info_.capabilities.maxImageExtent;
            const VkExtent2D& min_extent = surface_info_.capabilities.minImageExtent;
			
			if(capable_extent.width != UINT32_MAX)
			{
                swapchain.extent = capable_extent;
                width_ = capable_extent.width;
                height_ = capable_extent.height;
			}
            else
            {
                swapchain.extent.width = util::clamp(static_cast<uint32_t>(width_), min_extent.width, max_extent.width);
                swapchain.extent.height = util::clamp(static_cast<uint32_t>(height_), min_extent.height, max_extent.height);
            }
            swapchain_create_info_.imageExtent = swapchain.extent;

            swapchain.format = surface_info_.formats[0];

			if(required_format_ != VK_FORMAT_MAX_ENUM)
			{
                swapchain.format = FindFormat(required_format_, required_color_space_);
				MVK_CHECK_FATAL(swapchain.format.format != VK_FORMAT_MAX_ENUM && swapchain.format.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR,
								"SwapchainBuilder::BuildSwapchain - Required swapchain image format not available when using the specified surface and physical device (GPU)");
			}
            else
            {
                VkSurfaceFormatKHR prefered = FindFormat(format_, color_space_);
            	if(prefered.format != VK_FORMAT_MAX_ENUM)
            	{
                    swapchain.format = prefered;
            	}
            }


            swapchain.present_mode = VK_PRESENT_MODE_FIFO_KHR;
			for(const auto present_mode : surface_info_.present_modes)
			{
				if(present_mode == prefered_present_mode_)
				{
                    swapchain.present_mode = present_mode;
                    break;
				}
			}

			if(surface_info_.capabilities.maxImageCount > 0 && swapchain_create_info_.minImageCount > surface_info_.capabilities.maxImageCount)
			{
                swapchain_create_info_.minImageCount = surface_info_.capabilities.maxImageCount;
			}

			// TODO: Tu treba warning neki stavit ako nije podrzana transformacija bolje
            if(pre_transform_ == VK_SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM_KHR || !(pre_transform_ & surface_info_.capabilities.supportedTransforms))
            {
                pre_transform_ = surface_info_.capabilities.currentTransform;
            }
			
            swapchain_create_info_.presentMode = swapchain.present_mode;
            swapchain_create_info_.imageFormat = swapchain.format.format;
            swapchain_create_info_.imageColorSpace = swapchain.format.colorSpace;
            swapchain_create_info_.imageExtent = swapchain.extent;
            swapchain_create_info_.preTransform = pre_transform_;
			
			
            swapchain.vk_swapchain = device.CreateSwapchain(swapchain_create_info_);

            uint32_t image_count = 0;
            vkGetSwapchainImagesKHR(device.GetVkDevice(), swapchain, &image_count, nullptr);

			if(image_count)
			{
                swapchain.images.resize(image_count);
                VkImage* images = reinterpret_cast<VkImage*>(swapchain.images.data());
                vkGetSwapchainImagesKHR(device.GetVkDevice(), swapchain, &image_count, images);

                for (uint32_t i = image_count - 1; i > 0; --i)
                {
                    VkImage image = images[i];
                    Swapchain::SwapchainImage& swapchain_image = swapchain.images[i];
                    swapchain_image.vk_image = image;
                    image = VK_NULL_HANDLE;
                }

				for(uint32_t i = 0; i < image_count; ++i)
				{
                    Swapchain::SwapchainImage& swapchain_image = swapchain.images[i];

                    swapchain_image.vk_image_view = device.CreateImageView(swapchain_image.vk_image,
                                                                           VK_IMAGE_VIEW_TYPE_2D,
                                                                           swapchain.format.format,
                                                                           mapping_,
                                                                           range_);
					
				}
			}
        }

		
	private:

		VkSurfaceFormatKHR FindFormat(const VkFormat format, const VkColorSpaceKHR color_space) const noexcept
		{
            for (const auto& available_format : surface_info_.formats)
            {
                if(available_format.format == format && available_format.colorSpace == color_space)
                {
                    return available_format;
                }
            }

            return VkSurfaceFormatKHR{ VK_FORMAT_MAX_ENUM, VK_COLOR_SPACE_MAX_ENUM_KHR };
		}
		
        SurfaceInfo surface_info_{};
        bool surface_info_queried{ true };
        VkPhysicalDevice gpu_{ VK_NULL_HANDLE };
        VkFormat format_{ VK_FORMAT_B8G8R8A8_SRGB };
        VkColorSpaceKHR color_space_{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        VkFormat required_format_{ VK_FORMAT_MAX_ENUM };
        VkColorSpaceKHR required_color_space_{ VK_COLOR_SPACE_MAX_ENUM_KHR };
        VkPresentModeKHR prefered_present_mode_{ VK_PRESENT_MODE_MAILBOX_KHR };
        VkSwapchainCreateInfoKHR swapchain_create_info_{};
        VkSurfaceTransformFlagBitsKHR pre_transform_{ VK_SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM_KHR };
        VkComponentMapping mapping_{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        VkImageSubresourceRange range_{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		
        int width_{ 0 };
        int height_{ 0 };
	};



    
    
} // namespace mvk


#endif