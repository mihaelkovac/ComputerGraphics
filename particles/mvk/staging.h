#ifndef MVK_STAGING_H
#define MVK_STAGING_H

#include <vulkan/vulkan_core.h>
#include "device_memory.h"
#include "commands.h"

namespace mvk
{

	

	template<size_t FramesInFlight = 2>
	struct StagingManager
	{

	protected:
		struct StagingBuffer
		{
			VkDeviceSize offset{ 0 };
			mvk::CommandBuffer cmdBuffer{ nullptr };
			VkBuffer buffer{ VK_NULL_HANDLE };
			VkFence fence{ VK_NULL_HANDLE };
			byte* data{ nullptr };
		};

	public:
		struct Stager
		{
			void imageCopy()
			{
				VkBufferImageCopy copy{};
				copy.bufferOffset = buffer.offset;

				
			}

			void bufferCopy() noexcept
			{
				
			}

			void transitionLayout() noexcept
			{
				
			}

			void flush() noexcept
			{
				
			}
			
		private:
			int size;
			int allign;
			StagingBuffer& buffer;
			
		};
		
	protected:
		VkCommandPool pool_{ VK_NULL_HANDLE };
		VkDeviceMemory deviceMemory_{};
		VkBuffer stagingBuffer_{ VK_NULL_HANDLE };
		uint8_t* data_{ nullptr };
		std::array<StagingBuffer, FramesInFlight> buffers_;
		
	};
}

#endif


