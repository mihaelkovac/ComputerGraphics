#include "textures.h"

#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb/stb_image.h>
#endif


TextureData::~TextureData() noexcept
{
    if(pixels)
    {
        stbi_image_free(pixels);
    }
}

TextureData loadTexture(const char* textureLocation)
{
    TextureData data{};

    data.pixels = stbi_load(textureLocation, &data.width, &data.height, &data.channels, STBI_rgb_alpha);
    data.size = data.width * data.height * 4;
    return data;
}
