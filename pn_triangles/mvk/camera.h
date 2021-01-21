#ifndef MVK_CAMERA_H
#define MVK_CAMERA_H

#include <glm/vec3.hpp>

namespace mvk
{

    struct Camera
    {
        glm::vec3 position{2.f, 2.f, 2.f};
        glm::vec3 center{0.f, 0.f, 0.f};
    };
    
} // namespace mvk


#endif