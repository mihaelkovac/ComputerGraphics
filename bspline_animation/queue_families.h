#ifndef QUEUE_FAMILIES_H
#define QUEUE_FAMILIES_H

#include <optional>

#include <vulkan/vulkan.h>

struct QueueFamilyIndices 
{

    void setGraphicsFamily(uint32_t val) noexcept;

    void setPresentFamily(uint32_t val) noexcept;

    void setTransferFamily(uint32_t val) noexcept;

    const std::optional<uint32_t>& graphicsFamily() const noexcept;
    
    const std::optional<uint32_t>& presentFamily() const noexcept;
    
    const std::optional<uint32_t>& transferFamily() const noexcept;

    bool isComplete() const noexcept;

    uint32_t uniqueFamiliesCount() const noexcept;

    void setUniqueFamilies(uint32_t* data) const;


private:
    uint32_t uniqueFamiliesCount_ = 0;
    std::optional<uint32_t> graphicsFamily_;
    std::optional<uint32_t> presentFamily_;
    std::optional<uint32_t> transferFamily_;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept;



#endif