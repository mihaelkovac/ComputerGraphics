#include "pn_triangle_app.h"

#include "vertex.h"


void PNTriangleApp::Run(int width, int height) noexcept
{
	InitWindow(width, height);

	context_.Init(width, height, window_);

	InitViews();

	InitRenderPass();
	InitDescriptorPool();
	InitDescriptorSetLayouts();
	InitPipelines();

	InitCommandPools();
	InitFramebuffers();

	static constexpr const char* MODEL_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/models/teddy.obj";
	const Model model = Model::Load(MODEL_LOCATION);
	InitBuffers(model);

	InitDescriptorSets();
	InitUniforms();


	while(!glfwWindowShouldClose(window_))
	{
		glfwPollEvents();
		Draw();
	}
	
	
}

void PNTriangleApp::Release() noexcept
{

	context_.ReleaseDepthResource();
	
	context_.device.DestroyFramebuffers(framebuffers_.data(), framebuffers_.size());

	context_.device.DestroyCommandBuffers(command_pool_, command_buffers_.data(), command_buffers_.size());

	context_.device.DestroyPipelineCache(pipeline_cache_);

	context_.device.DestroyPipeline(base_object_.pipeline);
	context_.device.DestroyPipeline(base_object_.wire_pipeline);
	context_.device.DestroyPipelineLayout(base_object_.layout);

	context_.device.DestroyPipeline(pn_object_.pipeline);
	context_.device.DestroyPipeline(pn_object_.wire_pipeline);
	context_.device.DestroyPipelineLayout(pn_object_.layout);
	
	context_.device.DestroyRenderPass(render_pass_);

	context_.device.DestroyBuffer(base_object_.uniform_buff.object, base_object_.uniform_buff.allocation);
	context_.device.DestroyBuffer(pn_object_.uniform_buff.object, pn_object_.uniform_buff.allocation);

	context_.device.DestroyBuffer(vert_buffer_, vert_alloc_);
	context_.device.DestroyBuffer(index_buffer_, index_alloc_);

	context_.device.DestroyDescriptorPool(descriptor_pool_);

	context_.device.DestroyDescriptorSetLayout(base_object_.descriptor_set_layout);
	context_.device.DestroyDescriptorSetLayout(pn_object_.descriptor_set_layout);

	context_.device.DestroyCommandPool(command_pool_);
	context_.device.DestroyCommandPool(transfer_command_pool_);
	
	context_.Release();
	glfwDestroyWindow(window_);
	glfwTerminate();
}

void PNTriangleApp::InitWindow(int width, int height) noexcept
{
	MVK_CHECK_FATAL(glfwInit(), "PNTriangleApp::Run - Failed to initialize GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window_ = glfwCreateWindow(width, height, "PNTriangle Example App", nullptr, nullptr);
	glfwSetWindowUserPointer(window_, this);
	
	glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scan_code, int action, int mods)
	{
		void* ptr = glfwGetWindowUserPointer(window);
		auto* app = static_cast<PNTriangleApp*>(ptr);
		float& tess_level = app->tsc_uniform.tess_level;
		
		if(action == GLFW_PRESS)
		{
			switch(key)
			{
			case GLFW_KEY_W:
				app->wireframe_enabled_ = !app->wireframe_enabled_;
				printf("Wireframe %s", app->wireframe_enabled_ ? "Enabled" : "Disabled");
				break;
			case GLFW_KEY_KP_ADD:
			case GLFW_KEY_I:
				tess_level += tess_level >= 10.f ? 0.f : 0.25f;
				printf("Current tessellation level: %.2f", tess_level);
				break;
			case GLFW_KEY_KP_SUBTRACT:
			case GLFW_KEY_D:
				tess_level -= tess_level <= 1.f ? 0.f : 0.25f;
				printf("Current tessellation level: %.2f", tess_level);
				break;
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			default: 
				break;
			}
		}

	});
}

void PNTriangleApp::InitRenderPass() noexcept
{

	std::array<VkAttachmentDescription, 2> attachments
	{
		context_.GetSwapchainColorAttachment(VK_SAMPLE_COUNT_1_BIT),
		context_.GetDepthAttachment(VK_SAMPLE_COUNT_1_BIT)
	};
	
	VkAttachmentReference color_ref{};
	color_ref.attachment = 0;
	color_ref.layout = attachments[0].finalLayout;

	VkAttachmentReference depth_ref{};
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pColorAttachments = &color_ref;
	subpass.colorAttachmentCount = 1;
	subpass.pDepthStencilAttachment = &depth_ref;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	
	
	VkRenderPassCreateInfo pass_info{};
	pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	pass_info.pSubpasses = &subpass;
	pass_info.subpassCount = 1;
	pass_info.pDependencies = &dependency;
	pass_info.dependencyCount = 1;
	pass_info.pAttachments = attachments.data();
	pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());

	render_pass_ = context_.device.CreateRenderPass(pass_info);
		
}

void PNTriangleApp::InitViews() noexcept
{
	float f_width = static_cast<float>(context_.swapchain.extent.width);
	float f_height = static_cast<float>(context_.swapchain.extent.height);

	VkViewport& left_view = base_object_.view;
	VkViewport& right_view = pn_object_.view;
	
	left_view.width = f_width / 2.f;
	left_view.height = f_height;
	left_view.minDepth = 0.f;
	left_view.maxDepth = 1.f;
	left_view.x = 0.f;
	left_view.y = 0.f;

	right_view.width = f_width / 2.f;
	right_view.height = f_height;
	right_view.minDepth = 0.f;
	right_view.maxDepth = 1.f;
	right_view.x = f_width / 2.f;
	right_view.y = 0.f;
}

void PNTriangleApp::InitDescriptorSetLayouts() noexcept
{
	// Base object layout
	mvk::ShaderStageFlags stage_flags{};
	std::array<VkDescriptorSetLayoutBinding, 1> base_bindings{};

	base_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	base_bindings[0].binding = 0;
	base_bindings[0].descriptorCount = 1;
	stage_flags = mvk::ShaderStage::Vertex;
	base_bindings[0].stageFlags = stage_flags;
	
	VkDescriptorSetLayoutCreateInfo base_layout_info{};
	base_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	base_layout_info.bindingCount = static_cast<uint32_t>(base_bindings.size());
	base_layout_info.pBindings = base_bindings.data();

	base_object_.descriptor_set_layout = context_.device.CreateDescriptorSetLayout(base_layout_info);
	

	// PNTriangle object layout
	std::array<VkDescriptorSetLayoutBinding, 2> pn_bindings{};
	mvk::ShaderStageFlags pn_stages[]{ mvk::ShaderStage::TessellationControl, mvk::ShaderStage::TessellationEval };
	for(uint32_t i = 0; i < pn_bindings.size(); ++i)
	{
		pn_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pn_bindings[i].binding = i;
		pn_bindings[i].descriptorCount = 1;
		pn_bindings[i].stageFlags = pn_stages[i];
	}

	VkDescriptorSetLayoutCreateInfo pn_layout_info{};
	pn_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	pn_layout_info.bindingCount = static_cast<uint32_t>(pn_bindings.size());
	pn_layout_info.pBindings = pn_bindings.data();

	pn_object_.descriptor_set_layout = context_.device.CreateDescriptorSetLayout(pn_layout_info);
	
}

void PNTriangleApp::InitPipelines() noexcept
{
	pipeline_cache_ = context_.device.CreatePipelineCache();

	static constexpr const char* VERTEX_SHADER_LOCATION = "";
	static constexpr const char* FRAGMENT_SHADER_LOCATION = "";
	static constexpr const char* TESSELLATION_CONTROL_SHADER_LOCATION = "";
	static constexpr const char* TESSELLATION_EVALUATION_SHADER_LOCATION = "";

	VkShaderModule vert_shader = context_.device.CreateShaderModule(VERTEX_SHADER_LOCATION);
	VkShaderModule frag_shader = context_.device.CreateShaderModule(FRAGMENT_SHADER_LOCATION);
	VkShaderModule tcs_shader = context_.device.CreateShaderModule(TESSELLATION_CONTROL_SHADER_LOCATION);
	VkShaderModule tes_shader = context_.device.CreateShaderModule(TESSELLATION_EVALUATION_SHADER_LOCATION);

	

	const VkPipelineShaderStageCreateInfo shaders[]
	{
		mvk::pipe::NewShaderStage(mvk::ShaderStage::Vertex, vert_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::Fragment, frag_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::TessellationControl, tcs_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::TessellationEval, tes_shader)
	};

	const std::array<VkVertexInputAttributeDescription, 3> attribute_description = Vertex::GetAtributeDescriptions();
	const VkVertexInputBindingDescription binding_description = Vertex::GetBindingDescription();
	
	VkPipelineVertexInputStateCreateInfo vertex_input{ mvk::pipe::NewVertexInput() };

	vertex_input.pVertexAttributeDescriptions = attribute_description.data();
	vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
	vertex_input.pVertexBindingDescriptions = &binding_description;
	vertex_input.vertexBindingDescriptionCount = 1;

	const VkPipelineInputAssemblyStateCreateInfo input_assembly{ mvk::pipe::NewInputAssembly() };

	VkPipelineViewportStateCreateInfo viewports[]
	{
		mvk::pipe::NewViewportState(),
		mvk::pipe::NewViewportState()
	};
	

	//VkRect2D rect{};
	//rect.extent = context_.swapchain.extent;
	////rect.extent.width /= 2;
	//rect.offset = { 0, 0 };

	//viewports[0].pScissors = &rect;
	//viewports[0].scissorCount = 1;
	//viewports[0].pViewports = &base_object_.view;
	//viewports[0].viewportCount = 1;
	//
	//viewports[1].pScissors = &rect;
	//viewports[1].scissorCount = 1;
	//viewports[1].pViewports = &pn_object_.view;
	//viewports[1].viewportCount = 1;

	const VkPipelineMultisampleStateCreateInfo multisampling{ mvk::pipe::NewMultisampling() };

	const VkPipelineRasterizationStateCreateInfo base_rasterization{ mvk::pipe::NewRasterizer() };
	VkPipelineRasterizationStateCreateInfo wireframe_rasterization{ mvk::pipe::NewRasterizer() };

	wireframe_rasterization.polygonMode = VK_POLYGON_MODE_LINE;

	const VkPipelineColorBlendAttachmentState color_blend_attachment{ mvk::pipe::NewColorBlendAttachment() };

	const VkPipelineColorBlendStateCreateInfo color_blend{ mvk::pipe::NewColorBlend() };
	
	const VkPipelineDepthStencilStateCreateInfo depth_stencil{ mvk::pipe::NewDepthStencil() };
	
	const VkPipelineTessellationStateCreateInfo tessellation{ mvk::pipe::NewTessellation() };

	VkPipelineDynamicStateCreateInfo dynamic_state{ mvk::pipe::NewDynamicState() };
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	std::array<VkDynamicState, 2> dynamic_enables
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_enables.size());
	dynamic_state.pDynamicStates = dynamic_enables.data();

	VkPipelineLayoutCreateInfo layout_info{ mvk::pipe::NewLayout() };
	layout_info.setLayoutCount = 1;
	layout_info.pSetLayouts = &base_object_.descriptor_set_layout;
	
	base_object_.layout = context_.device.CreatePipelineLayout(layout_info);

	layout_info.pSetLayouts = &pn_object_.descriptor_set_layout;
	pn_object_.layout = context_.device.CreatePipelineLayout(layout_info);

	const VkPipelineRasterizationStateCreateInfo* rasterizers[]{ &base_rasterization, &wireframe_rasterization };
	std::array<VkGraphicsPipelineCreateInfo, 4> pipeline_infos{};
	for(int i = 0; i < 2; ++i)
	{
		pipeline_infos[i].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_infos[i].layout = base_object_.layout;
		pipeline_infos[i].pColorBlendState = &color_blend;
		pipeline_infos[i].pDepthStencilState = &depth_stencil;
		pipeline_infos[i].pDynamicState = &dynamic_state;
		pipeline_infos[i].pInputAssemblyState = &input_assembly;
		pipeline_infos[i].pMultisampleState = &multisampling;
		pipeline_infos[i].pRasterizationState = rasterizers[i];
		pipeline_infos[i].pTessellationState = &tessellation;
		pipeline_infos[i].pStages = shaders;
		pipeline_infos[i].stageCount = 2;
		pipeline_infos[i].pVertexInputState = &vertex_input;
		pipeline_infos[i].pViewportState = &viewports[0];
		pipeline_infos[i].subpass = 0;
		pipeline_infos[i].renderPass = render_pass_;

		pipeline_infos[i + 1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_infos[i + 1].layout = pn_object_.layout;
		pipeline_infos[i + 1].pColorBlendState = &color_blend;
		pipeline_infos[i + 1].pDepthStencilState = &depth_stencil;
		pipeline_infos[i + 1].pDynamicState = &dynamic_state;
		pipeline_infos[i + 1].pInputAssemblyState = &input_assembly;
		pipeline_infos[i + 1].pMultisampleState = &multisampling;
		pipeline_infos[i + 1].pRasterizationState = rasterizers[i];
		pipeline_infos[i + 1].pTessellationState = &tessellation;
		pipeline_infos[i + 1].pStages = shaders;
		pipeline_infos[i + 1].stageCount = 4;
		pipeline_infos[i + 1].pVertexInputState = &vertex_input;
		pipeline_infos[i + 1].pViewportState = &viewports[1];
		pipeline_infos[i + 1].subpass = 0;
		pipeline_infos[i + 1].renderPass = render_pass_;

	}

	VkPipeline pipelines[4];
	
	context_.device.CreateGraphicsPipelines(pipeline_infos.data(), pipelines, pipeline_infos.size(), pipeline_cache_);

	base_object_.pipeline = pipelines[0];
	pn_object_.pipeline = pipelines[1];
	base_object_.wire_pipeline = pipelines[2];
	pn_object_.wire_pipeline = pipelines[3];
	
	context_.device.DestroyShaderModule(vert_shader);
	context_.device.DestroyShaderModule(frag_shader);
	context_.device.DestroyShaderModule(tcs_shader);
	context_.device.DestroyShaderModule(tes_shader);
}

void PNTriangleApp::InitCommandPools() noexcept
{
	command_pool_.vk_command_pool = context_.device.CreateCommandPool(context_.device.graphics_family_index);
	transfer_command_pool_.vk_command_pool = context_.device.CreateCommandPool(context_.device.transfer_family_index);
}

void PNTriangleApp::InitFramebuffers() noexcept
{
	framebuffers_.resize(context_.swapchain.images.size());
	auto [width, height] = context_.swapchain.extent;
	
	for(size_t i = 0; i < framebuffers_.size(); ++i)
	{
		std::array<VkImageView, 2> attachments
		{
			context_.swapchain.images[i].vk_image_view,
			context_.depth.image_view
		};

		framebuffers_[i] = context_.device.CreateFramebuffer(render_pass_, width, height, 1, attachments.data(),
		                                                     attachments.size());
	}
}


void PNTriangleApp::InitBuffers(const Model& model) noexcept
{
	
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	std::unordered_map<Vertex, uint32_t> unique_vertices{};
	for (const auto& shape : model.shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{


			Vertex vertex{};

			vertex.pos =
			{
				model.attributes.vertices[3 * index.vertex_index + 0],
				model.attributes.vertices[3 * index.vertex_index + 1],
				model.attributes.vertices[3 * index.vertex_index + 2]
			};
			if (!model.attributes.texcoords.empty())
			{
				vertex.tex_coord =
				{
					model.attributes.texcoords[2 * index.texcoord_index + 0],
					// vulkan je top to bottom, a tinyobjloader je u koord sustavu gdje je 0 donja
					1.0f - model.attributes.texcoords[2 * index.texcoord_index + 1]
				};

			}

			vertex.color = { 1.0f, 1.0f, 1.0f };

			const auto [emplace_it, emplace_happened] = unique_vertices.try_emplace(vertex, static_cast<uint32_t>(vertices.size()));
			if (emplace_happened)
			{
				vertices.emplace_back(std::move(vertex));
			}
			indices.emplace_back(emplace_it->second);

		}
	}

	vert_count_ = vertices.size();
	
	mvk::CommandBuffer transfer_cmd_buffer{};
	context_.device.CreateCommandBuffers(transfer_command_pool_, &transfer_cmd_buffer, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	mvk::BufferUsageFlags usages{ mvk::BufferUsage::TransferDst, mvk::BufferUsage::Vertex };

	auto buffer_info = mvk::Buffer::CreateInfo(sizeof(Vertex) * vertices.size(), usages);
	auto alloc_info = mvk::Allocation::CreateInfo(VMA_MEMORY_USAGE_GPU_ONLY, mvk::DeviceMemoryProperty::DeviceLocal);
	context_.device.CreateBuffer(vert_buffer_, vert_alloc_, buffer_info, alloc_info);

	usages = mvk::BufferUsage::Index;
	buffer_info.usage = usages;

	context_.device.CreateBuffer(index_buffer_, index_alloc_, buffer_info, alloc_info);

	VmaAllocationCreateInfo transfer_buffer_alloc_info = mvk::Allocation::CreateInfo(VMA_MEMORY_USAGE_CPU_TO_GPU, {},
{ mvk::DeviceMemoryProperty::HostVisible, mvk::DeviceMemoryProperty::HostCoherent });
	
	
	usages = mvk::BufferUsage::TransferSrc;
	buffer_info.usage = usages;
	//buffer_info.size = sizeof(Vertex) * vertices.size() + indices.size();

	mvk::AllocObj<mvk::Buffer> transfer_buff = context_.device.CreateBuffer(buffer_info, transfer_buffer_alloc_info);
	transfer_buff.Fill(vertices.data(), vertices.size() * sizeof(Vertex), 0);
	transfer_buff.Fill(indices.data(), indices.size() * sizeof(uint32_t), vertices.size() * sizeof(Vertex));
	auto commands = transfer_cmd_buffer.Record();

	VkBufferCopy region{};
	region.size = sizeof(Vertex) * vertices.size();
	region.srcOffset = 0;
	region.dstOffset = 0;
	commands.CopyBuffer(transfer_buff.object, vert_buffer_, &region, 1);

	//region.size = sizeof(int) * indices.size();
	region.srcOffset = sizeof(Vertex) * vertices.size();
	commands.CopyBuffer(transfer_buff.object, index_buffer_, &region, 1);

	commands.Finish();

	VkSubmitInfo submit;
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pCommandBuffers = &transfer_cmd_buffer.vk_cmd_buff;
	submit.commandBufferCount = 1;

	MVK_VALIDATE_RESULT(vkQueueSubmit(context_.device.transfer_family_queue, 1, &submit, VK_NULL_HANDLE),
		"Failed to submit transfer of model to vertex and index buffers");

	MVK_VALIDATE_RESULT(vkQueueWaitIdle(context_.device.transfer_family_queue), "Interrupted while waiting for transfer to vertex and index buffers to finish");

	context_.device.DestroyCommandBuffers(transfer_command_pool_, &transfer_cmd_buffer);
	context_.device.DestroyBuffer(transfer_buff.object, transfer_buff.allocation);
}

void PNTriangleApp::InitDescriptorPool() noexcept
{
	VkDescriptorPoolSize pool_size{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(framebuffers_.size() * 3ull) };

	descriptor_pool_ = context_.device.CreateDescriptorPool(&pool_size, 1, framebuffers_.size()/* * 2*/);
	//base_object_.descriptor_pool = context_.device.CreateDescriptorPool(&pool_size, 1, framebuffers_.size());
	//pn_object_.descriptor_pool = context_.device.CreateDescriptorPool(&pool_size, 1, framebuffers_.size());
	
}

void PNTriangleApp::InitDescriptorSets() noexcept
{
	base_object_.descriptors.resize(context_.swapchain.images.size());
	pn_object_.descriptors.resize(context_.swapchain.images.size() * 2);

	std::vector<VkDescriptorSetLayout> base_layouts{ context_.swapchain.images.size(), base_object_.descriptor_set_layout };
	std::vector<VkDescriptorSetLayout> pn_layouts{ context_.swapchain.images.size(), pn_object_.descriptor_set_layout };

	context_.device.AllocateDescriptorSets(descriptor_pool_,
										   base_layouts.data(),
										   base_object_.descriptors.data(),
										   static_cast<uint32_t>(base_object_.descriptors.size()));

	context_.device.AllocateDescriptorSets(descriptor_pool_,
									 pn_layouts.data(),
									   pn_object_.descriptors.data(),
									       static_cast<uint32_t>(pn_object_.descriptors.size()));

	for(size_t i = 0; i < base_layouts.size(); ++i)
	{
		const VkDescriptorBufferInfo uniform_info
		{
			base_object_.uniform_buff.object,
			i * sizeof(UniformBasePipeilneVert),
			sizeof(UniformBasePipeilneVert)
		};

		auto write = mvk::pipe::WriteDescriptorSet(base_object_.descriptors[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
		                                           &uniform_info);

		context_.device.UpdateDescriptorSet(&write);
	}

	for(size_t i = 0, n = pn_layouts.size(); i < n; ++i)
	{
		const VkDescriptorBufferInfo tsc_uniform_info
		{
			pn_object_.uniform_buff.object,
			i * sizeof(UniformTsc),
			sizeof(UniformTsc)
		};

		const VkDescriptorBufferInfo tes_uniform_info
		{
			pn_object_.uniform_buff.object,
			i * sizeof(UniformTes) + n * sizeof(UniformTsc),
			sizeof(UniformTes)
		};

		const std::array<VkWriteDescriptorSet, 2> writes
		{
			mvk::pipe::WriteDescriptorSet(pn_object_.descriptors[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &tsc_uniform_info),
			mvk::pipe::WriteDescriptorSet(pn_object_.descriptors[i + n], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &tes_uniform_info)
		};

		context_.device.UpdateDescriptorSet(writes.data(), writes.size());
	}

}

void PNTriangleApp::InitUniforms() noexcept
{
	

	const VkDeviceSize tes_uniform_size = sizeof(UniformTes);
	const VkDeviceSize tsc_uniform_size = sizeof(UniformTsc);

	const VkDeviceSize base_uniform_size = sizeof(UniformBasePipeilneVert);

	context_.device.CreateUniformBuffer(base_object_.uniform_buff.object, base_object_.uniform_buff.allocation,
	                                    base_uniform_size * framebuffers_.size());
	context_.device.CreateUniformBuffer(pn_object_.uniform_buff.object, pn_object_.uniform_buff.allocation,
	                                    (tes_uniform_size + tsc_uniform_size) * framebuffers_.size());
	
	for(uint32_t i = 0; i < framebuffers_.size(); ++i)
	{
		UpdateUniform(i);
	}
}

void PNTriangleApp::UpdateUniform(const uint32_t image_index) noexcept
{
	// Base pipeline uniform update
	const float aspect_ratio = context_.GetAspectRatio();
	UniformBasePipeilneVert base_uniform{};
	base_uniform.model = glm::mat4(1.f);
	base_uniform.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	base_uniform.projection = glm::perspective(glm::radians(45.0f), aspect_ratio, .1f, 10.f);

	base_uniform.projection[1][1] *= -1; // Kod GLM-a obrnuto od Vulkana pa moram negirat

	auto* base_mapped_data = reinterpret_cast<UniformBasePipeilneVert*>(base_object_.uniform_buff.allocation.alloc_info.pMappedData);
	memcpy(base_mapped_data + image_index, &base_uniform, sizeof(UniformBasePipeilneVert));


	// PNPipelineUniformUpdate
	UniformTes tes_uniform{};
	tes_uniform.view = base_uniform.view;
	tes_uniform.model = base_uniform.model;
	tes_uniform.projection = base_uniform.projection;
	tes_uniform.tess_alpha = 1.f; // TODO: Ovo provjeri

	auto* pn_mapped_data = reinterpret_cast<char*>(pn_object_.uniform_buff.allocation.alloc_info.pMappedData);
	memcpy(pn_mapped_data + image_index * sizeof(UniformTsc), &tsc_uniform, sizeof(UniformTsc));
	memcpy(pn_mapped_data + framebuffers_.size() * sizeof(UniformTsc) + image_index * sizeof(UniformTes), &tes_uniform,
	       sizeof(UniformTes));
}

void PNTriangleApp::InitCommandBuffers() noexcept
{
	command_buffers_.resize(context_.swapchain.images.size());

	context_.device.CreateCommandBuffers(command_pool_, command_buffers_.data(), command_buffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkClearValue val[2]{};
	val[0].color= { 0.5f, 0.5f, 0.5f, 1.f };
	val[1].depthStencil = { 1.0f, 0 };
	
	auto render_pass_info = render_pass_.BeginInfo(VK_NULL_HANDLE, val, 2);
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = context_.swapchain.extent;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = context_.swapchain.extent;
	
	for(size_t i = 0; i < command_buffers_.size(); ++i)
	{
		render_pass_info.framebuffer = framebuffers_[i];

		auto commands = command_buffers_[i].Record();
		
		commands.BeginRenderPass(render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		commands.BindViewport(base_object_.view);
		commands.SetScissor(scissor);

		commands.BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
									base_object_.layout,
									&base_object_.descriptors[i]);
		
		commands.BindVertexBuffers(&vert_buffer_, 1);

		commands.BindIndexBuffers(index_buffer_);

		if(wireframe_enabled_)
		{
			commands.BindPipeline(base_object_.wire_pipeline);
		}
		else
		{
			commands.BindPipeline(base_object_.pipeline);
		}

		commands.SubmitDraw(vert_count_);

		commands.BindViewport(pn_object_.view);

		if(wireframe_enabled_)
		{
			commands.BindPipeline(pn_object_.wire_pipeline);
		}
		else
		{
			commands.BindPipeline(pn_object_.pipeline);
		}

		commands.SubmitDraw(vert_count_);
		
		commands.EndRenderPass();
		
		commands.Finish();
	}
}

void PNTriangleApp::Draw() noexcept
{
	const int current_frame = context_.sync.current_frame;

	auto& current_fence = context_.sync.fences[current_frame];
	vkWaitForFences(context_.device, 1, &context_.sync.fences[current_frame], VK_TRUE, UINT64_MAX);
	vkResetFences(context_.device, 1, &context_.sync.fences[current_frame]);

	uint32_t image_index;
	VkResult result = context_.AcquireSwapchainImage(image_index);

	if(VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
	{
		fprintf(stderr, "Failed to acquire image");
		abort();
	}

	if(context_.sync.fences_in_use[image_index] != VK_NULL_HANDLE)
	{
		vkWaitForFences(context_.device, 1, &context_.sync.fences_in_use[image_index], VK_TRUE, UINT64_MAX);
	}

	context_.sync.fences_in_use[image_index] = context_.sync.fences[current_frame];

	UpdateUniform(image_index);

	context_.submitter.DrawFrame(context_.device, command_buffers_[image_index], context_.sync);

	context_.submitter.PresentFrame(context_.device, context_.sync, image_index);

	context_.sync.NextFrame();
	
}
