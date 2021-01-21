#ifndef UNIFORM_ALLOC_H
#define UNIFORM_ALLOC_H

#include "mvk/device_memory.h"

template<typename Device, size_t BlockSize = 64 * 1024 * 1024>
struct UniformBuffPoolAllocationPolicy : public mvk::DefaultAllocPolicy<Device>
{
	template<typename Instance>
	void Init(Instance& instance)
	{
		mvk::DefaultAllocPolicy<Device>::Init(instance);

		VmaPoolCreateInfo pool_info{};
		pool_info.blockSize = BlockSize;
		pool_info.maxBlockCount = 2;

		const VkBufferCreateInfo example_buff = mvk::Buffer::CreateInfo(1024, { mvk::BufferUsage::Uniform });
		const VmaAllocationCreateInfo example_alloc = mvk::Allocation::CreateInfo(VMA_MEMORY_USAGE_CPU_TO_GPU, {}, { mvk::DeviceMemoryProperty::HostVisible, mvk::DeviceMemoryProperty::HostCoherent });

		uint32_t mem_type_index;
		vmaFindMemoryTypeIndexForBufferInfo(this->allocator, &example_buff, &example_alloc, &mem_type_index);

		pool_info.memoryTypeIndex = mem_type_index;

		vmaCreatePool(this->allocator, &pool_info, &uniform_mem_pool_);
	}

	void Release() noexcept
	{
		vmaDestroyPool(this->allocator, uniform_mem_pool_);
		mvk::DefaultAllocPolicy<Device>::Release();
	}

	void CreateUniformBuffer(mvk::Buffer& buffer,
						     mvk::Allocation& allocation,
							 const VkDeviceSize size) noexcept
	{
		
		VmaAllocationCreateInfo alloc_info{};
		alloc_info.pool = uniform_mem_pool_;
		//alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		const VkBufferCreateInfo buffer_info = mvk::Buffer::CreateInfo(size, mvk::BufferUsage::Uniform);

		this->CreateBuffer(buffer, allocation, buffer_info, alloc_info);
	}

	bool FillUniform(const void* data, const size_t size, const mvk::Allocation& alloc) noexcept
	{
		void* mapped_memory = mvk::DefaultAllocPolicy<Device>::MapMemory(alloc);
		if(!mapped_memory)
		{
			return false;
		}
		
		memcpy(mapped_memory, data, size);
		if(!mvk::DefaultAllocPolicy<Device>::FlushMemory(alloc, 0, size))
		{
			return false;
		}
		
		mvk::DefaultAllocPolicy<Device>::UnmapMemory(alloc);
		return true;
	}


	mvk::AllocObj<mvk::Buffer> CreateUniformBuffer(const VkDeviceSize size) noexcept
	{
		mvk::AllocObj<mvk::Buffer> buffer;

		CreateUniformBuffer(buffer.object, buffer.allocation, size);
		
		return buffer;
	}

	
protected:
	VmaPool uniform_mem_pool_{ nullptr };
};

#endif // UNIFORM_ALLOC_H
