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

layout(binding=0,r32ui) readonly uniform highp uimage2D g_HairAlpha;
layout(binding=1,r32ui) readonly uniform highp uimage2D g_PPLLHeads;
layout(std430,binding=0) coherent buffer PPLLLayout { 
    HairNode g_HairNodes[]; 
};

#define KBufSize 8

void main()
{
    uint node_idx = imageLoad(g_PPLLHeads,ivec2(gl_FragCoord.xy)).r;
    if (node_idx == 0u)
        discard;
    // uint node_idx_next = g_HairNodes[node_idx].next;
    // if (node_idx_next == 0u)
    //     out_HairColor = vec4(0,0,1,1);
    // else if (node_idx_next > 1000u)
    //     out_HairColor = vec4(1,0,0,1);
    // else
    //     out_HairColor = vec4(0, 1, 0, 1);
    // return;
    HairNode kbuf[KBufSize];

    int kbuf_ed = 0;
    int node_cnt = 0;
    for (; node_idx!=0u; node_idx = g_HairNodes[node_idx].next) {
        node_cnt++;
        // if (node_cnt == 8)
        //     break;
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

    float rem = 1.f, total_alpha = 0.;
    vec3 color = vec3(0,0,0);
    for (int i = 0; i < kbuf_ed; ++i) {
        vec4 c = UnpackUintIntoVec4(kbuf[i].color);
        color += c.rgb*rem*c.a;
        total_alpha += rem*c.a;
        rem *= (1.-c.a);
    }
    
    uint alpha = imageLoad(g_HairAlpha,ivec2(gl_FragCoord.xy)).r;
    float alphaness = float(alpha)/255.;
    out_HairColor = vec4(color/total_alpha,alphaness);
    // out_HairColor = vec4(float(node_cnt)/16.,0,0,1);
    // out_HairColor = vec4(alphaness,0,0,1);
}

#endstage
