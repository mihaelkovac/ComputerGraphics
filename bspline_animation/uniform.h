#ifndef UNIFORM_H
#define UNIFORM_H

// #define GLM_FORCE_DEAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

struct UniformBuffObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

};


#endif