#include "app.h"
#include "vertex.h"
#include "queue_families.h"
#include "uniform.h"
#include "formats.h"
#include "models.h"
#include "textures.h"

#include <optional>
#include <vector>
#include <array>
#include <cstdio>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <cstring>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DETPH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>



#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif



VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    fprintf(stderr, "Validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) noexcept {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}



constexpr std::array<const char*, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

constexpr std::array<const char*, 1> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};





std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = 
        {
            static_cast<uint32_t>(width), 
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}


App::~App()
{
    cleanup();
}

void App::framebufferResized(bool hasResized) noexcept
{
    framebufferResized_ = hasResized;
}

void App::initWindow(int width, int height) {
    if(!glfwInit())
    {
        VK_ERR("Error while initializing GLFW\n");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
                                             {
                                                auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
                                                app->framebufferResized(true);
                                             });
    
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
                                {
                                    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
                                    
                                });
}

void App::initVulkan() noexcept {
    createInstance();
    setupDebugMessenger();
    createSurface();

    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
    createImageViews();

    createRenderPass();
    createDescriptorSetLayouts();
    createPipes();


    createCommandPool(drawCmdPool, false, [](QueueFamilyIndices indices) { return indices.graphicsFamily().value();});
    createCommandPool(transferCmdPool, true, [](QueueFamilyIndices indices) { return indices.transferFamily().value(); });

    createDepthResources();
    createFramebuffers();

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    loadModels();
    createVertexBuffers();
    createIndexBuffer();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffers();

    createSyncObjects();
}

void App::mainLoop() noexcept {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}



void App::recreateSwapchain() noexcept
{   
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(window, &width, &height);
    while(0 == width || 0 == height)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapchain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createPipes();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

void App::cleanupSwapchain() noexcept
{

    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthMemory, nullptr);

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) 
    {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(device, drawCmdPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(device, splineObj.pipeline, nullptr);
    vkDestroyPipelineLayout(device, splineObj.pipelayout, nullptr);

    vkDestroyPipeline(device, planeObj.pipeline, nullptr);
    vkDestroyPipelineLayout(device, planeObj.pipeLayout, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < swapChainImageViews.size(); i++) 
    {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    for(size_t i = 0; i < uniformBuffers.size(); ++i)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    }
    vkFreeMemory(device, uniformBuffersMemory, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void App::cleanup() noexcept {

    cleanupSwapchain();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureMemory, nullptr);

    vkDestroyDescriptorSetLayout(device, planeObj.descriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, splineObj.descriptorLayout, nullptr);

    vkDestroyBuffer(device, planeObj.vertBuffer, nullptr);
    vkFreeMemory(device, planeObj.vertexBuffMem, nullptr);
    vkDestroyBuffer(device, splineObj.vertBuffer, nullptr);
    vkFreeMemory(device, splineObj.vertexBuffMem, nullptr);

    vkDestroyBuffer(device, planeObj.indBuffer, nullptr);
    vkFreeMemory(device, planeObj.indexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, drawCmdPool, nullptr);
    vkDestroyCommandPool(device, transferCmdPool, nullptr);

    vkDestroyPipelineCache(device, cache, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void App::createInstance() noexcept {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        VK_ERR("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        VK_ERR("failed to create instance!");
    }
}


void App::setupDebugMessenger() noexcept
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        VK_ERR("failed to set up debug messenger!");
    }
}

void App::createSurface() noexcept
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        VK_ERR("failed to create window surface!");
    }
}

void App::pickPhysicalDevice() noexcept
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        VK_ERR("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        VK_ERR("failed to find a suitable GPU!");
    }
}

void App::createLogicalDevice() noexcept
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    uint32_t uniqueFamilies[3]{};
    indices.setUniqueFamilies(uniqueFamilies);
    auto uniqueCount = indices.uniqueFamiliesCount();

    float queuePriority = 1.0f;

    for(size_t i = 0; i < uniqueCount; ++i)
    {
        uint32_t queueFamily = uniqueFamilies[i];
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        VK_ERR("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily().value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily().value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.transferFamily().value(), 0, &transferQueue);
}

void App::createSwapChain() noexcept
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[3];
    indices.setUniqueFamilies(queueFamilyIndices);
    uint32_t uniqueindices = indices.uniqueFamiliesCount();

    if(uniqueindices > 1)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = uniqueindices;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    

    // if (indices.graphicsFamily() != indices.presentFamily() || indices.get != indices.transferFamily_) {
    //     createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    //     createInfo.queueFamilyIndexCount = 2;
    //     createInfo.pQueueFamilyIndices = queueFamilyIndices;
    // } else {
    //     createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        VK_ERR("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspect;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    if (vkCreateImageView(device, &createInfo, nullptr, &view) != VK_SUCCESS)
    {
        VK_ERR("failed to create image view!");
    }

    return view;
}


void App::createImageViews() noexcept
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = createImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void App::createRenderPass() noexcept
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                 physicalDevice,
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthRef;
    
    

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        VK_ERR("failed to create render pass!");
    }
}

std::vector<uint8_t> readFile(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if(!file)
    {
        fprintf(stderr, "Error while trying to read binary file: %s\n", filename);
        abort();
    }
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    
    
    std::vector<uint8_t> result(size);
    fread(result.data(), 1, size, file);

    return result;

}

void App::createDescriptorSetLayouts() noexcept
{
//    std::array<VkDescriptorSetLayoutBinding, 2> bindings = 
//    {
//        BSplineVertUniform::getBinding(),
//        BSplineGeomUniform::getBinding()
//    };

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if(VK_SUCCESS != vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &planeObj.descriptorLayout))
    {
        VK_ERR("Failed to create descriptor set layout!\n");
    }

    // VkDescriptorSetLayoutBinding splineUboLayoutBinding{};
    // uboLayoutBinding.binding = 0;
    // uboLayoutBinding.descriptorCount = 1;
    // uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    // uboLayoutBinding.pImmutableSamplers = nullptr;
    // uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
    // VkDescriptorSetLayoutCreateInfo splineLayoutInfo{};
    // layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if(VK_SUCCESS != vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &splineObj.descriptorLayout))
    {
        VK_ERR("Failed to create descriptor set layout!\n");
    }


}


void App::createPipes() noexcept
{
    VkPipelineCacheCreateInfo cacheInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .flags = 0
    };
    
    if(VK_SUCCESS != vkCreatePipelineCache(device, &cacheInfo, nullptr, &cache))
    {
        VK_ERR("Failed to create pipeline cache!");
    }

    static constexpr const char* MODEL_VERTEX_SHADER = "D:/workspace_cpp/lab1_rg/build/shaders/texturedModel.vert.spv"; 
    static constexpr const char* MODEL_FRAGMENT_SHADER = "D:/workspace_cpp/lab1_rg/build/shaders/texturedModel.frag.spv";

    static constexpr const char* BSPLINE_VERTEX_SHADER = "D:/workspace_cpp/lab1_rg/build/shaders/bspline.vert.spv";
    static constexpr const char* BSPLINE_GEOMETRY_SHADER = "D:/workspace_cpp/lab1_rg/build/shaders/bspline.geom.spv";
    static constexpr const char* BSPLINE_FRAGMENT_SHADER = "D:/workspace_cpp/lab1_rg/build/shaders/bspline.frag.spv";

    auto modelVertShaderCode = readFile(MODEL_VERTEX_SHADER);
    auto modelFragShaderCode = readFile(MODEL_FRAGMENT_SHADER);
    auto bsplineVertShaderCode = readFile(BSPLINE_VERTEX_SHADER);
    // auto bsplineGeomShaderCode = readFile(BSPLINE_GEOMETRY_SHADER);
    auto bsplineFragShaderCode = readFile(BSPLINE_FRAGMENT_SHADER);

    VkShaderModule modelVertShaderModule = createShaderModule(modelVertShaderCode);
    VkShaderModule modelFragShaderModule = createShaderModule(modelFragShaderCode);

    VkShaderModule bsplineVertShaderModule = createShaderModule(bsplineVertShaderCode);
    // VkShaderModule bsplineGeomShaderModule = createShaderModule(bsplineGeomShaderCode);
    VkShaderModule bsplineFragShaderModule = createShaderModule(bsplineFragShaderCode);

    VkPipelineShaderStageCreateInfo modelVertShaderStageInfo{};
    modelVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    modelVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    modelVertShaderStageInfo.module = modelVertShaderModule;
    modelVertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo modelFragShaderStageInfo{};
    modelFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    modelFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    modelFragShaderStageInfo.module = modelFragShaderModule;
    modelFragShaderStageInfo.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> modelShaderStages = 
    {
        modelVertShaderStageInfo,
        modelFragShaderStageInfo
    };

    VkPipelineShaderStageCreateInfo bsplineVertShaderStageInfo{};
    bsplineVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    bsplineVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    bsplineVertShaderStageInfo.module = bsplineVertShaderModule;
    bsplineVertShaderStageInfo.pName = "main";

    // VkPipelineShaderStageCreateInfo bsplineGeomShaderStageInfo{};
    // bsplineGeomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // bsplineGeomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    // bsplineGeomShaderStageInfo.module = bsplineGeomShaderModule;
    // bsplineGeomShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo bsplineFragShaderStageInfo{};
    bsplineFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    bsplineFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    bsplineFragShaderStageInfo.module = bsplineFragShaderModule;
    bsplineFragShaderStageInfo.pName = "main";
    
    std::array<VkPipelineShaderStageCreateInfo, 2> bsplineShaderStages = 
    {
        bsplineVertShaderStageInfo,
        // bsplineGeomShaderStageInfo,
        bsplineFragShaderStageInfo
    };

    auto modelBindingDescription = Vertex::getBindingDescription();
    auto modelAttributeDescriptions = Vertex::getAttributeDescriptions();
    auto bsplineBindingDescription = BSplineVertex::bindingDescription();
    auto bsplineAttributeDescriptions = BSplineVertex::attributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &modelBindingDescription;
    
    vertexInputInfo.pVertexAttributeDescriptions = modelAttributeDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(modelAttributeDescriptions.size());

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 3.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE; // usporedba dubine fragmenta s depth bufferom za odbacivanje
    depthStencil.depthWriteEnable = VK_TRUE; // upisi novu dubinu fragmenti u depth buffer
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // manja dubina -> blize
    // Omogocuju da se zadrze fragmenti unutar samo odredjene dubine
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;

    depthStencil.stencilTestEnable = VK_FALSE;
    


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &planeObj.descriptorLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &planeObj.pipeLayout) != VK_SUCCESS)
    {
        VK_ERR("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = modelShaderStages.size();
    pipelineInfo.pStages = modelShaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = planeObj.pipeLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, cache, 1, &pipelineInfo, nullptr, &planeObj.pipeline) != VK_SUCCESS)
    {
        VK_ERR("failed to create graphics pipeline!");
    }

    pipelineLayoutInfo.pSetLayouts = &splineObj.descriptorLayout;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &splineObj.pipelayout) != VK_SUCCESS)
    {
        VK_ERR("failed to create pipeline layout!");
    }

    // TODO: OVO JOS PROVJERI
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    
    // inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY; // OVO NISAM SIGURAN
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    vertexInputInfo.pVertexBindingDescriptions = &bsplineBindingDescription;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = bsplineAttributeDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(bsplineAttributeDescriptions.size());

    pipelineInfo.pStages = bsplineShaderStages.data();
    pipelineInfo.stageCount = static_cast<uint32_t>(bsplineShaderStages.size());
    pipelineInfo.layout = splineObj.pipelayout;

    if (vkCreateGraphicsPipelines(device, cache, 1, &pipelineInfo, nullptr, &splineObj.pipeline) != VK_SUCCESS)
    {
        VK_ERR("failed to create graphics pipeline!");
    }


    vkDestroyShaderModule(device, modelFragShaderModule, nullptr);
    vkDestroyShaderModule(device, modelVertShaderModule, nullptr);

    vkDestroyShaderModule(device, bsplineVertShaderModule, nullptr);
    // vkDestroyShaderModule(device, bsplineGeomShaderModule, nullptr);
    vkDestroyShaderModule(device, bsplineFragShaderModule, nullptr);

    
}

void App::createFramebuffers() noexcept
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments
        {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            VK_ERR("failed to create framebuffer!");
        }
    }
}

void App::createImage(uint32_t width,
                      uint32_t height,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags props,
                      VkImage& image,
                      VkDeviceMemory& imageMemory) noexcept
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;
    

    if(VK_SUCCESS != vkCreateImage(device, &imageInfo, nullptr, &image))
    {
        VK_ERR("Failed to create texture image\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(VK_SUCCESS != vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory))
    {
        VK_ERR("Error while trying to allocate memory for texture\n");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void App::transitionImageLayout(VkImage image,
                                VkFormat format,
                                VkImageLayout oldLayout,
                                VkImageLayout newlayout,
                                VkCommandBuffer buffer) noexcept
{
    // VkCommandBuffer buffer = beginTempCommandBuffer(drawCmdPool);

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    VkImageMemoryBarrier barrier{};

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newlayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        VK_ERR("Specified invalid old and new layout for transitionImageLayout");
    }

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newlayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    vkCmdPipelineBarrier(buffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);


    // endTempCommandBuffer(buffer, graphicsQueue, drawCmdPool);
}

void App::copyBufferImage(VkCommandBuffer cmdbuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) noexcept
{
    // VkCommandBuffer commandBuffer = beginTempCommandBuffer(drawCmdPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(cmdbuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // endTempCommandBuffer(commandBuffer, graphicsQueue, drawCmdPool);
}


void App::createDepthResources() noexcept
{
    VkFormat depthFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                               physicalDevice,
                                               VK_IMAGE_TILING_OPTIMAL,
                                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    createImage(swapChainExtent.width,
                swapChainExtent.height,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImage,
                depthMemory);

    depthImageView = createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    

}

void App::createTextureImage() noexcept
{
    static constexpr const char* TEXTURE_PATH = "D:/workspace_cpp/lab1_rg/assets/textures/viking_room.png";

    VkBuffer stagingBuffer;
    VkDeviceMemory stageBuffMemory;
    
    const TextureData texture = loadTexture(TEXTURE_PATH);

    if(!texture.pixels)
    {
        VK_ERR("Error in loading texture data from file");
    }

    VkDeviceSize bufferSize = texture.size;

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stageBuffMemory);

    void* data;
    vkMapMemory(device, stageBuffMemory, 0, bufferSize, 0, &data);
    memcpy(data, texture.pixels, static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stageBuffMemory);

    createImage(texture.width,
                texture.height,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage,
                textureMemory);

    VkCommandBuffer commandBuffer = beginTempCommandBuffer(drawCmdPool);

    transitionImageLayout(textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          commandBuffer);
    
    copyBufferImage(commandBuffer,
                    stagingBuffer,
                    textureImage,
                    static_cast<uint32_t>(texture.width),
                    static_cast<uint32_t>(texture.height));

    transitionImageLayout(textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          commandBuffer);
    
    endTempCommandBuffer(commandBuffer, graphicsQueue, drawCmdPool);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stageBuffMemory, nullptr);

}


void App::createTextureImageView() noexcept
{
   textureImageView = createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void App::createTextureSampler() noexcept
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.f;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 0.f;

    if(VK_SUCCESS != vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler))
    {
        VK_ERR("Error while trying to create sampler");
    }
}

void App::createCommandBuffers() noexcept {
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = drawCmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        VK_ERR("failed to allocate command buffers!");
    }

    for (size_t i = 0, n = commandBuffers.size(); i < n; i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            VK_ERR("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, planeObj.pipeline);

            VkBuffer vertexBuffers[] = {planeObj.vertBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[i], planeObj.indBuffer, 0, VK_INDEX_TYPE_UINT32);
            
            vkCmdBindDescriptorSets(commandBuffers[i],
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    planeObj.pipeLayout,
                                    0,
                                    1,
                                    &descriptorSets[i],
                                    0,
                                    nullptr);
            // vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(spline.points.size()), 1, 0, 0);
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(planeObj.indices.size()), 1, 0, 0, 0);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, splineObj.pipeline);
            vertexBuffers[0] = splineObj.vertBuffer;

            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdBindDescriptorSets(commandBuffers[i],
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    splineObj.pipelayout,
                                    0,
                                    1,
                                    &descriptorSets[n + i],
                                    0,
                                    nullptr);
            
            vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(spline.points.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            VK_ERR("failed to record command buffer!");
        }
    }
}

void App::createSyncObjects() noexcept
{
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            VK_ERR("failed to create synchronization objects for a frame!");
        }
    }
}

uint32_t App::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) noexcept
{

    VkPhysicalDeviceMemoryProperties gpuMemProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &gpuMemProperties);

    for(uint32_t i = 0; i < gpuMemProperties.memoryTypeCount; ++i)
    {
        if((typeFilter & (1 << i)) &&
           (gpuMemProperties.memoryTypes[i].propertyFlags & properties) == properties
          ) 
        {
            return i;
        }
    }

    VK_ERR("Error while trying to find the suitable memory type");
}




void App::createBuffer(VkDeviceSize size,
                       VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags props,
                       VkBuffer& buff,
                       VkDeviceMemory& buffMemory) noexcept
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


    if(VK_SUCCESS != vkCreateBuffer(device, &bufferInfo, nullptr, &buff))
    {
        VK_ERR("Error while trying to create buffer\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buff, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Mora biti COHERENT ili moramo rucno flushat kod mapiranja
    // host visible da bi mogli u njega opce kopirati nase vertex podatke - sporije
    // Stavi na Device local i koristi staging buffer
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, props);

    // Flush i coherent bit samo garantiraju da se memorija bude mapirala do sljedeceg poziva vkQueueSubmit


    if(VK_SUCCESS != vkAllocateMemory(device, &allocInfo, nullptr, &buffMemory))
    {
        VK_ERR("Failed to allocate GPU memory for the vertex buffer");
    }

    vkBindBufferMemory(device, buff, buffMemory, 0);

   
}

void loadBSplineModel(BSpline& obj)
{
    static constexpr const char* SPLINE_OBJ_PATH = "D:/workspace_cpp/lab1_rg/assets/models/path.obj";

    BSpline::load(obj, SPLINE_OBJ_PATH);
}


void App::loadModels() noexcept
{
    loadBSplineModel(spline);
    animation = spline.animate();
    // static constexpr const char* MODEL_PATH = "../assets/models/viking_room.obj";
    // static constexpr const char* MODEL_PATH = "D:/workspace_cpp/lab1_rg/assets/models/aircraft747.obj";
    static constexpr const char* MODEL_PATH = "D:/fileovi/teddy.obj";
    Model model;

    loadModel(MODEL_PATH, model);

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for(const auto& shape : model.shapes)
    {
        for(const auto& index : shape.mesh.indices)
        {
            
            
            Vertex vertex{};

            vertex.pos = 
            {
                model.attributes.vertices[3 * index.vertex_index + 0],
                model.attributes.vertices[3 * index.vertex_index + 1],
                model.attributes.vertices[3 * index.vertex_index + 2]
            };
            if(!model.attributes.texcoords.empty())
            {
                vertex.texCoord = 
                {
                    model.attributes.texcoords[2 * index.texcoord_index + 0],
                    // vulkan je top to bottom, a tinyobjloader je u koord sustavu gdje je 0 donja
                    1.0f - model.attributes.texcoords[2 * index.texcoord_index + 1]
                };

            }

            vertex.color = {1.0f, 1.0f, 1.0f};

            const auto [emplaceIt, emplaceHappened] = uniqueVertices.try_emplace(vertex, static_cast<uint32_t>(planeObj.vertices.size()));
            if(emplaceHappened)
            {
                planeObj.vertices.emplace_back(std::move(vertex));
            }
            planeObj.indices.emplace_back(emplaceIt->second);

        }
    }
}

void App::createVertexBuffers() noexcept
{
    VkDeviceSize planeBufferSize = sizeof(Vertex) * planeObj.vertices.size();
    VkDeviceSize splineBufferSize = sizeof(spline.path[0]) * spline.path.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stageBuffMemory;
    
    createBuffer(planeBufferSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 planeObj.vertBuffer,
                 planeObj.vertexBuffMem);
    
    createBuffer(splineBufferSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 splineObj.vertBuffer,
                 splineObj.vertexBuffMem);

    createBuffer(planeBufferSize + splineBufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stageBuffMemory);
    
    // void* data;
    // vkMapMemory(device, stageBuffMemory, 0, planeBufferSize, 0, &data);
    // memcpy(data, planeObj.vertices.data(), planeBufferSize);
    // vkUnmapMemory(device, stageBuffMemory);
    
    // copyBuffer(stagingBuffer, planeObj.vertBuffer, planeBufferSize);

    // vkDestroyBuffer(device, stagingBuffer, nullptr);
    // vkFreeMemory(device, stageBuffMemory, nullptr);

    // createBuffer(splineBufferSize,
    //              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //              stagingBuffer,
    //              stageBuffMemory);
    
    // data = nullptr;
    // vkMapMemory(device, stageBuffMemory, 0, splineBufferSize, 0, &data);
    // memcpy(data, spline.path.data(), splineBufferSize);
    // vkUnmapMemory(device, stageBuffMemory);
    
    // copyBuffer(stagingBuffer, splineObj.vertBuffer, splineBufferSize);

    // vkDestroyBuffer(device, stagingBuffer, nullptr);
    // vkFreeMemory(device, stageBuffMemory, nullptr);

    
    VkCommandBuffer commandBuffer = beginTempCommandBuffer(transferCmdPool);

        VkBufferCopy cpy{};
        cpy.size = planeBufferSize;
        cpy.srcOffset = 0;
        cpy.dstOffset = 0;

        void* data;
        vkMapMemory(device, stageBuffMemory, 0, planeBufferSize, 0, &data);
        memcpy(data, planeObj.vertices.data(), static_cast<size_t>(planeBufferSize));
        vkUnmapMemory(device, stageBuffMemory);
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, planeObj.vertBuffer, 1, &cpy);
        
        cpy.size = splineBufferSize;
        cpy.srcOffset = planeBufferSize;
        data = nullptr;
        vkMapMemory(device, stageBuffMemory, planeBufferSize, splineBufferSize, 0, &data);
        memcpy(data, spline.path.data(), static_cast<size_t>(splineBufferSize));
        vkUnmapMemory(device, stageBuffMemory);
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, splineObj.vertBuffer, 1, &cpy);

    endTempCommandBuffer(commandBuffer, transferQueue, transferCmdPool);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stageBuffMemory, nullptr);
}

void App::createIndexBuffer() noexcept
{
    VkDeviceSize bufferSize = sizeof(planeObj.indices[0]) * planeObj.indices.size();
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingMemory);

    void* data;
    vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, planeObj.indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 planeObj.indBuffer,
                 planeObj.indexBufferMemory);

    copyBuffer(stagingBuffer, planeObj.indBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

}

void createBuff(VkDevice device, VkBuffer& buff, VkDeviceSize size, VkBufferUsageFlags usage) noexcept
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(VK_SUCCESS != vkCreateBuffer(device, &bufferInfo, nullptr, &buff))
    {
        VK_ERR("Error while trying to create buffer\n");
    }
}

void App::createUniformBuffers() noexcept
{
    constexpr VkDeviceSize bufferSize = sizeof(UniformBuffObject);
    // constexpr VkDeviceSize bufferSize = sizeof(BSplineVertUniform) + sizeof(BSplineGeomUniform);
    uniformBuffers.reserve(swapChainImages.size() * 2);
    
    size_t totalMemory = 0;
    for(size_t i = 0, n = swapChainImages.size() * 2; i < n; ++i)
    {
        VkBuffer& buffer = uniformBuffers.emplace_back();
        createBuff(device, buffer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, uniformBuffers[0], &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    allocInfo.allocationSize = memReqs.size * uniformBuffers.size();

    if(VK_SUCCESS != vkAllocateMemory(device, &allocInfo, nullptr, &uniformBuffersMemory))
    {
        VK_ERR("Error while trying to allocate memory for uniform buffers!");
    }

    for(size_t i = 0, n = uniformBuffers.size(); i < n; ++i)
    {
        vkBindBufferMemory(device, uniformBuffers[i], uniformBuffersMemory, memReqs.size * i);
    }

    uniformBufferSize = memReqs.size;
        
}

VkCommandBuffer App::beginTempCommandBuffer(VkCommandPool& pool) noexcept
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void App::endTempCommandBuffer(VkCommandBuffer& buffer, VkQueue& queue, VkCommandPool& pool) noexcept
{
    vkEndCommandBuffer(buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, pool, 1, &buffer);
}

void App::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) noexcept
{

    VkCommandBuffer commandBuffer = beginTempCommandBuffer(transferCmdPool);
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

    endTempCommandBuffer(commandBuffer, transferQueue, transferCmdPool);

}

void App::createDescriptorPool() noexcept
{

    std::array<VkDescriptorPoolSize, 2> poolSizes;

    // Uniform buffer descriptor
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * 2);
    
    // Combined image sampler descriptor
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = static_cast<uint32_t>(swapChainImages.size() * 2);

    if(VK_SUCCESS != vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool))
    {
        VK_ERR("Error while trying to inialize descriptor pool");
    }
}

void App::createDescriptorSets() noexcept
{
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size() * 2);

    for(size_t i = 0, n = swapChainImages.size(); i < n; ++i)
    {
        layouts[i]     = planeObj.descriptorLayout;
        layouts[n + i] = splineObj.descriptorLayout;
    }

    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size() * 2);
    allocInfo.pSetLayouts        = layouts.data();

    descriptorSets.resize(swapChainImages.size() * 2);
    if(VK_SUCCESS != vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()))
    {
        VK_ERR("Error while allocating descriptor sets");
    }

    for(size_t i = 0, n = swapChainImages.size(); i < n; ++i)
    {
        VkDescriptorBufferInfo modelUniformDescriptor{};
        modelUniformDescriptor.buffer = uniformBuffers[i];
        modelUniformDescriptor.offset = 0;
        modelUniformDescriptor.range  = sizeof(UniformBuffObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;
        
        VkDescriptorBufferInfo splineUniformDescriptor{};
        splineUniformDescriptor.buffer = uniformBuffers[n + i];
        splineUniformDescriptor.offset = 0;
        splineUniformDescriptor.range = sizeof(UniformBuffObject);


        std::array<VkWriteDescriptorSet, 3> writers{};

        writers[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writers[0].dstSet           = descriptorSets[i];
        writers[0].dstBinding       = 0;
        writers[0].dstArrayElement  = 0;
        writers[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writers[0].descriptorCount  = 1;
        writers[0].pBufferInfo      = &modelUniformDescriptor;
        writers[0].pImageInfo       = nullptr;
        writers[0].pTexelBufferView = nullptr;

        writers[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writers[1].dstSet           = descriptorSets[i];
        writers[1].dstBinding       = 1;
        writers[1].dstArrayElement  = 0;
        writers[1].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writers[1].descriptorCount  = 1;
        writers[1].pBufferInfo      = nullptr;
        writers[1].pImageInfo       = &imageInfo;
        writers[1].pTexelBufferView = nullptr;

        writers[2].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writers[2].dstSet           = descriptorSets[n + i];
        writers[2].dstBinding       = 0;
        writers[2].dstArrayElement  = 0;
        writers[2].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writers[2].descriptorCount  = 1;
        writers[2].pBufferInfo      = &splineUniformDescriptor;
        writers[2].pImageInfo       = nullptr;
        writers[2].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writers.size()), writers.data(), 0, nullptr);
    }
}

void App::updateUniformBuffer(uint32_t currentImage) noexcept
{
    // static auto startTime = std::chrono::high_resolution_clock::now();

    // auto currentTime = std::chrono::high_resolution_clock::now();

    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
    // const auto& [pos, tang, bitang] = animation.update();
    // glm::vec3 w = glm::normalize(tang);
    // glm::vec3 u = glm::normalize(glm::cross(w, glm::normalize(bitang)));
    // glm::vec3 v = glm::normalize(glm::cross(w, u));
    // glm::mat4 DRM = glm::inverse(glm::transpose(glm::mat3(w, u, v)));
    
    UniformBuffObject ubo{};
    // ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    
    ubo.model = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 0.3f, 0.3f));
    // ubo.model = glm::translate(ubo.model, pos);
    // ubo.model = glm::rotate(ubo.model, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f));
    // ubo.model = ubo.model * DRM;
    // ubo.model = glm::scale(ubo.model, glm::vec3(1.f, 5.f, 5.f));

    ubo.view  = glm::lookAt(glm::vec3(-3.f, -3.f, -30.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.proj  = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 100.f);
    
    // GLM je za OpenGL gdi je Y-koordinata clip kordinate invertirana -> promjeni predznak scale factora za Y
    ubo.proj[1][1] *= -1;

    
    void* data;
    vkMapMemory(device, uniformBuffersMemory, uniformBufferSize * currentImage, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory);

    ubo.model = glm::mat4(1.f);
    // ubo.model = glm::scale(ubo.model, glm::vec3(5.f, 5.f, 5.f));
    // for(size_t i = 0; i < spline.path.size(); ++i)
    // {
    //     auto point = glm::vec4(spline.path[i], 1.0);
    //     auto projecirano = ubo.model * ubo.view * ubo.proj * point;
    //     printf("%f %f %f %f\n", projecirano.x, projecirano.y, projecirano.z, projecirano.w);
    // }
    vkMapMemory(device, uniformBuffersMemory, uniformBufferSize * (currentImage + swapChainImages.size()), sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory);

}

void App::drawFrame() noexcept {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (VK_ERROR_OUT_OF_DATE_KHR == result)
    {
        recreateSwapchain();
        return;
    } else if (VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
    {
        VK_ERR("Failed to acquire swap chain image!\n");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        VK_ERR("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if(VK_ERROR_OUT_OF_DATE_KHR == result || VK_SUBOPTIMAL_KHR == result || framebufferResized_)
    {
        framebufferResized_ = false;
        recreateSwapchain();
    } else if (VK_SUCCESS != result)
    {
        VK_ERR("Failed to present to the swapchain image!\n");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkShaderModule App::createShaderModule(const std::vector<uint8_t>& code) noexcept {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        VK_ERR("failed to create shader module!");
    }

    return shaderModule;
}



SwapChainSupportDetails App::querySwapChainSupport(VkPhysicalDevice device) const noexcept {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool App::isDeviceSuitable(VkPhysicalDevice device) const noexcept {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() &&
           extensionsSupported  &&
           swapChainAdequate &&
           supportedFeatures.samplerAnisotropy &&
           supportedFeatures.geometryShader &&
           supportedFeatures.wideLines;
}

bool App::checkDeviceExtensionSupport(VkPhysicalDevice device) const noexcept {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}




bool App::checkValidationLayerSupport() const noexcept {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void App::run() noexcept
{
    initWindow(width_, height_);
    initVulkan();
    mainLoop();
    cleanup();
}

