#ifndef MVK_CONTEXT_H
#define MVK_CONTEXT_H

#include "instance.h"
#include "device.h"


namespace mvk
{
    


    template<bool ComputeFamilyRequired>
    using DefaultDeviceCompute = Device<DefaultAllocPolicy, DefaultDeviceQueuePolicy<ComputeFamilyRequired>>;

    template<
        typename Device = DefaultDevice,
        typename Instance = Instance
            >
    struct Context
    {
        
        void release()
        {
            vkDestroySurfaceKHR(instance.getVkInstance(), vkSurface, nullptr);
            device.release();
            instance.release();
        }

        Instance instance{};
        VkSurfaceKHR vkSurface{};
        // VkSurfaceCapabilitiesKHR capabilities;
        Device device{};
    };

    
} // namespace mvk


#endif