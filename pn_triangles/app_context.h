#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <array>

#define MVK_DEBUG 0
#include "sync.h"
#include "uniform_alloc.h"
#include "mvk/context.h"
#include "mvk/swapchain.h"



struct DepthRes
{
	mvk::Image image{};
	mvk::Allocation alloc{};
	mvk::ImageView image_view{};
	VkFormat format{};
};




struct AppContext : public mvk::Context<mvk::Device<UniformBuffPoolAllocationPolicy>>
{

	void Init(int& width, int& height, GLFWwindow* window) noexcept
	{
		InitInstance();
		
		MVK_VALIDATE_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &vk_surface),
			"Failed to create Vulkan Surface");

		InitDevice();

		InitSwapchain(width, height);
		
		InitDepthResources();

		InitFrameSync();

	}

	void ReleaseDepthResource() noexcept
	{
		device.DestroyImageView(depth.image_view);
		device.DestroyImage(depth.image, depth.alloc);
	}
	
	void Release() noexcept
	{

		swapchain.Release(device, device.GetAllocationCallbacks());
		sync.Release(device, device.GetAllocationCallbacks());
		device.Release();
		vkDestroySurfaceKHR(instance, vk_surface, device.GetAllocationCallbacks());
		instance.Release();
	}

	[[nodiscard]] constexpr VkAttachmentDescription GetSwapchainColorAttachment(const VkSampleCountFlagBits samples) const noexcept
	{
		VkAttachmentDescription description{};
		description.format = swapchain.format.format;
		description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		description.samples = samples;
		description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		return description;
	}

	[[nodiscard]] constexpr VkAttachmentDescription GetDepthAttachment(const VkSampleCountFlagBits samples) const noexcept
	{
		VkAttachmentDescription description{};
		description.format = depth.format;
		description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.samples = samples;
		description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		return description;
	}

	VkResult AcquireSwapchainImage(uint32_t& image_index) noexcept
	{
		return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, sync.image_ready_semaphore[sync.current_frame], VK_NULL_HANDLE, &image_index);
	}


	VkSurfaceCapabilitiesKHR surface_capabilities{};
	VkSurfaceKHR vk_surface{ VK_NULL_HANDLE };
	
	mvk::Swapchain swapchain{};
	FrameSync<2> sync{};
	FrameSubmitter submitter{};
	
	DepthRes depth{};

private:

	void InitFrameSync() noexcept
	{
		sync.Init(swapchain.images.size(), device);
		submitter.Init(&swapchain.vk_swapchain);
	}


	
	void InitInstance() noexcept
	{
		mvk::InstanceBuilder builder{};
		const auto& available_extensions = builder.GetAvailableInstanceExtensions();
		auto available_layers = builder.GetAvailableInstanceLayers();

		bool validation_present = false;

		for(const auto& layer : available_layers)
		{
			if(strcmp(layer.layerName, MVK_KHRONOS_VALIDATION_LAYER) == 0)
			{
				validation_present = true;
				break;
			}
		}

		if(!validation_present)
		{
			MVK_CHECK_FATAL(false, "No Vulkan validation layer support");
		}

		uint32_t count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&count);

		builder.AddInstanceExtensions(glfw_extensions, count);
#if MVK_DEBUG
		builder.AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			   .AddInstanceLayer(MVK_KHRONOS_VALIDATION_LAYER);
#endif

		builder.BuildInstance(instance);
	}
	
	void InitDevice() noexcept
	{
		mvk::DefaultDeviceBuilder<false> builder{};

		builder.EnableDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			   .SetRequiredGPUType(mvk::GPUType::Discrete)
			   .SetDeviceFeature(mvk::DeviceFeature::TessellationShader)
			   .SetDeviceFeature(mvk::DeviceFeature::FillModeNonSolid)
			   .SetDeviceFeature(mvk::DeviceFeature::MultiViewport)
			   .BuildDevice(instance, vk_surface, device, nullptr);
	}

	void InitDepthResources() noexcept
	{
		std::array<VkFormat, 3> format_candidates
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		depth.format = device.FindSupportedFormat(format_candidates.data(),
												  format_candidates.size(),
												  VK_IMAGE_TILING_OPTIMAL,
												  mvk::FormatFeature::DepthStencilAttachment);

		auto depth_image_info = mvk::Image::CreateInfo(mvk::ImageUsage::DepthStencilAttachment);
		depth_image_info.format = depth.format;
		depth_image_info.imageType = VK_IMAGE_TYPE_2D;
		depth_image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		depth_image_info.extent.width = swapchain.extent.width;
		depth_image_info.extent.height= swapchain.extent.height;
		depth_image_info.extent.depth = 1;
		depth_image_info.mipLevels = 1;
		depth_image_info.arrayLayers = 1;
		depth_image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		depth_image_info.flags = 0;
		depth_image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		
		auto depth_alloc_info = mvk::Allocation::CreateInfo(VMA_MEMORY_USAGE_GPU_ONLY, mvk::DeviceMemoryProperty::DeviceLocal);

		
		device.CreateImage(depth.image, depth.alloc, depth_image_info, depth_alloc_info);

		auto mapping = mvk::NewComponentMapping();
		auto range = mvk::NewSubresourceRange(mvk::ImageAspect::Depth);
		
		depth.image_view = device.CreateImageView(depth.image, VK_IMAGE_VIEW_TYPE_2D, depth.format, mapping, range);
	}


	void InitSwapchain(int& width, int& height) noexcept
	{

		mvk::SwapchainBuilder builder{ vk_surface, device.vk_gpu };
		
		std::array<uint32_t, 2> indices{ device.graphics_family_index, device.presentation_family_index };
		const uint32_t unique_indices = 1 + device.graphics_family_index != device.presentation_family_index;

		surface_capabilities = builder.GetCapabilities();
		builder.SetPreferredFormatAndColorspace(VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			   .SetPreferedPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
			   .SetSurfaceExtent(width, height)
			   .SetMinImageCount(surface_capabilities.minImageCount + 1)
			   .SetClipped(true);

		if(unique_indices > 1)
		{
			builder.SetQueueFamilyIndices(indices.data(), unique_indices);
		}

		builder.BuildSwapchain(swapchain, device);
	}
};


#endif // APP_CONTEXT_H
