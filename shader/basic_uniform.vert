#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in vec4 VertexTangent;


uniform struct LightInfo{
    vec4 Position;
    vec3 La;
    vec3 L;
}Light;

out vec3 LightDir;
out vec3 ViewDir;
out vec2 TexCoord;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main()
{
    vec3 normal=normalize(NormalMatrix*VertexNormal);
    vec3 tangent=normalize(NormalMatrix*vec3(VertexTangent));
    vec3 binormal=normalize(cross(normal, tangent)*VertexTangent.w);
    vec3 Position = (ModelViewMatrix*vec4(VertexPosition, 1.0)).xyz;

    mat3 toObjetLocal=mat3(
        tangent.x, binormal.x, normal.x,
        tangent.y, binormal.y, normal.y,
        tangent.z, binormal.z, normal.z
    );

    LightDir=toObjetLocal*Light.Position.xyz-Position;
    ViewDir=toObjetLocal*normalize(-Position);
    TexCoord = VertexTexCoord;
        
    gl_Position = MVP*vec4(VertexPosition,1.0);
}
