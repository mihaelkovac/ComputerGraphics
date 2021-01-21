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
                 uint32_t vulkanVersion = util::Underlying(VulkanVersion::Version_1_0)) noexcept :
            vk_instance(instance),
            version(vulkanVersion)
        {
        }



        #if MVK_DEBUG
        Instance(VkInstance instance,
                 VkDebugUtilsMessengerEXT debugMsnger,
                 uint32_t vulkanVersion) noexcept : vk_instance(instance), version(vulkanVersion), debug_messenger(debugMsnger)
        {
        }
        #endif

        std::vector<GPUInfo> GetGPUs() const noexcept
        {
            uint32_t count = 0;
            vkEnumeratePhysicalDevices(vk_instance, &count, nullptr);

            MVK_CHECK_FATAL(count, "No physical devices present");

            std::vector<GPUInfo> gpus(count);
            VkPhysicalDevice* devices = reinterpret_cast<VkPhysicalDevice*>(gpus.data());
            vkEnumeratePhysicalDevices(vk_instance, &count, devices);

            for(uint32_t i = count - 1; i > 0; --i)
            {
                gpus[i].vk_gpu = devices[i];
                devices[i] = 0;
            }


            for(uint32_t i = 0; i < count; ++i)
            {
                GPUInfo& gpu = gpus[i];
                vkGetPhysicalDeviceProperties(gpu.vk_gpu, &gpu.properties);
                vkGetPhysicalDeviceFeatures(gpu.vk_gpu, &gpu.features);
                
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(gpu.vk_gpu, &queueFamilyCount, nullptr);

                gpu.supported_queue_families.resize(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(gpu.vk_gpu, &queueFamilyCount, gpu.supported_queue_families.data());

                vkGetPhysicalDeviceMemoryProperties(gpu.vk_gpu, &gpu.memory_properties);
            }
            return gpus;

        }

        // template<template<typename> typename GPUPickPolicy,
        //          typename QueueFamilyPolicy>
        // DeviceBuilder<GPUPickPolicy, QueueFamilyPolicy> makeDeviceBuilder() noexcept
        // {
        //     return 
        // }

        uint32_t GetVkAPIVersion() const noexcept
        {
            return version;
        }

        operator VkInstance() noexcept
        {
            return vk_instance;
        }

        VkInstance GetVkInstance() noexcept
        {
            return vk_instance;
        }

        void Release()
        {
#if MVK_DEBUG
            DestroyDebugUtilsMessengerEXT(vk_instance, debug_messenger, nullptr);
#endif

            vkDestroyInstance(vk_instance, nullptr);
        }

        VkInstance vk_instance;
        uint32_t version;
        #ifdef MVK_DEBUG
        VkDebugUtilsMessengerEXT debug_messenger{};
        #endif
    };

    struct InstanceBuilder
    {
        using Instance = Instance;

        InstanceBuilder()
        {
            app_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info_.apiVersion = util::Underlying<VulkanVersion>(VulkanVersion::Version_1_0);
            app_info_.applicationVersion = util::Version(1, 0, 0);
            app_info_.engineVersion = util::Version(1, 0, 0);
            app_info_.pEngineName = "NoEngine";
            app_info_.pApplicationName = "Application";


            instance_info_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            #if MVK_DEBUG
            debug_info_.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_info_.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug_info_.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debug_info_.pfnUserCallback = defaulDebugCallback;
            #endif

            
            
        }

        InstanceBuilder& SetAppName(const char* appName) noexcept
        {
            app_info_.pApplicationName = appName;
            return *this;
        }

        InstanceBuilder& SetEngineName(const char* engineName) noexcept
        {
            app_info_.pEngineName = engineName;
            return *this;
        }

        InstanceBuilder& SetEngineVersion(uint32_t major, uint32_t minor, uint32_t patch) noexcept
        {
            app_info_.engineVersion = util::Version(major, minor, patch);
            return *this;
        }

        InstanceBuilder& SetAppVersion(uint32_t major, uint32_t minor, uint32_t patch) noexcept
        {
            app_info_.applicationVersion = util::Version(major, minor, patch);
            return *this;
        }

        InstanceBuilder& SetVulkanAPIVersion(VulkanVersion version) noexcept
        {
            app_info_.apiVersion = util::Underlying<VulkanVersion>(version);
            return *this;
        }

        InstanceBuilder& AddInstanceLayer(const char* layer_name) noexcept
        {
            enabled_layers_.push_back(layer_name);
            return *this;
        }

        InstanceBuilder& AddInstanceExtension(const char* extension_name) noexcept
        {
            enabled_extensions_.push_back(extension_name);
            return *this;
        }

        InstanceBuilder& AddInstanceLayers(const char** layer_names, size_t layer_names_size) noexcept
        {
            if(!layer_names) return *this;

            for(size_t i = 0; i < layer_names_size; ++i)
            {
                enabled_layers_.push_back(layer_names[i]);
            }

            return *this;
        }

        InstanceBuilder& AddInstanceExtensions(const char** extension_names, size_t extension_names_size) noexcept
        {
            if(!extension_names) return *this;

            for(size_t i = 0; i < extension_names_size; ++i)
            {
                enabled_extensions_.push_back(extension_names[i]);
            }
            
            return *this;
        }

        std::vector<VkLayerProperties> GetAvailableInstanceLayers() const noexcept
        {
            uint32_t count = 0;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> layers(count);
            vkEnumerateInstanceLayerProperties(&count, layers.data());

            return layers;
        }

        std::vector<VkExtensionProperties> GetAvailableInstanceExtensions(const char* layer_name = nullptr) const noexcept
        {
            uint32_t count = 0;
            vkEnumerateInstanceExtensionProperties(layer_name, &count, nullptr);

            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(layer_name, &count, extensions.data());
            return extensions;
        }


        void BuildInstance(Instance& instance, VkAllocationCallbacks* alloc_cbs = nullptr) noexcept
        {
            
            instance_info_.pApplicationInfo        = &app_info_;

            instance_info_.enabledExtensionCount   = static_cast<uint32_t>(enabled_extensions_.size());
            instance_info_.ppEnabledExtensionNames = enabled_extensions_.data();

            instance_info_.enabledLayerCount       = static_cast<uint32_t>(enabled_layers_.size());
            instance_info_.ppEnabledLayerNames     = enabled_layers_.data();

            
            //#if MVK_DEBUG
            //instance_info_.pNext = &debug_info_;
            //#endif
            
            instance.version = app_info_.apiVersion;
            MVK_VALIDATE_RESULT(vkCreateInstance(&instance_info_, alloc_cbs, &instance.vk_instance), "Failed to initalize Vulkan instance");


            #if MVK_DEBUG
            VkDebugUtilsMessengerEXT debugMessenger;
            MVK_VALIDATE_RESULT(CreateDebugUtilsMessengerEXT(instance.vk_instance, &debug_info_, nullptr, &instance.debug_messenger), "Failed to create debug messenger");
            #endif

        }

        #if MVK_DEBUG
        InstanceBuilder& SetDebugCallback(DebugCallback callback)
        {
            debug_info_.pfnUserCallback = callback;
            return *this;
        }
        #endif

    protected:
        VkApplicationInfo app_info_{};
        VkInstanceCreateInfo instance_info_{};


        std::vector<const char*> enabled_extensions_;
        std::vector<const char*> enabled_layers_;
        
        #if MVK_DEBUG
        VkDebugUtilsMessengerCreateInfoEXT debug_info_{};
        #endif
    };

    
} // namespace mvk


#endif