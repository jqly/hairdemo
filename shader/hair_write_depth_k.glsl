#stage vertex
#include "version"

layout (location = 0) in vec3 vs_Position;

void main() {
    gl_Position = vec4(vs_Position, 1.f);
}

#endstage

#stage fragment
#include "version"

out vec4 out_HairColor;

layout(binding=0,r32ui) uniform readonly highp uimage2D g_DepthCache;


void main()
{
    ivec2 pos = ivec2(gl_FragCoord.xy);

    uint z1 = imageLoad(g_DepthCache,ivec2(pos.x*2+0,pos.y)).r;
    uint z2 = imageLoad(g_DepthCache,ivec2(pos.x*2+1,pos.y)).r;
    gl_FragDepth = uintBitsToFloat(z2+1u);
    // if (z2 == floatBitsToUint(1.))
    //     gl_FragDepth = 1.;
    // else
    //     gl_FragDepth = uintBitsToFloat(z1+1u)+2.*abs(uintBitsToFloat(z2)-uintBitsToFloat(z1));
}

#endstage
