#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;

} ubo;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outColor;

void main()
{
    gl_PointSize = 1.0;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
    outColor = vec3(1.0, 1.0, 1.0); // bijela

}