#ifndef MVK_SYNC_H
#define MVK_SYNC_H


#include <array>
#include <vector>
#include <vulkan/vulkan.h>

#include "mvk/commands.h"


template<size_t MaxFramesInFlight>
struct FrameSync
{
	
	template<typename Device>
	void Init(const size_t swapchain_image_count, Device& device) noexcept
	{
		fences_in_use.resize(swapchain_image_count);

		for(size_t i = 0; i < MaxFramesInFlight; ++i)
		{
			render_finished_semaphroes[i] = device.CreateSemaphore();
			image_ready_semaphore[i]	  = device.CreateSemaphore();
			fences[i]					  = device.CreateFence(true);
		}
	}

	void Release(VkDevice device, const VkAllocationCallbacks* cbs) noexcept
	{
		for(size_t i = 0; i < MaxFramesInFlight; ++i)
		{
			vkDestroySemaphore(device, render_finished_semaphroes[i], cbs);
			vkDestroySemaphore(device, image_ready_semaphore[i], cbs);
			vkDestroyFence(device, fences[i], cbs);
		}
	}

	void NextFrame() noexcept
	{
		current_frame = (current_frame + 1) % MaxFramesInFlight;
	}

	void WaitOnFences(VkDevice device) noexcept
	{
		vkWaitForFences(device, fences.size(), fences.data(), VK_TRUE, 2 * 1000000000);
	}


	std::array<VkSemaphore, MaxFramesInFlight> render_finished_semaphroes{};
	std::array<VkSemaphore, MaxFramesInFlight> image_ready_semaphore{};
	std::array<VkFence, MaxFramesInFlight> fences{};

	std::vector<VkFence> fences_in_use{};

	int current_frame = 0;
};



struct FrameSubmitter
{

	void Init(const VkSwapchainKHR* swapchain) noexcept
	{
		submition.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submition.commandBufferCount = 1;
		submition.waitSemaphoreCount = 1;
		submition.signalSemaphoreCount = 1;

		wait_stage = mvk::PipelineStage::ColorAttachmentOutput;
		submition.pWaitDstStageMask = &wait_stage.flags;

		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.swapchainCount = 1;
		present.pSwapchains = swapchain;
	}
	
	template<typename Device, size_t N>
	void DrawFrame(const Device& device, mvk::CommandBuffer& draw_cmd_buff, const FrameSync<N>& sync) noexcept
	{
		submition.pCommandBuffers = &draw_cmd_buff.vk_cmd_buff;
		submition.pWaitSemaphores = &sync.image_ready_semaphore[sync.current_frame];
		submition.pSignalSemaphores = &sync.render_finished_semaphroes[sync.current_frame];
		
		vkResetFences(device.vk_device, 1, &sync.fences[sync.current_frame]);

		MVK_VALIDATE_RESULT(vkQueueSubmit(device.graphics_family_queue, 1, &submition, sync.fences[sync.current_frame]),
				"Failed to submit frame draw command");
	}

	template<typename Device, size_t N>
	void PresentFrame(const Device& device, const FrameSync<N>& sync, const uint32_t image_index) noexcept
	{
		present.pWaitSemaphores = &sync.render_finished_semaphroes[sync.current_frame];
		present.pImageIndices = &image_index;

		//auto result = vkQueuePresentKHR(device.graphics_family_queue, &present);

		MVK_VALIDATE_RESULT(vkQueuePresentKHR(device.graphics_family_queue, &present),
			"Failed to present rendered frame");
	}
	
private:
	VkSubmitInfo submition{};
	mvk::PipelineStageFlags wait_stage{};

	VkPresentInfoKHR present{};
};


#endif // MVK_SYNC_H
