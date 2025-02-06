#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 LightIntensity;
uniform struct LightInfo{
    vec4 Position;
    vec3 Ld;
    vec3 La;
    vec3 Ls;
}Light;

uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void getCamSpaceValues(out vec3 norm, out vec3 position){
    norm=normalize(NormalMatrix*VertexNormal);
    position=(ModelViewMatrix*vec4(VertexPosition, 1.0)).xyz;
}

void main()
{
    vec3 n, pos;
    getCamSpaceValues(n, pos);
    vec3 ambient=Light.La*Material.Ka;
    vec3 s=normalize(vec3(Light.Position)-pos);
    float sDotN=max(dot(s,n), 0.0);
    vec3 diffuse=Light.Ld*Material.Kd*sDotN;
    vec3 spec=vec3(0.0);
    if (sDotN>0.0){
        vec3 v=normalize(-pos.xyz);
        vec3 r=reflect(-s,n);
        spec=Light.Ls*Material.Ks*pow(max(dot(r,v),0.0),Material.Shininess);
    }
    LightIntensity=diffuse+ambient+spec;
    gl_Position = MVP*vec4(VertexPosition,1.0);
}
