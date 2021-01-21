#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>

#define DEBUG 1

namespace constants
{

#if DEBUG
    constexpr bool enableValidationLayers = true;
#else
    constexpr bool enableValidationLayers = false;
#endif
    
    inline const std::vector<const char*> enabledLayerNames = {"VK_LAYER_KHRONOS_validation"};
 

    const char* ENGINE_NAME = "vkAsh 0.0";
    const char* APP_NAME = "Probna aplikacija";
}




#endif