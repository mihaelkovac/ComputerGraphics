#version 450

// PN patch data
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

layout (binding = 1) uniform UBO 
{
    mat4 model;
    mat4 projection;
    mat4 view;
    float tess_alpha;
} ubo;

layout(triangles, fractional_odd_spacing, cw) in;

layout(location = 0) in vec3 in_normal[];
layout(location = 3) in PNPatch pn_patch[];

layout(location = 0) out vec4 out_normal;
layout(location = 1) out vec4 out_position;

#define UVW gl_TessCoord

void main()
{
    vec3 UVW_square = UVW * UVW;
    vec3 UVW_cubed  = UVW_square * UVW;

    // extract control points
    vec3 B210 = vec3(pn_patch[0].B210, pn_patch[1].B210, pn_patch[2].B210);
    vec3 B120 = vec3(pn_patch[0].B120, pn_patch[1].B120, pn_patch[2].B120);
    vec3 B021 = vec3(pn_patch[0].B021, pn_patch[1].B021, pn_patch[2].B021);
    vec3 B012 = vec3(pn_patch[0].B012, pn_patch[1].B012, pn_patch[2].B012);
    vec3 B102 = vec3(pn_patch[0].B102, pn_patch[1].B102, pn_patch[2].B102);
    vec3 B201 = vec3(pn_patch[0].B201, pn_patch[1].B201, pn_patch[2].B201);
    vec3 B111 = vec3(pn_patch[0].B111, pn_patch[1].B111, pn_patch[2].B111);

    vec3 N110 = normalize(vec3(pn_patch[0].N110, pn_patch[1].N110, pn_patch[2].N110));
    vec3 N011 = normalize(vec3(pn_patch[0].N011, pn_patch[1].N011, pn_patch[2].N011));
    vec3 N101 = normalize(vec3(pn_patch[0].N101, pn_patch[1].N101, pn_patch[2].N101));

    // compute interpolated pos
    vec3 bar_pos = UVW[2] * gl_in[0].gl_Position.xyz +
                   UVW[0] * gl_in[1].gl_Position.xyz +
                   UVW[1] * gl_in[2].gl_Position.xyz;

    UVW_square *= 3.0;

    // compute PN position
    vec3 pn_pos = gl_in[0].gl_Position.xyz * UVW_cubed[2] +
                  gl_in[1].gl_Position.xyz * UVW_cubed[0] +
                  gl_in[2].gl_Position.xyz * UVW_cubed[1] +
                  B210 * UVW_square[2] * UVW[0]           +
                  B120 * UVW_square[0] * UVW[2]           +
                  B201 * UVW_square[2] * UVW[1]           +
                  B021 * UVW_square[0] * UVW[1]           +
                  B102 * UVW_square[1] * UVW[2]           +
                  B012 * UVW_square[1] * UVW[0]           +
                  B111 * 6.0 * UVW[0] * UVW[1] * UVW[2];

    // Compute normal
    vec3 bar_normal = UVW[2] * in_normal[0] +
                      UVW[0] * in_normal[1] +
                      UVW[1] * in_normal[2];

    vec3 pn_normal  = UVW_square[2] * in_normal[0] +
                      UVW_square[0] * in_normal[1] +
                      UVW_square[1] * in_normal[2] +
                            N110 * UVW[2] * UVW[0] +
                            N011 * UVW[0] * UVW[1] +
                            N101 * UVW[2] * UVW[1];
    

    // Compute final normal and position by interpolation
    vec3 final_normal = (1 - ubo.tess_alpha) * bar_normal + ubo.tess_alpha * pn_normal;
    out_normal = normalize(ubo.model * vec4(final_normal, 0.0));

    vec3 final_pos = (1.0 - ubo.tess_alpha) * bar_pos + ubo.tess_alpha * pn_pos;

    gl_Position  = ubo.projection * ubo.view * ubo.model * vec4(final_pos, 1.0);
    out_position = ubo.model * vec4(final_pos, 1.0);

}