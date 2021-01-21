#ifndef MVK_DEBUG_H
#define MVK_DEBUG_H


#include <vulkan/vulkan.h>


#define MVK_DEBUG_CB VKAPI_ATTR VkBool32 VKAPI_CALL

#define MVK_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

namespace mvk
{

    using DebugCallback = PFN_vkDebugUtilsMessengerCallbackEXT;

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    
    MVK_DEBUG_CB defaulDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* cb_data, void* user_data)
    {
        
        fprintf(stderr, "Validation layer: %s\n", cb_data->pMessage);

        return VK_FALSE;
    }


    // using DebugCallback = PFN
    
} // namespace mvk



#endif