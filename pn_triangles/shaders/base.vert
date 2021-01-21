#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 proj;
    mat4 view;

} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec4 out_normal;
layout(location = 1) out vec4 out_position;


void main()
{
    
     gl_Position  = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0);
     out_position = ubo.model * vec4(in_position, 1.0);
     out_normal = normalize(ubo.model * vec4(in_normal, 0.0));
}