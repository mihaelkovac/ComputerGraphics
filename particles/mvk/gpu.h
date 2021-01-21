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

    struct DeviceFeatureFlags
    {
        void addFeature(DeviceFeature feature) noexcept
        {
            auto underlying_value = util::underlying(feature);
            // TODO: Popravi
            flags_ |= underlying_value / sizeof(VkBool32);
        }

    private:
        uint64_t flags_;
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
        constexpr inline bool test() const noexcept
        {
            return familyFlags & (util::underlying(Family));
        }

        constexpr void set(FamilyType type) noexcept
        {
            familyFlags |= util::underlying(type);
        }


    private:
        uint32_t familyFlags;
    };


    struct GPUInfo
    {
        
        bool isFeatureAvailable(DeviceFeature feature) const noexcept
        {
            const char* featurePtr = reinterpret_cast<const char*>(&features);
            const VkBool32* result = reinterpret_cast<const VkBool32*>(featurePtr + util::underlying(feature));
            return static_cast<bool>(*result);
        }

        bool areFeaturesAvailable(std::initializer_list<DeviceFeature> features) const noexcept
        {
            for(auto feature : features)
            {
                if(!isFeatureAvailable(feature))
                    return false;
            }

            return true;
        }

        GPUType getType() const noexcept
        {
            return static_cast<GPUType>(properties.deviceType);
        }

        bool isType(GPUType type) const noexcept
        {
            return static_cast<GPUType>(properties.deviceType) == type;
        }


        std::vector<VkExtensionProperties> getSupportedExtensions(const char* layerName = nullptr) const noexcept
        {
            std::vector<VkExtensionProperties> extensions{};
            uint32_t count = 0;
            vkEnumerateDeviceExtensionProperties(vkGPU, layerName, &count, nullptr);

            if(count)
            {   
                extensions.resize(count);
                vkEnumerateDeviceExtensionProperties(vkGPU, layerName, &count, extensions.data());
            }

            return extensions;
        }

        std::vector<VkLayerProperties> getSupportedLayers() const noexcept
        {
            std::vector<VkLayerProperties> layers{};
            uint32_t count = 0;
            vkEnumerateDeviceLayerProperties(vkGPU, &count, nullptr);

            if(count)
            {
                layers.resize(count);
                vkEnumerateDeviceLayerProperties(vkGPU, &count, layers.data());
            }

            return layers;
        }
    
        const VkQueueFamilyProperties& getQueueFamilyProperties(size_t index) const noexcept
        {
            return supportedQueueFamilies[index];
        }

        bool isFamilyOfType(size_t index, FamilyType type) const noexcept
        {
            return supportedQueueFamilies[index].queueFlags & util::underlying(type);
        }


        mutable VkPhysicalDevice vkGPU;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        std::vector<VkQueueFamilyProperties> supportedQueueFamilies;
    };

    struct GPU
    {

        const VkPhysicalDeviceProperties& getProperties() const noexcept
        {
            return properties;
        }

        const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const noexcept
        {
            return memoryProperties;
        }

        operator VkPhysicalDevice() noexcept
        {
            return gpu;
        }

        VkPhysicalDevice getVkHandle() noexcept
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
        static bool supportsExtensions(const GPUInfo& gpu, const std::vector<const char*>& extensions) noexcept
        {
            if(extensions.empty())
            {
                return true;
            }

            auto supportedExtensions = gpu.getSupportedExtensions();

            bool extensionsSupported = true;
            for(const char* requiredExtension : extensions)
            {
                bool extensionSupported = false;
                for(const auto& supportedExtension : supportedExtensions)
                {
                    if(strcmp(requiredExtension, supportedExtension.extensionName) == 0)
                    {
                        extensionSupported = true;
                    }
                }

            	if(!extensionSupported)
            	{
                    extensionsSupported = false;
                    break;
            	}
            }

            return extensionsSupported;
        }

        static bool supportsLayers(const GPUInfo& gpu, const std::vector<const char*>& layers) noexcept
        {
            if(layers.empty())
            {
                return true;
            }

            auto supportedLayers = gpu.getSupportedLayers();

            for(const char* requiredLayer : layers)
            {
                for(const auto& supportedLayer : supportedLayers)
                {
                    if(strcmp(requiredLayer, supportedLayer.layerName) != 0)
                    {
                        return false;
                    }
                }
            }

            return true;
        }


        static bool featureSupported(const GPUInfo& gpu, DeviceFeature feature) noexcept
        {
            const auto& supportedFeatures = gpu.features;

            const char* featureData = reinterpret_cast<const char*>(&supportedFeatures);
            const VkBool32* featurePtr = reinterpret_cast<const VkBool32*>(featureData + util::underlying(feature));
            return *featurePtr; 
        }

        static bool allFeaturesSupported(const GPUInfo& gpu, const std::vector<DeviceFeature>& features) noexcept
        {
            for(DeviceFeature feature : features)
            {
                if(!featureSupported(gpu, feature))
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

        PickedGPUInfo pickGPU(const std::vector<GPUInfo>& gpus,
                              GPUType requiredGPUType,
                              const std::vector<const char*>& extensions,
                              const std::vector<const char*>& layers,
                              const std::vector<DeviceFeature>& features,
                              VkSurfaceKHR surface) const noexcept
        {

            for(const auto& gpu : gpus)
            {
                if(!gpu.isType(requiredGPUType))
                {
                    continue;
                }
                
                SurfaceInfo surfaceInfo{gpu.vkGPU, surface};
                
                auto queueIndices = QueueFamilyPolicy::findQueueFamilies(gpu, surfaceInfo);

                if(queueIndices.allFamiliesFound()             &&
                   surfaceInfo.supportedByGPU()        &&
                   allFeaturesSupported(gpu, features) &&
                   supportsExtensions(gpu, extensions) &&
                   supportsLayers(gpu, layers))
                {
                    return PickedGPUInfo{&gpu, queueIndices, std::move(surfaceInfo)};
                } 
            }

            return {};

        }

    };

    
} // namespace mvk


#endif