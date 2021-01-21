#ifndef MVK_COMMANDS_H
#define MVK_COMMANDS_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>


#include "utils.h"
#include "pipelines.h"
#include "device_memory.h"


namespace mvk
{

	enum class CommandPoolFlag
    {
        DontCare = 0,
        Transient = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        ResetCommand = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        Protected = VK_COMMAND_POOL_CREATE_PROTECTED_BIT
        // Undefined = VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM
    };

    using CommandPoolFlags = util::EnumFlags<CommandPoolFlag, VkCommandPoolCreateFlags>;



	enum class CommandBufferUsage
	{
		Undefined = 0,
		OneTime = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		RenderPassContinue = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		Simultaneous = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		MaxEnum = VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM
	};

    using CommandBufferUsageFlags = util::EnumFlags<CommandBufferUsage, VkCommandBufferUsageFlags>;

	enum class DependencyFlag
	{
		NoDependency = 0,
	    ByRegion = VK_DEPENDENCY_BY_REGION_BIT,
	    DeviceGroup = VK_DEPENDENCY_DEVICE_GROUP_BIT,
	    ViewLocal = VK_DEPENDENCY_VIEW_LOCAL_BIT,
	    ViewLocalKHR = VK_DEPENDENCY_VIEW_LOCAL_BIT_KHR,
	    DeviceGroupKHR = VK_DEPENDENCY_DEVICE_GROUP_BIT_KHR,
	    MaxEnum = VK_DEPENDENCY_FLAG_BITS_MAX_ENUM
	};

    using DependencyFlags = util::EnumFlags<DependencyFlag, VkDependencyFlags>;

	
   
	
	struct BarrierRequest
	{

		BarrierRequest(const PipelineStageFlags src_pipe_stage,
					   const PipelineStageFlags dst_pipe_stage,
					   const DependencyFlags dependency_flags = DependencyFlag::NoDependency) noexcept
            : src_stage(src_pipe_stage), dst_stage(dst_pipe_stage), dependencies(dependency_flags)
		{
		}

		
        VkMemoryBarrier* memory_barriers{ nullptr };
        size_t memory_barrier_count{ 0 };
        VkBufferMemoryBarrier* buffer_memory_barriers{ nullptr };
        size_t buffer_memory_barrier_count{ 0 };
        VkImageMemoryBarrier* image_memory_barriers{ nullptr };
        size_t image_memory_barrier_count{ 0 };
        DependencyFlags dependencies{ DependencyFlag::NoDependency };
        PipelineStageFlags src_stage{ PipelineStage::Undefined };
        PipelineStageFlags dst_stage{ PipelineStage::Undefined };	
    };


    struct CommandBuffer
    {
        
    	CommandBuffer(VkCommandBuffer buffer = nullptr) noexcept : vk_cmd_buff(buffer)
    	{
    	}


        struct Recording
    	{

    		void Finish() const
            {
                MVK_VALIDATE_RESULT(vkEndCommandBuffer(cmd_buffer_), "CommandBufferRecordin::Finish - Failed to end command buffer recording");
    		}

            void SubmitDraw(const uint32_t vertex_count,
							const uint32_t first_vert = 0,
							const uint32_t instance_count = 1,
							const uint32_t first_instance = 0) const noexcept
            {
    			
                vkCmdDraw(cmd_buffer_, vertex_count, instance_count, first_vert, first_instance);
            }

            void SubmitDrawIndexed(const uint32_t index_count,
								   const uint32_t first_index = 0,
								   const uint32_t vertex_offset = 0,
								   const uint32_t instance_count = 1,
								   const uint32_t first_instance = 0) const noexcept
            {
                vkCmdDrawIndexed(cmd_buffer_, index_count, instance_count, first_index, vertex_offset, first_instance);
            }

    		void BindPipeline(VkPipeline pipeline, const VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const noexcept
    		{
                
                vkCmdBindPipeline(cmd_buffer_, bind_point, pipeline);
    		}

    		void BindDescriptorSets(VkPipelineBindPoint bind_point,
									VkPipelineLayout layout,
									const VkDescriptorSet* sets,
									const uint32_t set_count = 1,
									const uint32_t first_set = 0,
									const uint32_t dynamic_off_count = 0,
									const uint32_t* dynamic_offs = nullptr) const noexcept
    		{
                vkCmdBindDescriptorSets(cmd_buffer_, bind_point, layout, first_set, set_count, sets, dynamic_off_count, dynamic_offs);
    		}

    		void BindVertexBuffers(Buffer* buffers,
								   const uint32_t buff_count = 1,
								   const uint32_t first_bind = 0,
								   const VkDeviceSize* offsets = nullptr) const noexcept
    		{
                const VkDeviceSize default_offset = 0;
    			if(!offsets && buff_count == 1)
    			{
                    offsets = &default_offset;
    			}

                vkCmdBindVertexBuffers(cmd_buffer_, first_bind, buff_count, reinterpret_cast<VkBuffer*>(buffers), offsets);
    		}

    		void Dispatch(const uint32_t group_count_x,
						  const uint32_t group_count_y = 1,
						  const uint32_t group_count_z = 1) const noexcept
    		{
                vkCmdDispatch(cmd_buffer_, group_count_x, group_count_y, group_count_z);
    		}

    	
    		void PipelineBarrier(const BarrierRequest& request) const noexcept
    		{
    			
                vkCmdPipelineBarrier(cmd_buffer_,
									 request.src_stage,
									 request.dst_stage,
									 request.dependencies,
									 static_cast<uint32_t>(request.memory_barrier_count),
									 request.memory_barriers,
									 static_cast<uint32_t>(request.buffer_memory_barrier_count),
									 request.buffer_memory_barriers,
									 static_cast<uint32_t>(request.image_memory_barrier_count),
									 request.image_memory_barriers);
    		}


            void BeginRenderPass(const VkRenderPassBeginInfo& render_pass_begin, const VkSubpassContents subpass_contents) const noexcept
    		{
                vkCmdBeginRenderPass(cmd_buffer_, &render_pass_begin, subpass_contents);
    		}

            void BindViewport(const VkViewport& viewport) const noexcept
    		{
                vkCmdSetViewport(cmd_buffer_, 0, 1, &viewport);
    		}

            void SetScissor(const VkRect2D& scissor) const noexcept
    		{
                vkCmdSetScissor(cmd_buffer_, 0, 1, &scissor);
    		}

            void EndRenderPass() const noexcept
    		{
                vkCmdEndRenderPass(cmd_buffer_);
    		}

            void CopyBuffer(VkBuffer src, VkBuffer dst, VkBufferCopy* regions, size_t region_count) const noexcept
    		{
                vkCmdCopyBuffer(cmd_buffer_, src, dst, static_cast<uint32_t>(region_count), regions);
    		}

            void BindIndexBuffers(Buffer& buffer,
								  const VkDeviceSize offset = 0,
								  const VkIndexType index_type = VK_INDEX_TYPE_UINT32) const noexcept
    		{
                vkCmdBindIndexBuffer(cmd_buffer_, buffer, offset, index_type);
    		}

        private:
    		Recording(CommandBuffer& cmd_buff) noexcept : cmd_buffer_(cmd_buff)
    		{
    		}


            friend struct CommandBuffer;
    		
            CommandBuffer& cmd_buffer_;
    	};
    	
    	[[nodiscard]] Recording Record(const CommandBufferUsageFlags usage = 0, const VkCommandBufferInheritanceInfo* inheritance_info = nullptr)
    	{
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = usage;
            begin_info.pInheritanceInfo = inheritance_info;
            MVK_VALIDATE_RESULT(vkBeginCommandBuffer(vk_cmd_buff, &begin_info), "CommandBuffer::Record - Failed to begin command buffer");

            return Recording{ *this };
    	}

    	

    	operator VkCommandBuffer() const noexcept
        {
            return vk_cmd_buff;
        }

        VkCommandBuffer vk_cmd_buff;
    };

    static_assert(sizeof(CommandBuffer) == sizeof(VkCommandBuffer), "CommandBuffer must be a thin wrapper");

    struct CommandPool
    {

        VkCommandPool vk_command_pool;

        struct CommandBufferAllocateRequest
        {

            util::ValueWrapper<VkCommandBufferLevel> level{ info.level };
            util::ValueWrapper<uint32_t> command_buffer_count{ info.commandBufferCount };

        private:
            friend struct CommandPool;
            VkCommandBufferAllocateInfo info{};

        };

        operator VkCommandPool() const noexcept
        {
            return vk_command_pool;
        }

        [[nodiscard]] CommandBufferAllocateRequest CreateCommandBufferRequest(const VkCommandBufferLevel level, size_t count)
        {
            CommandBufferAllocateRequest request{};
            VkCommandBufferAllocateInfo& info = request.info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = vk_command_pool;
            info.commandBufferCount = static_cast<uint32_t>(count);
            info.level = level;
            return request;
        }

    };
    
} // namespace mvk



#endif

