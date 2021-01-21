#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 frag_normal;
layout(location = 1) in vec4 frag_position;

layout(location = 0) out vec4 out_color;

void main() 
{
	vec4 source_pos = vec4(5.0, 0.0, -5.0, 1.0);
	float k_amb = 0.7;
	vec3 source_color = vec3(1.f, 1.f, 1.f);
	vec3 amb = k_amb * source_color;

	vec3 normal = normalize(frag_normal.xyz);
	//vec4 light_dir = normalize(source_pos - frag_position);
	vec3 light_dir = vec3(normalize(vec3(-4.0, -4.0, 0.0)));
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = diff * source_color;

	vec3 frag_color = vec3(0.3, 0.3, 0.3);
	out_color = vec4(clamp((amb + diffuse) * frag_color, 0.2, 1.0), 1);

}