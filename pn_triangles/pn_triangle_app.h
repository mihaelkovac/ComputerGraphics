#ifndef PN_TRIANGLE_APP_H
#define PN_TRIANGLE_APP_H

#include "app_context.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DETPH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


#include "model.h"
#include "vertex.h"
#include "mvk/camera.h"

struct PNTriangledObject
{
	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout layout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptor_set_layout{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> descriptor_sets{};
	VkViewport view{};
	VkPipeline wire_pipeline{ VK_NULL_HANDLE };
	std::vector<mvk::AllocObj<mvk::Buffer>> uniform_buffs{};
};

struct BaseObject
{
	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout layout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptor_set_layout{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> descriptors_sets{};
	VkViewport view{};
	VkPipeline wire_pipeline{ VK_NULL_HANDLE };
	std::vector<mvk::AllocObj<mvk::Buffer>> uniform_buffs{};
};


struct PNTriangleApp
{
	void InitCamera() noexcept;
	void Run(int width, int height, const char* model_path) noexcept;

	void Release() noexcept;
	
private:

	void InitWindow(int width, int height) noexcept;

	void InitViews() noexcept;
	
	void InitRenderPass() noexcept;

	void InitDescriptorSetLayouts() noexcept;

	void InitPipelines() noexcept;

	void InitCommandPools() noexcept;

	void InitFramebuffers() noexcept;
	
	void LoadMesh(const Model& model) noexcept;
	
	void InitBuffers() noexcept;
	
	void InitUniforms() noexcept;
	
	void UpdateUniform(const uint32_t image_index) noexcept;

	void InitDescriptorPool() noexcept;

	void InitDescriptorSets() noexcept;

	void RecordCommands(const mvk::CommandBuffer::Recording& commands,
						VkPipeline pipeline,
						const VkViewport& view,
						const VkRect2D& scissor,
						VkDescriptorSet* vk_descriptors,
						VkPipelineLayout layout) noexcept;
	
	void InitCommandBuffers() noexcept;

	void Draw() noexcept;

	[[nodiscard]]
	float GetAspectRatio() const noexcept;
	
	AppContext context_;
	GLFWwindow* window_{ nullptr };
	mvk::CommandPool command_pool_{};
	mvk::CommandPool transfer_command_pool_{};
	mvk::RenderPass render_pass_{ VK_NULL_HANDLE };

	BaseObject base_object_{};
	PNTriangledObject pn_object_{};
	VkDescriptorPool descriptor_pool_{ VK_NULL_HANDLE };
	VkPipelineCache pipeline_cache_{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> framebuffers_{};
	
	mvk::Buffer vert_buffer_{};
	mvk::Allocation vert_alloc_{};
	
	mvk::Buffer index_buffer_{};
	mvk::Allocation index_alloc_{};

	std::vector<mvk::CommandBuffer> command_buffers_{};
	bool wireframe_enabled_{ false };

	std::vector<Vertex> vertices_{};
	std::vector<uint32_t> indices_{};
	bool model_rotating_{ false };

	struct UniformTes
	{
		glm::mat4 model{};
		glm::mat4 projection{};
		glm::mat4 view{};
		float tess_alpha{ 1.f };
	};

	struct UniformTsc
	{
		float tess_level{ 1.f };
	};

	struct UniformBasePipeilneVert
	{
		glm::mat4 model{};
		glm::mat4 projection{};
		glm::mat4 view{};
	};

	UniformTsc tsc_uniform{};
	UniformTes tes_uniform{};

	float angle_y = 0.f;
	float angle_z = 0.f;
	mvk::Camera camera_{};
};


inline void PNTriangleApp::InitCamera() noexcept
{
	camera_.position = glm::vec3{ -3.f, -3.f, -30.f };
	camera_.center = glm::vec3{ 0.f, 0.f, 0.f };
}

void PNTriangleApp::Run(int width, int height, const char* model_path) noexcept
{
	InitWindow(width, height);
	InitCamera();

	context_.Init(width, height, window_);

	InitViews();

	InitRenderPass();
	InitFramebuffers();
	
	InitDescriptorPool();
	InitDescriptorSetLayouts();
	
	InitPipelines();

	InitCommandPools();

	//static constexpr const char* MODEL_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/models/teddy.obj";
	const Model model = Model::Load(model_path);
	LoadMesh(model);
	
	InitBuffers();

	InitUniforms();
	InitDescriptorSets();

	InitCommandBuffers();

	while (!glfwWindowShouldClose(window_))
	{
		glfwPollEvents();
		Draw();
	}


}

void PNTriangleApp::Release() noexcept
{
	context_.sync.WaitOnFences(context_.device);
	
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

	for(size_t i = 0, n = base_object_.uniform_buffs.size(); i < n; ++i)
	{
		context_.device.DestroyBuffer(base_object_.uniform_buffs[i].object, base_object_.uniform_buffs[i].allocation);
		
		context_.device.DestroyBuffer(pn_object_.uniform_buffs[2 * i].object, pn_object_.uniform_buffs[2 * i].allocation);
		context_.device.DestroyBuffer(pn_object_.uniform_buffs[2 * i + 1].object, pn_object_.uniform_buffs[2 * i + 1].allocation);
	}

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
		auto& camera = app->camera_;
		float& tess_level = app->tsc_uniform.tess_level;
		float& tess_alpha = app->tes_uniform.tess_alpha;
		float& angle_y = app->angle_y;
		float& angle_z = app->angle_z;
		
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			switch (key)
			{
			case GLFW_KEY_TAB:
				app->wireframe_enabled_ = !app->wireframe_enabled_;
				printf("Wireframe %s\n", app->wireframe_enabled_ ? "Enabled" : "Disabled");
				break;
			case GLFW_KEY_KP_ADD:
				tess_level += tess_level >= 10.f ? 0.f : 0.25f;
				printf("Current tessellation level: %.2f\n", tess_level);
				break;
			case GLFW_KEY_KP_SUBTRACT:
				tess_level -= tess_level <= 1.f ? 0.f : 0.25f;
				printf("Current tessellation level: %.2f\n", tess_level);
				break;
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			case GLFW_KEY_UP:
				camera.position -= 0.1f * (camera.position - camera.center);
				break;
			case GLFW_KEY_DOWN:
				camera.position += 0.1f * (camera.position - camera.center);
				break;
			case GLFW_KEY_A:
				angle_y -= glm::radians(5.f);
				break;
			case GLFW_KEY_D:
				angle_y += glm::radians(5.f);
				break;
			case GLFW_KEY_W:
				angle_z += glm::radians(5.f);
				break;
			case GLFW_KEY_S:
				angle_z -= glm::radians(5.f);
				break;
			case GLFW_KEY_R:
				angle_y = 0.f;
				angle_z = 0.f;
				break;
			default:
				break;
			}
		}
		
	});

	
	glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods)
	{
		auto* app = static_cast<PNTriangleApp*>(glfwGetWindowUserPointer(window));

		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			app->model_rotating_ = action != GLFW_RELEASE;
		}
	});
	
	glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double x, double y)
	{

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
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT/* | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT*/;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT/* | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT*/;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT/* | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT*/;

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

void PNTriangleApp::InitPipelines() noexcept
{
	pipeline_cache_ = context_.device.CreatePipelineCache();

	static constexpr const char* VERTEX_SHADER_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/shaders/base.vert.spv";
	static constexpr const char* PN_VERTEX_SHADER_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/shaders/pn.vert.spv";
	static constexpr const char* FRAGMENT_SHADER_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/shaders/base.frag.spv";
	static constexpr const char* TESSELLATION_CONTROL_SHADER_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/shaders/pn.tesc.spv";
	static constexpr const char* TESSELLATION_EVALUATION_SHADER_LOCATION = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/shaders/pn.tese.spv";

	VkShaderModule vert_shader = context_.device.CreateShaderModule(VERTEX_SHADER_LOCATION);
	VkShaderModule pn_vert_shader = context_.device.CreateShaderModule(PN_VERTEX_SHADER_LOCATION);
	VkShaderModule frag_shader = context_.device.CreateShaderModule(FRAGMENT_SHADER_LOCATION);
	VkShaderModule tcs_shader = context_.device.CreateShaderModule(TESSELLATION_CONTROL_SHADER_LOCATION);
	VkShaderModule tes_shader = context_.device.CreateShaderModule(TESSELLATION_EVALUATION_SHADER_LOCATION);

	const std::array<VkPipelineShaderStageCreateInfo, 2> base_shaders
	{
		mvk::pipe::NewShaderStage(mvk::ShaderStage::Vertex, vert_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::Fragment, frag_shader)
	};
	const std::array<VkPipelineShaderStageCreateInfo, 4> pn_shaders
	{
		mvk::pipe::NewShaderStage(mvk::ShaderStage::Vertex, pn_vert_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::TessellationControl, tcs_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::TessellationEval, tes_shader),
		mvk::pipe::NewShaderStage(mvk::ShaderStage::Fragment, frag_shader)
	};

	const auto attribute_description = Vertex::GetAtributeDescriptions();
	const VkVertexInputBindingDescription binding_description = Vertex::GetBindingDescription();

	VkPipelineVertexInputStateCreateInfo vertex_input{ mvk::pipe::NewVertexInput() };

	vertex_input.pVertexAttributeDescriptions = attribute_description.data();
	vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
	vertex_input.pVertexBindingDescriptions = &binding_description;
	vertex_input.vertexBindingDescriptionCount = 1;

	const VkPipelineInputAssemblyStateCreateInfo input_assembly[]
	{
		mvk::pipe::NewInputAssembly(),
		mvk::pipe::NewInputAssembly(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
	};

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

	VkPipelineColorBlendStateCreateInfo color_blend{ mvk::pipe::NewColorBlend() };
	color_blend.attachmentCount = 1;
	color_blend.pAttachments = &color_blend_attachment;
	
	const VkPipelineDepthStencilStateCreateInfo depth_stencil{ mvk::pipe::NewDepthStencil() };

	const VkPipelineTessellationStateCreateInfo tessellation{ mvk::pipe::NewTessellation(3) };

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
	for (int i = 0; i < 2; ++i)
	{
		pipeline_infos[2 * i].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_infos[2 * i].layout = base_object_.layout;
		pipeline_infos[2 * i].pColorBlendState = &color_blend;
		pipeline_infos[2 * i].pDepthStencilState = &depth_stencil;
		pipeline_infos[2 * i].pDynamicState = &dynamic_state;
		pipeline_infos[2 * i].pInputAssemblyState = &input_assembly[0];
		pipeline_infos[2 * i].pMultisampleState = &multisampling;
		pipeline_infos[2 * i].pRasterizationState = rasterizers[i];
		//pipeline_infos[i].pTessellationState = nullptr;
		pipeline_infos[2 * i].pStages = base_shaders.data();
		pipeline_infos[2 * i].stageCount = base_shaders.size();
		pipeline_infos[2 * i].pVertexInputState = &vertex_input;
		pipeline_infos[2 * i].pViewportState = &viewports[0];
		pipeline_infos[2 * i].subpass = 0;
		pipeline_infos[2 * i].renderPass = render_pass_;

		pipeline_infos[2 * i + 1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_infos[2 * i + 1].layout = pn_object_.layout;
		pipeline_infos[2 * i + 1].pColorBlendState = &color_blend;
		pipeline_infos[2 * i + 1].pDepthStencilState = &depth_stencil;
		pipeline_infos[2 * i + 1].pDynamicState = &dynamic_state;
		pipeline_infos[2 * i + 1].pInputAssemblyState = &input_assembly[1];
		pipeline_infos[2 * i + 1].pMultisampleState = &multisampling;
		pipeline_infos[2 * i + 1].pRasterizationState = rasterizers[i];
		pipeline_infos[2 * i + 1].pTessellationState = &tessellation;
		pipeline_infos[2 * i + 1].pStages = pn_shaders.data();
		pipeline_infos[2 * i + 1].stageCount = pn_shaders.size();
		pipeline_infos[2 * i + 1].pVertexInputState = &vertex_input;
		pipeline_infos[2 * i + 1].pViewportState = &viewports[1];
		pipeline_infos[2 * i + 1].subpass = 0;
		pipeline_infos[2 * i + 1].renderPass = render_pass_;

	}

	VkPipeline pipelines[4];

	context_.device.CreateGraphicsPipelines(pipeline_infos.data(), pipelines, pipeline_infos.size(), pipeline_cache_);

	base_object_.pipeline = pipelines[0];
	pn_object_.pipeline = pipelines[1];
	base_object_.wire_pipeline = pipelines[2];
	pn_object_.wire_pipeline = pipelines[3];

	context_.device.DestroyShaderModule(vert_shader);
	context_.device.DestroyShaderModule(pn_vert_shader);
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

	for (size_t i = 0; i < framebuffers_.size(); ++i)
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

void PNTriangleApp::LoadMesh(const Model& model) noexcept
{
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

			vertex.normal = 
			{
				model.attributes.normals[3 * index.normal_index + 0],
				model.attributes.normals[3 * index.normal_index + 1],
				model.attributes.normals[3 * index.normal_index + 2]
			};

			//if (!model.attributes.texcoords.empty())
			//{
			//	vertex.normal =
			//	{
			//		model.attributes.texcoords[2 * index.texcoord_index + 0],
			//		// vulkan je top to bottom, a tinyobjloader je u koord sustavu gdje je 0 donja
			//		1.0f - model.attributes.texcoords[2 * index.texcoord_index + 1]
			//	};

			//}

			//vertex.color = { 1.0f, 1.0f, 1.0f };

			const auto [emplace_it, emplace_happened] = unique_vertices.try_emplace(vertex, static_cast<uint32_t>(vertices_.size()));
			if (emplace_happened)
			{
				vertices_.emplace_back(std::move(vertex));
			}
			indices_.emplace_back(emplace_it->second);

		}
	}
}

void PNTriangleApp::InitBuffers() noexcept
{	
	mvk::CommandBuffer transfer_cmd_buffer{};
	context_.device.CreateCommandBuffers(transfer_command_pool_, &transfer_cmd_buffer, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	mvk::BufferUsageFlags usages{ mvk::BufferUsage::TransferDst, mvk::BufferUsage::Vertex };

	auto vertex_buffer_info = mvk::Buffer::CreateInfo(sizeof(Vertex) * vertices_.size(), usages);
	auto alloc_info = mvk::Allocation::CreateInfo(VMA_MEMORY_USAGE_GPU_ONLY, mvk::DeviceMemoryProperty::DeviceLocal);
	context_.device.CreateBuffer(vert_buffer_, vert_alloc_, vertex_buffer_info, alloc_info);

	usages = { mvk::BufferUsage::TransferDst, mvk::BufferUsage::Index };
	auto index_buffer_info = mvk::Buffer::CreateInfo(sizeof(uint32_t) * indices_.size(), usages);

	
	context_.device.CreateBuffer(index_buffer_, index_alloc_, index_buffer_info, alloc_info);
	
	auto transfer_buffer_alloc_info = 
		mvk::Allocation::CreateInfo(VMA_MEMORY_USAGE_CPU_ONLY,
				{},
				{ mvk::DeviceMemoryProperty::HostVisible, mvk::DeviceMemoryProperty::HostCoherent },
							   VMA_ALLOCATION_CREATE_MAPPED_BIT);


	usages = mvk::BufferUsage::TransferSrc;
	auto transfer_buffer_info = mvk::Buffer::CreateInfo(vertex_buffer_info.size + index_buffer_info.size, usages);
	
	mvk::AllocObj<mvk::Buffer> transfer_buff = context_.device.CreateBuffer(transfer_buffer_info, transfer_buffer_alloc_info);
	
	MVK_CHECK_FATAL(transfer_buff.Fill(vertices_.data(), vertex_buffer_info.size, 0),
		"PNTriangleApp::InitBuffers - Failed to fill transfer buffer with vertex data");
	
	MVK_CHECK_FATAL(transfer_buff.Fill(indices_.data(), index_buffer_info.size, vertex_buffer_info.size),
		"App::InitBuffers - Failed to fill transfer buffer with index data");
	
	auto commands = transfer_cmd_buffer.Record(mvk::CommandBufferUsage::OneTime);
	
	VkBufferCopy region{};
	region.size = vertex_buffer_info.size;
	region.srcOffset = 0;
	region.dstOffset = 0;
	commands.CopyBuffer(transfer_buff.object, vert_buffer_, &region, 1);

	region.size = index_buffer_info.size;
	region.srcOffset = vertex_buffer_info.size;
	commands.CopyBuffer(transfer_buff.object, index_buffer_, &region, 1);

	commands.Finish();

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pCommandBuffers = &transfer_cmd_buffer.vk_cmd_buff;
	submit.commandBufferCount = 1;

	MVK_VALIDATE_RESULT(vkQueueSubmit(context_.device.transfer_family_queue, 1, &submit, VK_NULL_HANDLE),
		"Failed to submit transfer of model to vertex and index buffers");

	MVK_VALIDATE_RESULT(vkQueueWaitIdle(context_.device.transfer_family_queue), "Interrupted while waiting for transfer to vertex and index buffers to finish");

	context_.device.DestroyCommandBuffers(transfer_command_pool_, &transfer_cmd_buffer);
	context_.device.DestroyBuffer(transfer_buff.object, transfer_buff.allocation);
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
	for (uint32_t i = 0; i < pn_bindings.size(); ++i)
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

void PNTriangleApp::InitDescriptorPool() noexcept
{
	const size_t count = context_.swapchain.images.size();
	const VkDescriptorPoolSize pool_size{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(count * 3ull) };

	descriptor_pool_ = context_.device.CreateDescriptorPool(&pool_size, 1, count * 2ull);

}

void PNTriangleApp::InitDescriptorSets() noexcept
{
	base_object_.descriptors_sets.resize(context_.swapchain.images.size());
	pn_object_.descriptor_sets.resize(context_.swapchain.images.size());

	std::vector<VkDescriptorSetLayout> base_layouts{ base_object_.descriptors_sets.size(), base_object_.descriptor_set_layout };
	std::vector<VkDescriptorSetLayout> pn_layouts{ pn_object_.descriptor_sets.size(), pn_object_.descriptor_set_layout };

	context_.device.AllocateDescriptorSets(descriptor_pool_,
		base_layouts.data(),
		base_object_.descriptors_sets.data(),
		static_cast<uint32_t>(base_object_.descriptors_sets.size()));

	context_.device.AllocateDescriptorSets(descriptor_pool_,
		pn_layouts.data(),
		pn_object_.descriptor_sets.data(),
		static_cast<uint32_t>(pn_object_.descriptor_sets.size()));

	for (size_t i = 0; i < base_layouts.size(); ++i)
	{
		const VkDescriptorBufferInfo uniform_info
		{
			base_object_.uniform_buffs[i].object,
			0,
			sizeof(UniformBasePipeilneVert)
		};

		auto write = mvk::pipe::WriteDescriptorSet(base_object_.descriptors_sets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
			&uniform_info);

		context_.device.UpdateDescriptorSet(&write);
	}

	for (size_t i = 0, n = pn_layouts.size(); i < n; ++i)
	{
		const VkDescriptorBufferInfo tsc_uniform_info
		{
			pn_object_.uniform_buffs[2 * i].object,
			0,
			sizeof(UniformTsc)
		};

		const VkDescriptorBufferInfo tes_uniform_info
		{
			pn_object_.uniform_buffs[2 * i + 1].object,
			0,
			sizeof(UniformTes)
		};

		const std::array<VkWriteDescriptorSet, 2> writes
		{
			mvk::pipe::WriteDescriptorSet(pn_object_.descriptor_sets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &tsc_uniform_info),
			mvk::pipe::WriteDescriptorSet(pn_object_.descriptor_sets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &tes_uniform_info)
		};

		context_.device.UpdateDescriptorSet(writes.data(), writes.size());
	}

}

inline void PNTriangleApp::RecordCommands(const mvk::CommandBuffer::Recording& commands,
										  VkPipeline pipeline,
										  const VkViewport& view,
										  const VkRect2D& scissor,
										  VkDescriptorSet* descriptor,
										  VkPipelineLayout layout) noexcept
{
	commands.SetScissor(scissor);
	commands.BindViewport(view);

	commands.BindPipeline(pipeline);

	commands.BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout,
		descriptor);

	commands.BindVertexBuffers(&vert_buffer_, 1);
	commands.BindIndexBuffers(index_buffer_);

	commands.SubmitDrawIndexed(indices_.size());
}

void PNTriangleApp::InitUniforms() noexcept
{
	const VkDeviceSize tsc_uniform_size = sizeof(UniformTsc);
	const VkDeviceSize tes_uniform_size = sizeof(UniformTes);

	const VkDeviceSize base_uniform_size = sizeof(UniformBasePipeilneVert);

	const size_t n = framebuffers_.size();
	base_object_.uniform_buffs.resize(n);
	pn_object_.uniform_buffs.resize(n * 2);

	
	for(size_t i = 0; i < n; ++i)
	{
		context_.device.CreateUniformBuffer(base_object_.uniform_buffs[i].object, base_object_.uniform_buffs[i].allocation,
			base_uniform_size);
		
		context_.device.CreateUniformBuffer(pn_object_.uniform_buffs[2 * i].object, pn_object_.uniform_buffs[2 * i].allocation,
			tsc_uniform_size);
		
		context_.device.CreateUniformBuffer(pn_object_.uniform_buffs[2 * i + 1].object, pn_object_.uniform_buffs[2 * i + 1].allocation,
			tes_uniform_size);

	}

	for (uint32_t i = 0; i < framebuffers_.size(); ++i)
	{
		UpdateUniform(i);
	}
}

void PNTriangleApp::UpdateUniform(const uint32_t image_index) noexcept
{
	// Base pipeline uniform update
	const float aspect_ratio = GetAspectRatio();
	UniformBasePipeilneVert base_uniform{};

	base_uniform.model = glm::rotate(glm::mat4(1.f), glm::radians(130.f), glm::vec3(0.f, 0.f, 1.f));
	base_uniform.model = glm::rotate(base_uniform.model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
	base_uniform.model = glm::rotate(base_uniform.model, angle_y, glm::vec3(0.f, 1.f, 0.f));
	base_uniform.model = glm::rotate(base_uniform.model, angle_z, glm::vec3(0.f, 0.f, 1.f));
	base_uniform.model = glm::scale(base_uniform.model, glm::vec3(0.3f, 0.3f, 0.3f));
	base_uniform.view = glm::lookAt(camera_.position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	base_uniform.projection = glm::perspective(glm::radians(45.f), GetAspectRatio(), 0.1f, 100.f);

	base_uniform.projection[1][1] *= -1; // Kod GLM-a obrnuto od Vulkana pa moram negirat
	auto& base_uniform_alloc = base_object_.uniform_buffs[image_index].allocation;
	context_.device.FillUniform(&base_uniform, sizeof(UniformBasePipeilneVert), base_uniform_alloc);


	// PNPipelineUniformUpdate
	tes_uniform.view = base_uniform.view;
	tes_uniform.model = base_uniform.model;
	tes_uniform.projection = base_uniform.projection;
	const auto& tsc_alloc = pn_object_.uniform_buffs[2 * image_index].allocation;
	const auto& tes_alloc = pn_object_.uniform_buffs[2 * image_index + 1].allocation;
	context_.device.FillUniform(&tsc_uniform, sizeof(UniformTsc), tsc_alloc);
	context_.device.FillUniform(&tes_uniform, sizeof(UniformTes), tes_alloc);
}

void PNTriangleApp::InitCommandBuffers() noexcept
{
	const size_t image_count = context_.swapchain.GetImageCount();
	command_buffers_.resize(image_count * 2);

	context_.device.CreateCommandBuffers(command_pool_, command_buffers_.data(), command_buffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkClearValue val[2]{};
	val[0].color = { 0.7f, 0.7f, 0.7f, 1.f };
	val[1].depthStencil = { 1.0f, 0 };

	auto render_pass_info = render_pass_.BeginInfo(VK_NULL_HANDLE, val, 2);
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = context_.swapchain.extent;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = context_.swapchain.extent;

	//const VkViewport viewport{0, 0, context_.swapchain.extent.width, context_.swapchain.extent.height, 0.f, 1.f};

	for (size_t i = 0; i < image_count; ++i)
	{
		render_pass_info.framebuffer = framebuffers_[i];

		auto commands = command_buffers_[i].Record();

		commands.BeginRenderPass(render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		// Draw regular object
		RecordCommands(commands,
					   base_object_.pipeline,
					   base_object_.view,
					   scissor,
					   &base_object_.descriptors_sets[i],
					   base_object_.layout);

		RecordCommands(commands,
					   pn_object_.pipeline,
					   pn_object_.view,
					   scissor,
					   &pn_object_.descriptor_sets[i],
					   pn_object_.layout);

		commands.EndRenderPass();
		commands.Finish();

		auto wireframe_commands = command_buffers_[image_count + i].Record();

		wireframe_commands.BeginRenderPass(render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		
		RecordCommands(wireframe_commands,
					   base_object_.wire_pipeline,
					   base_object_.view,
					   scissor,
					   &base_object_.descriptors_sets[i],
					   base_object_.layout);

		RecordCommands(wireframe_commands,
					   pn_object_.wire_pipeline,
					   pn_object_.view,
					   scissor,
					   &pn_object_.descriptor_sets[i],
					   pn_object_.layout);

		wireframe_commands.EndRenderPass();
		wireframe_commands.Finish();

	}
}

void PNTriangleApp::Draw() noexcept
{
	const int current_frame = context_.sync.current_frame;

	auto& current_fence = context_.sync.fences[current_frame];
	vkWaitForFences(context_.device, 1, &current_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(context_.device, 1, &current_fence);

	uint32_t image_index;
	VkResult result = context_.AcquireSwapchainImage(image_index);

	if (VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
	{
		fprintf(stderr, "Failed to acquire image");
		abort();
	}

	if (context_.sync.fences_in_use[image_index] != VK_NULL_HANDLE)
	{
		vkWaitForFences(context_.device, 1, &context_.sync.fences_in_use[image_index], VK_TRUE, UINT64_MAX);
	}

	context_.sync.fences_in_use[image_index] = current_fence;

	UpdateUniform(image_index);

	context_.submitter.DrawFrame(context_.device, command_buffers_[image_index + wireframe_enabled_ * context_.swapchain.GetImageCount()], context_.sync);

	context_.submitter.PresentFrame(context_.device, context_.sync, image_index);

	context_.sync.NextFrame();

}

inline float PNTriangleApp::GetAspectRatio() const noexcept
{
	return 0.5f * static_cast<float>(context_.swapchain.extent.width) / context_.swapchain.extent.height;
}


#endif // PN_TRIANGLE_APP_H
