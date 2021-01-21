#ifndef MVK_BUFFER_H
#define MVK_BUFFER_H

#include <vulkan/vulkan.h>

#include "utils.h"

namespace mvk
{

    enum class BufferUsage
    {
        Undefined = 0,
        TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        UniformTexture = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
        TexelStorage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
        Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Indirect = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        TransformFeedback = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
        TransformFeedbackCounter = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
        ConditionalRendering = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
        RayTracingKHR = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR,
        RayTracingNV = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
        ShaderDeviceAddressEXT = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        ShaderDeviceAddressKHR = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    using BufferUsageFlags = util::EnumFlags<BufferUsage, VkBufferUsageFlags>;


    enum class BufferCreateFlag
    {
        Undefined = 0,
        SparseBinding = VK_BUFFER_CREATE_SPARSE_BINDING_BIT,
        SparseResidency = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT,
        SparseAliased = VK_BUFFER_CREATE_SPARSE_ALIASED_BIT,
        Protected = VK_BUFFER_CREATE_PROTECTED_BIT,
        DeviceCaptureReplay = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
        DeviceCaptureReplayEXT = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
        DeviceCaptureReplayKHR = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT
    };

    using BufferCreateFlags = util::EnumFlags<BufferCreateFlag, VkBufferCreateFlags>;
    
    
} // namespace mvk



#endif