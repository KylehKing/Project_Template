#version 460

in vec3 LightDir;
in vec3 ViewDir;
in vec2 TexCoord;

layout (binding=0) uniform sampler2D ColorTex;
layout (binding=1) uniform sampler2D NormalMapTex;

layout (location = 0) out vec4 FragColor;


uniform struct LightInfo{
    vec4 Position;
    vec3 La;
    vec3 L;
}Light;

uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;


vec3 blinnPhong (vec3 n){
    vec3 diffuse=vec3(0.0), spec=vec3(0.0);

    vec3 texColor = texture(ColorTex, TexCoord).rgb;

    vec3 ambient=Light.La*Material.Ka;
    vec3 s=normalize(LightDir);
    float sDotN=max(dot(s,n), 0.0);
    diffuse=texColor*sDotN;
    if (sDotN>0.0){
            vec3 v=normalize(ViewDir);
            vec3 h=normalize(v+s);
            spec=Material.Ks*pow(max(dot(h,n),0.0),Material.Shininess);
    }
    return ambient+(diffuse+spec)*Light.L;
}

void main() {
    vec3 norm=texture(NormalMapTex, TexCoord).xyz;
    norm.xy= 2.0*norm.xy-1.0;
    FragColor= vec4(blinnPhong(normalize(norm)), 1.0);
}
