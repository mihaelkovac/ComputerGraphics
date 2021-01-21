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
        VmaAllocation alloc_handle{0};
        VmaAllocationInfo alloc_info{};

        static constexpr VmaAllocationCreateInfo CreateInfo(const VmaMemoryUsage usage,
															const DeviceMemoryProperties prefered_memory_properties,
															const DeviceMemoryProperties required_memory_properties = DeviceMemoryProperty::Undefined,
                                                            const VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT) noexcept
        {
            VmaAllocationCreateInfo alloc_info{};
            alloc_info.flags = flags;
            alloc_info.usage = usage;
            alloc_info.preferredFlags = prefered_memory_properties;
            alloc_info.requiredFlags = required_memory_properties;

            return alloc_info;
        }


    };

    struct Buffer
    {
        VkBuffer vk_buffer{0};

        static constexpr VkBufferCreateInfo CreateInfo(const VkDeviceSize size,
                                                       const BufferUsageFlags usage,
                                                       const BufferCreateFlags flags = BufferCreateFlag::Undefined) noexcept
        {
            VkBufferCreateInfo buffer_info{};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.usage = usage;
            buffer_info.flags = flags;
            buffer_info.size = size;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            return buffer_info;
        }

    	constexpr VkBufferMemoryBarrier CreateBarrier(const uint32_t src_family_index,
													  const uint32_t dst_family_index,
													  const AccessFlags src_access,
													  const AccessFlags dst_access) noexcept
        {
            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.buffer = vk_buffer;
            barrier.srcQueueFamilyIndex = src_family_index;
            barrier.dstQueueFamilyIndex = dst_family_index;
            barrier.srcAccessMask = src_access;
            barrier.dstAccessMask = dst_access;
        	

            return barrier;
        }

    	operator VkBuffer() const noexcept
        {
            return vk_buffer;
        }
    };

    struct Image
    {
        VkImage vk_image{0};

        static constexpr VkImageCreateInfo CreateInfo(const ImageUsageFlags usage,
                                                      const ImageCreateFlags flags = ImageCreateFlag::Undefined) noexcept
        {
            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.flags = flags;
            image_info.usage = usage;
            
            return image_info;   
        }

    	constexpr VkImageMemoryBarrier CreateBarrier(const uint32_t src_family_index,
										             const uint32_t dst_family_index,
										             const AccessFlags src_access,
										             const AccessFlags dst_access) noexcept
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = vk_image;
            barrier.srcQueueFamilyIndex = src_family_index;
            barrier.dstQueueFamilyIndex = dst_family_index;
            barrier.srcAccessMask = src_access;
            barrier.dstAccessMask = dst_access;
        	
            return barrier;
        }

    	operator VkImage() const noexcept
        {
            return vk_image;
        }
    };
    static_assert(sizeof(Image) == sizeof(VkImage) && sizeof(Buffer) == sizeof(VkBuffer),
        "mvk::Image and mvk::Buffer must be same memory sized wrappers for VkImage and VkBuffer respectively");


    template<typename Obj>
    struct AllocObj
    {
        Obj object;
        Allocation allocation;

        bool Fill(const void* data, const size_t data_size, const size_t offset = 0) noexcept
        {
            const VkDeviceSize mem_location = static_cast<VkDeviceSize>(data_size + offset);
	        if(allocation.alloc_info.pMappedData != nullptr && mem_location < allocation.alloc_info.size)
	        {
                memcpy(static_cast<char*>(allocation.alloc_info.pMappedData) + offset, data, data_size);
                return true;
	        }
            return false;
        }
    };
	
    template<typename Device>
    struct DefaultAllocPolicy
    {
        template<typename Instance>
        void Init(Instance& instance) noexcept
        {
	        auto* _this = static_cast<Device*>(this);
            VmaAllocatorCreateInfo allocator_info{};
            allocator_info.vulkanApiVersion = instance.GetVkAPIVersion();
            allocator_info.physicalDevice   = _this->GetVkGPU();
            allocator_info.device           = _this->GetVkDevice();
            allocator_info.instance         = instance.GetVkInstance();
            
            _this->ValidateVkResult(vmaCreateAllocator(&allocator_info, &allocator),
                "Failed to initialize VMA allocator");
        }

    	
        void CreateBuffer(Buffer& buffer,
						  Allocation& allocation,
						  const VkBufferCreateInfo& buffer_info,
						  const VmaAllocationCreateInfo& alloc_info) noexcept
        {


            auto* _this = static_cast<Device*>(this);


            _this->ValidateVkResult(vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer.vk_buffer, &allocation.alloc_handle, &allocation.alloc_info),
                "DefaultAllocPolicy::CreateBuffer - Failed to create and allocate buffer");

        }

        AllocObj<Buffer> CreateBuffer(const VkBufferCreateInfo& buffer_info,
									  const VmaAllocationCreateInfo& alloc_info) noexcept
        {
            AllocObj<Buffer> buffer_alloc{};

            CreateBuffer(buffer_alloc.object, buffer_alloc.allocation, buffer_info, alloc_info);

        	
            return buffer_alloc;
        }

        void CreateImage(Image& image,
			             Allocation& alloc,
			             const VkImageCreateInfo& image_info,
			             const VmaAllocationCreateInfo& alloc_info) noexcept
        {

            auto* _this = static_cast<Device*>(this);

            _this->ValidateVkResult(vmaCreateImage(allocator, &image_info, &alloc_info, &image.vk_image, &alloc.alloc_handle, &alloc.alloc_info),
                "DefaultAllocPolicy::CreateImage - Failed to create and allocate image");

        }

        AllocObj<Image> CreateImage(const VkImageCreateInfo& image_info,
									const VmaAllocationCreateInfo& alloc_info) noexcept
        {
            Image image{};
            Allocation alloc{};
        	
            CreateImage(image, alloc, image_info, alloc_info);
            return { image, alloc };
        }


    	Allocation AllocateMemory(const VmaAllocationCreateInfo& allocInfo, const VkMemoryRequirements& requirements) noexcept
        {
            Allocation allocation{};
            Device* _this = static_cast<Device*>(this);
        	
            _this->ValidateVkResult(vmaAllocateMemory(allocator, &requirements, &allocInfo, &allocation.alloc_handle, &allocation.alloc_info),
                "Failed to allocate memory");
        	
            return allocation;
        }

    	
        void BufferFromPool(Buffer& buffer,
							Allocation& alloc,
							VmaPool pool,
							const VkDeviceSize buffer_size,
							const BufferUsageFlags usage) noexcept
        {
            Device* _this = static_cast<Device*>(this);

            VmaAllocationCreateInfo allocation{};
            allocation.pool = pool;

            VkBufferCreateInfo buffer_info{};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = buffer_size;
            buffer_info.usage = usage;
        	
            _this->ValidateVkResult(vmaCreateBuffer(allocator, &buffer_info, &allocation, &buffer.vk_buffer, &alloc.alloc_handle, &alloc.alloc_info),
                "DefaultAllocationPolicy::BufferFromPool - Failed to create buffer from given pool");
        	
        }

    	void* MapMemory(const Allocation& allocation) noexcept
        {
            void* data = nullptr;
            Device* _this = static_cast<Device*>(this);
            _this->ValidateVkResult(vkMapMemory(_this->GetVkDevice(), allocation.alloc_info.deviceMemory, allocation.alloc_info.offset, allocation.alloc_info.size, 0, &data), "DefaultAllocPolicy::MapMemory - Failed to map memory");
            return data;
        }

    	bool FlushMemory(const Allocation& alloc, const size_t offset, const size_t size) noexcept
        {
            const size_t actual_size = size == 0 ? alloc.alloc_info.size : (size - offset);
            Device* _this = static_cast<Device*>(this);
            _this->ValidateVkResult(vmaFlushAllocation(allocator, alloc.alloc_handle, alloc.alloc_info.offset + offset, actual_size),
                "DefaultAllocPolicy::FlushMemory - Failed to flush mapped memory");
            return true;
        }

    	void UnmapMemory(const Allocation& allocation) noexcept
    	{
            vkUnmapMemory(static_cast<Device*>(this)->GetVkDevice(), allocation.alloc_info.deviceMemory);
    	}

    	

    	void DeallocateMemory(const Allocation& alloc) noexcept
        {
            vmaFreeMemory(allocator, alloc.alloc_handle);
        	
        }

        void DestroyBuffer(Buffer& buffer, Allocation& allocation) noexcept
        {
            
            vmaDestroyBuffer(allocator, buffer.vk_buffer, allocation.alloc_handle);

        }

        void DestroyImage(Image& image, Allocation& allocation) noexcept
        {
            vmaDestroyImage(allocator, image.vk_image, allocation.alloc_handle);
        }
        

        void Release() noexcept
        {
            vmaDestroyAllocator(allocator);
        }



    protected:
        VmaAllocator allocator{};
    	
    };

#else

    uint32_t findMemoryType(const VkPhysicalDeviceMemoryProperties& memory_properties,
                            const uint32_t memoryTypeBits,
                            const bool hostVisibleRequired) noexcept
    {

        const DeviceMemoryProperties requiredMemoryProperties = hostVisibleRequired ? 
                                                                DeviceMemoryProperties{DeviceMemoryProperty::HostVisible, DeviceMemoryProperty::HostCoherent} :
                                                                0;
        
        const DeviceMemoryProperties preferedMemoryProperties = DeviceMemoryProperty::DeviceLocal;

        for(uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
        {

            if(!(memoryTypeBits & (1 << i)))
            {
                continue;
            }

            const DeviceMemoryProperties properties = memory_properties.memoryTypes[i].propertyFlags;

            if(!properties.Contains(requiredMemoryProperties) || !properties.Contains(preferedMemoryProperties))
            {
                continue;
            }

            return i;
        }

        for(uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
        {
            if(!(memoryTypeBits & (1 << i)))
            {
                continue;
            }

            const DeviceMemoryProperties properties = memory_properties.memoryTypes[i].propertyFlags;
            if(!properties.Contains(requiredMemoryProperties))
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

            void Init(VkDevice device, VkAllocationCallbacks* callbacks) noexcept
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

            void Release(VkDevice device, VkAllocationCallbacks* cbs) noexcept
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
                if(!properties.Contains(required) || !properties.Contains(prefered))
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
                if(!properties.Contains(required))
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
            if(!pool->Init())
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

namespace std
{
	template<typename AllocObj>
	struct tuple_size<::mvk::AllocObj<AllocObj>> : integral_constant<size_t, 2>
	{};

    template<typename AllocObj>
    struct tuple_element<0, ::mvk::AllocObj<AllocObj>>
    {
        using type = AllocObj;
    };

    template<typename AllocObj>
    struct tuple_element<1, ::mvk::AllocObj<AllocObj>>
    {
        using type = ::mvk::Allocation;
    };

}

template<typename AllocObj, size_t Index>
std::tuple_element_t<Index, mvk::AllocObj<AllocObj>> get(const mvk::AllocObj<AllocObj>& pair) noexcept
{
    static_assert(Index < 2, "Only two elements in mvk::AllocObj tuple");
	
    if constexpr (Index == 0) return pair.object;
    if constexpr (Index == 1) return pair.allocation;

}

	
	
#endif