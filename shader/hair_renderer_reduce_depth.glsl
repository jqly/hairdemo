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

layout(binding=0,r32ui) uniform uimage2D g_DepthFirst3;

void main()
{
    // TODO: test it.
    uint zcandidate = floatBitsToUint(gl_FragCoord.z);
    ivec2 fcoord = ivec2(gl_FragCoord.xy);

    for (int i = 0; i < 3; ++i) {
        uint zold = imageAtomicMin(g_DepthFirst3, ivec2(fcoord.x*3+i,fcoord.y),zcandidate);
        if (zold == floatBitsToUint(1.) || zold == zcandidate)
            break;
        zcandidate = max(zold,zcandidate);
    }
    gl_FragDepth = uintBitsToFloat(zcandidate+1);

}

#endstage
