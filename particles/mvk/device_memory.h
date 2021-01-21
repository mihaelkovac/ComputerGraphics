#ifndef MVK_DEVICE_MEMORY_H
#define MVK_DEVICE_MEMORY_H


#include <vulkan/vulkan.h>


#include "utils.h"
#include "buffer.h"
#include "image.h"

#ifndef MVK_USE_VMA_ALLOCATOR
    #undef MVK_USE_VMA_ALLOCATOR
    #define MVK_USE_VMA_ALLOCATOR 1
#endif

#if MVK_USE_VMA_ALLOCATOR
    #define VMA_IMPLEMENTATION
    #include "vk_mem_alloc.h"
#endif

namespace mvk
{

    enum class DeviceMemoryProperty
    {
        Undefined = 0,
        DeviceLocal = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        HostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        HostCoherent = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        HostCached = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        LazyAllocated = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
        Protected = VK_MEMORY_PROPERTY_PROTECTED_BIT,
        DeviceCoherentAMD = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
        DeviceUncachedAMD = VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD
    };

    using DeviceMemoryProperties = util::EnumFlags<DeviceMemoryProperty, VkMemoryPropertyFlags>;

    enum class SharingMode
    {
        Concurrent = VK_SHARING_MODE_CONCURRENT,
        Exclusive  = VK_SHARING_MODE_EXCLUSIVE,
        MaxEnum    = VK_SHARING_MODE_MAX_ENUM
    };

	enum class AccessFlag
	{
		Base = 0,
        IndirectCommandRead = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        IndexRead = VK_ACCESS_INDEX_READ_BIT,
        VertexAttributeRead = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        UniformRead = VK_ACCESS_UNIFORM_READ_BIT,
        InputAttachmentRead = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        ShaderRead = VK_ACCESS_SHADER_READ_BIT,
        ShaderWrite = VK_ACCESS_SHADER_WRITE_BIT,
        ColorAttachmentRead = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        ColorAttachmentWrite = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        DepthStencilAttachmentRead = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        DepthStencilAttachmentWrite = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        TransferRead = VK_ACCESS_TRANSFER_READ_BIT,
        TransferWrite = VK_ACCESS_TRANSFER_WRITE_BIT,
        HostRead = VK_ACCESS_HOST_READ_BIT,
        HostWrite = VK_ACCESS_HOST_WRITE_BIT,
        MemoryRead = VK_ACCESS_MEMORY_READ_BIT,
        MemoryWrite = VK_ACCESS_MEMORY_WRITE_BIT,
        TransformFeedbackWrite = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
        TransformFeedbackRead = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
        TransformFeedbackCounterWrite = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
        ConditionalRendering = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
        ColorAttachmentReadNoncoherent = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
        AccelerationStructureRead = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        AccelerationStructureWrite = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        ShadingRateImageRead = VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV,
        FragmentDensityMapRead= VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
        CommandPreprocessRead = VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV,
        CommandPreprocessWrite = VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV
	};

    using AccessFlags = util::EnumFlags<AccessFlag, VkAccessFlags>;
    

#if MVK_USE_VMA_ALLOCATOR



    struct Allocation
    {
        VmaAllocation vmaAlloc{0};
        VmaAllocationInfo vmaAllocInfo{};

        static constexpr VmaAllocationCreateInfo createInfo(const VmaMemoryUsage usage,
															const DeviceMemoryProperties preferedMemoryProperties,
															const DeviceMemoryProperties requiredMemoryProperties = DeviceMemoryProperty::Undefined,
                                                            const VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT) noexcept
        {
            VmaAllocationCreateInfo allocInfo{};
            allocInfo.flags = flags;
            allocInfo.usage = usage;
            allocInfo.preferredFlags = preferedMemoryProperties;
            allocInfo.requiredFlags = requiredMemoryProperties;

            return allocInfo;
        }


    };

    struct Buffer
    {
        VkBuffer vkBuffer{0};

        static constexpr VkBufferCreateInfo createInfo(const VkDeviceSize size,
                                                       const BufferUsageFlags usage,
                                                       const BufferCreateFlags flags = BufferCreateFlag::Undefined) noexcept
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.usage = usage;
            bufferInfo.flags = flags;
            bufferInfo.size = size;

            return bufferInfo;
        }

    	constexpr VkBufferMemoryBarrier createBarrier(const uint32_t srcFamilyIndex,
													  const uint32_t dstFamilyIndex,
													  const AccessFlags srcAccess,
													  const AccessFlags dstAccess) noexcept
        {
            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.buffer = vkBuffer;
            barrier.srcQueueFamilyIndex = srcFamilyIndex;
            barrier.dstQueueFamilyIndex = dstFamilyIndex;
            barrier.srcAccessMask = srcAccess;
            barrier.dstAccessMask = dstAccess;
        	

            return barrier;
        }

    	operator VkBuffer() const noexcept
        {
            return vkBuffer;
        }
    };

    struct Image
    {
        VkImage vkImage{0};

        static constexpr VkImageCreateInfo createInfo(const ImageUsageFlags usage,
                                                      const ImageCreateFlags flags = ImageCreateFlag::Undefined) noexcept
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.flags = flags;
            imageInfo.usage = usage;

            return imageInfo;   
        }

    	constexpr VkImageMemoryBarrier createBarrier(const uint32_t srcFamilyIndex,
										             const uint32_t dstFamilyIndex,
										             const AccessFlags srcAccess,
										             const AccessFlags dstAccess) noexcept
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = vkImage;
            barrier.srcQueueFamilyIndex = srcFamilyIndex;
            barrier.dstQueueFamilyIndex = dstFamilyIndex;
            barrier.srcAccessMask = srcAccess;
            barrier.dstAccessMask = dstAccess;
        	
            return barrier;
        }

    	operator VkImage() const noexcept
        {
            return vkImage;
        }
    };
    static_assert(sizeof(Image) == sizeof(VkImage) && sizeof(Buffer) == sizeof(VkBuffer),
        "mvk::Image and mvk::Buffer must be same memory sized wrappers for VkImage and VkBuffer respectively");

    template<typename Device>
    struct DefaultAllocPolicy
    {
        template<typename Instance>
        void init(Instance& instance) noexcept
        {
	        auto* _this = static_cast<Device*>(this);
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.vulkanApiVersion = instance.getVkAPIVersion();
            allocatorInfo.physicalDevice   = _this->getVkGPU();
            allocatorInfo.device           = _this->getVkDevice();
            allocatorInfo.instance         = instance.getVkInstance();
            
            _this->validateVkResult(vmaCreateAllocator(&allocatorInfo, &allocator),
                "Failed to initialize VMA allocator");
        }

    	

        std::pair<Buffer, Allocation> createBuffer(const VkBufferCreateInfo& bufferInfo,
												   const VmaAllocationCreateInfo& allocInfo) noexcept
        {
            std::pair<Buffer, Allocation> buffer_alloc{};

            auto* _this = static_cast<Device*>(this);
        	
            _this->validateVkResult(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer_alloc.first.vkBuffer, &buffer_alloc.second.vmaAlloc, &buffer_alloc.second.vmaAllocInfo),
                "DefaultAllocPolicy::createBuffer - Failed to create and allocate buffer");

        	
            return buffer_alloc;
        }

        std::pair<Image, Allocation> createImage(const VkImageCreateInfo& imageInfo,
														const VmaAllocationCreateInfo& allocInfo) noexcept
        {
            Image image{};
            Allocation alloc{};
        	
            auto* _this = static_cast<Device*>(this);
        	
            _this->validateVkResult(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image.vkImage, &alloc.vmaAlloc, &alloc.vmaAllocInfo),
                "DefaultAllocPolicy::createImage - Failed to create and allocate image");
        	
            return { image, alloc };
        }

    	Allocation allocateMemory(const VmaAllocationCreateInfo& allocInfo, const VkMemoryRequirements& requirements) noexcept
        {
            Allocation allocation{};
            Device* _this = static_cast<Device*>(this);
        	
            _this->validateVkResult(vmaAllocateMemory(allocator, &requirements, &allocInfo, &allocation.vmaAlloc, &allocation.vmaAllocInfo),
                "Failed to allocate memory");
        	
            return allocation;
        }

    	


    	void* mapMemory(const Allocation& allocation) noexcept
        {
            void* data = nullptr;
            Device* _this = static_cast<Device*>(this);
            _this->validateVkResult(vmaMapMemory(allocator, allocation.vmaAlloc, &data), "DefaultAllocPolicy::mapMemory - Failed to map memory");

            return data;
        }


    	void unmapMemory(const Allocation& allocation) noexcept
    	{
            vmaUnmapMemory(allocator, allocation.vmaAlloc);
    	}

    	

    	void deallocateMemory(const Allocation& alloc) noexcept
        {
            vmaFreeMemory(allocator, alloc.vmaAlloc);
        	
        }

        void destroyBuffer(Buffer& buffer, Allocation& allocation) noexcept
        {
            
            vmaDestroyBuffer(allocator, buffer.vkBuffer, allocation.vmaAlloc);

        }

        void destroyImage(Image& image, Allocation& allocation) noexcept
        {
            
            vmaDestroyImage(allocator, image.vkImage, allocation.vmaAlloc);
        }
        

        void release() noexcept
        {
            vmaDestroyAllocator(allocator);
        }



    private:
        VmaAllocator allocator{};
    	
    };

#else

    uint32_t findMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties,
                            const uint32_t memoryTypeBits,
                            const bool hostVisibleRequired) noexcept
    {

        const DeviceMemoryProperties requiredMemoryProperties = hostVisibleRequired ? 
                                                                DeviceMemoryProperties{DeviceMemoryProperty::HostVisible, DeviceMemoryProperty::HostCoherent} :
                                                                0;
        
        const DeviceMemoryProperties preferedMemoryProperties = DeviceMemoryProperty::DeviceLocal;

        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {

            if(!(memoryTypeBits & (1 << i)))
            {
                continue;
            }

            const DeviceMemoryProperties properties = memoryProperties.memoryTypes[i].propertyFlags;

            if(!properties.contains(requiredMemoryProperties) || !properties.contains(preferedMemoryProperties))
            {
                continue;
            }

            return i;
        }

        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            if(!(memoryTypeBits & (1 << i)))
            {
                continue;
            }

            const DeviceMemoryProperties properties = memoryProperties.memoryTypes[i].propertyFlags;
            if(!properties.contains(requiredMemoryProperties))
            {
                continue;
            }

            return i;
        }

        
        return UINT32_MAX;

    }

    template<typename Device>
    struct DefaultAllocPolicy
    {
    public:
        struct Allocation
        {
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkDeviceSize size = 0;
            VkDeviceSize offset = 0;
            uint8_t* data = nullptr;
        private:
            uint32_t poolId = 0;
            uint32_t blockId = 0;

        };

    private:
        struct DeviceMemPool
        {

            DeviceMemPool(const uint32_t poolId, const VkDeviceSize poolSize, uint32_t memoryTypeIndex, bool hostVisible) :
                            id(poolId),
                            size(poolSize),
                            freeSize(poolSize),
                            memTypeIndex(memoryTypeIndex),
                            currentBlockId(0)
            {
            }

            struct Block
            {
                uint32_t id;
                VkDeviceSize size;
                VkDeviceSize offset;
                Block* prev;
                Block* next;
                bool free;
            };

            void init(VkDevice device, VkAllocationCallbacks* callbacks) noexcept
            {
                if(memTypeIndex == UINT32_MAX)
                {
                    return false;
                }

                VkMemoryAllocateInfo vkAlloc{};
                vkAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                vkAlloc.memoryTypeIndex = memTypeIndex;
                vkAlloc.allocationSize = size;

                VkResult vkAllocResult = vkAllocateMemory(device, &vkAlloc, callbacks, &deviceMemory);
                
                if(VK_SUCCESS != vkAllocResult || deviceMemory == VK_NULL_HANDLE)
                {
                    return false;
                }

                if(hostVisible)
                {
                    vkMapMemory(device, deviceMemory, 0, size, 0, reinterpret_cast<void**>(&data));
                }

                head = new Block();
                head->id = currentBlockId++;
                head->size = size;
                head->offset = 0;
                head->prev = nullptr;
                head->next = nullptr;
                head->free = true;

                return true;
            }

            bool allocate(const uint32_t allocSize,
                          const uint32_t align,
                          Allocation& allocation) noexcept
            {
                const uint32_t totalSize = allocSize + align;
                if(totalSize > freeSize)
                {
                    return false;
                }

                Block* last = nullptr;
                Block* curr = head;
                while(curr)
                {
                    if(!curr->free)
                    {
                        continue;
                    }

                    if(curr->size >= totalSize)
                    {
                        curr->free = false;
                        allocation.poolId = this->id;
                        allocation.blockId = curr->id;
                        allocation.size = totaLSize;
                        allocation.offset = curr->offset;
                        return true;
                    }

                    last = curr;
                    curr = curr->next;
                }

                
            }

            void release(VkDevice device, VkAllocationCallbacks* cbs) noexcept
            {
                if(hostVisible && data != nullptr)
                {
                    vkUnmapMemory(device, deviceMemory);
                }

                vkFreeMemory(device, deviceMemory, cbs);

                Block* prev = nullptr;
                Block* curr = head;

                while(true)
                {
                    if(curr->next == nullptr)
                    {
                        delete curr;
                        break;
                    }
                    else
                    {
                        prev = curr;
                        curr = curr->next;
                        delete prev;
                    }
                }

                head = nullptr;
            }
            
            uint32_t id;
            uint32_t currentBlockId;
            Block* head;
            VkDeviceSize size;
            VkDeviceSize freeSize;
            VkDeviceMemory deviceMemory;
            uint32_t memTypeIndex;
            uint8_t* data;
            bool hostVisible;

        };
        
        bool tryAllocFromPools(const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties,
                               const uint32_t size,
                               const uint32_t align,
                               const uint32_t memoryTypeBits,
                               const bool requireHostVisible,
                               Allocation& allocation) noexcept
        {
            
            const size_t poolCount = pools.size();
            
            const DeviceMemoryProperties requried = requireHostVisible ?
                                                    DeviceMemoryProperties{DeviceMemoryProperty::HostVisible, DeviceMemoryProperty::HostCoherent} :
                                                    0;
            const DeviceMemoryProperties prefered = DeviceMemoryProperty::DeviceLocal;

            for(size_t i = 0; i < poolCount; ++i)
            {
                DeviceMemPool* pool = pools[i];
                const uint32_t poolMemoryType = pool->memTypeIndex;

                if(requireHostVisible && !pool->hostVisilbe)
                {
                    continue;
                }

                const DeviceMemoryProperties properties = gpuMemoryProperties.memoryTypes[i].propertyFlags;
                if(!properties.contains(required) || !properties.contains(prefered))
                {
                    continue;
                }

                if(pool->allocate(size, align, allocation))
                {
                    return true;
                }
            }

            for(size_t i = 0; i < poolCount; ++i)
            {

                DeviceMemPool* pool = pools[i];
                const uint32_t pool = pool->memTypeIndex;

                if(requireHostVisible && !pool->hostVisible)
                {
                    continue;
                }

                const DeviceMemoryProperties properties = gpuMemoryProperties.memoryTypes[i].propertyFlags;
                if(!properties.contains(required))
                {
                    continue;
                }

                if(pool->allocate(size, align, allocation))
                {
                    return true;
                }
            }

            return false;
        }

        uint32_t currenPoolId = 0;

        uint32_t deviceLocalPoolSizeMB;
        uint32_t hostVisiblePoolSizeMB;

        std::vector<DeviceMemPool*> pools;
        std::vector<Allocation> garbage;

    public:
        struct Allocation
        {
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkDeviceSize size = 0;
            VkDeviceSize offset = 0;
            uint8_t* data 0 = nullptr;
        private:
            uint32_t poolId = 0;
            uint32_t blockId = 0;

        };

        Allocation allocate(const uint32_t size,
                            const uint32_t align,
                            const uint32_t memoryTypeBits,
                            const bool requireHostVisible) noexcept
        {
            Allocation allocation{};

            if(tryAllocFromPool(size, align, memoryTypeBits, requireHostVisible, allocation))
            {
                return allocation;
            }

            const VkPhysicalDeviceMemoryProperties& gpuMemoryProperties = static_cast<Device*>(this)->getGPUMemoryProperties();

            VkDeviceSize poolSize = requireHostVisible ? hostVisiblePoolSizeMB : deviceLocalPoolSizeMB;
            DeviceMemPool* pool = new DeviceMemPool(currenPoolId++, poolSize, findMemoryType(gpuMemoryProperties, memoryTypeBits, requireHostVisible), requireHostVisible);
            if(!pool->init())
            {
                MVK_CHECK_FATAL(false, "DefaultAllocPolicy::allocate failed to allocate new memory pool");
            }
            else
            {
                pools.emplace_back(pool);
            }

            pool->allocate(size, align, allocation);

            return allocation;
    
        }

        void free(Allocation& alloc) noexcept
        {

        }

        void emptyGarbage() noexcept
        {

        }

    
    };
#endif    
} // namespace mvk


#endif