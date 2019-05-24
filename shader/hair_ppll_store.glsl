#stage vertex
#include "version"
#include "common.glsl"

layout(location=0) in vec3 vs_Position;
layout(location=1) in vec4 vs_Tangent;

out vec3 gs_Position;
out vec4 gs_Tangent;

uniform mat4 g_Model, g_ViewProj;

void main()
{
    PositionToWorldAndClipSpace(
        g_Model, g_ViewProj, vs_Position, gs_Position, gl_Position);
    gs_Tangent.xyz = (g_Model*vec4(vs_Tangent.xyz,0)).xyz;
    gs_Tangent.w = vs_Tangent.w;
}

#endstage

#stage geometry
#include "version"

layout(lines) in;
layout(triangle_strip, max_vertices=4) out;

in vec3 gs_Position[];
in vec4 gs_Tangent[];

out vec3 fs_Position;
out vec4 fs_Tangent;
out vec4 fs_WinE0E1;

uniform float g_HairRadius;
uniform vec3 g_Eye;
uniform mat4 g_ViewProj;
uniform vec2 g_WinSize;

void main()
{
    vec3 p0 = gs_Position[0], p1 = gs_Position[1];
    vec4 t0 = gs_Tangent[0], t1 = gs_Tangent[1];

    float ratio0 = fract(t0.w), ratio1 = fract(t1.w);

    float radius_scale = 1./max(g_WinSize.x,g_WinSize.y);

    float radius0 = g_HairRadius * ratio0 * radius_scale;
    float radius1 = g_HairRadius * ratio1 * radius_scale;

    vec3 nt0 = normalize(t0.xyz);
    vec3 nt1 = normalize(t1.xyz);

    vec3 viewDir0 = normalize(p0 - g_Eye);
    vec3 right0 = normalize(cross(nt0, viewDir0));
    vec2 proj_right0 = normalize((g_ViewProj*vec4(right0, 0)).xy);
    
    vec3 viewDir1 = normalize(p1 - g_Eye);
    vec3 right1 = normalize(cross(nt1, viewDir1));
    vec2 proj_right1 = normalize((g_ViewProj*vec4(right1, 0)).xy);

    float expandPixels = 0.71; //sqrt(2)/2

    vec3 tmp1;
    vec4 tmp2;

    tmp1 = p0 - right0 * radius0;
    tmp2 = g_ViewProj*vec4(tmp1,1);
    vec4 e0_root = vec4(tmp2.xyz/tmp2.w,1) - vec4(proj_right0*expandPixels/g_WinSize.y,0,0);

    tmp1 = p1 - right1*radius1;
    tmp2 = g_ViewProj*vec4(tmp1,1);
    vec4 e0_tip = vec4(tmp2.xyz/tmp2.w,1) - vec4(proj_right1*expandPixels/g_WinSize.y,0,0);

    tmp1 = p0 + right0*radius0;
    tmp2 = g_ViewProj*vec4(tmp1,1);
    vec4 e1_root = vec4(tmp2.xyz/tmp2.w,1) + vec4(proj_right0*expandPixels/g_WinSize.y,0,0);

    tmp1 = p1 + right1*radius1;
    tmp2 = g_ViewProj*vec4(tmp1,1);
    vec4 e1_tip = vec4(tmp2.xyz/tmp2.w,1) + vec4(proj_right1*expandPixels/g_WinSize.y,0,0);

    // Fixed: Quad may be rendered as a butterfly-shape.
    if (dot(proj_right0,proj_right1)<0.) {
        vec4 tmp = e1_tip;
        e1_tip = e0_tip;
        e0_tip = tmp;
    }

    ////
    // Emit verts.
    ////

    fs_Position = p0 - right0 * radius0;
    fs_Tangent = t0;
    fs_WinE0E1 = vec4(e0_root.xy,e1_root.xy);
    gl_Position = e0_root;
    EmitVertex();

    fs_Position = p1 - right1*radius1;
    fs_Tangent = t1;
    fs_WinE0E1 = vec4(e0_tip.xy,e1_tip.xy);
    gl_Position = e0_tip;
    EmitVertex();

    fs_Position = p0 + right0 * radius0;
    fs_Tangent  = t0;
    fs_WinE0E1 = vec4(e0_root.xy,e1_root.xy);
    gl_Position = e1_root;
    EmitVertex();

    fs_Position = p1 + right1*radius1;
    fs_Tangent = t1;
    fs_WinE0E1 = vec4(e0_tip.xy,e1_tip.xy);
    gl_Position = e1_tip;
    EmitVertex();
}

#endstage

#stage fragment
#include "version"
#include "hair_common.glsl"

layout(early_fragment_tests) in;

in vec3 fs_Position;
in vec4 fs_Tangent;
in vec4 fs_WinE0E1;

uniform vec2 g_WinSize,g_ShadowWinSize;
uniform int g_MaxHairNodes;
uniform vec3 g_Eye, g_PointLightPos;
uniform float g_HairTransparency, g_HairShadowTransparency;

layout(binding=0,offset=0) uniform atomic_uint g_Counter;
layout(binding=0,r32ui) coherent uniform highp uimage2D g_PPLLHeads;
layout(std430,binding=0) coherent buffer PPLLLayout { 
    HairNode g_HairNodes[]; 
};

layout(binding=1,r32ui) uniform readonly highp uimage2D g_ShadowDepthCache;

uniform mat4 g_LightViewProj;

float ComputeLitness(mat4 light_vp, vec3 position);

void main()
{
    uint hair_node_idx = atomicCounterIncrement(g_Counter) + 1u;
    if (int(hair_node_idx) >= g_MaxHairNodes)
        return;

    ivec2 win_addr = ivec2(gl_FragCoord.xy);

    uint prev_hair_node_idx = imageAtomicExchange(g_PPLLHeads, win_addr, hair_node_idx);
    // return;
    float coverage = ComputePixelCoverage(fs_WinE0E1.xy,fs_WinE0E1.zw,gl_FragCoord.xy,g_WinSize);
    coverage *= fract(fs_Tangent.w);
    float litness = ComputeLitness(g_LightViewProj, fs_Position);
    vec3 hair_color = HairShading(
        g_Eye - fs_Position,
        g_PointLightPos-fs_Position,
        fs_Tangent.xyz);

    g_HairNodes[hair_node_idx] = HairNode(
        floatBitsToUint(gl_FragCoord.z),
        PackVec4IntoUint(vec4(hair_color*litness, coverage*(1.-g_HairTransparency))),
        prev_hair_node_idx, 0u);
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float ComputeLitness(mat4 light_vp, vec3 position)
{
    vec4 tmp = light_vp * vec4(position,1.);
    vec3 pos = tmp.xyz / tmp.w;
    pos = .5*pos+.5;
    ivec2 idx = ivec2((pos.xy+vec2(rand(position.xy),rand(position.yz))*.00)*g_ShadowWinSize);
    float total_litness = 1.;
    uint z0 = floatBitsToUint(pos.z);
    for (int i = -9; i <= 9; ++i) {
        for (int j = -9; j <= 9; ++j) {
            ivec2 loc = ivec2(idx.x+i,idx.y+j);
            float litness = 1.;
            for (int ii = 0; ii < 4; ++ii) {
                uint z = imageLoad(g_ShadowDepthCache,ivec2(loc.x*4+ii,loc.y)).r;
                if (z < z0)
                    litness *= g_HairShadowTransparency;
                else
                    break;
            }
            total_litness += litness;
        }
    }

    return total_litness/(19.*19.);
}

#endstage
