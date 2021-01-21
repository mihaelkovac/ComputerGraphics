#version 450

struct PNPatch
{
    float B210;
    float B120;
    float B021;
    float B012;
    float B102;
    float B201;
    float B111;
    float N110;
    float N011;
    float N101;
};

layout (binding = 0) uniform UBO 
{
	float tess_level;
} ubo; 

layout(vertices = 3) out;

layout(location = 0) in vec3 in_normal[];

layout(location = 0) out vec3 out_normal[3];
layout(location = 3) out PNPatch out_patch[3];

float w(int i, int j)
{
	vec3 Pj = gl_in[j].gl_Position.xyz;
	vec3 Pi = gl_in[i].gl_Position.xyz;
	
	vec3 Ni = in_normal[i];

	return dot(Pj - Pi, Ni);
}

float v(int i, int j)
{
	vec3 Pj = gl_in[j].gl_Position.xyz;
	vec3 Pi = gl_in[i].gl_Position.xyz;

	vec3 Ni = in_normal[i];
	vec3 Nj = in_normal[j];

	vec3 Pj_minus_Pi = Pj - Pi;

	vec3 Ni_plus_Nj  = Ni + Nj;

	return 2.0 * dot(Pj_minus_Pi, Ni_plus_Nj) / dot(Pj_minus_Pi, Pj_minus_Pi);
}

void CalculatePoints()
{
	float P0 = gl_in[0].gl_Position[gl_InvocationID];
	float P1 = gl_in[1].gl_Position[gl_InvocationID];
	float P2 = gl_in[2].gl_Position[gl_InvocationID];

	out_patch[gl_InvocationID].B210 = (2.0 * P0 + P1 - w(0, 1) * N0) / 3.0;
	out_patch[gl_InvocationID].B120 = (2.0 * P1 + P0 - w(1, 0) * N1) / 3.0;
	out_patch[gl_InvocationID].B021 = (2.0 * P1 + P2 - w(1, 2) * N1) / 3.0;
	out_patch[gl_InvocationID].B012 = (2.0 * P2 + P1 - w(2, 1) * N2) / 3.0;
	out_patch[gl_InvocationID].B102 = (2.0 * P2 + P0 - w(2, 0) * N2) / 3.0;
	out_patch[gl_InvocationID].B201 = (2.0 * P0 + P2 - w(0, 2) * N0) / 3.0;

	float E = (out_patch[gl_InvocationID].B210 +
			   out_patch[gl_InvocationID].B120 +
			   out_patch[gl_InvocationID].B021 +
			   out_patch[gl_InvocationID].B012 +
			   out_patch[gl_InvocationID].B102 +
			   out_patch[gl_InvocationID].B201) / 6.0;


	float V = (P0 + P1 + P2) / 3.0;

	out_patch[gl_InvocationID].B111 = E + (E - V) / 2.0;
}

void CalculateNormals()
{
	
	float N0 = in_normal[0][gl_InvocationID];
	float N1 = in_normal[1][gl_InvocationID];
	float N2 = in_normal[2][gl_InvocationID];
	
	// Later normalized, calculate directly for now
	out_patch[gl_InvocationID].N110 = N0 + N1 - v(0, 1) * (P1 - P0);
	out_patch[gl_InvocationID].N011 = N1 + N2 - v(1, 2) * (P2 - P1);
	out_patch[gl_InvocationID].N101 = N2 + N0 - v(2, 0) * (P0 - P2);
	
}

void main()
{

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	out_normal[gl_InvocationID]         = in_normal[gl_InvocationID];
	
	CalculatePoints();

	CalculateNormals();

	// set tess levels
	gl_TessLevelOuter[gl_InvocationID] = ubo.tess_level;
	gl_TessLevelInner[0] = ubo.tess_level;
}