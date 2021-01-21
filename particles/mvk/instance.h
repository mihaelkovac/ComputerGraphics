#ifndef MVK_INSTANCE_H
#define MVK_INSTANCE_H

#include <vulkan/vulkan.h>

#include <vector>

#include "utils.h"
#include "gpu.h"
#include "surface.h"

#if MVK_DEBUG
#include "debug.h"
#endif

#define MVK_KHRONOS_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"



namespace mvk
{


    enum class VulkanVersion
    {
        Version_1_0 = VK_API_VERSION_1_0,
        Version_1_1 = VK_API_VERSION_1_1,
        Version_1_2 = VK_API_VERSION_1_2
    };



    

    struct Instance
    {

        Instance(VkInstance instance = VK_NULL_HANDLE,
                 uint32_t vulkanVersion = util::underlying(VulkanVersion::Version_1_0)) noexcept :
            vkInstance(instance),
            version(vulkanVersion)
        {
        }



        #if MVK_DEBUG
        Instance(VkInstance instance,
                 VkDebugUtilsMessengerEXT debugMsnger,
                 uint32_t vulkanVersion) noexcept : vkInstance(instance), version(vulkanVersion), debugMessenger(debugMsnger)
        {
        }
        #endif

        std::vector<GPUInfo> getGPUs() const noexcept
        {
            uint32_t count = 0;
            vkEnumeratePhysicalDevices(vkInstance, &count, nullptr);

            MVK_CHECK_FATAL(count, "No physical devices present");

            std::vector<GPUInfo> gpus(count);
            VkPhysicalDevice* devices = reinterpret_cast<VkPhysicalDevice*>(gpus.data());
            vkEnumeratePhysicalDevices(vkInstance, &count, devices);

            for(uint32_t i = count - 1; i > 0; --i)
            {
                gpus[i].vkGPU = devices[i];
                devices[i] = 0;
            }


            for(uint32_t i = 0; i < count; ++i)
            {
                GPUInfo& gpu = gpus[i];
                vkGetPhysicalDeviceProperties(gpu.vkGPU, &gpu.properties);
                vkGetPhysicalDeviceFeatures(gpu.vkGPU, &gpu.features);
                
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(gpu.vkGPU, &queueFamilyCount, nullptr);

                gpu.supportedQueueFamilies.resize(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(gpu.vkGPU, &queueFamilyCount, gpu.supportedQueueFamilies.data());

                vkGetPhysicalDeviceMemoryProperties(gpu.vkGPU, &gpu.memoryProperties);
            }
            return gpus;

        }

        // template<template<typename> typename GPUPickPolicy,
        //          typename QueueFamilyPolicy>
        // DeviceBuilder<GPUPickPolicy, QueueFamilyPolicy> makeDeviceBuilder() noexcept
        // {
        //     return 
        // }

        uint32_t getVkAPIVersion() const noexcept
        {
            return version;
        }

        operator VkInstance() noexcept
        {
            return vkInstance;
        }

        VkInstance getVkInstance() noexcept
        {
            return vkInstance;
        }

        void release()
        {
#if MVK_DEBUG
            DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
#endif

            vkDestroyInstance(vkInstance, nullptr);
        }

        VkInstance vkInstance;
        uint32_t version;
        #ifdef MVK_DEBUG
        VkDebugUtilsMessengerEXT debugMessenger{};
        #endif
    };

    struct InstanceBuilder
    {
        using Instance = Instance;

        InstanceBuilder()
        {
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.apiVersion = util::underlying<VulkanVersion>(VulkanVersion::Version_1_0);
            appInfo.applicationVersion = util::version(1, 0, 0);
            appInfo.engineVersion = util::version(1, 0, 0);
            appInfo.pEngineName = "NoEngine";
            appInfo.pApplicationName = "Application";


            instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            #ifdef MVK_DEBUG
            debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugInfo.pfnUserCallback = defaulDebugCallback;
            #endif

            
            
        }

        InstanceBuilder& setAppName(const char* appName) noexcept
        {
            appInfo.pApplicationName = appName;
            return *this;
        }

        InstanceBuilder& setEngineName(const char* engineName) noexcept
        {
            appInfo.pEngineName = engineName;
            return *this;
        }

        InstanceBuilder& setEngineVersion(uint32_t major, uint32_t minor, uint32_t patch) noexcept
        {
            appInfo.engineVersion = util::version(major, minor, patch);
            return *this;
        }

        InstanceBuilder& setAppVersion(uint32_t major, uint32_t minor, uint32_t patch) noexcept
        {
            appInfo.applicationVersion = util::version(major, minor, patch);
            return *this;
        }

        InstanceBuilder& setVulkanAPIVersion(VulkanVersion version) noexcept
        {
            appInfo.apiVersion = util::underlying<VulkanVersion>(version);
            return *this;
        }

        InstanceBuilder& addInstanceLayer(const char* layerName) noexcept
        {
            enabledLayers_.push_back(layerName);
            return *this;
        }

        InstanceBuilder& addInstanceExtension(const char* extensionName) noexcept
        {
            enabledExtensions_.push_back(extensionName);
            return *this;
        }

        InstanceBuilder& addInstanceLayers(const char** layerNames, size_t layerNamesSize) noexcept
        {
            if(!layerNames) return *this;

            for(size_t i = 0; i < layerNamesSize; ++i)
            {
                enabledLayers_.push_back(layerNames[i]);
            }

            return *this;
        }

        InstanceBuilder& addInstanceExtensions(const char** extensionNames, size_t extensionNamesSize) noexcept
        {
            if(!extensionNames) return *this;

            for(size_t i = 0; i < extensionNamesSize; ++i)
            {
                enabledExtensions_.push_back(extensionNames[i]);
            }
            
            return *this;
        }

        std::vector<VkLayerProperties> getAvailableInstanceLayers() const noexcept
        {
            uint32_t count = 0;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> layers(count);
            vkEnumerateInstanceLayerProperties(&count, layers.data());

            return layers;
        }

        std::vector<VkExtensionProperties> getAvailableInstanceExtensions(const char* layerName = nullptr) const noexcept
        {
            uint32_t count = 0;
            vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);

            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(layerName, &count, extensions.data());
            return extensions;
        }


        void buildInstance(Instance& instance, VkAllocationCallbacks* allocCBs = nullptr) noexcept
        {
            
            instanceInfo.pApplicationInfo        = &appInfo;

            instanceInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions_.size());
            instanceInfo.ppEnabledExtensionNames = enabledExtensions_.data();

            instanceInfo.enabledLayerCount       = static_cast<uint32_t>(enabledLayers_.size());
            instanceInfo.ppEnabledLayerNames     = enabledLayers_.data();

            
            //#if MVK_DEBUG
            //instanceInfo.pNext = &debugInfo;
            //#endif
            
            instance.version = appInfo.apiVersion;
            MVK_VALIDATE_RESULT(vkCreateInstance(&instanceInfo, allocCBs, &instance.vkInstance), "Failed to initalize Vulkan instance");


            #if MVK_DEBUG
            VkDebugUtilsMessengerEXT debugMessenger;
            MVK_VALIDATE_RESULT(CreateDebugUtilsMessengerEXT(instance.vkInstance, &debugInfo, nullptr, &instance.debugMessenger), "Failed to create debug messenger");
            #endif

        }

        #ifdef MVK_DEBUG
        InstanceBuilder& setDebugCallback(DebugCallback callback)
        {
            debugInfo.pfnUserCallback = callback;
            return *this;
        }
        #endif

    protected:
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo instanceInfo{};


        std::vector<const char*> enabledExtensions_;
        std::vector<const char*> enabledLayers_;
        
        #if MVK_DEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        #endif
    };

    
} // namespace mvk


#endif