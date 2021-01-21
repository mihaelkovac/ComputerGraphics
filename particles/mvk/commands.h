#ifndef MVK_COMMANDS_H
#define MVK_COMMANDS_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "utils.h"
#include "pipelines.h"


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

	
    struct CommandPool
    {

        VkCommandPool commandPool;

    	struct CommandBufferAllocateRequest
    	{
    		
	        util::ValueWrapper<VkCommandBufferLevel> level{info.level};
	        util::ValueWrapper<uint32_t> commandBufferCount{ info.commandBufferCount };

    	private:
            friend struct CommandPool;
            VkCommandBufferAllocateInfo info{};
    		
    	public:
    	};

        CommandBufferAllocateRequest createCommandBufferRequest(const VkCommandBufferLevel level, VkCommandBuffer* buffersData, size_t count)
        {
            CommandBufferAllocateRequest request{};
            VkCommandBufferAllocateInfo& info = request.info;
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = commandPool;
            info.commandBufferCount = static_cast<uint32_t>(count);
            info.level = level;
            return request;
        }
        
    };
	
	struct BarrierRequest
	{

		BarrierRequest(const PipelineStageFlags srcPipeStage, const PipelineStageFlags dstPipeStage, const DependencyFlags = DependencyFlag::NoDependency) noexcept
            : memoryBarriers(nullptr), memoryBarrierCount(0),
			  bufferMemoryBarriers(nullptr), bufferMemoryBarrierCount(0),
			  imageMemoryBarriers(nullptr), imageMemoryBarrierCount(0),
			  srcStage(srcPipeStage), dstStage(dstPipeStage)
		{
		}

		
        VkMemoryBarrier* memoryBarriers{ nullptr };
        size_t memoryBarrierCount{ 0 };
        VkBufferMemoryBarrier* bufferMemoryBarriers{ nullptr };
        size_t bufferMemoryBarrierCount{ 0 };
        VkImageMemoryBarrier* imageMemoryBarriers{ nullptr };
        size_t imageMemoryBarrierCount{ 0 };
        DependencyFlags dependencies{ DependencyFlag::NoDependency };
        PipelineStageFlags srcStage{ PipelineStage::Undefined };
        PipelineStageFlags dstStage{ PipelineStage::Undefined };	
    };


    struct CommandBuffer
    {
        
    	CommandBuffer(VkCommandBuffer buffer = nullptr) noexcept : vkCmdBuff(buffer)
    	{
    	}


        struct Recording
    	{

    		void finish()
    		{
                MVK_VALIDATE_RESULT(vkEndCommandBuffer(cmdBuffer), "CommandBufferRecordin::Finish - Failed to end command buffer recording");
    		}

            void submitDraw(uint32_t vertexCount, uint32_t firstVert = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0) noexcept
            {
    			
                vkCmdDraw(cmdBuffer, vertexCount, instanceCount, firstVert, firstInstance);
            }

            void submitDrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0,
                uint32_t vertexOffset = 0, uint32_t instanceCount = 1,
                uint32_t firstInstance = 0) noexcept
            {
                vkCmdDrawIndexed(cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
            }

    		void bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint) noexcept
    		{
                
                vkCmdBindPipeline(cmdBuffer, bindPoint, pipeline);
    		}

    		void bindDescriptorSets(VkPipelineBindPoint bindPoint,
									VkPipelineLayout layout,
									const VkDescriptorSet* sets,
									const uint32_t setCount = 1,
									const uint32_t firstSet = 0,
									const uint32_t dynamicOffCount = 0,
									const uint32_t* dynamicOffs = nullptr) noexcept
    		{
                vkCmdBindDescriptorSets(cmdBuffer, bindPoint, layout, firstSet, setCount, sets, dynamicOffCount, dynamicOffs);
    		}

    		void bindVertexBuffers(const VkBuffer* buffers,
								  const uint32_t buffCount,
								  const uint32_t firstBind = 0,
								  const VkDeviceSize* offsets = nullptr) noexcept
    		{
    			
                vkCmdBindVertexBuffers(cmdBuffer, firstBind, buffCount, buffers, offsets);
    		}

    		void dispatch(const uint32_t groupCountX,
						  const uint32_t groupCountY = 1,
						  const uint32_t groupCountZ = 1) noexcept
    		{
                vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
    		}

    	
    		void pipelineBarrier(const BarrierRequest& request) noexcept
    		{
    			
                vkCmdPipelineBarrier(cmdBuffer,
									 request.srcStage,
									 request.dstStage,
									 request.dependencies,
									 static_cast<uint32_t>(request.memoryBarrierCount),
									 request.memoryBarriers,
									 static_cast<uint32_t>(request.bufferMemoryBarrierCount),
									 request.bufferMemoryBarriers,
									 static_cast<uint32_t>(request.imageMemoryBarrierCount),
									 request.imageMemoryBarriers);
    		}


            void beginRenderPass(const VkRenderPassBeginInfo& renderPassBegin, const VkSubpassContents subpassContents) noexcept
    		{
                vkCmdBeginRenderPass(cmdBuffer, &renderPassBegin, subpassContents);
    		}

            void bindViewport(const VkViewport& viewport) noexcept
    		{
                vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    		}

            void setScissor(const VkRect2D& scissor) noexcept
    		{
                vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    		}

            void endRenderPass() noexcept
    		{
                vkCmdEndRenderPass(cmdBuffer);
    		}

            void copyBuffer(VkBuffer src, VkBuffer dst, VkBufferCopy* regions, size_t regionCount) noexcept
    		{
                vkCmdCopyBuffer(cmdBuffer, src, dst, static_cast<uint32_t>(regionCount), regions);
    		}

        private:
    		Recording(CommandBuffer& cmdBuff) noexcept : cmdBuffer(cmdBuff)
    		{
    		}


            friend struct CommandBuffer;
    		
            CommandBuffer& cmdBuffer;
    	};
    	
    	Recording record(const CommandBufferUsageFlags usage = CommandBufferUsage::OneTime, const VkCommandBufferInheritanceInfo* inheritanceInfo = nullptr)
    	{
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = usage;
            beginInfo.pInheritanceInfo = inheritanceInfo;
            MVK_VALIDATE_RESULT(vkBeginCommandBuffer(vkCmdBuff, &beginInfo), "CommandBuffer::record - Failed to begin command buffer");

            return Recording{ *this };
    	}

    	

    	operator VkCommandBuffer() const noexcept
        {
            return vkCmdBuff;
        }

        VkCommandBuffer vkCmdBuff;
    };

    static_assert(sizeof(CommandBuffer) == sizeof(VkCommandBuffer), "CommandBuffer must be a thin wrapper");

	
    
} // namespace mvk



#endif

