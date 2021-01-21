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
        
        void Release()
        {
            device.Release();
            instance.Release();
        }

        Instance instance{};
        Device device{};
    };

    
} // namespace mvk


#endif