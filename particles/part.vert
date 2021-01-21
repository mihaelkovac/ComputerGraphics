
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 vel;
layout(location = 3) in vec3 inColorChange;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main()
{
	gl_PointSize = 3.0;
	fragColor = vec3(inColor);
	gl_Position = vec4(inPos.xy, 1.0, 1.0);

}
