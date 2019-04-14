#stage vertex

#include "version"

layout (location = 0) in vec3 vs_Position;

void main() {
    gl_Position = vec4(vs_Position, 1.f);
}

#endstage

#stage fragment

#include "version"
#include "hair_common.glsl"

out vec4 out_HairColor;

layout(binding=0,r8) uniform image2D g_Transparency;

layout(binding=1,r32ui) uniform uimage2D g_PPLLHeads;
layout(binding=0,std430) buffer PPLLLayout { 
    HairNode g_HairNodes[]; 
};

#define KBufSize 8

void main()
{
    uint node_idx = imageLoad(g_PPLLHeads,ivec2(gl_FragCoord.xy)).r;
    if (node_idx == 0)
        discard;

    HairNode kbuf[KBufSize];

    int kbuf_ed = 0;
    for (; node_idx!=0; node_idx = g_HairNodes[node_idx].next) {
        HairNode node = g_HairNodes[node_idx];

        if (kbuf_ed < KBufSize)
            kbuf[kbuf_ed++] = node;
        else if (node.depth < kbuf[KBufSize-1].depth)
            kbuf[KBufSize-1] = node;
        else
            continue;

        int tail = kbuf_ed-1;
        while (tail>=1 && kbuf[tail].depth < kbuf[tail-1].depth) {
            HairNode tmp = kbuf[tail];
            kbuf[tail] = kbuf[tail-1];
            kbuf[tail-1] = tmp;
            tail--;
        }
    }

    float rem = 1.f;
    vec3 color = vec3(0,0,0);
    for (int i = 0; i < kbuf_ed; ++i) {
        vec4 c = UnpackUintIntoVec4(kbuf[i].color);
        color += c.rgb*rem*c.a;
        rem *= (1.-c.a);
    }
    
    float alpha = 1. - imageLoad(g_Transparency,ivec2(gl_FragCoord.xy)).r;
    out_HairColor = vec4(color,alpha);
}

#endstage
