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
	
    namespace pipe
    {

        constexpr VkPipelineVertexInputStateCreateInfo newVertexInput() noexcept
        {
            VkPipelineVertexInputStateCreateInfo vertexInput{};
            vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            return vertexInput;
        }

        constexpr VkPipelineInputAssemblyStateCreateInfo newInputAssembly() noexcept
        {
            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;
            inputAssembly.flags = 0;
            return inputAssembly;
        }

        constexpr VkPipelineShaderStageCreateInfo newShaderStage() noexcept
        {
            VkPipelineShaderStageCreateInfo shadersInfo{};
            shadersInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shadersInfo.pName = "main";

            return shadersInfo;
        }

        constexpr VkPipelineDynamicStateCreateInfo newDynamicState() noexcept
        {
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            return dynamicState;
        }

        constexpr VkPipelineViewportStateCreateInfo newViewportState() noexcept
        {
            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            return viewportState;
        }

        constexpr VkPipelineTessellationStateCreateInfo newTessellation() noexcept
        {
            VkPipelineTessellationStateCreateInfo tessellation{};
            tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

            return tessellation;
        }

        constexpr VkPipelineRasterizationStateCreateInfo newRasterizer() noexcept
        {
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.flags = 0;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;
            return rasterizer;
        }

        constexpr VkPipelineMultisampleStateCreateInfo newMultisampling() noexcept
        {

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            return multisampling;
        }

        constexpr VkPipelineColorBlendStateCreateInfo newColorBlend() noexcept
        {
            VkPipelineColorBlendStateCreateInfo colorBlend{};
            colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            //colorBlend.logicOpEnable = VK_FALSE;
            //colorBlend.logicOp = VK_LOGIC_OP_COPY;
            //colorBlend.blendConstants[0] = 0.f;
            //colorBlend.blendConstants[1] = 0.f;
            //colorBlend.blendConstants[2] = 0.f;
            //colorBlend.blendConstants[3] = 0.f;

            return colorBlend;
        }

        constexpr VkPipelineDepthStencilStateCreateInfo newDepthStencil() noexcept
        {
            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.minDepthBounds = 0.f;
            depthStencil.maxDepthBounds = 1.f;
            depthStencil.stencilTestEnable = VK_FALSE;

            return depthStencil;
        }

        constexpr VkPipelineColorBlendAttachmentState newColorBlendAttachment() noexcept
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            return colorBlendAttachment;
        }

        constexpr VkPipelineLayoutCreateInfo newLayout() noexcept
        {
            VkPipelineLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutInfo.setLayoutCount = 0;
            layoutInfo.pSetLayouts = nullptr;
            layoutInfo.pushConstantRangeCount = 0;
            layoutInfo.pPushConstantRanges = nullptr;

            return layoutInfo;
        }

        constexpr VkDescriptorSetLayoutCreateInfo createLayout() noexcept
        {
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

            return layoutInfo;
        }

        constexpr VkDescriptorSetLayoutBinding descriptorLayoutBinding(const ShaderStageFlags stages,
            const VkDescriptorType descriptorType,
            const uint32_t binding,
            const uint32_t count = 1,
            const VkSampler* samplers = nullptr) noexcept
        {
            VkDescriptorSetLayoutBinding setBinding{};
            setBinding.descriptorCount = count;
            setBinding.stageFlags = stages;
            setBinding.descriptorType = descriptorType;
            setBinding.pImmutableSamplers = samplers;
            setBinding.binding = binding;
        	
            return setBinding;
        }

        constexpr VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            const VkDescriptorType type,
            const uint32_t binding,
            VkDescriptorBufferInfo* bufferInfo,
            const uint32_t descriptorCount = 1) noexcept
        {

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = type;
            write.dstSet = dstSet;
            write.dstBinding = binding;
            write.descriptorCount = descriptorCount;
            write.pBufferInfo = bufferInfo;

            return write;

        }


    }
	
    struct ComputePipelineRequest
    {
        
        VkPipelineShaderStageCreateInfo* shaderInfo{ nullptr };
    };


	


    struct GraphicsPipelineRequest
    {

        VkPipelineVertexInputStateCreateInfo* vertexInput{ nullptr };
        VkPipelineInputAssemblyStateCreateInfo* inputAssembly{ nullptr };
        VkPipelineShaderStageCreateInfo* shadersInfo{ nullptr };
        uint32_t shaderCount{ 0 };
        VkPipelineDynamicStateCreateInfo* dynamicState{ nullptr };
        VkPipelineViewportStateCreateInfo* viewportState{ nullptr };
        VkPipelineTessellationStateCreateInfo* tessellation{ nullptr };
        VkPipelineRasterizationStateCreateInfo* rasterizer{ nullptr };
        VkPipelineMultisampleStateCreateInfo* multisampling{ nullptr };
        VkPipelineColorBlendStateCreateInfo* colorBlend{ nullptr };
        VkPipelineDepthStencilStateCreateInfo* depthStencil{ nullptr };
        VkRenderPass renderPass{ 0 };
        uint32_t subpass{ 0 };
    };

    



}


#endif