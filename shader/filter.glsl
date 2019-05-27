#stage compute
#include "version"
#define KERNEL_SIZE 21
layout(local_size_x=16,local_size_y=16) in;

layout(binding=0, r32ui) uniform readonly uimage2D g_MSMSrc;
layout(binding=1, r32ui) uniform writeonly uimage2D g_MSMTgt;

uniform int g_KthPass;

void main()
{
    // float kernel[] = {
    //     0.044695,0.081355,0.124789,
    //     0.161305,0.175713,0.161305,
    //     0.124789,0.081355,0.044695
    // };

    float kernel[] = {
        0.011254,0.016436,0.023066,0.031105,0.040306,
        0.050187,0.060049,0.069041,0.076276,0.080977,
        0.082607,0.080977,0.076276,0.069041,0.060049,
        0.050187,0.040306,0.031105,0.023066,0.016436,0.011254
    };

    ivec2 fragpos = ivec2(gl_GlobalInvocationID.xy);

    vec4 depth = vec4(0,0,0,0);

    if (g_KthPass==0) {
        for (int di=0,i=-KERNEL_SIZE/2; i<=KERNEL_SIZE/2; ++di,++i) {
            ivec2 loc = fragpos + ivec2(0,i);
            vec4 moments;
            for (int i = 0; i < 4; ++i) {
                uint z = imageLoad(g_MSMSrc,ivec2(loc.x*4+i,loc.y)).r;
                moments[i] = uintBitsToFloat(z);
            }
            depth += kernel[di]*moments;
        }
    }
    else if (g_KthPass==1) {
        for (int di=0,i=-KERNEL_SIZE/2; i<=KERNEL_SIZE/2; ++di,++i) {
            ivec2 loc = fragpos + ivec2(i,0);
            vec4 moments;
            for (int i = 0; i < 4; ++i) {
                uint z = imageLoad(g_MSMSrc,ivec2(loc.x*4+i,loc.y)).r;
                moments[i] = uintBitsToFloat(z);
            }
            depth += kernel[di]*moments;
        }
    }

    for (int i = 0; i < 4; ++i) {
        uint b = floatBitsToUint(depth[i]);
        uvec4 bb = uvec4(b,0,0,0);
        imageStore(g_MSMTgt,ivec2(fragpos.x*4+i,fragpos.y), bb);
    }
}
#endstage
