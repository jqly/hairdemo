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
    if (dot(proj_right0,proj_right1)<0) {
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

in vec3 fs_Position;
in vec4 fs_Tangent;
in vec4 fs_WinE0E1;

uniform vec2 g_WinSize;
uniform float g_HairTransparency;

layout(binding=0,r32ui) uniform uimage2D g_DepthCache;
layout(binding=1,r32ui) uniform uimage2D g_HairAlpha;

void main()
{
    float coverage = ComputePixelCoverage(fs_WinE0E1.xy,fs_WinE0E1.zw,gl_FragCoord.xy,g_WinSize);
    coverage *= fract(fs_Tangent.w);
    float hair_alpha = coverage*(1.-g_HairTransparency);

    ivec2 fcoord = ivec2(gl_FragCoord.xy);

    imageAtomicAdd(g_HairAlpha, fcoord, uint(hair_alpha*255));

    // gl_FragDepth = 1.;
    // return;

    uint zcandidate = floatBitsToUint(gl_FragCoord.z);
    uint zold = imageAtomicMin(g_DepthCache, ivec2(fcoord.x*2+0,fcoord.y),zcandidate);
    uint z1 = min(zcandidate,zold);
    zcandidate = max(zcandidate,zold);

    zold = imageAtomicMin(g_DepthCache, ivec2(fcoord.x*2+1,fcoord.y),zcandidate);
    uint z2 = min(zcandidate,zold);
    zcandidate = max(zcandidate,zold);

    if (z2 == floatBitsToUint(1.))
        gl_FragDepth = 1.;
    else
        gl_FragDepth = uintBitsToFloat(z1)+8.*abs(uintBitsToFloat(z2)-uintBitsToFloat(z1));
    
}

#endstage
