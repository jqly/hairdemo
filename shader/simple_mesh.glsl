#stage vertex
#include "version"

layout (location=0) in vec3 vs_Position;
layout (location=1) in vec3 vs_Normal;
layout (location=2) in vec2 vs_TexCoord;

out vec3 fs_Position;
out vec3 fs_Normal;
out vec2 fs_TexCoord;

uniform mat4 g_Model,g_ViewProj;

void main() 
{
    vec4 position = g_Model * vec4(vs_Position,1);
    fs_Position = position.xyz;
    fs_Normal = mat3(transpose(inverse(g_Model))) * vs_Normal;

    gl_Position = g_ViewProj*position;
    fs_TexCoord = vs_TexCoord;
}

#endstage

#stage fragment
#include "version"

in vec3 fs_Position;
in vec3 fs_Normal;
in vec2 fs_TexCoord;

out vec4 FragColor;


layout(binding=0) uniform sampler2D g_ShadowMap;
layout(binding=1) uniform sampler2D g_DiffuseMap;

uniform vec3 g_Eye;
uniform vec3 g_SunLightDir;

void main()
{
    vec3 sun_light_dir = normalize(g_SunLightDir);
    vec3 view_dir = normalize(g_Eye - fs_Position);

    vec3 normal = normalize(fs_Normal);

    float cosNL = dot(normal,sun_light_dir);
    float diffuse = max(cosNL,0.);
    vec3 diffuse_color = diffuse*texture(g_DiffuseMap,fs_TexCoord).rgb;

    FragColor = vec4(diffuse_color,1);
}

#endstage