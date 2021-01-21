#include "app.h"

#include <cstdio>
#include <vulkan/vulkan.hpp>

// const int MAX_FRAMES_IN_FLIGHT = 2;

// const std::vector<const char*> validationLayers = {
//     "VK_LAYER_KHRONOS_validation"
// };

// const std::vector<const char*> deviceExtensions = {
//     VK_KHR_SWAPCHAIN_EXTENSION_NAME
// };

// #ifdef NDEBUG
// constexpr bool enableValidationLayers = false;
// #else
// constexpr bool enableValidationLayers = true;
// #endif



int main() {
    

    App app(800, 600);

    app.run();

    return 0;
}