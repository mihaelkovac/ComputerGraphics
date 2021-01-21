#ifndef LAB_H
#define LAB_H

#define MVK_DEBUG 1


#include "mvk/instance.h"
#include "mvk/device.h"
#include "mvk/image.h"
#include "mvk/context.h"
#include "mvk/swapchain.h"
#include "frame_sync.h"

#include <cstdint>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


template<typename T>
constexpr T clamp(T value, T min, T max) noexcept
{
    return value > min ? (value < max ? value : max) : min;
}


using ContextBase = mvk::Context<mvk::Device<mvk::DefaultAllocPolicy,
                                              mvk::DefaultQueuePolicy<mvk::FamilyType::Compute,
                                                                      mvk::FamilyType::Graphics,
                                                                      mvk::FamilyType::Presentation,
                                                                      mvk::FamilyType::Transfer>>>;

static constexpr size_t MaxFramesInFlight = 2;

struct LabContext : public ContextBase
{

    GLFWwindow* window = nullptr;
    int width = 0;
    int height = 0;
    VkSurfaceCapabilitiesKHR surfaceCapabilites{};

    mvk::Swapchain swapchain{};
    FrameSync<MaxFramesInFlight> frameSync{};

    VkCommandPool commandPool{ 0 };
    VkFormat depthFormat{ VK_FORMAT_MAX_ENUM };
    mvk::Image depthImage{};
    mvk::Allocation depthAlloc{};
    VkImageView depthImageView{ VK_NULL_HANDLE };

    void init(int windowWidth, int windowHeight, const char* title = "Lab") noexcept
    {
        width = windowWidth;
        height = windowHeight;
        initWindow(title);

        initInstance();

        initSurface();

        initDevice();
        

        initSwapchain();
    	
        frameSync.init(swapchain.images.size(), device);
    	
        commandPool = device.createCommandPool(device.graphicsFamilyIndex, 0);

        createDepthResource();
        
    }

    void releaseDepthResource()
    {

        vkDestroyImageView(device, depthImageView, device.getAllocationCallbacks());
        device.destroyImage(depthImage, depthAlloc);
    }

    void releaseSwapchain() noexcept
    {
        this->swapchain.release(device.getVkDevice(), device.getAllocationCallbacks());
    }


	void releaseGLFW() noexcept
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }


protected:

    void initWindow(const char* title) noexcept
    {
        MVK_CHECK_FATAL(glfwInit(), "Failed to init GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        // glfwSetWindowUserPointer(window, this);
        // glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
        //                                         {
        //                                             auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        //                                             app->framebufferResized(true);
        //                                         });
    }

    void initInstance() noexcept
    {
        mvk::InstanceBuilder builder{};
        const auto& availableExtensions = builder.getAvailableInstanceExtensions();
        auto availableLayers = builder.getAvailableInstanceLayers();

        bool validationPresent = false;
        for(const auto& layer : availableLayers)
        {
	        if(strcmp(layer.layerName, MVK_KHRONOS_VALIDATION_LAYER) == 0)
	        {
                validationPresent = true;
                break;
	        }
        }

    	if(!validationPresent)
    	{
            fprintf(stderr ,"Validation layer not supported!");
            abort();
    	}
    	
        uint32_t count = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        builder.addInstanceExtensions(glfwExtensions, count);
        #if MVK_DEBUG
        builder.addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        builder.addInstanceLayer(MVK_KHRONOS_VALIDATION_LAYER);
        #endif
        
        builder.buildInstance(instance);
        
    }

    void initSurface() noexcept
    {
        MVK_VALIDATE_RESULT(glfwCreateWindowSurface(this->instance.vkInstance, window, nullptr, &this->vkSurface), "Failed to initialize Vulkan surface");
    }
    

    
    void initDevice() noexcept
    {
        mvk::DefaultDeviceBuilder<true> builder{};
        
        builder.enableDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        builder.setRequiredGPUType(mvk::GPUType::Discrete);
        builder.setDeviceFeature(mvk::DeviceFeature::GeometryShader);
        builder.buildDevice(this->instance, this->vkSurface, this->device, nullptr);
    }

    


    void initSwapchain() noexcept
    {
        mvk::SurfaceInfo swapchainSupportInfo(this->device.vkGPU, this->vkSurface);

        this->surfaceCapabilites = swapchainSupportInfo.capabilities;
        
        swapchain.format = swapchainSupportInfo.formats[0];

        for(const auto& format : swapchainSupportInfo.formats)
        {
            if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                swapchain.format = format;
                break;
            }
        }

        swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for(const auto presentMode : swapchainSupportInfo.presentModes)
        {
            if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchain.presentMode = presentMode;
                break;
            }
        }

        if(surfaceCapabilites.currentExtent.width != UINT32_MAX)
        {
            swapchain.extent = surfaceCapabilites.currentExtent;
            width = swapchain.extent.width;
            height = swapchain.extent.height;
        }
        else
        {
            glfwGetFramebufferSize(window, &width, &height);
        	
        	
            swapchain.extent.width = clamp(static_cast<uint32_t>(width), surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width);
            swapchain.extent.height = clamp(static_cast<uint32_t>(height), surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height);
        }

        uint32_t imageCount = surfaceCapabilites.minImageCount + 1;
        if(surfaceCapabilites.maxImageCount > 0 && imageCount > surfaceCapabilites.maxImageCount)
        {
            imageCount = surfaceCapabilites.maxImageCount;
        }
    
        bool differenteFamilies = device.presentationFamilyIndex != device.graphicsFamilyIndex;    
        std::array<uint32_t, 2> indices{device.presentationFamilyIndex, device.graphicsFamilyIndex};

        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = this->vkSurface;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.minImageCount = imageCount;
        swapchainInfo.imageColorSpace = swapchain.format.colorSpace;
        swapchainInfo.imageFormat  = swapchain.format.format;
        swapchainInfo.imageExtent = swapchain.extent;
        swapchainInfo.presentMode = swapchain.presentMode;
        swapchainInfo.preTransform = surfaceCapabilites.currentTransform;
        //swapchainInfo.queueFamilyIndexCount = 1 + differenteFamilies;
        //swapchainInfo.pQueueFamilyIndices = indices.data();
        //swapchainInfo.imageSharingMode = differenteFamilies ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.clipped = VK_TRUE;

        swapchain.init(device, swapchainInfo);
        
    }


    void createDepthResource()
    {
        const std::array<VkFormat, 3> formatCandidates
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT
        };

        depthFormat = device.findSupportedFormat(formatCandidates.data(),
                                                           formatCandidates.size(),
                                                           VK_IMAGE_TILING_OPTIMAL,
                                                           mvk::FormatFeature::DepthStencilAttachment);

        VkImageCreateInfo imageInfo = mvk::Image::createInfo(mvk::ImageUsage::DepthStencilAttachment, 0);
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.width = swapchain.extent.width;
        imageInfo.extent.height = swapchain.extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.mipLevels = 1;
        imageInfo.format = depthFormat;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.flags = 0;

        VmaAllocationCreateInfo allocInfo = mvk::Allocation::createInfo(VMA_MEMORY_USAGE_GPU_ONLY, mvk::DeviceMemoryProperty::DeviceLocal);

        auto [depthImage_, depthAlloc_] = device.createImage(imageInfo, allocInfo);
        depthImage = depthImage_;
        depthAlloc = depthAlloc_;
        depthImageView = device.createImageView(depthImage.vkImage, VK_IMAGE_VIEW_TYPE_2D, depthFormat,
                                                mvk::createComponentMapping(),
                                                mvk::createSubresourceRange(mvk::ImageAspect::Depth));
		
        
    }

   
};

#endif