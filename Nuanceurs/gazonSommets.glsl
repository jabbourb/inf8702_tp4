#version 430 core
struct Light
{
        vec3 Ambient; 
        vec3 Diffuse;
        vec3 Specular;
        vec4 Position;  // Si .w = 1.0 -> Direction de lumiere directionelle.
        vec3 SpotDir;
        float SpotExp;
        float SpotCutoff;
        vec3 Attenuation; //Constante, Lineraire, Quadratique
};

struct Mat
{
        vec4 Ambient; 
        vec4 Diffuse;
        vec4 Specular;
        vec4 Exponent;
        float Shininess;
};

layout(location = 0) in vec2 vt;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec3 vp;

uniform int pointLightOn;
uniform int spotLightOn;
uniform int dirLightOn;
uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 MV_N;
uniform mat4 M;
uniform mat4 Light0VP;
uniform mat4 Light1VP;
uniform mat4 Light2VP;

uniform Light Lights[3]; // 0:ponctuelle   1:spot  2:directionnelle
uniform Mat Material;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragLight0Vect;
out vec3 fragLight1Vect;
out vec3 fragLight2Vect;
out vec4 fragLightCoord0;
out vec4 fragLightCoord1;
out vec4 fragLightCoord2;


vec3 fnormal(void)
{
    //Compute the normal 
    vec3 normal = MV_N * vn;
    normal = normalize(normal);
    return normal;
}


void ftexgen()
{
   // pour le mappage correct du gazon
   fragTexCoord = vt;
}


void main (void)
{
    // Eye-coordinate position of vertex, needed in various calculations
    vec3 ecPosition3;
    vec4 ecPosition = MV * vec4(vp,1.0);

    // Do fixed functionality vertex transform
    ecPosition3 = (vec3 (ecPosition)) / ecPosition.w;
    fragLight0Vect = vec3 (Lights[0].Position) - ecPosition3;
    fragLight1Vect = vec3 (Lights[1].Position) - ecPosition3;
    fragLight2Vect = vec3 (-Lights[2].Position);
    fragNormal = fnormal();
    ftexgen(); 
    gl_Position = MVP * vec4(vp,1.0);

	// Envoyer au nuanceur de fragment les coordonées en espace clip/écran
	// du point de vue de chaque lumière:
	fragLightCoord0 = Light0VP * M * vec4(vp,1.0);
	fragLightCoord1 = Light1VP * M * vec4(vp,1.0);
	fragLightCoord2 = Light2VP * M * vec4(vp,1.0);
}
