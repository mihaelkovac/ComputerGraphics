#ifndef MVK_GPU_H
#define MVK_GPU_H

#include <vulkan/vulkan.h>

#include <cstring>


#include "utils.h"
#include "surface.h"
#include "queue.h"

namespace mvk
{

     enum class DeviceFeature
    {
        RobustBufferAccess = offsetof(VkPhysicalDeviceFeatures, robustBufferAccess),
        FullDrawIndexUint32 = offsetof(VkPhysicalDeviceFeatures, fullDrawIndexUint32),
        ImageCubeArray = offsetof(VkPhysicalDeviceFeatures, imageCubeArray),
        IndependentBlend = offsetof(VkPhysicalDeviceFeatures, independentBlend),
        GeometryShader = offsetof(VkPhysicalDeviceFeatures, geometryShader),
        TessellationShader = offsetof(VkPhysicalDeviceFeatures, tessellationShader),
        SampleRateShading = offsetof(VkPhysicalDeviceFeatures, sampleRateShading),
        DualSrcBlend = offsetof(VkPhysicalDeviceFeatures, dualSrcBlend),
        LogicOp = offsetof(VkPhysicalDeviceFeatures, logicOp),
        MultiDrawIndirect = offsetof(VkPhysicalDeviceFeatures, multiDrawIndirect),
        DrawIndirectFirstInstance = offsetof(VkPhysicalDeviceFeatures, drawIndirectFirstInstance),
        DepthClamp = offsetof(VkPhysicalDeviceFeatures, depthClamp),
        DepthBiasClamp = offsetof(VkPhysicalDeviceFeatures, depthBiasClamp),
        FillModeNonSolid = offsetof(VkPhysicalDeviceFeatures, fillModeNonSolid),
        DepthBounds = offsetof(VkPhysicalDeviceFeatures, depthBounds),
        WideLines = offsetof(VkPhysicalDeviceFeatures, wideLines),
        LargePoints = offsetof(VkPhysicalDeviceFeatures, largePoints),
        AlphaToOne = offsetof(VkPhysicalDeviceFeatures, alphaToOne),
        MultiViewport = offsetof(VkPhysicalDeviceFeatures, multiViewport),
        SamplerAnisotropy = offsetof(VkPhysicalDeviceFeatures, samplerAnisotropy),
        TextureCompressionETC2 = offsetof(VkPhysicalDeviceFeatures, textureCompressionETC2),
        TextureCompressionASTC_LDR = offsetof(VkPhysicalDeviceFeatures, textureCompressionASTC_LDR),
        TextureCompressionBC = offsetof(VkPhysicalDeviceFeatures, textureCompressionBC),
        OcclusionQueryPrecise = offsetof(VkPhysicalDeviceFeatures, occlusionQueryPrecise),
        PipelineStatisticsQuery = offsetof(VkPhysicalDeviceFeatures, pipelineStatisticsQuery),
        VertexPipelineStoresAndAtomics = offsetof(VkPhysicalDeviceFeatures, vertexPipelineStoresAndAtomics),
        FragmentStoresAndAtomics = offsetof(VkPhysicalDeviceFeatures, fragmentStoresAndAtomics),
        ShaderTessellationAndGeometryPointSize = offsetof(VkPhysicalDeviceFeatures, shaderTessellationAndGeometryPointSize),
        ShaderImageGatherExtended = offsetof(VkPhysicalDeviceFeatures, shaderImageGatherExtended),
        ShaderStorageImageExtendedFormats = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageExtendedFormats),
        ShaderStorageImageMultisample = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageMultisample),
        ShaderStorageImageReadWithoutFormat = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageReadWithoutFormat),
        ShaderStorageImageWriteWithoutFormat = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageWriteWithoutFormat),
        ShaderUniformBufferArrayDynamicIndexing = offsetof(VkPhysicalDeviceFeatures, shaderUniformBufferArrayDynamicIndexing),
        ShaderSampledImageArrayDynamicIndexing = offsetof(VkPhysicalDeviceFeatures, shaderSampledImageArrayDynamicIndexing),
        ShaderStorageBufferArrayDynamicIndexing = offsetof(VkPhysicalDeviceFeatures, shaderStorageBufferArrayDynamicIndexing),
        ShaderStorageImageArrayDynamicIndexing = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageArrayDynamicIndexing),
        ShaderClipDistance = offsetof(VkPhysicalDeviceFeatures, shaderClipDistance),
        ShaderCullDistance = offsetof(VkPhysicalDeviceFeatures, shaderCullDistance),
        ShaderFloat64 = offsetof(VkPhysicalDeviceFeatures, shaderFloat64),
        ShaderInt64 = offsetof(VkPhysicalDeviceFeatures, shaderInt64),
        ShaderInt16 = offsetof(VkPhysicalDeviceFeatures, shaderInt16),
        ShaderResourceResidency = offsetof(VkPhysicalDeviceFeatures, shaderResourceResidency),
        ShaderResourceMinLod = offsetof(VkPhysicalDeviceFeatures, shaderResourceMinLod),
        SparseBinding = offsetof(VkPhysicalDeviceFeatures, sparseBinding),
        SparseResidencyBuffer = offsetof(VkPhysicalDeviceFeatures, sparseResidencyBuffer),
        SparseResidencyImage2D = offsetof(VkPhysicalDeviceFeatures, sparseResidencyImage2D),
        SparseResidencyImage3D = offsetof(VkPhysicalDeviceFeatures, sparseResidencyImage3D),
        SparseResidency2Samples = offsetof(VkPhysicalDeviceFeatures, sparseResidency2Samples),
        SparseResidency4Samples = offsetof(VkPhysicalDeviceFeatures, sparseResidency4Samples),
        SparseResidency8Samples = offsetof(VkPhysicalDeviceFeatures, sparseResidency8Samples),
        SparseResidency16Samples = offsetof(VkPhysicalDeviceFeatures, sparseResidency16Samples),
        SparseResidencyAliased = offsetof(VkPhysicalDeviceFeatures, sparseResidencyAliased),
        VariableMultisampleRate = offsetof(VkPhysicalDeviceFeatures, variableMultisampleRate),
        InheritedQueries = offsetof(VkPhysicalDeviceFeatures, inheritedQueries)
    };

    enum class GPUType
    {
        Other = VK_PHYSICAL_DEVICE_TYPE_OTHER,
        Integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
        Discrete = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        Virtual = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
        CPU = VK_PHYSICAL_DEVICE_TYPE_CPU,
        Invalid = VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM
    };

    enum class GPUMemoryType
    {
        Device = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        HostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        HostCoherent = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        HostCached = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        LazyAllocated = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
        Protected = VK_MEMORY_PROPERTY_PROTECTED_BIT,
        DeviceCoherentAMD = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
        DeviceUncachedAMD = VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD,
        Undefined = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM
    };

    enum class GPUHeapType
    {
        DeviceLocal      = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
        MulitInstance    = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT,
        MultiInstanceKHR = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR,
        Undefined        = VK_MEMORY_HEAP_FLAG_BITS_MAX_ENUM
    };
     
    

    

    struct FamilyTypeGroup
    {
        template<FamilyType Family>
        constexpr inline bool Test() const noexcept
        {
            return family_flags_ & (util::Underlying(Family));
        }

        constexpr void Set(FamilyType type) noexcept
        {
            family_flags_ |= util::Underlying(type);
        }


    private:
        uint32_t family_flags_;
    };


    struct GPUInfo
    {
        
        bool IsFeatureAvailable(DeviceFeature feature) const noexcept
        {
            const char* featurePtr = reinterpret_cast<const char*>(&features);
            const VkBool32* result = reinterpret_cast<const VkBool32*>(featurePtr + util::Underlying(feature));
            return static_cast<bool>(*result);
        }

        bool AreFeaturesAvailable(std::initializer_list<DeviceFeature> features) const noexcept
        {
            for(auto feature : features)
            {
                if(!IsFeatureAvailable(feature))
                    return false;
            }

            return true;
        }

        GPUType GetType() const noexcept
        {
            return static_cast<GPUType>(properties.deviceType);
        }

        bool IsType(GPUType type) const noexcept
        {
            return static_cast<GPUType>(properties.deviceType) == type;
        }


        std::vector<VkExtensionProperties> GetSupportedExtensions(const char* layer_name = nullptr) const noexcept
        {
            std::vector<VkExtensionProperties> extensions{};
            uint32_t count = 0;
            vkEnumerateDeviceExtensionProperties(vk_gpu, layer_name, &count, nullptr);

            if(count)
            {   
                extensions.resize(count);
                vkEnumerateDeviceExtensionProperties(vk_gpu, layer_name, &count, extensions.data());
            }

            return extensions;
        }

        std::vector<VkLayerProperties> GetSupportedLayers() const noexcept
        {
            std::vector<VkLayerProperties> layers{};
            uint32_t count = 0;
            vkEnumerateDeviceLayerProperties(vk_gpu, &count, nullptr);

            if(count)
            {
                layers.resize(count);
                vkEnumerateDeviceLayerProperties(vk_gpu, &count, layers.data());
            }

            return layers;
        }
    
        const VkQueueFamilyProperties& GetQueueFamilyProperties(size_t index) const noexcept
        {
            return supported_queue_families[index];
        }

        bool isFamilyOfType(size_t index, FamilyType type) const noexcept
        {
            return supported_queue_families[index].queueFlags & util::Underlying(type);
        }


        mutable VkPhysicalDevice vk_gpu;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memory_properties;
        std::vector<VkQueueFamilyProperties> supported_queue_families;
    };

    struct GPU
    {

        const VkPhysicalDeviceProperties& GetProperties() const noexcept
        {
            return properties;
        }

        const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const noexcept
        {
            return memoryProperties;
        }

        operator VkPhysicalDevice() noexcept
        {
            return gpu;
        }

        VkPhysicalDevice GetVkHandle() noexcept
        {
            return gpu;
        }

    protected:
        VkPhysicalDevice gpu;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;

    };

    template<typename QueueFamilyIndices>
    struct PickedGPUInfo
    {
        const GPUInfo* gpu = nullptr;
        QueueFamilyIndices indices{};
        SurfaceInfo surface{};
    };

    struct GPUPickPolicyBase
    {
        static bool SupportsExtensions(const GPUInfo& gpu, const std::vector<const char*>& extensions) noexcept
        {
            if(extensions.empty())
            {
                return true;
            }

            auto supported_extensions = gpu.GetSupportedExtensions();

            bool extensions_supported = true;
            for(const char* required_extension : extensions)
            {
                bool extension_supported = false;
                for(const auto& supported_extension : supported_extensions)
                {
                    if(strcmp(required_extension, supported_extension.extensionName) == 0)
                    {
                        extension_supported = true;
                    }
                }

            	if(!extension_supported)
            	{
                    extensions_supported = false;
                    break;
            	}
            }

            return extensions_supported;
        }

        static bool SupportsLayers(const GPUInfo& gpu, const std::vector<const char*>& layers) noexcept
        {
            if(layers.empty())
            {
                return true;
            }

            auto supported_layers = gpu.GetSupportedLayers();

            for(const char* required_layer : layers)
            {
                for(const auto& supported_layer : supported_layers)
                {
                    if(strcmp(required_layer, supported_layer.layerName) != 0)
                    {
                        return false;
                    }
                }
            }

            return true;
        }


        static bool FeatureSupported(const GPUInfo& gpu, DeviceFeature feature) noexcept
        {
            const auto& supported_features = gpu.features;

            const char* feature_data = reinterpret_cast<const char*>(&supported_features);
            const VkBool32* feature_ptr = reinterpret_cast<const VkBool32*>(feature_data + util::Underlying(feature));
            return *feature_ptr; 
        }

        static bool AllFeaturesSupported(const GPUInfo& gpu, const std::vector<DeviceFeature>& features) noexcept
        {
            for(DeviceFeature feature : features)
            {
                if(!FeatureSupported(gpu, feature))
                {
                    return false;
                }
            }

            return true;
        }
    };

    template<typename QueueFamilyPolicy = DefaultQueueFamilyPolicy<false>>
    struct DefaultGPUPickPolicy : GPUPickPolicyBase, QueueFamilyPolicy
    {

        using PickedGPUInfo = PickedGPUInfo<typename QueueFamilyPolicy::QueueFamilyIndicesT>;

        PickedGPUInfo PickGPU(const std::vector<GPUInfo>& gpus,
                              GPUType requiredGPUType,
                              const std::vector<const char*>& extensions,
                              const std::vector<const char*>& layers,
                              const std::vector<DeviceFeature>& features,
                              VkSurfaceKHR surface) const noexcept
        {

            for(const auto& gpu : gpus)
            {
                if(!gpu.IsType(requiredGPUType))
                {
                    continue;
                }
                
                SurfaceInfo surface_info{gpu.vk_gpu, surface};
                
                auto queue_indices = QueueFamilyPolicy::FindQueueFamilies(gpu, surface_info);

                if(queue_indices.AllFamiliesFound()             &&
                   surface_info.SupportedByGPU()        &&
                   AllFeaturesSupported(gpu, features) &&
                   SupportsExtensions(gpu, extensions) &&
                   SupportsLayers(gpu, layers))
                {
                    return PickedGPUInfo{&gpu, queue_indices, std::move(surface_info)};
                } 
            }

            return {};

        }

    };

    
} // namespace mvk


#endif