#include "queue_families.h"

#include <vector>


bool QueueFamilyIndices::isComplete() const noexcept 
{
    return graphicsFamily_.has_value() && presentFamily_.has_value() && transferFamily_.has_value();
}

#define _CHECK_FAMILY_VAL(_family, _value)   \
        (!_family.has_value() ||             \
        _family.value() != _value)           \

#define _FAMILY_SETTER(_check_family_1, _check_family_2, _set_family, _val)  \
    do {                                                                     \
    if(_CHECK_FAMILY_VAL(_check_family_1, _val) &&                           \
       _CHECK_FAMILY_VAL(_check_family_2, _val))                             \
    {                                                                        \
        ++uniqueFamiliesCount_;                                              \
    }                                                                        \
    _set_family = _val;                                                      \
    } while(0)                                                               \

void QueueFamilyIndices::setGraphicsFamily(uint32_t val) noexcept
{
    _FAMILY_SETTER(presentFamily_, transferFamily_, graphicsFamily_, val);

}   

void QueueFamilyIndices::setPresentFamily(uint32_t val) noexcept
{
    _FAMILY_SETTER(transferFamily_, graphicsFamily_, presentFamily_, val);
}

void QueueFamilyIndices::setTransferFamily(uint32_t val) noexcept
{
    _FAMILY_SETTER(graphicsFamily_, presentFamily_, transferFamily_, val);
}

const std::optional<uint32_t>& QueueFamilyIndices::graphicsFamily() const noexcept
{
    return graphicsFamily_;
}

const std::optional<uint32_t>& QueueFamilyIndices::presentFamily() const noexcept
{
    return presentFamily_;
}

const std::optional<uint32_t>& QueueFamilyIndices::transferFamily() const noexcept
{
    return transferFamily_;
}

uint32_t QueueFamilyIndices::uniqueFamiliesCount() const noexcept
{
    return uniqueFamiliesCount_;
}

void QueueFamilyIndices::setUniqueFamilies(uint32_t* data) const
{
    size_t i = 0;
    if(graphicsFamily_.has_value())
    {
        data[i++] = graphicsFamily_.value();
    }
    
    if(presentFamily_.has_value() && presentFamily_ != graphicsFamily_)
    {
        data[i++] = presentFamily_.value();
    }

    if(transferFamily_.has_value() && transferFamily_ != presentFamily_ && transferFamily_ != graphicsFamily_)
    {
        data[i++] = transferFamily_.value();
    }
}


QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            indices.setGraphicsFamily(i);
        } else if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            indices.setTransferFamily(i);
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) 
        {
            indices.setPresentFamily(i);
        }


        if (indices.isComplete()) 
        {
            break;
        }

        i++;
    }

    if(!indices.transferFamily().has_value() && indices.graphicsFamily().has_value())
    {
        indices.setTransferFamily(indices.graphicsFamily().value());
    }

    return indices;
}