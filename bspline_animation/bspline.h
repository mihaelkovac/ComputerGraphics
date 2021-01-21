#ifndef BSLPINE_H
#define BSLPINE_H

#include <vector>
#include <array>
#include <tuple>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>


struct BSplineVertex
{
    glm::vec3 pos;

    static constexpr VkVertexInputBindingDescription bindingDescription() noexcept
    {
        VkVertexInputBindingDescription inputBinding{};
        inputBinding.binding = 0;
        inputBinding.stride = sizeof(BSplineVertex);
        inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return inputBinding;
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions() noexcept
    {
        VkVertexInputAttributeDescription positionDescription{};
        positionDescription.binding = 0;
        positionDescription.format  = VK_FORMAT_R32G32B32_SFLOAT;
        positionDescription.location = 0;
        positionDescription.offset = offsetof(BSplineVertex, pos);

        return {positionDescription};
    }
};

struct BSpline
{
    BSpline(float tStep = 0.0005f) noexcept : tStep_(tStep) {}

    static constexpr glm::mat4 B =
    {
        glm::vec4(-1/6.f,  3/6.f, -3/6.f,  1/6.f),
        glm::vec4( 3/6.f, -6/6.f,  3/6.f,  0/6.f),
        glm::vec4(-3/6.f,  0/6.f,  3/6.f,  0/6.f),
        glm::vec4( 1/6.f,  4/6.f,  1/6.f,  0/6.f)
    };

    static constexpr glm::mat3x4 d_B =
    {
        glm::vec4(-1/2.f,  3/2.f, -3/2.f,  1/2.f),
        glm::vec4( 2/2.f, -4/2.f,  2/2.f,  0/2.f),
        glm::vec4(-1/2.f,  0/2.f,  1/2.f,  0/2.f)
    };

    static constexpr glm::mat2x4 dd_B =
    {
        glm::vec4(-1,  3, -3,  1),
        glm::vec4( 1, -2,  1,  0)
    };

    glm::vec3 point(float t, int i) const noexcept;
            
    glm::vec3 tangent(float t, int i) const noexcept;

    glm::vec3 bitangent(float t, int i) const noexcept;

    static BSpline load(const char* pathFile, float tStep);

    static void load(BSpline& obj, const char* pathFile);

    struct Animation
    {
        constexpr Animation(const BSpline* spline = nullptr) noexcept : spline_(spline) {}

        using AnimationTuple = std::tuple<const glm::vec3&, const glm::vec3&, const glm::vec3&>;

        AnimationTuple update() noexcept;

        void setSpline(const BSpline* spline) noexcept;

        float t = 0.f;
        int segment = 1;
        const BSpline* spline_;

    private:
        glm::vec3 pos{0.f, 0.f, 0.f};
        glm::vec3 tang{0.f, 0.f, 0.f};
        glm::vec3 bitang{0.f, 0.f, 0.f};
    };

    Animation animate() const noexcept;

    std::vector<glm::vec3> path;
    std::vector<BSplineVertex> points;
    float tStep_;
};



#endif