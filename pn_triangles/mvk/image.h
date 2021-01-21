#ifndef MVK_IMAGE_H
#define MVK_IMAGE_H

#include <vulkan/vulkan.h>

namespace mvk
{


    enum class ImageUsage
    {
        Undefined = 0,
        TrasnferSrc = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        TransferDst = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        Sampled = VK_IMAGE_USAGE_SAMPLED_BIT,
        Storage = VK_IMAGE_USAGE_STORAGE_BIT,
        ColorAttachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        DepthStencilAttachment = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        TransientAttachment = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        InputAttachment = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        ShadingRateImage = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV,
        FragmentDensityMap = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT
    };

    using ImageUsageFlags = util::EnumFlags<ImageUsage, uint16_t>;


    enum class ImageAspect
    {
        Color = VK_IMAGE_ASPECT_COLOR_BIT,
        Depth = VK_IMAGE_ASPECT_DEPTH_BIT,
        Stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
        Metadata = VK_IMAGE_ASPECT_METADATA_BIT,
        Plane0 = VK_IMAGE_ASPECT_PLANE_0_BIT,
        Plane1 = VK_IMAGE_ASPECT_PLANE_1_BIT,
        Plane2 = VK_IMAGE_ASPECT_PLANE_2_BIT,
        MemoryPlane0 = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
        MemoryPlane1 = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
        MemoryPlane2 = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
        MemoryPlane3 = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
        Plane0KHR = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR,
        Plane1KHR = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR,
        Plane2KHR = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR
    };

    using ImageAspectFlags = util::EnumFlags<ImageAspect, uint16_t>;

    enum class FormatFeature
    {
        Undefined = 0,
        SampledImage = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
        StorageImage = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
        StorageImageAtomic = VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,
        UniformTexelBuffer = VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,
        StorageTexelBuffer = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT,
        StorageTexelBufferAtomic = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
        VertexBuffer = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
        ColorAttachment = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
        ColorAttachmentBlend = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,
        DepthStencilAttachment = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        BlitSrc = VK_FORMAT_FEATURE_BLIT_SRC_BIT,
        BlitDst = VK_FORMAT_FEATURE_BLIT_DST_BIT,
        SampleImageFilterLinear = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
        TransferSrc = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
        TransferDst = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
        MidpointChromSamples = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,
        SampledImageYCBCRConversionLinearFilter = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,
        SampledImageYCBCRConversionSeperateRecounstructionFilter = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
        SampledImageYCBCRConversionChromaReconstructionExplicit = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        SampledImageYCBCRConversionChromaReconstructionExplicitForceable = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,
        Disjoint = VK_FORMAT_FEATURE_DISJOINT_BIT,
        CositedChromaSamples = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,
        SampledImageFilterMinmax = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT,
        SampledImageFilterCubicIMG = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG,
        AccelerationStructureVertexBufferKHR = VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR,
        FragmentDensityMapEXT = VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT,
        TransferSrcKHR = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR,
        TransferDstKHR = VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR,
        SampledImageFilterMinmaxEXT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT,
        MidpointChromSamplesKHR = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR,
        SampledImageYCBCRConversionLinearFilterKHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR,
        SampledImageYCBCRConversionSeperateRecounstructionFilterKHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR,
        SampledImageYCBCRConversionChromaReconstructionExplicitKHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR,
        SampledImageYCBCRConversionChromaReconstructionExplicitForceableKHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR,
        DisjointKHR = VK_FORMAT_FEATURE_DISJOINT_BIT_KHR,
        CositedChromSamplesKHR = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT_KHR,
        SampledImageFilterCubicEXT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT,
        MaxEnum = VK_FORMAT_FEATURE_FLAG_BITS_MAX_ENUM
    };

    using FormatFeatureFlags = util::EnumFlags<FormatFeature, VkFormatFeatureFlags>;

    enum class ImageCreateFlag
    {
        Undefined = 0,
        SparseBinding = VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
        SparseResidency = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
        SparseAliased = VK_IMAGE_CREATE_SPARSE_ALIASED_BIT,
        MutableFormat = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        CubeCompatible = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        Alias = VK_IMAGE_CREATE_ALIAS_BIT,
        SplitInstanceBindRegions = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT,
        Array2DCompatible = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,
        BlockTexelViewCompatible = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
        ExtendedUsage = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT,
        Disjoint = VK_IMAGE_CREATE_DISJOINT_BIT,
        CornerSampledNV = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV,
        SampleLocationsCompatbileDepthEXT = VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT,
        SubsampledEXT = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT,
        SplitInstanceBindReginsKHR = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR,
        Array2DCompatibleKHR = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR,
        BlockTexelViewCompatbileKHR = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR,
        ExtendedUsageKHR = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR,
        DisjointKHR = VK_IMAGE_CREATE_DISJOINT_BIT_KHR,
        AliasKHR = VK_IMAGE_CREATE_ALIAS_BIT_KHR,
        MaxEnum = VK_IMAGE_CREATE_FLAG_BITS_MAX_ENUM
    };

    using ImageCreateFlags = util::EnumFlags<ImageCreateFlag, VkImageCreateFlags>;



    VkImageSubresourceRange NewSubresourceRange(const ImageAspectFlags aspect = ImageAspectFlags(ImageAspect::Color),
                                                   const uint32_t baseMipLevel = 0,
                                                   const uint32_t levelCount = 1,
                                                   const uint32_t baseArrayLayer = 0,
                                                   const uint32_t layerCount = 1) noexcept
    {

        VkImageSubresourceRange range;
        range.aspectMask = aspect.flags;
        range.baseMipLevel = baseMipLevel;
        range.levelCount = levelCount;
        range.baseArrayLayer = baseArrayLayer;
        range.layerCount = layerCount;
    

        return range;
    }

    VkComponentMapping NewComponentMapping(VkComponentSwizzle r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                              VkComponentSwizzle g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                              VkComponentSwizzle b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                              VkComponentSwizzle a = VK_COMPONENT_SWIZZLE_IDENTITY)
    {
        VkComponentMapping mapping{};
        mapping.r = r;
        mapping.g = g;
        mapping.b = b;
        mapping.a = a;
        return mapping;
    }

    struct ImageView
    {
        constexpr ImageView() = default;

    	ImageView(VkImageView image_view) noexcept : vk_view(image_view)
    	{
    	}
		
    	operator VkImageView() const noexcept
    	{
            return vk_view;
    	}
    	
        VkImageView vk_view{ VK_NULL_HANDLE };
    	
    };
	
} // namespace mvk



#endif