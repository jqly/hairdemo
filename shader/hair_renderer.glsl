#stage vertex

#include "version"
#include "common.glsl"

layout(location=0) in vec3 vs_Position;
layout(location=1) in vec4 vs_Tangent;

out vec3 fs_Position;
out vec4 fs_Tangent;

uniform mat4 g_Model, g_ViewProj;

void main()
{
    PositionToWorldAndClipSpace(
        g_Model, g_ViewProj, vs_Position, fs_Position, gl_Position);
    fs_Tangent.xyz = (g_Model*vec4(vs_Tangent.xyz,0)).xyz;
    fs_Tangent.w = vs_Tangent.w;
}

#endstage

#stage fragment

#include "version"

layout(early_fragment_tests) in;

in vec3 fs_Position;
in vec4 fs_Tangent;
out vec4 out_Color;

void main()
{
    float x = gl_FragCoord.x, y = gl_FragCoord.y;
    float res = 0.;
    for (int i = -25; i < 25; ++i) {
        for (int j = -25; j < 25; ++j) {
            float x_ = x-i;
            float y_ = y-j;
            res += exp((x_*x_+y_*y_)/2.);
        }
    }

    out_Color = vec4((fs_Position*.5+.5).xy,res,1);
}

#endstage