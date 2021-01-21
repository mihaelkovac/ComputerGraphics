#ifndef VERTEX_H
#define VERTEX_H

#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

struct Vertex
{
    glm::vec3 pos;
    //glm::vec3 color;
    glm::vec3 normal;

    bool operator==(const Vertex& o) const noexcept
    {
        return pos == o.pos     &&
			   //color == o.color &&
			   normal == o.normal;
    }

    static constexpr VkVertexInputBindingDescription GetBindingDescription() noexcept
    {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 2> GetAtributeDescriptions() noexcept
    {
        std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};

        // describe inPosition
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, normal);

        // describe inColor
        //attribute_descriptions[2].binding = 0;
        //attribute_descriptions[2].location = 2;
        //attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        //attribute_descriptions[2].offset = offsetof(Vertex, color);

        return attribute_descriptions;

    };

};



namespace std
{

    template<> struct hash<Vertex>
    {
        size_t operator()(const Vertex& vert) const noexcept
        {
#define _HASH(_var) hash<decltype(_var)>{}(_var)

            size_t hpos    = _HASH(vert.pos);
            //size_t hcolor  = _HASH(vert.color);
            size_t hnormal = _HASH(vert.normal);

            return ((hpos ^ (hnormal << 1)) >> 1)/* ^ (hcolor << 1)*/;
        }
    };

};



#endif