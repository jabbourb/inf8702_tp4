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

uniform sampler2D colorMap;
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;

uniform Light Lights[3];
uniform Mat Material;
uniform int pointLightOn;
uniform int spotLightOn;
uniform int dirLightOn;

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragLight0Vect;
in vec3 fragLight1Vect;
in vec3 fragLight2Vect;
in vec4 fragLightCoord0;
in vec4 fragLightCoord1;
in vec4 fragLightCoord2;


layout(location = 0) out vec3 color;
layout(location = 1) out float depth;

// Accumulateurs pour les facteurs d'éclairage
vec4 Ambient;
vec4 Diffuse;

// Fonction indiquant si le fragment courant est dans l'ombre
// Dans l'ombre : retourner 0.5
// Sinon, retourner 1.0
float Ombrage(in vec4 fragLightCoord, in sampler2D shadowMap)
{
	// Variables utiles:
	vec3 projCoords;
	float closestDepth;
	float currentDepth;
	float shadow  = 1.0;

	// Exclude fragments outside far-clipping plane
	if (fragLightCoord.w < 0)
		return shadow;

	// Comme nous ne passons pas par gl_Position ici, nos coordonées passées en "clip-space" ne sont pas
	// pas converties en coordonées normalisées d'écran.
	// Il faut donc le faire explicitement ici :
	// (https://en.wikipedia.org/wiki/Transformation_matrix#Perspective_projection)
    projCoords = fragLightCoord.xyz / fragLightCoord.w;
   
    // On transforme ces coordonées situées en [-1,1] vers [0,1]:
    projCoords = 0.5 * projCoords + 0.5;

    // On échantillone dans le shadowmap:
    closestDepth = texture(shadowMap, projCoords.xy).z;

    // On récupère la profondeur du fragment courant:
    currentDepth = projCoords.z;

    // On compare les pronfondeurs et modifie shadow en conséquence:
	if (closestDepth < currentDepth)
		shadow = 0.5;

    return shadow;
}  


// Calcul pour une lumière ponctuelle
void pointLight(in vec3 lightVect)
{
   float nDotVP;       // Produit scalaire entre VP et la normale
   float attenuation;  // facteur d'atténuation calculé
   float d;            // distance entre lumière et fragment
   vec3  VP;           // Vecteur lumière

   // Calculer vecteur lumière
   VP = lightVect;

   // Calculer distance à la lumière
   d = length(VP);

   // Normaliser VP
   VP = normalize(VP);

   // Calculer l'atténuation due à la distance
   attenuation = 1.0 / (Lights[0].Attenuation.x +
       Lights[0].Attenuation.y * d +
       Lights[0].Attenuation.z * d * d);

   nDotVP = max(0.0, dot(fragNormal, VP));

   // Calculer les contributions ambiantes et diffuses
   Ambient  += vec4(Lights[0].Ambient, 1.0) * attenuation;
   Diffuse  += vec4(Lights[0].Diffuse, 1.0) * nDotVP * attenuation * Ombrage(fragLightCoord0, shadowMap0);
}


// Calcul pour une lumière directionnelle
void directionalLight(in vec3 lightVect)
{
   float nDotVP;         // Produit scalaire entre VP et la normale

   nDotVP = max(0.0, dot(fragNormal, normalize(vec3 (-Lights[2].Position))));

   // Calculer les contributions ambiantes et diffuses
   Ambient  += vec4(Lights[2].Ambient,1.0);
   Diffuse  += vec4(Lights[2].Diffuse,1.0) * nDotVP * Ombrage(fragLightCoord2, shadowMap2);
}


// Calcul pour une lumière "spot"
void spotLight(in vec3 lightVect)
{
   float nDotVP;            // Produit scalaire entre VP et la normale
   float spotDot;           // Cosinus angle entre VP et direction spot
   float spotAttenuation;   // Facteur d'atténuation du spot
   float attenuation;       // Facteur d'atténuation du à la distance
   float lightToSurfaceAngle;// 
   float d;                 // Distance à la lumière
   vec3  VP;                // Vecteur lumière

   // Calculer le vecteur Lumière
   VP = lightVect;

   // Calculer la distance à al lumière
   d = length(VP);

   // Normaliser VP
   VP = normalize(VP);

   // Calculer l'atténuation due à la distance
   attenuation = 1.0 / (Lights[1].Attenuation.x +
       Lights[1].Attenuation.y * d +
       Lights[1].Attenuation.z * d * d);

   // Le fragment est-il à l'intérieur du cône de lumière ?
   spotDot = dot(-VP, normalize(Lights[1].SpotDir));
   lightToSurfaceAngle = degrees(acos(spotDot));

   if (lightToSurfaceAngle > Lights[1].SpotCutoff)
   {
       spotAttenuation = 0.0; // en dehors... aucune contribution
   }
   else
   {
       spotAttenuation = pow(spotDot, Lights[1].SpotExp);

   }

   // Combine les atténuation du spot et de la distance
   attenuation *= spotAttenuation;

   nDotVP = max(0.0, dot(fragNormal, VP));

   // Calculer les contributions ambiantes et diffuses
   Ambient  += vec4(Lights[1].Ambient, 1.0) * attenuation;
   Diffuse  += vec4(Lights[1].Diffuse, 1.0) * nDotVP * attenuation * Ombrage(fragLightCoord1, shadowMap1);
}


vec4 flight(in vec3 normal)
{
    vec4 color;
    vec3 ecPosition3;

    // Réinitialiser les accumulateurs
    Ambient  = vec4 (0.0);
    Diffuse  = vec4 (0.0);

    if (pointLightOn == 1) {
        pointLight(fragLight0Vect);
    }

    if (dirLightOn == 1) {
        directionalLight(fragLight2Vect);
    }
    
    if (spotLightOn == 1) {
        spotLight(fragLight1Vect);
    }

    color = (Ambient * Material.Ambient + Diffuse  * Material.Diffuse);
    color = clamp( color, 0.0, 1.0 );
    return color;
}



void main (void) 
{
    color = flight(fragNormal).xyz;
	color *= texture(colorMap, fragTexCoord.xy).rgb * 1.30;
	color = clamp(color, 0.0, 1.0 );

	depth = gl_FragCoord.z;
   
}
