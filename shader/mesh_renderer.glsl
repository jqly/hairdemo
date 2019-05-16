#stage vertex
#include "version"
#include "common.glsl"

layout(location=0) in vec3 vs_Position;
layout(location=1) in vec3 vs_Normal;
layout(location=2) in vec2 vs_Texcoord;

out vec3 fs_Position;
out vec4 fs_Normal;
out vec2 fs_Texcoord;

uniform mat4 g_Model, g_ViewProj;

void main()
{
    fs_Position = dot(g_Model, vec4(vs_Position, 1.)).xyz;
    gl_Position = dot(g_ViewProj, vec4(fs_Position, 1.));
    fs_Normal = dot(mat3(transpose(inverse(g_Model))), vs_Normal);
    fs_Texcoord = vs_Texcoord;
}

#endstage

#stage fragment
#include "version"

in vec3 fs_Position;
in vec4 fs_Normal;
in vec2 fs_Texcoord;

out vec4 out_Color;

uniform vec3 g_Eye, g_PointLightPos;
uniform vec2 g_WinSize;

layout(binding=1) uniform sampler2D g_DiffuseMap;

void main()
{
    vec3 normal = normalize(fs_Normal);
    vec3 light_direction = normalize(g_PointLightPos-fs_Position);

    vec3 diffuseAlbedo = texture(g_DiffuseMap, fs_Texcoord).rgb;

    float nDotL = clamp(dot(normal, light_direction), 0., 1.);
    ivec2 screen_pos = ivec2(gl_FragCoord.xy);

	vec3 lighting = vec3(0.,0.,0.);

    lighting += nDotL * diffuseAlbedo;
    lighting += vec3(0.2f, 0.5f, 1.0f) * 0.1f * diffuseAlbedo;
    out_Color = vec4(max(lighting, 0.0001f), 1.0f);
}

#endstage
