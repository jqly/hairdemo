struct HairNode {
    uint depth;
    uint color;
    uint next;
};

float ComputePixelCoverage(vec2 p0, vec2 p1, vec2 pixel_loc, vec2 win_size)
{
    p0 = (p0+1.)*.5;
    p1 = (p1+1.)*.5;
    p0 *= win_size;
    p1 *= win_size;

    float p0dist = length(p0 - pixel_loc);
    float p1dist = length(p1 - pixel_loc);
    float hairWidth = length(p0 - p1);

    // // will be 1.f if pixel outside hair, 0.f if pixel inside hair
    // float outside = max(step(hairWidth, p0dist), step(hairWidth, p1dist));

    // // if outside, set sign to -1, else set sign to 1
    // float sign = outside > 1e-3f ? -1.f : 1.f;

    // // signed distance (positive if inside hair, negative if outside hair)
    // float relDist = sign * clamp(min(p0dist, p1dist),0.,2.);

    // // returns coverage based on the relative distance
    // // 0, if completely outside hair edge
    // // 1, if completely inside hair edge
    // return (relDist + 2.f) * 0.25f;

    float dist = max(p0dist, p1dist);
    if (dist < hairWidth-2.)
        return 1.;
    return 1. - (2.+dist-hairWidth)/2.;
    
}

vec3 HairShading(
    vec3 eye_dir, 
    vec3 light_dir, 
    vec3 tangent)
{

    vec3 hair_base_color = vec3(.44,.32,.23);

    float randn = 1.;
    vec3 randv = vec3(1.,1.,1.)*vec3(randn,randn,randn);

    float Ka = 0.5, Kd = .5, Ks1 = .12, Ex1 = 24, Ks2 = .16, Ex2 = 6.;
    // float Ka = 0., Kd = .4, Ks1 = .4, Ex1 = 80, Ks2 = .5, Ex2 = 8.;

    light_dir = normalize(light_dir);
    eye_dir = normalize(eye_dir);
    tangent = normalize(tangent);// + In.Tangent*randv*4);

    float cosTL = (dot(tangent, light_dir));
    float sinTL = sqrt(1 - cosTL*cosTL);
    float vDiffuse = sinTL;

    float alpha = (randn*10.)*3.1415926/180.;

    float cosTRL = -cosTL;
    float sinTRL = sinTL;
    float cosTE = (dot(tangent, eye_dir));
    float sinTE = sqrt(1.- cosTE*cosTE);

    float cosTRL_r = cosTRL*cos(2.*alpha) - sinTRL*sin(2.*alpha);
    float sinTRL_r = sqrt(1. - cosTRL_r*cosTRL_r);
    float vSpecular_r = max(0., cosTRL_r*cosTE + sinTRL_r*sinTE);

    float cosTRL_trt = cosTRL*cos(-3.*alpha) - sinTRL*sin(-3.*alpha);
    float sinTRL_trt = sqrt(1. - cosTRL_trt*cosTRL_trt);
    float vSpecular_trt = max(0, cosTRL_trt*cosTE + sinTRL_trt*sinTE);
    
    vec3 vColor = Ka * hair_base_color +
                    vec3(1.,1.,1.) * (
                    Kd * vDiffuse* hair_base_color +
                    Ks1 * pow(vSpecular_r, Ex1)  +
                    Ks2 * pow(vSpecular_trt, Ex2) * hair_base_color);
    
    return vColor;
}

uint PackVec4IntoUint(vec4 val)
{
    return (uint(val.x * 255) << 24) | 
            (uint(val.y * 255) << 16) | 
            (uint(val.z * 255) << 8) | 
            uint(val.w * 255);
}

vec4 UnpackUintIntoVec4(uint val)
{
    return vec4(
        float((val & 0xFF000000) >> 24) / 255.0, 
        float((val & 0x00FF0000) >> 16) / 255.0, 
        float((val & 0x0000FF00) >> 8) / 255.0, 
        float((val & 0x000000FF)) / 255.0);
}