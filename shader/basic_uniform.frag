#version 460

in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 WorldPos;

layout (binding = 0) uniform sampler2D RenderTex;
layout (location = 0) out vec4 FragColor;
const vec3 lum=vec3(0.2126, 0.7152, 0.0722);

uniform struct LightInfo {
    vec4 Position;
    vec3 La;
    vec3 L;
} Light;

uniform struct MaterialInfo {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Shininess;
} Material;

uniform struct FogInfo {
    float MaxDist;
    float MinDist;
    vec3 Color;
} Fog;

uniform sampler2D DiffTex;
uniform sampler2D NormalTex;
uniform float EdgeThreshold;
uniform int Pass;

// Calculate TBN matrix using derivatives
mat3 calculateTBN(vec3 N) {
    vec3 Q1 = dFdx(WorldPos);
    vec3 Q2 = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    float det = st1.s * st2.t - st2.s * st1.t;
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(-Q1 * st2.s + Q2 * st1.s);
    
    T = normalize(T - N * dot(N, T));
    B = cross(N, T);
    
    return mat3(T, B, N);
}

vec3 blinnPhong(vec3 position, vec3 normal) {
    vec3 ambient = Light.La * Material.Ka;
    
    vec3 s = normalize(Light.Position.xyz - position);
    float sDotN = max(dot(s, normal), 0.0);
    vec3 diffuse = Material.Kd * sDotN;
    
    vec3 spec = vec3(0.0);
    if(sDotN > 0.0) {
        vec3 v = normalize(-position.xyz);
        vec3 h = normalize(v + s);
        spec = Material.Ks * pow(max(dot(h, normal), 0.0), Material.Shininess);
    }
    
    return ambient + Light.L * (diffuse + spec);
}

float luminance(vec3 color) {
    return dot(lum,color);
}

vec4 pass1(){
    vec3 normalMap = texture(NormalTex, TexCoord).rgb * 2.0 - 1.0;
    mat3 TBN = calculateTBN(normalize(Normal));
    
    vec3 normal = normalize(TBN * normalMap);
    
    vec4 texColor = texture(DiffTex, TexCoord);
    vec3 lightColor = blinnPhong(Position, normal);
    vec3 finalColor = lightColor * texColor.rgb;
    
    float dist = abs(Position.z);
    float fogFactor = (Fog.MaxDist - dist) / (Fog.MaxDist - Fog.MinDist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    finalColor = mix(Fog.Color, finalColor, fogFactor);
    
    FragColor = vec4(finalColor, 1.0);

    return FragColor;
}

vec4 pass2() {
    ivec2 pix = ivec2(gl_FragCoord.xy);
    float s00 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,1)).rgb);
    float s10 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,0)).rgb);
    float s20 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,-1)).rgb);
    float s01 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(0,1)).rgb);
    float s21 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(0,-1)).rgb);
    float s02 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,1)).rgb);
    float s12 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,0)).rgb);
    float s22 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,-1)).rgb);
    
    float sx = s00+2*s10+s20-(s02+2*s12+s22);
    float sy = s00+2*s01+s02-(s20+2*s21+s22);
    float g = sx*sx+sy*sy;
    
    if (g > EdgeThreshold)
        return vec4(1.0);
    else
        return texelFetch(RenderTex, pix, 0);
}

void main() {
    if (Pass == 1) FragColor = pass1();
    if (Pass == 2) FragColor = pass2();
}
