#ifndef APP_H
#define APP_H

#include "lab.h"
#include <glm/glm.hpp>

#define PARTICLE_COUNT 1024 * 256

struct Pipelines
{
	VkPipelineCache cache{ 0 };

	VkPipelineLayout computeLayout{ 0 };
	VkPipeline computePipeline{ 0 };

	VkPipelineLayout graphicsLayout{ 0 };
	VkPipeline graphicsPipeline{ 0 };
};

struct Compute
{
	VkCommandPool commandPool{ nullptr };
	std::vector<VkCommandBuffer> commandBuffers{};
	/*mvk::CommandBuffer commandBuffer{ nullptr };*/
	std::vector<std::pair<mvk::Buffer, mvk::Allocation>> particleBuffersAlloc{};
	std::vector<std::pair<mvk::Buffer, mvk::Allocation>> uniformBuffers{};
	VkDescriptorSetLayout descriptorSetLayout{ nullptr };
	std::vector<VkDescriptorSet> descriptorSets{};
	//VkSemaphore semaphore{ nullptr };
	std::array<VkSemaphore, MaxFramesInFlight> semaphores{};
	std::array<VkFence, MaxFramesInFlight> fences{};
	std::vector<VkFence> computesInFlight{};

	struct UniformBuffObj
	{
		float deltaT{0.f};
		float destX{0.f};
		float destY{0.f};
		int32_t numOfParticles = PARTICLE_COUNT;
	};

	std::vector<UniformBuffObj> ubos{};
};

struct alignas(16) Particle
{
	glm::vec2 pos{};
	glm::vec2 velocity{};
	glm::vec3 color{};
	glm::vec3 colorChange{};

	static constexpr VkVertexInputBindingDescription getBindingDescription() noexcept
	{
		VkVertexInputBindingDescription description{};
		description.binding = 0;
		description.stride = sizeof(Particle);
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return description;
	}

	static constexpr std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() noexcept
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		// describe inPosition
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Particle, pos);

		// describe inColor
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Particle, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Particle, velocity);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Particle, colorChange);

		return attributeDescriptions;

	}
};



struct Graphics
{
	// Command pool za grafiku vec imam u contextu
	/*VkDescriptorSetLayout descriptorSetLayout{0};
	VkDescriptorSet descriptorSets{0};*/
	std::array<VkSemaphore, MaxFramesInFlight> semaphores{};


};

struct App
{
	void run(int width, int height) noexcept;

	void release() noexcept;
	
private:

	
	void initRenderPass() noexcept;

	void initDescriptorPool() noexcept;

	void initFrameBuffers() noexcept;

	void createCommandBuffers() noexcept;
	
	void initGraphics() noexcept;
	
	void initPipelines() noexcept;

	void initComputePipeline() noexcept;

	void initGraphicsPipeline() noexcept;

	void initStorageBuffers() noexcept;
	
	void initCompute() noexcept;

	void initComputeDescriptors();

	void recordComputeCommands() noexcept;

	void recordGraphicsCommands() noexcept;
	
	void recordCommands() noexcept;

	void prepareParticleData() noexcept;

	void updateUniform(const uint32_t imgIndex) noexcept;

	void draw() noexcept;

	void initSyncPrimtives() noexcept;
	
	void init(int width, int height) noexcept;

	VkRenderPass renderPass{ VK_NULL_HANDLE };

	Pipelines pipelines{};
	LabContext context{};
	Compute compute{};
	Graphics graphics{};

	VkDescriptorPool descriptorPool{ nullptr };
	std::vector<VkFramebuffer> framebuffers{};
	std::vector<VkCommandBuffer> commandBuffers{};
	VkCommandPool transferPool{ 0 };

	glm::vec2 mousePosition;
	

};



#endif