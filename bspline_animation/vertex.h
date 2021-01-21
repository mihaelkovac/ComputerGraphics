#ifndef VERTEX_H
#define VERTEX_H

#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex& o) const noexcept
    {
        return pos == o.pos &&
               color == o.color &&
               texCoord == o.texCoord;
    }

    static constexpr VkVertexInputBindingDescription getBindingDescription() noexcept
    {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return description;
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() noexcept
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        // describe inPosition
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos);

        // describe inColor
        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, color);

        attributeDescriptions[2].binding  = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset   = offsetof(Vertex, texCoord);

        return attributeDescriptions;

    };

};



namespace std
{

    template<> struct hash<Vertex>
    {
        size_t operator()(const Vertex& vert) const noexcept
        {
            #define _HASH(_var) hash<decltype(_var)>{}(_var)

            size_t hpos   = _HASH(vert.pos);
            size_t hcolor = _HASH(vert.color);
            size_t htex   = _HASH(vert.texCoord);

            return ((hpos ^ (hcolor << 1)) >> 1) ^ (htex << 1);
        }
    };

};



#endif