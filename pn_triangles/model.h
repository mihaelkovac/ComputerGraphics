#ifndef MODEL_H
#define MODEL_H

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <vector>
#include <string>

using Attributes = tinyobj::attrib_t;
using Shape = tinyobj::shape_t;
using Material = tinyobj::material_t;

struct Model
{

    Attributes attributes;
    std::vector<Shape> shapes;
    std::vector<Material> materials;

	static Model Load(const char* path) noexcept
	{
        std::string err;
        std::string warn;

        Model model{};

        if (!tinyobj::LoadObj(&model.attributes,
            &model.shapes,
            &model.materials,
            &warn,
            &err,
            path))
        {
            fprintf(stderr, "Object loading - WARN  - %s", warn.c_str());
            fprintf(stderr, "Object loading - ERROR - %s", err.c_str());
            abort();
        }

        return model;
	}

};



#endif // MODEL_H
