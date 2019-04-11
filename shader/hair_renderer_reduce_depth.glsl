#stage vertex

#include "version"
#include "common.glsl"

layout(location=0) in vec3 vs_Position;

uniform mat4 g_Model, g_ViewProj;

void main()
{
    PositionToClipSpace(
        g_Model, g_ViewProj, vs_Position, gl_Position);
}

#endstage

#stage fragment

#include "version"

void main()
{
    gl_FragDepth = gl_FragCoord.z+.00001;
}

#endstage