#ifndef MVK_DEVICE_H
#define MVK_DEVICE_H

#include <array>
#include <vulkan/vulkan.h>

#include <vector>

#include "commands.h"
#include "device_memory.h"
#include "gpu.h"
#include "pipelines.h"
#include "queue.h"
#include "render_pass.h"

namespace mvk
{

    struct DefaultVkValidationPolicy
    {
        void ValidateVkResult(VkResult result, const char* failureMessage) const noexcept
        {
            MVK_VALIDATE_RESULT(result, failureMessage);
        }

        void Init() const noexcept
        {
        }
    };

    struct NoAllocationCallback
    {
        constexpr VkAllocationCallbacks* GetAllocationCallbacks() const noexcept
        {
            return nullptr;
        }

        void Init() const noexcept
        {
        }
    };


    template<bool RequireCompute>
    using DefaultDeviceQueuePolicy = typename util::TernaryTypeOp<
        RequireCompute,
        DefaultQueuePolicy<FamilyType::Compute, FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>,
        DefaultQueuePolicy<FamilyType::Graphics, FamilyType::Presentation, FamilyType::Transfer>>::Type;



    template<
        template<typename> typename AllocationPolicy = DefaultAllocPolicy,
        typename QueuePolicy = DefaultDeviceQueuePolicy<false>,
        typename VkValidationPolicy = DefaultVkValidationPolicy,
        typename AllocationCallbackPolicy = NoAllocationCallback
			>
    struct Device : public QueuePolicy, public VkValidationPolicy, public AllocationCallbackPolicy, public AllocationPolicy<Device<AllocationPolicy, QueuePolicy, VkValidationPolicy, AllocationCallbackPolicy>>
    {
        using AllocPolicy = AllocationPolicy<Device<AllocationPolicy, QueuePolicy, VkValidationPolicy, AllocationCallbackPolicy>>;

        constexpr Device() = default;

        template<typename Instance, typename QueueFamilies>
        void Init(Instance& instance,
				  VkPhysicalDevice gpu,
	              VkPhysicalDeviceMemoryProperties memory_props,
				  VkDevice device,
				  const QueueFamilies& families) noexcept
        {
            vk_gpu = gpu;
            memory_properties = memory_props;
            vk_device = device;
            QueuePolicy::Init(device, families);
            AllocPolicy::Init(instance);
            VkValidationPolicy::Init();
            AllocationCallbackPolicy::Init();
        }

        void Release() noexcept
        {
            AllocPolicy::Release();
            vkDestroyDevice(vk_device, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void GetSurfaceCapabilites(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& capabilities) const noexcept
        {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_gpu, surface, &capabilities);
        }


        VkCommandPool CreateCommandPool(uint32_t family_index, CommandPoolFlags pool_type = {}, const void* next = nullptr)
        {
            VkCommandPoolCreateInfo pool{};
            pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool.queueFamilyIndex = family_index;
            pool.pNext = next;
            pool.flags = pool_type;


            VkCommandPool result = VK_NULL_HANDLE;
            VkValidationPolicy::ValidateVkResult(vkCreateCommandPool(vk_device, &pool, AllocationCallbackPolicy::GetAllocationCallbacks(), &result),
                "Failed to create command pool");

            return result;
        }

        void CreateCommandBuffers(VkCommandPool pool,
								  CommandBuffer* buffers,
								  const size_t count,
								  const VkCommandBufferLevel level)
        {
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = pool;
            alloc_info.commandBufferCount = static_cast<uint32_t>(count);
            alloc_info.level = level;

            VkValidationPolicy::ValidateVkResult(vkAllocateCommandBuffers(vk_device, &alloc_info, reinterpret_cast<VkCommandBuffer*>(buffers)), "Device::CreateCommandBuffers - failed to allocate command buffers");
        }

        VkPipelineCache CreatePipelineCache(const bool synchronized_externally = false, const void* inital_data = nullptr, const uint32_t inital_data_size = 0) noexcept
        {
            VkPipelineCacheCreateInfo cache_info{};
            cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            cache_info.flags = synchronized_externally ? VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT : 0;
            cache_info.initialDataSize = inital_data_size;
            cache_info.pInitialData = inital_data;

            VkPipelineCache cache = VK_NULL_HANDLE;

            VkValidationPolicy::ValidateVkResult(vkCreatePipelineCache(vk_device, &cache_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &cache), "Failed to initalize pipeline cache");

            return cache;
        }

        void CreateComputePipelines(const ComputePipelineRequest& request) noexcept
        {
            for (uint32_t i = 0; i < request.pipe_count; ++i)
            {
                VkPipelineLayout& layout = request.layouts[i];

                VkComputePipelineCreateInfo& pipeInfo = request.info_storage[i];
                pipeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                pipeInfo.layout = layout;
                pipeInfo.stage = request.shader_info[i];


            }

            VkValidationPolicy::ValidateVkResult(vkCreateComputePipelines(vk_device,
																		  request.cache,
																		  request.pipe_count,
																		  request.info_storage,
																		  AllocationCallbackPolicy::GetAllocationCallbacks(),
																		  request.pipeline_storage), 
												 "Device::CreateComputePipelines - Failed to create compute pipelines");
        }

        void CreateGraphicsPipelines(const GraphicsPipelinesRequest& request) noexcept
        {
            size_t shader_index = 0;
            for (uint32_t i = 0; i < request.pipeline_count; ++i)
            {
                VkGraphicsPipelineCreateInfo& pipe_info = request.info_storage[i];

			#define _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(_info_member, _req_member)                        \
				pipe_info._info_member = request._req_member != nullptr ? &request._req_member[i] : nullptr \
	
                pipe_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipe_info.layout = request.layouts[i];
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pViewportState, viewport_states);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pVertexInputState, vertex_inputs);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pInputAssemblyState, input_assemblys);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pMultisampleState, multisampling);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pRasterizationState, rasterizer);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pTessellationState, tessellation);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pDynamicState, dynamic_states);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pDepthStencilState, depth_stencil);
                _GET_NON_NULL_GRAPHICS_REQUEST_MEMBER(pColorBlendState, color_blend);
                pipe_info.pStages = &request.shaders_info[shader_index];
                pipe_info.stageCount = request.shader_count[i];
                pipe_info.renderPass = request.render_pass[i];
            	if(request.subpass[i] != UINT32_MAX)
            	{
                    pipe_info.subpass = request.subpass[i];
            	}
                shader_index += request.shader_count[i];
            }

            VkValidationPolicy::ValidateVkResult(
	            vkCreateGraphicsPipelines(vk_device, request.cache, request.pipeline_count, request.info_storage,
	                                      AllocationCallbackPolicy::GetAllocationCallbacks(), request.pipelines),
                "Device::crateGraphicsPipelines - failed to create pipelines");
        }


        VkSwapchainKHR CreateSwapchain(const VkSwapchainCreateInfoKHR& swapchain_info)
        {
            VkSwapchainKHR swapchain = VK_NULL_HANDLE;
            VkValidationPolicy::ValidateVkResult(vkCreateSwapchainKHR(vk_device, &swapchain_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &swapchain), "Failed to create swapchain");
            return swapchain;
        }


        VkDescriptorPool CreateDescriptorPool(const VkDescriptorPoolSize* sizes, size_t pool_size_count, size_t max_sets, const void* next = nullptr) noexcept
        {

            VkDescriptorPool pool = VK_NULL_HANDLE;

            VkDescriptorPoolCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            info.pNext = next;
            info.pPoolSizes = sizes;
            info.poolSizeCount = static_cast<uint32_t>(pool_size_count);
            info.maxSets = static_cast<uint32_t>(max_sets);

            VkValidationPolicy::ValidateVkResult(vkCreateDescriptorPool(vk_device, &info, AllocationCallbackPolicy::GetAllocationCallbacks(), &pool),
                "Failed to create descriptor pool");

            return pool;
        }

        VkSemaphore CreateSemaphore(const void* next = nullptr, VkSemaphoreCreateFlags flags = 0) noexcept
        {
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = next;
            info.flags = 0;

            VkSemaphore semaphore;

            VkValidationPolicy::ValidateVkResult(vkCreateSemaphore(vk_device, &info, AllocationCallbackPolicy::GetAllocationCallbacks(), &semaphore),
                "Failed to create sempahore");

            return semaphore;
        }
    	
        VkFence CreateFence(bool signaled = false, const void* next = nullptr) noexcept
        {
            VkFenceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = next;
            if (signaled)
            {
                info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            }

            VkFence fence = VK_NULL_HANDLE;
            VkValidationPolicy::ValidateVkResult(vkCreateFence(vk_device, &info, AllocationCallbackPolicy::GetAllocationCallbacks(), &fence),
                "Failed to create fence");

            return fence;
        }

		VkFramebuffer CreateFramebuffer(VkRenderPass render_pass,
										const uint32_t width,
										const uint32_t height,
                                        const uint32_t layers,
                                        const VkImageView* attachemnts,
                                        const size_t attachment_count,
                                        const void* next = nullptr) noexcept
		{
            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.pNext = next;
            info.width = width;
            info.height = height;
            info.layers = layers;
            info.pAttachments = attachemnts;
            info.attachmentCount = static_cast<uint32_t>(attachment_count);
            info.renderPass = render_pass;
        	
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            VkValidationPolicy::ValidateVkResult(vkCreateFramebuffer(vk_device, &info, AllocationCallbackPolicy::GetAllocationCallbacks(), &framebuffer),
                "Failed to create frame buffer");

            return framebuffer;
        }

        VkShaderModule CreateShaderModule(const uint32_t* code,
							              const size_t code_size,
							              const void* next = nullptr) noexcept
        {
            VkShaderModuleCreateInfo shader_info{};
            shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_info.pCode = code;
            shader_info.codeSize = code_size;
            shader_info.pNext = next;

            VkShaderModule shader;
            VkValidationPolicy::ValidateVkResult(vkCreateShaderModule(vk_device, &shader_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &shader),
                "Device::CreateShaderModule - failed");

            return shader;
        }

    	VkShaderModule CreateShaderModule(const char* shader_file_location, const void* next = nullptr)
        {
            util::BinaryData shader_data = util::ReadFile(shader_file_location);
            MVK_CHECK_FATAL(shader_data.data, "Device::CreateShaderModule - Failed to read shader file");

            const auto shader_module = CreateShaderModule(reinterpret_cast<uint32_t*>(shader_data.data), shader_data.size);

            delete[] shader_data.data;
            return shader_module;
        }



        VkImageView CreateImageView(VkImage image,
						            VkImageViewType view_type,
						            VkFormat format,
						            const VkComponentMapping& components,
						            const VkImageSubresourceRange& subresource_range) noexcept
        {
            VkImageViewCreateInfo img_view_info{};
            img_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            img_view_info.image = image;
            img_view_info.viewType = view_type;
            img_view_info.format = format;
            img_view_info.components.r = components.r;
            img_view_info.components.g = components.g;
            img_view_info.components.b = components.b;
            img_view_info.components.a = components.a;
            img_view_info.subresourceRange.aspectMask = subresource_range.aspectMask;
            img_view_info.subresourceRange.baseMipLevel = subresource_range.baseMipLevel;
            img_view_info.subresourceRange.levelCount = subresource_range.levelCount;
            img_view_info.subresourceRange.baseArrayLayer = subresource_range.baseArrayLayer;
            img_view_info.subresourceRange.layerCount = subresource_range.layerCount;

            VkImageView image_view = VK_NULL_HANDLE;
            VkValidationPolicy::ValidateVkResult(vkCreateImageView(vk_device, &img_view_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &image_view), 
                "Device::CreateImageView - failed");

            return image_view;
        }

        VkFormat FindSupportedFormat(const VkFormat* candidates,
						             const size_t candidate_count,
						             const VkImageTiling tiling,
						             const FormatFeatureFlags features) noexcept
        {

            for (size_t i = 0; i < candidate_count; ++i)
            {
                VkFormatProperties properties;
                VkFormat format = candidates[i];
                vkGetPhysicalDeviceFormatProperties(vk_gpu, format, &properties);

                const FormatFeatureFlags linear_tiling_features = properties.linearTilingFeatures;
                const FormatFeatureFlags optimal_tiling_features = properties.optimalTilingFeatures;

                if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (linear_tiling_features.Contains(features) || optimal_tiling_features.Contains(features)))
                {
                    return format;
                }

            }

            return VK_FORMAT_MAX_ENUM;
        }

        VkDevice GetVkDevice() noexcept
        {
            return vk_device;
        }

        VkPhysicalDevice GetVkGPU() noexcept
        {
            return vk_gpu;
        }

        const VkPhysicalDeviceMemoryProperties& GetGPUMemoryProperties() const noexcept
        {
            return memory_properties;
        }

        operator VkDevice() noexcept
        {
            return vk_device;
        }

        [[nodiscard]] VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& descriptor_layout_info) noexcept
        {
            VkDescriptorSetLayout layout{0};
            VkValidationPolicy::ValidateVkResult(vkCreateDescriptorSetLayout(vk_device, &descriptor_layout_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &layout),
					"Device::CreateDescriptorSetLayout - Failed");

            return layout;
        }

        VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo& layout_info) noexcept
        {
            VkPipelineLayout layout{ 0 };
            VkValidationPolicy::ValidateVkResult(vkCreatePipelineLayout(vk_device, &layout_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &layout),
                "Device::CreatePipelineLayout - failed");
            return layout;
        }

        void AllocateDescriptorSets(VkDescriptorPool pool,
									const VkDescriptorSetLayout* layouts,
									VkDescriptorSet* sets,
									const uint32_t count) noexcept
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = pool;
            allocInfo.pSetLayouts = layouts;
            allocInfo.descriptorSetCount = count;

            VkValidationPolicy::ValidateVkResult(vkAllocateDescriptorSets(vk_device, &allocInfo, sets), 
                "Device::AllocateDescriptorSets - failed");
        }

        void DestroyCommandBuffers(VkCommandPool pool, mvk::CommandBuffer* buffers, const size_t buffer_count = 1)
        {
            vkFreeCommandBuffers(vk_device, pool, buffer_count, reinterpret_cast<VkCommandBuffer*>(buffers));
        }

        void DestroyShaderModule(VkShaderModule shader) noexcept
        {
            vkDestroyShaderModule(vk_device, shader, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

    	void DestroyImageView(VkImageView image_view) noexcept
        {
            vkDestroyImageView(vk_device, image_view, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        RenderPass CreateRenderPass(const VkRenderPassCreateInfo& pass_info) noexcept
        {
            VkRenderPass render_pass;
            VkValidationPolicy::ValidateVkResult(vkCreateRenderPass(vk_device, &pass_info, AllocationCallbackPolicy::GetAllocationCallbacks(), &render_pass),
                "Device::CreateRenderPass - Failed to create RenderPass");

            return { render_pass };
        }

        void CreateGraphicsPipelines(const VkGraphicsPipelineCreateInfo* infos, VkPipeline* pipelines, const size_t count, VkPipelineCache cache) noexcept
        {
            VkValidationPolicy::ValidateVkResult(vkCreateGraphicsPipelines(vk_device, cache,
																		   static_cast<uint32_t>(count), infos,
																		   AllocationCallbackPolicy::GetAllocationCallbacks(), pipelines),
                "Device::CreateGraphicsPipelines - Failed to create graphics pipelines");
        }

        void DestroyFramebuffers(VkFramebuffer* framebuffers, const size_t count) noexcept
        {
        	for(size_t i = 0; i < count; ++i)
        	{
				vkDestroyFramebuffer(vk_device, framebuffers[i], AllocationCallbackPolicy::GetAllocationCallbacks());
        	}
        }

        void DestroyPipelineCache(VkPipelineCache cache) noexcept
        {
            vkDestroyPipelineCache(vk_device, cache, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void DestroyPipeline(VkPipeline pipeline) noexcept
        {
            vkDestroyPipeline(vk_device, pipeline, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void DestroyPipelineLayout(VkPipelineLayout layout) noexcept
        {
            vkDestroyPipelineLayout(vk_device, layout, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void DestroyRenderPass(RenderPass& render_pass) noexcept
        {
            vkDestroyRenderPass(vk_device, render_pass.vk_render_pass,
                                AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void DestroyDescriptorPool(VkDescriptorPool descriptor_pool) noexcept
        {
            vkDestroyDescriptorPool(vk_device, descriptor_pool, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void DestroyDescriptorSetLayout(VkDescriptorSetLayout layout) noexcept
        {
            vkDestroyDescriptorSetLayout(vk_device, layout, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void DestroyCommandPool(CommandPool& command_pool) noexcept
        {
            vkDestroyCommandPool(vk_device, command_pool, AllocationCallbackPolicy::GetAllocationCallbacks());
        }

        void UpdateDescriptorSet(const VkWriteDescriptorSet* writes, const size_t write_count = 1,
								 const VkCopyDescriptorSet* copies = nullptr,  const size_t copy_count = 0) noexcept
        {
            vkUpdateDescriptorSets(vk_device, write_count, writes, copy_count, copies);
        }


        VkPhysicalDevice vk_gpu{ VK_NULL_HANDLE };
        VkPhysicalDeviceMemoryProperties memory_properties{};
        VkDevice vk_device{ VK_NULL_HANDLE };

    };


    using DefaultDevice = Device<>;


    template<
        template<typename> typename GPUPickPolicy = DefaultGPUPickPolicy,
        typename QueueFamilyPolicy = DefaultQueueFamilyPolicy<false>
    >
        struct DeviceBuilder : GPUPickPolicy<QueueFamilyPolicy>
    {

        using GPUPickPolicy_T = GPUPickPolicy<QueueFamilyPolicy>;
        using QueuePickPolicy = QueueFamilyPolicy;

        DeviceBuilder() noexcept
        {
            device_info_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_info_.pEnabledFeatures = &enabledFeatures_;

        }

        DeviceBuilder& SetDeviceFeature(DeviceFeature feature, bool enabled = true) noexcept
        {
            char* feature_data = reinterpret_cast<char*>(&enabledFeatures_);
            VkBool32* feature_member = reinterpret_cast<VkBool32*>(feature_data + util::Underlying(feature));
            *feature_member = static_cast<VkBool32>(enabled);

            if (enabled)
            {
                required_features_.emplace_back(feature);
            }
            else
            {
                auto it = required_features_.begin();
                for (auto end = required_features_.end(); it != end; ++it)
                {
                    if (*it == feature)
                    {
                        break;
                    }
                }

                required_features_.erase(it);
            }


            return *this;
        }

        DeviceBuilder& SetDeviceFeatures(std::initializer_list<DeviceFeature> features, bool enabled = true) noexcept
        {
            for (auto feature : features)
            {
                SetDeviceFeature(feature, enabled);
            }

            return *this;
        }


        DeviceBuilder& EnableDeviceExtension(const char* extension) noexcept
        {
            enabled_extensions_.push_back(extension);
            return *this;
        }

        DeviceBuilder& EnableDeviceLayer(const char* layer) noexcept
        {
            enabled_layers_.push_back(layer);
            return *this;
        }

        DeviceBuilder& SetRequiredGPUType(GPUType type) noexcept
        {
            required_gpu_type_ = type;
            return *this;
        }

        template <typename Device>
        void BuildDevice(Instance& instance,
			             VkSurfaceKHR vk_surface,
			             Device& device,
			             VkAllocationCallbacks* allocation_cbs = nullptr) noexcept
        {
            using PickedGPUInfo = typename GPUPickPolicy_T::PickedGPUInfo;

            auto available_gpus = instance.GetGPUs();
            VkInstance vk_instance = instance.GetVkInstance();
            PickedGPUInfo picked_gpu_info = GPUPickPolicy_T::PickGPU(available_gpus,
                required_gpu_type_,
                enabled_extensions_,
                enabled_layers_,
                required_features_,
                vk_surface);



            SetLayersAndExtensions();
            auto queueInfos = QueueFamilyPolicy::GetQueueInfos(picked_gpu_info.indices);

            device_info_.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
            device_info_.pQueueCreateInfos = queueInfos.data();


            VkPhysicalDevice gpu = picked_gpu_info.gpu->vk_gpu;
            VkDevice vk_device;
            vkCreateDevice(gpu, &device_info_, allocation_cbs, &vk_device);


            device.Init(instance, gpu, picked_gpu_info.gpu->memory_properties, vk_device, picked_gpu_info.indices);
        }

    protected:

        inline void SetLayersAndExtensions() noexcept
        {
            device_info_.ppEnabledExtensionNames = enabled_extensions_.data();
            device_info_.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions_.size());

            device_info_.ppEnabledLayerNames = enabled_layers_.data();
            device_info_.enabledLayerCount = static_cast<uint32_t>(enabled_layers_.size());
        }

        VkDeviceCreateInfo device_info_{};
        VkPhysicalDeviceFeatures enabledFeatures_{};
        GPUType required_gpu_type_;
        std::vector<DeviceFeature> required_features_{};
        std::vector<const char*> enabled_extensions_;
        std::vector<const char*> enabled_layers_;


    };

    template<bool RequireCompute = false>
    using DefaultDeviceBuilder = DeviceBuilder<DefaultGPUPickPolicy, DefaultQueueFamilyPolicy<RequireCompute>>;

} // namespace mvk


#endif