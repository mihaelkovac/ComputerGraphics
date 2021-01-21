#ifndef TEXTURES_H
#define TEXTURES_H

#include <vulkan/vulkan.h>




struct TextureData
{
    uint8_t* pixels;
    int width;
    int height;
    int channels;
    VkDeviceSize size;

    ~TextureData() noexcept;
};

TextureData loadTexture(const char* textureLocation);

#endif