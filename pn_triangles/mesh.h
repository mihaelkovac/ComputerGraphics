#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

#include "mvk/utils.h"


template<typename Vertex = glm::vec3,
         typename Index = uint32_t,
         typename Normal = glm::vec3,
         typename TexCoord = glm::vec2,
         typename Material = mvk::util::Void>
struct Mesh
{
	std::vector<Vertex> vertices{};
	std::vector<Index> indices{};
	std::vector<Normal> normals{};
	std::vector<TexCoord> tex_coords{};
	Material material;
};


#endif // MESH_H
