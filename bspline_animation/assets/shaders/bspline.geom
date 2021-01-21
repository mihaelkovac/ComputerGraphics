#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_VERTICES 64

// layout(constant_id = 1) const int MAX_VERTICES = 32;
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;

} ubo;

layout(lines_adjacency) in;

layout(line_strip, max_vertices = MAX_VERTICES) out;

// const mat4 B = mat4(vec4(-1/6,  1/2, -1/2,  1/6),
//                     vec4( 1/2,   -1,  1/2,    0),
//                     vec4(-1/2,    0,  1/2,    0),
//                     vec4( 1/6,  2/3,  1/6,    0));

const mat4 B = mat4(vec4(-1/6,  1/2, -1/2,  1/6),
                    vec4( 1/2,   -1,    0,  2/3),
                    vec4(-1/2,  1/2,  1/2,  1/6),
                    vec4( 1/6,    0,    0,    0));

layout(location = 0) in vec3 inColor[4];

layout(location = 0) out vec3 oColor;


void main()
{

    mat4 R = transpose(mat4(gl_in[0].gl_Position.xyzw,
                            gl_in[1].gl_Position.xyzw,
                            gl_in[2].gl_Position.xyzw,
                            gl_in[3].gl_Position.xyzw));

    
    for(int i = 0; i < MAX_VERTICES; ++i)
    {
        float t = i / (MAX_VERTICES - 1);
        vec4 T = vec4(t*t*t, t*t, t, 1);
        gl_Position = ubo.proj * (T * B * R);

        oColor = vec3(1, 1, 1);
        EmitVertex();
    }

    EndPrimitive();
    
}