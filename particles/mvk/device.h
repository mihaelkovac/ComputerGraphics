#ifndef MVK_DEVICE_H
#define MVK_DEVICE_H

#include <vulkan/vulkan.h>

#include <vector>

#include "commands.h"
#include "device_memory.h"
#include "gpu.h"
#include "pipelines.h"
#include "queue.h"

namespace mvk
{

    struct DefaultVkValidationPolicy
    {
        void validateVkResult(VkResult result, const char* failureMessage) const noexcept
        {
            MVK_VALIDATE_RESULT(result, failureMessage);
        }

        void init() const noexcept
        {
        }
    };

    struct NoAllocationCallback
    {
        constexpr VkAllocationCallbacks* getAllocationCallbacks() const noexcept
        {
            return nullptr;
        }

        void init() const noexcept
        {
        }
    };


    template<bool RequireCompute>
    using DefaultDeviceQueuePolicy = util::TernaryTypeOp<
        RequireCompute,
        DefaultQueuePolicy<FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>,
        DefaultQueuePolicy<FamilyType::Compute, FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>>;

    template <
        template <typename> typename AllocationPolicy = DefaultAllocPolicy,
        typename QueuePolicy = DefaultDeviceQueuePolicy<false>,
        typename VkValidationPolicy = DefaultVkValidationPolicy,
        typename AllocationCallbackPolicy = NoAllocationCallback>
    struct Device : public QueuePolicy, public VkValidationPolicy, public AllocationCallbackPolicy, public AllocationPolicy<Device<AllocationPolicy, QueuePolicy, VkValidationPolicy, AllocationCallbackPolicy>>
    {
        using AllocPolicy = AllocationPolicy<Device<AllocationPolicy, QueuePolicy, VkValidationPolicy, AllocationCallbackPolicy>>;

        constexpr Device() = default;

        template <typename Instance, typename QueueFamilies>
        void init(Instance &instance,
                  VkPhysicalDevice gpu,
                  VkPhysicalDeviceMemoryProperties memoryProps,
                  VkDevice device,
                  const QueueFamilies &families) noexcept
        {
            vkGPU = gpu;
            memoryProperties = memoryProps;
            vkDevice = device;
            QueuePolicy::init(device, families);
            AllocPolicy::init(instance);
            VkValidationPolicy::init();
            AllocationCallbackPolicy::init();
        }

        void release() noexcept
        {
            vkDestroyDevice(vkDevice, AllocationCallbackPolicy::getAllocationCallbacks());
        }

        void getSurfaceCapabilites(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& capabilities) const noexcept
        {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkGPU, surface, &capabilities);
        }


        VkCommandPool createCommandPool(uint32_t familyIndex, CommandPoolFlags poolType = {}, const void* next = nullptr)
        {
            VkCommandPoolCreateInfo pool{};
            pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool.queueFamilyIndex = familyIndex;
            pool.pNext = next;
            pool.flags = poolType;


            VkCommandPool result = VK_NULL_HANDLE;
            VkValidationPolicy::validateVkResult(vkCreateCommandPool(vkDevice, &pool, AllocationCallbackPolicy::getAllocationCallbacks(), &result),
                "Failed to create command pool");

            return result;
        }

        void createCommandBuffers(VkCommandPool pool,
								  VkCommandBuffer* buffers,
								  const uint32_t count,
								  const VkCommandBufferLevel level)
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = pool;
            allocInfo.commandBufferCount = count;
            allocInfo.level = level;

            VkValidationPolicy::validateVkResult(vkAllocateCommandBuffers(vkDevice, &allocInfo, buffers), "Device::createCommandBuffers - failed to allocate command buffers");
        }

        VkPipelineCache createPipelineCache(const bool synchronizedExternally = false, const void* initalData = nullptr, const uint32_t initalDataSize = 0) noexcept
        {
            VkPipelineCacheCreateInfo cacheInfo{};
            cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            cacheInfo.flags = synchronizedExternally ? VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT : 0;
            cacheInfo.initialDataSize = initalDataSize;
            cacheInfo.pInitialData = initalData;

            VkPipelineCache cache = VK_NULL_HANDLE;

            VkValidationPolicy::validateVkResult(vkCreatePipelineCache(vkDevice, &cacheInfo, AllocationCallbackPolicy::getAllocationCallbacks(), &cache), "Failed to initalize pipeline cache");

            return cache;
        }

        void createComputePipelines(const ComputePipelineRequest* pipeRequests,
            VkComputePipelineCreateInfo* createInfoStorage,
            VkPipelineLayout* layouts,
            VkPipeline* pipelineStorage,
            const uint32_t pipeCount,
            VkPipelineCache cache) noexcept
        {
            for (uint32_t i = 0; i < pipeCount; ++i)
            {
                const ComputePipelineRequest& request = pipeRequests[i];
                VkPipelineLayout& layout = layouts[i];

                VkComputePipelineCreateInfo& pipeInfo = createInfoStorage[i];
                pipeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                pipeInfo.layout = layout;
                pipeInfo.stage = *request.shaderInfo;


            }

            VkValidationPolicy::validateVkResult(vkCreateComputePipelines(vkDevice, cache, pipeCount, createInfoStorage, AllocationCallbackPolicy::getAllocationCallbacks(), pipelineStorage), "Device::createComputePipelines - Failed to create compute pipelines");
        }

        void createGraphicsPipelines(const GraphicsPipelineRequest* pipeRequests,
            VkGraphicsPipelineCreateInfo* createInfoStorage,
            VkPipelineLayout* layouts,
            VkPipeline* pipelineStorage,
            const uint32_t pipeCount,
            VkPipelineCache cache) noexcept
        {
            for (uint32_t i = 0; i < pipeCount; ++i)
            {
                const GraphicsPipelineRequest& request = pipeRequests[i];

                VkGraphicsPipelineCreateInfo& pipeInfo = createInfoStorage[i];
                pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipeInfo.layout = layouts[i];
                pipeInfo.pViewportState = request.viewportState;
                pipeInfo.pVertexInputState = request.vertexInput;
                pipeInfo.pInputAssemblyState = request.inputAssembly;
                pipeInfo.pMultisampleState = request.multisampling;
                pipeInfo.pRasterizationState = request.rasterizer;
                pipeInfo.pTessellationState = request.tessellation;
                pipeInfo.pDynamicState = request.dynamicState;
                pipeInfo.pDepthStencilState = request.depthStencil;
                pipeInfo.pColorBlendState = request.colorBlend;
                pipeInfo.pStages = request.shadersInfo;
                pipeInfo.stageCount = request.shaderCount;
                pipeInfo.renderPass = request.renderPass;
            	if(request.subpass != UINT32_MAX)
            	{
                    pipeInfo.subpass = request.subpass;
            	}

            }

            VkValidationPolicy::validateVkResult(
	            vkCreateGraphicsPipelines(vkDevice, cache, pipeCount, createInfoStorage,
	                                      AllocationCallbackPolicy::getAllocationCallbacks(), pipelineStorage),
                "Device::crateGraphicsPipelines - failed to create pipelines");
        }


        VkSwapchainKHR createSwapchain(const VkSwapchainCreateInfoKHR& swapchainInfo)
        {
            VkSwapchainKHR swapchain = VK_NULL_HANDLE;
            VkValidationPolicy::validateVkResult(vkCreateSwapchainKHR(vkDevice, &swapchainInfo, AllocationCallbackPolicy::getAllocationCallbacks(), &swapchain), "Failed to create swapchain");
            return swapchain;
        }


        VkDescriptorPool createDescriptorPool(const VkDescriptorPoolSize* sizes, size_t poolSizeCount, size_t maxSets, const void* next = nullptr) noexcept
        {

            VkDescriptorPool pool = VK_NULL_HANDLE;

            VkDescriptorPoolCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            info.pNext = next;
            info.pPoolSizes = sizes;
            info.poolSizeCount = static_cast<uint32_t>(poolSizeCount);
            info.maxSets = static_cast<uint32_t>(maxSets);

            VkValidationPolicy::validateVkResult(vkCreateDescriptorPool(vkDevice, &info, AllocationCallbackPolicy::getAllocationCallbacks(), &pool),
                "Failed to create descriptor pool");

            return pool;
        }

        VkSemaphore createSemaphore(const void* next = nullptr, VkSemaphoreCreateFlags flags = 0) noexcept
        {
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = next;
            info.flags = 0;
            
            VkSemaphore semaphore;

            VkValidationPolicy::validateVkResult(vkCreateSemaphore(vkDevice, &info, AllocationCallbackPolicy::getAllocationCallbacks(), &semaphore),
                "Failed to create sempahore");

            return semaphore;
        }

        VkFence createFence(bool signaled = false, const void* next = nullptr) noexcept
        {
            VkFenceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = next;
            if (signaled)
            {
                info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            }

            VkFence fence = VK_NULL_HANDLE;
            VkValidationPolicy::validateVkResult(vkCreateFence(vkDevice, &info, AllocationCallbackPolicy::getAllocationCallbacks(), &fence),
                "Failed to create fence");

            return fence;
        }

		VkFramebuffer createFramebuffer(VkRenderPass renderPass,
										const uint32_t width,
										const uint32_t height,
                                        const uint32_t layers,
                                        const VkImageView* attachemnts,
                                        const size_t attachmentCount,
                                        const void* next = nullptr) noexcept
		{
            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.pNext = next;
            info.width = width;
            info.height = height;
            info.layers = layers;
            info.pAttachments = attachemnts;
            info.attachmentCount = static_cast<uint32_t>(attachmentCount);
            info.renderPass = renderPass;
        	
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            VkValidationPolicy::validateVkResult(vkCreateFramebuffer(vkDevice, &info, AllocationCallbackPolicy::getAllocationCallbacks(), &framebuffer),
                "Failed to create frame buffer");

            return framebuffer;
        }

        VkShaderModule createShaderModule(
            const uint32_t* code,
            const size_t codeSize,
            const void* next = nullptr) noexcept
        {
            VkShaderModuleCreateInfo shaderInfo{};
            shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderInfo.pCode = code;
            shaderInfo.codeSize = codeSize;
            shaderInfo.pNext = next;

            VkShaderModule shader;
            VkValidationPolicy::validateVkResult(vkCreateShaderModule(vkDevice, &shaderInfo, AllocationCallbackPolicy::getAllocationCallbacks(), &shader),
                "Device::createShaderModule - failed");

            return shader;
        }



        VkImageView createImageView(VkImage image,
            VkImageViewType viewType,
            VkFormat format,
            const VkComponentMapping& components,
            const VkImageSubresourceRange& subresourceRange) noexcept
        {
            VkImageViewCreateInfo imgViewInfo{};
            imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imgViewInfo.image = image;
            imgViewInfo.viewType = viewType;
            imgViewInfo.format = format;
            imgViewInfo.components.r = components.r;
            imgViewInfo.components.g = components.g;
            imgViewInfo.components.b = components.b;
            imgViewInfo.components.a = components.a;
            imgViewInfo.subresourceRange.aspectMask = subresourceRange.aspectMask;
            imgViewInfo.subresourceRange.baseMipLevel = subresourceRange.baseMipLevel;
            imgViewInfo.subresourceRange.levelCount = subresourceRange.levelCount;
            imgViewInfo.subresourceRange.baseArrayLayer = subresourceRange.baseArrayLayer;
            imgViewInfo.subresourceRange.layerCount = subresourceRange.layerCount;

            VkImageView imageView = VK_NULL_HANDLE;
            VkValidationPolicy::validateVkResult(vkCreateImageView(vkDevice, &imgViewInfo, AllocationCallbackPolicy::getAllocationCallbacks(), &imageView), 
                "Device::createImageView - failed");

            return imageView;
        }

        VkFormat findSupportedFormat(const VkFormat *candidates,
                                     const size_t candidateCount,
                                     const VkImageTiling tiling,
                                     const FormatFeatureFlags features) noexcept
        {

            for (size_t i = 0; i < candidateCount; ++i)
            {
                VkFormatProperties properties;
                VkFormat format = candidates[i];
                vkGetPhysicalDeviceFormatProperties(vkGPU, format, &properties);

                const FormatFeatureFlags linearTilingFeatures = properties.linearTilingFeatures;
                const FormatFeatureFlags optimalTilingFeatures = properties.optimalTilingFeatures;

                if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                    (linearTilingFeatures.contains(features) || optimalTilingFeatures.contains(features)))
                {
                    return format;
                }

            }

            return VK_FORMAT_MAX_ENUM;
        }

        VkDevice getVkDevice() noexcept
        {
            return vkDevice;
        }

        VkPhysicalDevice getVkGPU() noexcept
        {
            return vkGPU;
        }

        const VkPhysicalDeviceMemoryProperties& getGPUMemoryProperties() const noexcept
        {
            return memoryProperties;
        }

        operator VkDevice() noexcept
        {
            return vkDevice;
        }

        VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& descriptorLayoutInfo) noexcept
        {
            VkDescriptorSetLayout layout{0};
            VkValidationPolicy::validateVkResult(vkCreateDescriptorSetLayout(vkDevice, &descriptorLayoutInfo, AllocationCallbackPolicy::getAllocationCallbacks(), &layout),
					"Device::createDescriptorSetLayout - Failed");

            return layout;
        }

        VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& layoutInfo) noexcept
        {
            VkPipelineLayout layout{ 0 };
            VkValidationPolicy::validateVkResult(vkCreatePipelineLayout(vkDevice, &layoutInfo, AllocationCallbackPolicy::getAllocationCallbacks(), &layout),
                "Device::createPipelineLayout - failed");
            return layout;
        }

        void allocateDescriptorSets(VkDescriptorPool pool,
									const VkDescriptorSetLayout* layouts,
									VkDescriptorSet* sets,
									const uint32_t count) noexcept
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = pool;
            allocInfo.pSetLayouts = layouts;
            allocInfo.descriptorSetCount = count;

            VkValidationPolicy::validateVkResult(vkAllocateDescriptorSets(vkDevice, &allocInfo, sets), 
                "Device::allocateDescriptorSets - failed");
        }

        void destroyCommandBuffers(VkCommandPool pool, VkCommandBuffer* buffers, const uint32_t bufferCount = 1)
        {
            vkFreeCommandBuffers(vkDevice, pool, bufferCount, buffers);
        }

        void destroyShaderModule(VkShaderModule shader) noexcept
        {
            vkDestroyShaderModule(vkDevice, shader, AllocationCallbackPolicy::getAllocationCallbacks());
        }


        VkPhysicalDevice vkGPU{ VK_NULL_HANDLE };
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        VkDevice vkDevice{ VK_NULL_HANDLE };

    };


    using DefaultDevice = Device<>;

    template <
        template <typename> typename GPUPickPolicy = DefaultGPUPickPolicy,
        typename QueueFamilyPolicy = DefaultQueueFamilyPolicy<false>>
    struct DeviceBuilder : GPUPickPolicy<QueueFamilyPolicy>
    {

        using GPUPickPolicy_T = GPUPickPolicy<QueueFamilyPolicy>;
        using QueuePickPolicy = QueueFamilyPolicy;

        DeviceBuilder() noexcept
        {
            deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceInfo.pEnabledFeatures = &enabledFeatures_;

        }

        DeviceBuilder& setDeviceFeature(DeviceFeature feature, bool enabled = true) noexcept
        {
            char* featureData = reinterpret_cast<char*>(&enabledFeatures_);
            VkBool32* featureMember = reinterpret_cast<VkBool32*>(featureData + util::underlying(feature));
            *featureMember = static_cast<VkBool32>(enabled);

            if (enabled)
            {
                requiredFeatures.emplace_back(feature);
            }
            else
            {
                auto it = requiredFeatures.begin();
                for (auto end = requiredFeatures.end(); it != end; ++it)
                {
                    if (*it == feature)
                    {
                        break;
                    }
                }

                requiredFeatures.erase(it);
            }


            return *this;
        }

        DeviceBuilder& setDeviceFeatures(std::initializer_list<DeviceFeature> features, bool enabled = true) noexcept
        {
            for (auto feature : features)
            {
                setDeviceFeature(feature, enabled);
            }

            return *this;
        }


        DeviceBuilder& enableDeviceExtension(const char* extension) noexcept
        {
            enabledExtensions_.push_back(extension);
            return *this;
        }

        DeviceBuilder& enableDeviceLayer(const char* layer) noexcept
        {
            enabledLayers_.push_back(layer);
            return *this;
        }

        DeviceBuilder& setRequiredGPUType(GPUType type) noexcept
        {
            requiredGPUType = type;
            return *this;
        }

        template <typename Device>
        void buildDevice(Instance &instance,
                         VkSurfaceKHR vkSurface,
                         Device &device,
                         VkAllocationCallbacks *allocationCbs = nullptr)
        {
            using PickedGPUInfo = typename GPUPickPolicy_T::PickedGPUInfo;

            auto availableGPUs = instance.getGPUs();
            VkInstance vkInstance = instance.getVkInstance();
            PickedGPUInfo pickedGPUInfo = GPUPickPolicy_T::pickGPU(availableGPUs,
                                                                   requiredGPUType,
                                                                   enabledExtensions_,
                                                                   enabledLayers_,
                                                                   requiredFeatures,
                                                                   vkSurface);

            setLayersAndExtensions();
            auto queueInfos = QueueFamilyPolicy::getQueueInfos(pickedGPUInfo.indices);

            deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
            deviceInfo.pQueueCreateInfos = queueInfos.data();


            VkPhysicalDevice gpu = pickedGPUInfo.gpu->vkGPU;
            VkDevice vkDevice;
            vkCreateDevice(gpu, &deviceInfo, allocationCbs, &vkDevice);


            device.init(instance, gpu, pickedGPUInfo.gpu->memoryProperties, vkDevice, pickedGPUInfo.indices);
        }

    protected:

        inline void setLayersAndExtensions() noexcept
        {
            deviceInfo.ppEnabledExtensionNames = enabledExtensions_.data();
            deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions_.size());

            deviceInfo.ppEnabledLayerNames = enabledLayers_.data();
            deviceInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers_.size());
        }

        VkDeviceCreateInfo deviceInfo{};
        VkPhysicalDeviceFeatures enabledFeatures_{};
        GPUType requiredGPUType;
        std::vector<DeviceFeature> requiredFeatures{};
        std::vector<const char*> enabledExtensions_;
        std::vector<const char*> enabledLayers_;


    };

    template<bool RequireCompute = false>
    using DefaultDeviceBuilder = DeviceBuilder<DefaultGPUPickPolicy, DefaultQueueFamilyPolicy<RequireCompute>>;

} // namespace mvk


#endif