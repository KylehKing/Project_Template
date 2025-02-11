#version 460

in vec3 Position;
in vec3 Normal;

layout (location = 0) out vec4 FragColor;


uniform struct SpotLightInfo{
    vec3 Position;
    vec3 La;
    vec3 L;
    vec3 Direction;
    float Exponent;
    float Cutoff;
}Spot;

uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;

const int levels=5;
const float scaleFactor=1.0/levels;

vec3 blinnphongSpot (vec3 position, vec3 n){
    vec3 diffuse=vec3(0.0), spec=vec3(0.0);
    vec3 ambient=Spot.La*Material.Ka;

    vec3 s=normalize(Spot.Position-position);

    float cosAng=dot(-s, normalize(Spot.Direction));
    float angle=acos(cosAng);
    float spotScale;

    if (angle>=0.0 && angle<Spot.Cutoff){
        spotScale=pow(cosAng, Spot.Exponent);
        float sDotN=max(dot(s,n), 0.0);
        diffuse=Material.Kd*floor(sDotN*levels)*scaleFactor;
        if (sDotN>0.0){
            vec3 v=normalize(-position.xyz);
            vec3 h=normalize(v+s);
            spec=Material.Ks*pow(max(dot(h,n),0.0),Material.Shininess);
        }
    }

    return ambient+spotScale*(diffuse+spec)*Spot.L;
}

void main() {
    FragColor = vec4(blinnphongSpot(Position, normalize(Normal)), 1.0);
}
