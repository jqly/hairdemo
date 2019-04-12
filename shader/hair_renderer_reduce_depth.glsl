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
    uint zcandidate = floatBitsToUint(gl_FragCoord.z);
    ivec2 fcoord = ivec2(gl_FragCoord.xy);
    uint zcandidate_ = imageAtomicMin(g_DepthFirst3, ivec2(fcoord.x*3+0,fcoord.y),zcandidate);
    zcandidate_ = imageAtomicMin(g_DepthFirst3, ivec2(fcoord.x*3+1,fcoord.y),zcandidate_);
    zcandidate_ = imageAtomicMin(g_DepthFirst3, ivec2(fcoord.x*3+2,fcoord.y),zcandidate_);
    gl_FragDepth = uintBitsToFloat(max(zcandidate_,zcandidate)+1);
}

#endstage