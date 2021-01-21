
#ifndef MVK_RENDER_PASS_H
#define RENDER_PASS_H

#include <vulkan/vulkan.h>

namespace mvk
{
	struct RenderPass
	{

		operator VkRenderPass() const noexcept
		{
			return vk_render_pass;
		}

		[[nodiscard]] VkRenderPassBeginInfo BeginInfo(VkFramebuffer framebuffer, const VkClearValue* clear_values, const size_t clear_value_count) noexcept
		{
			VkRenderPassBeginInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = vk_render_pass;
			info.framebuffer = framebuffer;
			info.pClearValues = clear_values;
			info.clearValueCount = clear_value_count;
			return info;
		}
		
		VkRenderPass vk_render_pass;
		
	};

	static_assert(sizeof(RenderPass) == sizeof(VkRenderPass), "mvk::RenderPass has to be a thin layer around VkRenderPass");


	struct RenderPassBuilder
	{
		constexpr RenderPassBuilder() noexcept
		{
			create_info_.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			VkSubpassDescription description{};
			
			
			
		}
		
	private:
		VkRenderPassCreateInfo create_info_{};
	};

}

#endif
