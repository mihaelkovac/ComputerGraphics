#include "app.h"

#include <random>

std::vector<uint8_t> readFile(const char* filename)
{
	FILE* file = nullptr;
	fopen_s(&file, filename, "rb");
	if (!file)
	{
		fprintf(stderr, "Error while trying to read binary file: %s\n", filename);
		abort();
	}
	fseek(file, 0L, SEEK_END);
	long size = ftell(file);
	fseek(file, 0L, SEEK_SET);


	std::vector<uint8_t> result(size);
	fread(result.data(), 1, size, file);

	return result;

}


void App::run(int width, int height) noexcept
{
	init(width, height);

	//for(uint32_t i = 0; i < compute.ubos.size(); ++i)
	//{
	//	updateUniform(i);
	//}

	while(!glfwWindowShouldClose(context.window))
	{
		draw();
		//updateUniform();
		glfwPollEvents();
	}

	vkDeviceWaitIdle(context.device);
}

void App::release() noexcept
{
	// TODO: ovaj release poboljsaj to sve
	
	auto cbs = context.device.getAllocationCallbacks();
	context.releaseDepthResource();
	
	for (size_t i = 0; i < framebuffers.size(); ++i)
	{
		vkDestroyFramebuffer(context.device, framebuffers[i], cbs);
	}

	context.device.destroyCommandBuffers(context.commandPool, commandBuffers.data(), static_cast<uint32_t>(commandBuffers.size()));

	context.device.destroyCommandBuffers(compute.commandPool, compute.commandBuffers.data(), static_cast<uint32_t>(commandBuffers.size()));

	vkDestroyPipelineCache(context.device, pipelines.cache, cbs);
	
	vkDestroyPipeline(context.device, pipelines.graphicsPipeline, cbs);
	vkDestroyPipelineLayout(context.device, pipelines.graphicsLayout, cbs);
	vkDestroyRenderPass(context.device, renderPass, cbs);

	
	
	vkDestroyPipeline(context.device, pipelines.computePipeline, cbs);
	vkDestroyPipelineLayout(context.device, pipelines.computeLayout, cbs);

	context.releaseSwapchain();
	
	for(size_t i = 0; i < compute.particleBuffersAlloc.size(); ++i)
	{
		context.device.destroyBuffer(compute.particleBuffersAlloc[i].first, compute.particleBuffersAlloc[i].second);
	}

	for(size_t i = 0; i < compute.uniformBuffers.size(); ++i)
	{
		context.device.destroyBuffer(compute.uniformBuffers[i].first, compute.uniformBuffers[i].second);
	}

	
	vkDestroyDescriptorPool(context.device, descriptorPool, cbs);

	vkDestroyDescriptorSetLayout(context.device, compute.descriptorSetLayout, cbs);

	context.frameSync.release(context.device, cbs);
	for(size_t i = 0; i < MaxFramesInFlight; ++i)
	{
		vkDestroySemaphore(context.device, compute.semaphores[i], cbs);
		vkDestroyFence(context.device, compute.fences[i], cbs);
		vkDestroySemaphore(context.device, graphics.semaphores[i], cbs);
	}


	//vkDestroySemaphore(context.device, compute.semaphore, cbs);
	//vkDestroySemaphore(context.device, graphics.semaphore, cbs);

	vkDestroyCommandPool(context.device, context.commandPool, cbs);
	vkDestroyCommandPool(context.device, transferPool, cbs);
	vkDestroyCommandPool(context.device, compute.commandPool, cbs);

	//vkDestroySemaphore(context.device, semaphores.presentationComplete, nullptr);
	//vkDestroySemaphore(context.device, semaphores.renderComplete, nullptr);
	
	context.release();
	context.releaseGLFW();
}


void App::initRenderPass() noexcept
{
	// TODO: Poboljsaj ovo s render passom
	std::array<VkAttachmentDescription, 2> attachments{};
	//std::array<VkAttachmentDescription, 1> attachments{};
	// Color attachment
	attachments[0].format = context.swapchain.format.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format = context.depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();


	MVK_VALIDATE_RESULT(vkCreateRenderPass(context.device, &renderPassInfo, context.device.getAllocationCallbacks(), &renderPass),
		"Failed to create render pass");
}

void App::initDescriptorPool() noexcept
{
	//const size_t uboN = compute.ubos.size();
	//const size_t particleBuffN = compute.particleBuffersAlloc.size();
	std::array<VkDescriptorPoolSize, 2> descriptorSizes
	{
		VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(context.swapchain.images.size())},
		VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(context.swapchain.images.size())}
	};

	descriptorPool = context.device.createDescriptorPool(descriptorSizes.data(), descriptorSizes.size(), context.swapchain.images.size());
	
}

void App::initFrameBuffers() noexcept
{
	framebuffers.resize(context.swapchain.images.size());
	auto [swapchainWidth, swapchainHeight] = context.swapchain.extent;
	
	for(size_t i = 0; i < framebuffers.size(); ++i)
	{
		VkImageView attachments[] = { context.swapchain.images[i].vkImageView, context.depthImageView };
		//VkImageView attachments[] = { context.swapchain.images[i].vkImageView};
		framebuffers[i] =
			context.device.createFramebuffer
			(
				renderPass,
				swapchainWidth,
				swapchainHeight,
				1,
				attachments,
				2
			);
	}
	
	context.frameSync.imagesInFlight.resize(context.swapchain.images.size());
}

void App::createCommandBuffers() noexcept
{
	commandBuffers.resize(context.swapchain.images.size());
	compute.commandBuffers.resize(context.swapchain.images.size());

	context.device.createCommandBuffers(context.commandPool, commandBuffers.data(),
	                                    static_cast<uint32_t>(commandBuffers.size()), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	
	context.device.createCommandBuffers(compute.commandPool, compute.commandBuffers.data(),
	                                    static_cast<uint32_t>(compute.commandBuffers.size()),
	                                    VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}


void App::initGraphicsPipeline() noexcept
{
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{ mvk::pipe::newShaderStage(), mvk::pipe::newShaderStage() };

	static constexpr const char* VERTEX_SHADER_LOCATION = "D:/workspace_cpp/Lab2RG/Lab2RG/vert.spv";
	static constexpr const char* FRAGMENT_SHADER_LOCATION = "D:/workspace_cpp/Lab2RG/Lab2RG/frag.spv";
	
	
	auto vertShaderBinary = readFile(VERTEX_SHADER_LOCATION);
	auto fragShaderBinary = readFile(FRAGMENT_SHADER_LOCATION);

	VkShaderModule vertShader = context.device.createShaderModule(reinterpret_cast<uint32_t*>(vertShaderBinary.data()), vertShaderBinary.size());
	VkShaderModule fragShader = context.device.createShaderModule(reinterpret_cast<uint32_t*>(fragShaderBinary.data()), fragShaderBinary.size());

	shaderStages[0].module = vertShader;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

	shaderStages[1].module = fragShader;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	// TODO: Dodaj specialization info za compute shader
	
	VkPipelineVertexInputStateCreateInfo vertexInput = mvk::pipe::newVertexInput();

	auto vertAttributeDescriptions = Particle::getAttributeDescriptions();
	auto vertBindingDescriptions = Particle::getBindingDescription();
	
	vertexInput.pVertexAttributeDescriptions = vertAttributeDescriptions.data();
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertAttributeDescriptions.size());
	vertexInput.pVertexBindingDescriptions = &vertBindingDescriptions;
	vertexInput.vertexBindingDescriptionCount = 1;
	
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = mvk::pipe::newInputAssembly();
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	VkPipelineViewportStateCreateInfo viewportState = mvk::pipe::newViewportState();

	VkRect2D rect{};
	rect.extent = context.swapchain.extent;
	rect.offset = { 0, 0 };

	VkViewport viewport{};
	viewport.width = static_cast<float>(context.swapchain.extent.width);
	viewport.height = static_cast<float>(context.swapchain.extent.height);
	viewport.maxDepth = 1.f;
	viewport.minDepth = 0.f;
	viewport.x = 0.f;
	viewport.y = 0.f;

	viewportState.pScissors = &rect;
	viewportState.scissorCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.viewportCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = mvk::pipe::newRasterizer();
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	
	VkPipelineMultisampleStateCreateInfo multisampling = mvk::pipe::newMultisampling();
	VkPipelineDepthStencilStateCreateInfo depthStencil = mvk::pipe::newDepthStencil();
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
	depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineColorBlendStateCreateInfo colorBlend = mvk::pipe::newColorBlend();
	VkPipelineColorBlendAttachmentState blendAttchment = mvk::pipe::newColorBlendAttachment();
	colorBlend.pAttachments = &blendAttchment;
	colorBlend.attachmentCount = 1;

	VkPipelineLayoutCreateInfo layoutInfo = mvk::pipe::newLayout();
	layoutInfo.setLayoutCount = 0;
	layoutInfo.pushConstantRangeCount = 0;

	pipelines.graphicsLayout = context.device.createPipelineLayout(layoutInfo);
	
	VkGraphicsPipelineCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.layout = pipelines.graphicsLayout;
	info.pColorBlendState = &colorBlend;
	info.pDepthStencilState = &depthStencil;
	info.pDynamicState = nullptr;
	info.pMultisampleState = &multisampling;
	info.pInputAssemblyState = &inputAssembly;
	info.pRasterizationState = &rasterizer;
	info.pStages = shaderStages.data();
	info.stageCount = static_cast<uint32_t>(shaderStages.size());
	info.pVertexInputState = &vertexInput;
	info.pViewportState = &viewportState;
	info.renderPass = renderPass;
	info.subpass = 0;

	MVK_VALIDATE_RESULT(
		vkCreateGraphicsPipelines(context.device, pipelines.cache, 1, &info, context.device.getAllocationCallbacks(), &
			pipelines.graphicsPipeline),
		"Failed to create graphics pipeline");
	// TODO: ovaj dio poboljast
	//mvk::GraphicsPipelineRequest graphicsRequest
	//{
	//	&vertexInput,
	//	&inputAssembly,
	//	shaderStages.data(),
	//	static_cast<uint32_t>(shaderStages.size()),
	//	nullptr,
	//	&viewportState,
	//	nullptr,
	//	&rasterizer,
	//	&multisampling,
	//	&colorBlend,
	//	&depthStencil,
	//	renderPass
	//};

	//VkGraphicsPipelineCreateInfo graphicsPipeInfo;
	//context.device.createGraphicsPipelines(&graphicsRequest, &graphicsPipeInfo, &pipelines.graphicsLayout, &pipelines.graphicsPipeline, 1, pipelines.cache);

	context.device.destroyShaderModule(vertShader);
	context.device.destroyShaderModule(fragShader);
}

void App::initStorageBuffers() noexcept
{
	VkBufferCreateInfo bufferInfo = mvk::Buffer::createInfo(sizeof(Compute::UniformBuffObj), mvk::BufferUsage::Uniform);
	VmaAllocationCreateInfo allocInfo =  mvk::Allocation::createInfo(VMA_MEMORY_USAGE_CPU_TO_GPU,
																	 {},
																	 {mvk::DeviceMemoryProperty::HostVisible,mvk::DeviceMemoryProperty::HostCoherent});
	const uint32_t swapchainImgCount = context.swapchain.images.size();
	compute.ubos.resize(swapchainImgCount);
	compute.uniformBuffers.resize(swapchainImgCount);
	compute.particleBuffersAlloc.resize(swapchainImgCount);
	for(uint32_t i = 0; i < swapchainImgCount; ++i)
	{
		compute.uniformBuffers[i] = context.device.createBuffer(bufferInfo, allocInfo);
		updateUniform(i);
	}


	bufferInfo = mvk::Buffer::createInfo(PARTICLE_COUNT * sizeof(Particle), { mvk::BufferUsage::Storage, mvk::BufferUsage::TransferDst, mvk::BufferUsage::Vertex });
	allocInfo = mvk::Allocation::createInfo(VMA_MEMORY_USAGE_GPU_ONLY, mvk::DeviceMemoryProperty::DeviceLocal, mvk::DeviceMemoryProperty::Undefined);

	for(uint32_t i = 0; i < swapchainImgCount; ++i)
	{
		 compute.particleBuffersAlloc[i] = context.device.createBuffer(bufferInfo, allocInfo);
	}
	

}

void App::initCompute() noexcept
{	
	/*compute.commandPool = context.device.createCommandPool(context.device.computeFamilyIndex, mvk::CommandPoolFlag::ResetCommand);*/

	recordComputeCommands();

	if(context.device.graphicsFamilyIndex != context.device.computeFamilyIndex)
	{
		mvk::CommandBuffer transferCmd{};
		context.device.createCommandBuffers(this->transferPool, &transferCmd.vkCmdBuff, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		auto commands = transferCmd.record();

		for(auto& [buff, alloc] : compute.particleBuffersAlloc)
		{
			VkBufferMemoryBarrier acquireBarrier = buff.createBarrier(context.device.graphicsFamilyIndex,
																							   context.device.computeFamilyIndex,
																							   mvk::AccessFlag::Base,
																							   mvk::AccessFlag::ShaderWrite);

			
			
			acquireBarrier.size = alloc.vmaAllocInfo.size;
			acquireBarrier.offset = alloc.vmaAllocInfo.offset;
			
			mvk::BarrierRequest request{ mvk::PipelineStage::VertexInput, mvk::PipelineStage::ComputeShader };
			request.bufferMemoryBarriers = &acquireBarrier;
			request.bufferMemoryBarrierCount = 1;

			commands.pipelineBarrier(request);

			VkBufferMemoryBarrier releaseBarrier = buff.createBarrier(context.device.computeFamilyIndex,
																							   context.device.graphicsFamilyIndex,
																							   mvk::AccessFlag::ShaderWrite,
																							   mvk::AccessFlag::Base);
			request.bufferMemoryBarriers = &releaseBarrier;

			commands.pipelineBarrier(request);
		}
		

		commands.finish();

		VkSubmitInfo submition{};
		submition.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submition.commandBufferCount = 1;
		submition.pCommandBuffers = &transferCmd.vkCmdBuff;

		VkFence fence = context.device.createFence();
		MVK_VALIDATE_RESULT(vkQueueSubmit(context.device.transferFamilyQueue, 1, &submition, fence),
			"Failed to submit transfer from compute to graphics");
		
		MVK_VALIDATE_RESULT(vkWaitForFences(context.device, 1, &fence, VK_TRUE, UINT64_MAX),
			"Interrupted while waiting for family transfer to finish");

		context.device.destroyCommandBuffers(transferPool, &transferCmd.vkCmdBuff, 1);
		vkDestroyFence(context.device, fence, context.device.getAllocationCallbacks());
	}

}

void App::initComputeDescriptors()
{
	compute.descriptorSets.resize(context.swapchain.images.size());
	
	std::vector<VkDescriptorSetLayout> layouts{ context.swapchain.images.size(), compute.descriptorSetLayout };

	context.device.allocateDescriptorSets(descriptorPool, layouts.data(), compute.descriptorSets.data(),
	                                      static_cast<uint32_t>(compute.descriptorSets.size()));
	for(size_t i = 0; i < layouts.size(); ++i)
	{
		VkDescriptorBufferInfo uniformBufferInfo{};
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.buffer = compute.uniformBuffers[i].first;
		uniformBufferInfo.range = sizeof(Compute::UniformBuffObj);

		VkDescriptorBufferInfo storageBuffInfo{};
		storageBuffInfo.offset = 0;
		storageBuffInfo.buffer = compute.particleBuffersAlloc[i].first;
		storageBuffInfo.range = PARTICLE_COUNT * sizeof(Particle);

		std::array<VkWriteDescriptorSet, 2> computeWriteSets
		{
			mvk::pipe::writeDescriptorSet(compute.descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferInfo),
			mvk::pipe::writeDescriptorSet(compute.descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &storageBuffInfo)
		};

		vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(computeWriteSets.size()), computeWriteSets.data(), 0, nullptr);
		
	}
	
	
}

void App::recordComputeCommands() noexcept
{
	for(size_t i = 0; i < compute.commandBuffers.size(); ++i)
	{
		mvk::CommandBuffer commandBuffer = compute.commandBuffers[i];
		auto commandRecording = commandBuffer.record(0);
		if (context.device.graphicsFamilyIndex != context.device.computeFamilyIndex)
		{
			auto& [buff, alloc] = compute.particleBuffersAlloc[i];
			VkBufferMemoryBarrier barrier = buff.createBarrier(context.device.graphicsFamilyIndex, context.device.computeFamilyIndex,
				mvk::AccessFlag::Base, mvk::AccessFlag::ShaderWrite);

			barrier.offset = alloc.vmaAllocInfo.offset;
			barrier.size = alloc.vmaAllocInfo.size;

			mvk::BarrierRequest request{ mvk::PipelineStage::VertexInput, mvk::PipelineStage::ComputeShader };
			request.bufferMemoryBarriers = &barrier;
			request.bufferMemoryBarrierCount = 1;


			commandRecording.pipelineBarrier(request);
		}

		commandRecording.bindPipeline(pipelines.computePipeline, VK_PIPELINE_BIND_POINT_COMPUTE);
		commandRecording.bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.computeLayout, &compute.descriptorSets[i]);
		commandRecording.dispatch(PARTICLE_COUNT / 256, 1, 1);

		if (context.device.graphicsFamilyIndex != context.device.computeFamilyIndex)
		{
			auto& [buff, alloc] = compute.particleBuffersAlloc[i];
			VkBufferMemoryBarrier barrier = buff.createBarrier(context.device.computeFamilyIndex, context.device.graphicsFamilyIndex,
				mvk::AccessFlag::ShaderWrite, mvk::AccessFlag::Base);

			barrier.offset = alloc.vmaAllocInfo.offset;
			barrier.size = alloc.vmaAllocInfo.size;

			mvk::BarrierRequest request{ mvk::PipelineStage::ComputeShader, mvk::PipelineStage::VertexInput };
			request.bufferMemoryBarriers = &barrier;
			request.bufferMemoryBarrierCount = 1;


			commandRecording.pipelineBarrier(request);
		}

		commandRecording.finish();
	}

	
	
}

void App::recordGraphicsCommands() noexcept
{

	std::array<VkClearValue, 2> clearValues;
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBegin{};
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.renderPass = renderPass;
	renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBegin.pClearValues = clearValues.data();
	renderPassBegin.renderArea.offset = { 0, 0 };
	renderPassBegin.renderArea.extent = context.swapchain.extent;

	for(size_t i = 0; i < commandBuffers.size(); ++i)
	{
		renderPassBegin.framebuffer = framebuffers[i];

		mvk::CommandBuffer cmdBuffer = commandBuffers[i];
		auto recording = cmdBuffer.record(0);
		
		auto& [buff, alloc] = compute.particleBuffersAlloc[i];

		if(context.device.graphicsFamilyIndex != context.device.computeFamilyIndex)
		{
			VkBufferMemoryBarrier barrier = buff.createBarrier(context.device.graphicsFamilyIndex, context.device.computeFamilyIndex,
																						mvk::AccessFlag::Base, mvk::AccessFlag::VertexAttributeRead);
			
			barrier.offset = alloc.vmaAllocInfo.offset;
			barrier.size = alloc.vmaAllocInfo.size;

			mvk::BarrierRequest request{mvk::PipelineStage::ComputeShader, mvk::PipelineStage::VertexInput};
			request.bufferMemoryBarriers = &barrier;
			request.bufferMemoryBarrierCount = 1;
			
			recording.pipelineBarrier(request);
			
		}

		recording.beginRenderPass(renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		recording.bindPipeline(pipelines.graphicsPipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

		VkDeviceSize offsets[1] = { 0 };
		recording.bindVertexBuffers(&buff.vkBuffer, 1, 0, offsets);
		recording.submitDraw(PARTICLE_COUNT);

		recording.endRenderPass();

		if (context.device.graphicsFamilyIndex != context.device.computeFamilyIndex)
		{

			VkBufferMemoryBarrier barrier = buff.createBarrier(context.device.graphicsFamilyIndex, context.device.computeFamilyIndex,
				mvk::AccessFlag::VertexAttributeRead, mvk::AccessFlag::Base);

			barrier.size = alloc.vmaAllocInfo.size;
			barrier.offset = alloc.vmaAllocInfo.offset;
			
			mvk::BarrierRequest request{mvk::PipelineStage::VertexInput, mvk::PipelineStage::ComputeShader};
			request.bufferMemoryBarriers = &barrier;
			request.bufferMemoryBarrierCount = 1;
			

			recording.pipelineBarrier(request);
		}

		recording.finish();
	}
	
	
}

void App::prepareParticleData() noexcept
{
	
	
	std::random_device randDevice{};
	std::uniform_real_distribution<float> posDistribution{-1.f, 1.f};
	std::uniform_real_distribution<float> colorChangeDistribution{ 0.0f, 0.15f };
	

	std::vector<Particle> particles{PARTICLE_COUNT};

	for(Particle& particle : particles)
	{
		particle.pos = { posDistribution(randDevice), posDistribution(randDevice) };
		particle.color = { posDistribution(randDevice), posDistribution(randDevice), posDistribution(randDevice) };
		particle.velocity = { 0.f, 0.f };
		particle.colorChange = { colorChangeDistribution(randDevice), colorChangeDistribution(randDevice), colorChangeDistribution(randDevice) };
	}

	transferPool = context.device.createCommandPool(context.device.transferFamilyIndex, mvk::CommandPoolFlag::Transient);

	mvk::CommandBuffer transferCmdBuffer{};
	context.device.createCommandBuffers(transferPool, &transferCmdBuffer.vkCmdBuff, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	
	const VkDeviceSize bufferSize = sizeof(Particle) * particles.size();

	VkBufferCreateInfo bufferInfo = mvk::Buffer::createInfo(bufferSize, mvk::BufferUsage::TransferSrc);
	VmaAllocationCreateInfo allocInfo = mvk::Allocation::createInfo
	(
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		mvk::DeviceMemoryProperty::Undefined,
		{ mvk::DeviceMemoryProperty::HostVisible, mvk::DeviceMemoryProperty::HostCoherent }
	);

	auto [transferBuffer, transferAlloc] = context.device.createBuffer(bufferInfo, allocInfo);
	
	void* data = context.device.mapMemory(transferAlloc);

	memcpy(data, particles.data(), particles.size());

	context.device.unmapMemory(transferAlloc);
	
	auto commands = transferCmdBuffer.record(mvk::CommandBufferUsage::OneTime);

	VkBufferCopy buffercpy{};
	buffercpy.srcOffset = 0;
	buffercpy.dstOffset = 0;
	buffercpy.size = transferAlloc.vmaAllocInfo.size;

	for(auto& [buff, alloc] : compute.particleBuffersAlloc)
	{
		commands.copyBuffer(transferBuffer, buff.vkBuffer, &buffercpy, 1);
		
		if(context.device.transferFamilyIndex != context.device.computeFamilyIndex)
		{
			VkBufferMemoryBarrier barrier = buff.createBarrier(context.device.transferFamilyIndex, context.device.computeFamilyIndex,
																						mvk::AccessFlag::VertexAttributeRead, mvk::AccessFlag::Base);
			const auto& particleAlloc = alloc.vmaAllocInfo;
			barrier.offset = particleAlloc.offset;
			barrier.size = particleAlloc.size;

			mvk::BarrierRequest request{ mvk::PipelineStage::VertexInput, mvk::PipelineStage::ComputeShader };
			request.bufferMemoryBarriers = &barrier;
			request.bufferMemoryBarrierCount = 1;
			
			
		}
	}



	commands.finish();

	VkSubmitInfo submition{};
	submition.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submition.commandBufferCount = 1;
	submition.pCommandBuffers = &transferCmdBuffer.vkCmdBuff;

	MVK_VALIDATE_RESULT(vkQueueSubmit(context.device.transferFamilyQueue, 1, &submition, VK_NULL_HANDLE),
		"Failed to submit particle data transfer to storage buffer");
	MVK_VALIDATE_RESULT(vkQueueWaitIdle(context.device.transferFamilyQueue),
		"Interrupted while waiting for transfer of particle data to finish");


	context.device.destroyCommandBuffers(transferPool, &transferCmdBuffer.vkCmdBuff, 1);
	context.device.destroyBuffer(transferBuffer, transferAlloc);
	
}

void App::updateUniform(const uint32_t imgIndex) noexcept
{
	static auto start = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - start).count();

	auto& ubo = compute.ubos[imgIndex];
	ubo.deltaT = time;
	ubo.destX = sin(glm::radians(time * 360.0f)) * 0.9f;
	ubo.destY = cos(glm::radians(time * 360.0f)) * 0.9f;

	auto& [buff, alloc] = compute.uniformBuffers[imgIndex];
	void* data;
	vkMapMemory(context.device, alloc.vmaAllocInfo.deviceMemory, alloc.vmaAllocInfo.offset, alloc.vmaAllocInfo.size, 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(context.device, alloc.vmaAllocInfo.deviceMemory);
	
	//for(uint32_t i = 0; i < compute.ubos.size(); ++i)
	//{
	//	if(imgIndex != i)
	//	{
	//		memcpy(&compute.ubos[i], &ubo, sizeof(ubo));
	//	}
	//}
}

void App::initComputePipeline() noexcept
{

	std::array<VkDescriptorSetLayoutBinding, 2> bindings
	{
		mvk::pipe::descriptorLayoutBinding(mvk::ShaderStage::Compute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0),
		mvk::pipe::descriptorLayoutBinding(mvk::ShaderStage::Compute, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
	};


	VkDescriptorSetLayoutCreateInfo descriptorSetLayout{};
	descriptorSetLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayout.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetLayout.pBindings = bindings.data();
	

	compute.descriptorSetLayout = context.device.createDescriptorSetLayout(descriptorSetLayout);

	VkPipelineLayoutCreateInfo layoutInfo = mvk::pipe::newLayout();
	layoutInfo.pSetLayouts = &compute.descriptorSetLayout;
	layoutInfo.setLayoutCount = 1;

	pipelines.computeLayout = context.device.createPipelineLayout(layoutInfo);

	initComputeDescriptors();
	
	VkPipelineShaderStageCreateInfo shaderStage = mvk::pipe::newShaderStage();
	shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

	static constexpr const char* COMPUTE_SHADER_LOCATION = "D:/workspace_cpp/Lab2RG/Lab2RG/comp.spv";
	
	auto fileData = readFile(COMPUTE_SHADER_LOCATION);
	VkShaderModule computeShader = context.device.createShaderModule(reinterpret_cast<uint32_t*>(fileData.data()), fileData.size());
	shaderStage.module = computeShader;
	

	mvk::ComputePipelineRequest request{&shaderStage};

	VkComputePipelineCreateInfo createInfoStorage{};
	context.device.createComputePipelines(&request, &createInfoStorage, &pipelines.computeLayout, &pipelines.computePipeline, 1U, pipelines.cache);

	context.device.destroyShaderModule(computeShader);



	
}

void App::draw() noexcept
{
	const size_t current = context.frameSync.current;
	VkFence waitFences[] = { context.frameSync.inFlightFences[current], compute.fences[current] };
	vkWaitForFences(context.device, 2, waitFences, VK_TRUE, UINT64_MAX);
	vkResetFences(context.device, 2, waitFences);

	uint32_t imgIndex;
	VkResult result = vkAcquireNextImageKHR(context.device, context.swapchain.vkSwapchain, UINT64_MAX,
	                                        context.frameSync.imgAvailableSemaphores[current], VK_NULL_HANDLE,
	                                        &imgIndex);

	if (VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
	{
		fprintf(stderr, "Failed to acquire image");
		abort();
	}

	if (context.frameSync.imagesInFlight[imgIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(context.device, 1, &context.frameSync.imagesInFlight[imgIndex], VK_TRUE, UINT64_MAX);
	}

	if(compute.computesInFlight[imgIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(context.device, 1, &compute.computesInFlight[imgIndex], VK_TRUE, UINT64_MAX);
	}
	
	context.frameSync.imagesInFlight[imgIndex] = context.frameSync.inFlightFences[current];
	compute.computesInFlight[imgIndex] = compute.fences[current];

	//updateUniform(imgIndex);
	
	VkPipelineStageFlags graphicsWaitFlags[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore graphicsWaitSemaphores[] = { compute.semaphores[current], context.frameSync.imgAvailableSemaphores[current]};
	VkSemaphore graphicsSignalSemaphroes[] = { graphics.semaphores[current], context.frameSync.renderFinishedSempahores[current] };
	
	VkSubmitInfo submition{};
	submition.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submition.commandBufferCount = 1;
	submition.pCommandBuffers = &commandBuffers[imgIndex];
	submition.waitSemaphoreCount = 2;
	submition.pWaitSemaphores = graphicsWaitSemaphores;
	submition.signalSemaphoreCount = 2;
	submition.pSignalSemaphores = graphicsSignalSemaphroes;
	submition.pWaitDstStageMask = graphicsWaitFlags;


	MVK_VALIDATE_RESULT(vkQueueSubmit(context.device.graphicsFamilyQueue, 1, &submition, context.frameSync.inFlightFences[current]),
		"Failed to submit graphics command buffer");

	VkPresentInfoKHR presentation{};
	presentation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentation.pWaitSemaphores = &context.frameSync.renderFinishedSempahores[current];
	presentation.waitSemaphoreCount = 1;
	presentation.pSwapchains = &context.swapchain.vkSwapchain;
	presentation.swapchainCount = 1;
	presentation.pImageIndices = &imgIndex;

	vkQueuePresentKHR(context.device.presentationFamilyQueue, &presentation);

	updateUniform(imgIndex);
	//vkQueueWaitIdle(context.device.presentationFamilyQueue);
	//if(context.device.presentationFamilyIndex != context.device.graphicsFamilyIndex)
	//{
	//	vkQueueWaitIdle(context.device.graphicsFamilyQueue);
	//}
	//

	
	mvk::PipelineStageFlags waitStage = mvk::PipelineStage::ComputeShader;
	VkSubmitInfo computeSubmit{};
	computeSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmit.commandBufferCount = 1;
	computeSubmit.pCommandBuffers = &compute.commandBuffers[imgIndex];
	computeSubmit.waitSemaphoreCount = 1;
	computeSubmit.pWaitSemaphores = &graphics.semaphores[current];
	computeSubmit.pWaitDstStageMask = &waitStage.flags;
	computeSubmit.signalSemaphoreCount = 1;
	computeSubmit.pSignalSemaphores = &compute.semaphores[current];
	MVK_VALIDATE_RESULT(vkQueueSubmit(context.device.computeFamilyQueue, 1, &computeSubmit, compute.fences[current]),
		"Failed to submit compute operations to VkQueue");
	
	context.frameSync.current = (current + 1) % MaxFramesInFlight;
}

void App::initSyncPrimtives() noexcept
{
	compute.computesInFlight.resize(context.swapchain.images.size());
	for(size_t i = 0; i < MaxFramesInFlight; ++i)
	{
		compute.fences[i] = context.device.createFence(true);
		compute.semaphores[i] = context.device.createSemaphore();
		graphics.semaphores[i] = context.device.createSemaphore();
	}

	VkSubmitInfo submition{};
	submition.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submition.signalSemaphoreCount = static_cast<uint32_t>(compute.semaphores.size());
	submition.pSignalSemaphores = compute.semaphores.data();
	MVK_VALIDATE_RESULT(vkQueueSubmit(context.device.computeFamilyQueue, 1, &submition, VK_NULL_HANDLE),
		"Failed to submit compute semaphores initial signal");
	MVK_VALIDATE_RESULT(vkQueueWaitIdle(context.device.computeFamilyQueue),
		"Interrupted while waiting for compute semaphroes to be signaled");
	
}

void App::init(int width, int height) noexcept
{
	context.init(width, height);

	
	compute.commandPool = context.device.createCommandPool(context.device.computeFamilyIndex);
	createCommandBuffers();
	initSyncPrimtives();
	//graphics.semaphore = context.device.createSemaphore();
	initRenderPass();
	pipelines.cache = context.device.createPipelineCache();
	initFrameBuffers();

	initDescriptorPool();

	initStorageBuffers();
	prepareParticleData();
	initGraphicsPipeline();
	initComputePipeline();
	initCompute();
	recordGraphicsCommands();

}






int main(int argc, const char** argv)
{
	App app{};

	app.run(800, 600);

	app.release();

	return 0;
}
