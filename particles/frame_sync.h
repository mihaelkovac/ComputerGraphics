#ifndef FRAME_SYNCHRONIZATION_H
#define FRAME_SYNCHRONIZATION_H


#include <array>
#include <vulkan/vulkan.h>


template<size_t MaxFramesInFlight>
struct FrameSync
{

    template<typename Device>
    void init(size_t swapchainImageCount, Device& device) noexcept
    {
        imagesInFlight.resize(swapchainImageCount);
        
        for(size_t i = 0; i < MaxFramesInFlight; ++i)
        {
            inFlightFences[i] = device.createFence(true);
            imgAvailableSemaphores[i] = device.createSemaphore();
            renderFinishedSempahores[i] = device.createSemaphore();
        }
    }

    void release(VkDevice device, const VkAllocationCallbacks* cbs) noexcept
    {
        for(size_t i = 0; i < MaxFramesInFlight; ++i)
        {
            vkDestroySemaphore(device, renderFinishedSempahores[i], cbs);
            vkDestroySemaphore(device, imgAvailableSemaphores[i], cbs);
            vkDestroyFence(device, inFlightFences[i], cbs);
        }
    }

    std::array<VkFence, MaxFramesInFlight> inFlightFences;
    std::array<VkSemaphore, MaxFramesInFlight> imgAvailableSemaphores;
    std::array<VkSemaphore, MaxFramesInFlight> renderFinishedSempahores;
    std::vector<VkFence> imagesInFlight;

    size_t current = 0;
};


#endif