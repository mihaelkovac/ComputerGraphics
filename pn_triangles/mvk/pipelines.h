#ifndef MVK_PIPELINES_H
#define MVK_PIPELINES_H

#include <vulkan/vulkan.h>

namespace mvk
{

    enum class ShaderStage
    {
        Undefined = 0,
        Vertex = VK_SHADER_STAGE_VERTEX_BIT,
        TessellationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TessellationEval = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
        Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        Compute = VK_SHADER_STAGE_COMPUTE_BIT,
        AllGraphics = VK_SHADER_STAGE_ALL_GRAPHICS,
        All = VK_SHADER_STAGE_ALL,
        Raygen = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        AnyHit = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        ClosestHit = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        Miss = VK_SHADER_STAGE_MISS_BIT_KHR,
        Intersection = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        Callable = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        Task = VK_SHADER_STAGE_TASK_BIT_NV,
        Mesh = VK_SHADER_STAGE_MESH_BIT_NV,
        RaygenNV = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        AnyHitNV = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        ClosestHitNV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        MissNV = VK_SHADER_STAGE_MISS_BIT_KHR,
        IntersectioNV = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        CallableNV = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        MaxEnum = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM
    };

    using ShaderStageFlags = util::EnumFlags<ShaderStage, VkShaderStageFlags>;

    enum class PipelineStage
    {
        Undefined = 0,
        TopOfPipe = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        DrawIndirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        VertexInput = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VertexShader = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        TessellationControlShader = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
        TessellationEvalShader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
        GeometryShader = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
        FragmentShader = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        EarlyFragmentTests = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        LateFragmentTests = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        ColorAttachmentOutput = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        ComputeShader = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Transfer = VK_PIPELINE_STAGE_TRANSFER_BIT,
        BottomOfPipe = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        Host = VK_PIPELINE_STAGE_HOST_BIT,
        AllGraphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        AllCommands = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        TransformFeedback = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
        ConditionalRendering = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
        RayTracingShader = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
        AccelerationStructureBuild = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        ShadingRateImage = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
        TaskShader = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
        MeshShader = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
        FragmentDensityProcess = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
        CommandPreprocess = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV
    };

    using PipelineStageFlags = util::EnumFlags<PipelineStage, VkPipelineStageFlags>;

    enum class PipelineBindPoint
    {
	    Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    	Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
    	RayTracing = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
    };
	
    namespace pipe
    {

        constexpr VkPipelineVertexInputStateCreateInfo NewVertexInput() noexcept
        {
            VkPipelineVertexInputStateCreateInfo vertex_input{};
            vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            return vertex_input;
        }

        constexpr VkPipelineInputAssemblyStateCreateInfo NewInputAssembly(const VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) noexcept
        {
            VkPipelineInputAssemblyStateCreateInfo input_assembly{};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = topology;
            input_assembly.primitiveRestartEnable = VK_FALSE;
            return input_assembly;
        }

        constexpr VkPipelineShaderStageCreateInfo NewShaderStage(const ShaderStage stage,
																 VkShaderModule module) noexcept
        {
            VkPipelineShaderStageCreateInfo shaders_info{};
            shaders_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaders_info.module = module;
            shaders_info.stage = static_cast<VkShaderStageFlagBits>(util::Underlying(stage));
            shaders_info.pName = "main";

            return shaders_info;
        }

        constexpr VkPipelineDynamicStateCreateInfo NewDynamicState() noexcept
        {
            VkPipelineDynamicStateCreateInfo dynamic_state{};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            return dynamic_state;
        }

        constexpr VkPipelineViewportStateCreateInfo NewViewportState() noexcept
        {
            VkPipelineViewportStateCreateInfo viewport_state{};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.scissorCount = 1;
            viewport_state.viewportCount = 1;
            return viewport_state;
        }

        constexpr VkPipelineTessellationStateCreateInfo NewTessellation(const uint32_t patch_ctrl_points) noexcept
        {
            VkPipelineTessellationStateCreateInfo tessellation{};
            tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
            tessellation.patchControlPoints = patch_ctrl_points;
            return tessellation;
        }

        constexpr VkPipelineRasterizationStateCreateInfo NewRasterizer() noexcept
        {
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.flags = 0;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;
            return rasterizer;
        }

        constexpr VkPipelineMultisampleStateCreateInfo NewMultisampling() noexcept
        {

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            return multisampling;
        }

        constexpr VkPipelineColorBlendStateCreateInfo NewColorBlend() noexcept
        {
            VkPipelineColorBlendStateCreateInfo color_blend{};
            color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend.logicOpEnable = VK_FALSE;
            color_blend.logicOp = VK_LOGIC_OP_COPY;
            color_blend.blendConstants[0] = 0.f;
            color_blend.blendConstants[1] = 0.f;
            color_blend.blendConstants[2] = 0.f;
            color_blend.blendConstants[3] = 0.f;

            return color_blend;
        }

        constexpr VkPipelineDepthStencilStateCreateInfo NewDepthStencil() noexcept
        {
            VkPipelineDepthStencilStateCreateInfo depth_stencil{};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = VK_TRUE;
            depth_stencil.depthWriteEnable = VK_TRUE;
            depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil.minDepthBounds = 0.f;
            depth_stencil.maxDepthBounds = 1.f;
            depth_stencil.stencilTestEnable = VK_FALSE;

            return depth_stencil;
        }

        constexpr VkPipelineColorBlendAttachmentState NewColorBlendAttachment() noexcept
        {
            VkPipelineColorBlendAttachmentState color_blend_attachment{};
            color_blend_attachment.blendEnable = VK_FALSE;
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            return color_blend_attachment;
        }

        constexpr VkPipelineLayoutCreateInfo NewLayout() noexcept
        {
            VkPipelineLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutInfo.setLayoutCount = 0;
            layoutInfo.pSetLayouts = nullptr;
            layoutInfo.pushConstantRangeCount = 0;
            layoutInfo.pPushConstantRanges = nullptr;

            return layoutInfo;
        }

        constexpr VkDescriptorSetLayoutCreateInfo NewDescriptorSetLayout() noexcept
        {
            VkDescriptorSetLayoutCreateInfo layout_info{};
            layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

            return layout_info;
        }

        constexpr VkDescriptorSetLayoutBinding DescriptorLayoutBinding(const ShaderStageFlags stages,
														               const VkDescriptorType descriptor_type,
														               const uint32_t binding,
														               const uint32_t count = 1,
														               const VkSampler* samplers = nullptr) noexcept
        {
            VkDescriptorSetLayoutBinding set_binding{};
            set_binding.descriptorCount = count;
            set_binding.stageFlags = stages;
            set_binding.descriptorType = descriptor_type;
            set_binding.pImmutableSamplers = samplers;
            set_binding.binding = binding;
        	
            return set_binding;
        }

    	[[nodiscard]]
        constexpr VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dst_set,
									                      const VkDescriptorType type,
									                      const uint32_t binding,
									                      const VkDescriptorBufferInfo* buffer_info,
									                      const uint32_t descriptor_count = 1) noexcept
		{

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = type;
            write.dstSet = dst_set;
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorCount = descriptor_count;
            write.pBufferInfo = buffer_info;
            write.pTexelBufferView = nullptr;
            write.pImageInfo = nullptr;
        	
            return write;

        }


    }
	
    struct ComputePipelineRequest
    {
        
        VkPipelineShaderStageCreateInfo* shader_info{ nullptr };
        VkComputePipelineCreateInfo* info_storage{ nullptr };
        VkPipelineLayout* layouts{ nullptr };
        VkPipeline* pipeline_storage;
        uint32_t pipe_count;
        VkPipelineCache cache;
    };


	


    struct GraphicsPipelinesRequest
    {

        VkPipelineVertexInputStateCreateInfo* vertex_inputs{ nullptr };
        VkPipelineInputAssemblyStateCreateInfo* input_assemblys{ nullptr };
        VkPipelineShaderStageCreateInfo* shaders_info{ nullptr };
        const uint32_t* shader_count{ nullptr };
        VkPipelineDynamicStateCreateInfo* dynamic_states{ nullptr };
        VkPipelineViewportStateCreateInfo* viewport_states{ nullptr };
        VkPipelineTessellationStateCreateInfo* tessellation{ nullptr };
        VkPipelineRasterizationStateCreateInfo* rasterizer{ nullptr };
        VkPipelineMultisampleStateCreateInfo* multisampling{ nullptr };
        VkPipelineColorBlendStateCreateInfo* color_blend{ nullptr };
        VkPipelineDepthStencilStateCreateInfo* depth_stencil{ nullptr };
        VkRenderPass* render_pass{ nullptr };
        uint32_t* subpass{ nullptr };
        VkGraphicsPipelineCreateInfo* info_storage{ nullptr };
        VkPipelineLayout* layouts{ nullptr };
        VkPipeline* pipelines{ nullptr };
        uint32_t pipeline_count{ 0 };
        VkPipelineCache cache{ VK_NULL_HANDLE };
    	
    };

    



}


#endif