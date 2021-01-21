#ifndef APP_H
#define APP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <array>


#include "queue_families.h"
#include "vertex.h"
#include "bspline.h"


#define VK_ERR(_msg)            \
    do {                        \
        fprintf(stderr, _msg);  \
        abort();                \
    } while(0)                  \


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


struct BSplineObj
{
    VkPipelineLayout pipelayout{ VK_NULL_HANDLE };
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkDescriptorSetLayout descriptorLayout{ VK_NULL_HANDLE };
    VkBuffer vertBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory vertexBuffMem{ VK_NULL_HANDLE };
};

struct PlaneObj
{
    VkPipelineLayout pipeLayout{ VK_NULL_HANDLE };
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkDescriptorSetLayout descriptorLayout{ VK_NULL_HANDLE };
    VkBuffer vertBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory vertexBuffMem{ VK_NULL_HANDLE };
    VkBuffer indBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory indexBufferMemory{ VK_NULL_HANDLE };
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
};

struct App
{

    App(int width, int height) : width_(width), height_(height) {}

    ~App();

    void run() noexcept;


    void framebufferResized(bool) noexcept;


private:

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    void initWindow(int width, int height);

    void initVulkan() noexcept;

    void mainLoop() noexcept;

    void cleanup() noexcept;

    void createInstance() noexcept;
    
    void setupDebugMessenger() noexcept;

    void createSurface() noexcept;

    void pickPhysicalDevice() noexcept;

    void createLogicalDevice() noexcept;

    void createSwapChain() noexcept;

    void createImageViews() noexcept;

    void createRenderPass() noexcept;

    void createDescriptorSetLayouts() noexcept;

    void createGraphicsPipeline() noexcept;

    void createPipes() noexcept;

    void createFramebuffers() noexcept;

    void createUniformBuffers() noexcept;

    void createDescriptorPool() noexcept;

    void createDescriptorSets() noexcept;

    void createDepthResources() noexcept;

    void createTextureImage() noexcept;

    void createTextureImageView() noexcept;

    void createTextureSampler() noexcept;

    void transitionImageLayout(VkImage image,
                               VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newlayout,
                               VkCommandBuffer cmdBuffer) noexcept;

    void copyBufferImage(VkCommandBuffer cmdBuffer,
                         VkBuffer buffer,
                         VkImage image,
                         uint32_t width,
                         uint32_t height) noexcept;

    void createImage(uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags props,
                     VkImage& image,
                     VkDeviceMemory& imageMemory) noexcept;

    template<typename QFamilyIndexGetter>
    void createCommandPool(VkCommandPool& cmdPool, bool transient, QFamilyIndexGetter getter) noexcept 
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = getter(queueFamilyIndices);
        if(transient)
        {
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        }

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
            VK_ERR("failed to create command pool!");
        }
    }

    void createCommandBuffers() noexcept;

    VkCommandBuffer beginTempCommandBuffer(VkCommandPool& pool) noexcept;

    void endTempCommandBuffer(VkCommandBuffer& buffer, VkQueue& queue, VkCommandPool& pool) noexcept;

    void createSyncObjects() noexcept;

    void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&) noexcept;

    void loadModels() noexcept;

    void createVertexBuffers() noexcept;

    void createIndexBuffer() noexcept;

    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags) noexcept;

    bool isDeviceSuitable(VkPhysicalDevice device) const noexcept;

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const noexcept;

    bool checkValidationLayerSupport() const noexcept;

    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) noexcept;

    void updateUniformBuffer(uint32_t index) noexcept;

    void drawFrame() noexcept;

    void recreateSwapchain() noexcept;

    void cleanupSwapchain() noexcept;

    VkShaderModule createShaderModule(const std::vector<uint8_t>& code) noexcept;

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const noexcept;

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineCache cache;
    BSplineObj splineObj;
    PlaneObj planeObj;

    VkCommandPool drawCmdPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkCommandPool transferCmdPool;
    
    BSpline spline;
    BSpline::Animation animation;


    std::vector<VkBuffer> uniformBuffers;
    VkDeviceMemory uniformBuffersMemory;
    VkDeviceSize uniformBufferSize;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkImage textureImage;
    VkDeviceMemory textureMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage depthImage;
    VkDeviceMemory depthMemory;
    VkImageView depthImageView;

    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    FILE* log_file{nullptr};

    int width_;
    int height_;

    bool framebufferResized_ = false;

};



#endif