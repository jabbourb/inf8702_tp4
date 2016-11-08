#version 430 core
#extension GL_NV_shadow_samplers_cube : enable

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


in vec4 fragColor;
in vec3 Position_cameraSpace;
in vec3 Normal_cameraSpace;

// Ouput data
layout(location = 0) out vec3 color;
layout(location = 1) out float depth;

//Uniformes et ins pour IBL:
uniform samplerCube diffMap;
uniform samplerCube specMap;
uniform mat4 modelMatrix;		// matrice de modélisation courante (M).
uniform vec3 EyePos_worldSpace;	// position de l'oeil en coord. universelles
uniform int IBLon;
in vec4 Position_objectSpace;   // position du sommet/fragment en coord. d'objet
in vec3 Normal_objectSpace;	    // normale du sommet/fragment en coord. d'objet


uniform int pointLightOn;
uniform int dirLightOn;
uniform int spotLightOn;
uniform Light Lights[3];
uniform Mat Material;

// Accumulateurs pour les facteurs d'éclairage
vec4 Ambient;
vec4 Diffuse;
vec4 Specular;

void pointLight_Phong(in int i, in vec3 normal, in vec3 eyeDir, in float shininess)
{
    // Direction of the light (from the fragment to the light)
	vec3 lightDir_cameraSpace = Lights[i].Position.xyz - Position_cameraSpace;
    float d = length(lightDir_cameraSpace);
    
    vec3 l = normalize(lightDir_cameraSpace);
    
	vec3 reflectionVector = reflect(-l, normal); //unit vector
	vec3 surfaceToCamera = -eyeDir; //also a unit vector
	float cosAngle = clamp(dot(surfaceToCamera, reflectionVector),0.0,1.0);
	float specularCoefficient = pow(cosAngle, shininess);

	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	float difffuse_intensity = clamp( dot( normal,l ), 0.0,1.0 );

    // Compute attenuation
    float attenuation = 1.0 / (Lights[i].Attenuation[0] +
	Lights[i].Attenuation[1] * d +
	Lights[i].Attenuation[2] * d * d);
    
    Ambient  += vec4(Lights[i].Ambient,1.0) * attenuation;
    Diffuse  += vec4(Lights[i].Diffuse * difffuse_intensity, 1.0) * attenuation;
    Specular += vec4(Lights[i].Specular * specularCoefficient,1.0) * attenuation;
}

void spotLight_Phong(in int i, in vec3 normal, in vec3 eyeDir, in float shininess)
{
    // Direction of the light (from the fragment to the light)
	vec3 lightDir_cameraSpace = Lights[i].Position.xyz - Position_cameraSpace;
    float d = length(lightDir_cameraSpace);
    vec3 l = normalize(lightDir_cameraSpace);
    
    // Compute attenuation
    float attenuation = 1.0 / (Lights[i].Attenuation[0] +
	Lights[i].Attenuation[1] * d +
	Lights[i].Attenuation[2] * d * d);
    
    // See if point on surface is inside cone of illumination
    vec3 s_l = normalize(Lights[i].SpotDir);
    float spotDot = dot(-l, s_l);
    float spotAttenuation;
    
    float lightToSurfaceAngle = degrees(acos(spotDot));
    if(lightToSurfaceAngle > Lights[i].SpotCutoff){
        spotAttenuation = 0.0;
    }
    else
        spotAttenuation = pow(spotDot, Lights[i].SpotExp);
    //attenuation *= spotAttenuation;
    attenuation = spotAttenuation;
    
    //Specular phong
    vec3 reflectionVector = reflect(-l, normal); //unit vector
	vec3 surfaceToCamera = -eyeDir; //also a unit vector
	float cosAngle = clamp(dot(surfaceToCamera, reflectionVector),0.0,1.0);
	float specularCoefficient = pow(cosAngle, shininess);

	//Diffuse
	float difffuse_intensity = clamp( dot( normal,l ), 0.0,1.0 );
    
    Ambient  += vec4(Lights[i].Ambient,1.0) * attenuation;
    Diffuse  += vec4(Lights[i].Diffuse * difffuse_intensity, 1.0) * attenuation;
    Specular += vec4(Lights[i].Specular * specularCoefficient,1.0) * attenuation;
    
}

// Calcul pour une lumière directionnelle
void directionalLight_Phong(in int i, in vec3 normal, in vec3 eyeDir, float shininess)
{
    // Direction of the light (from the fragment to the light)
	vec3 l = normalize( -Lights[i].Position.xyz );
    
	vec3 reflectionVector = reflect(-l, normal); //unit vector
	vec3 surfaceToCamera = -eyeDir; //also a unit vector
	float cosAngle = clamp(dot(surfaceToCamera, reflectionVector),0.0,1.0);
	float specularCoefficient = pow(cosAngle, shininess);

	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	float difffuse_intensity = clamp( dot( normal,l ), 0.0,1.0 );
	//vec3 specularComponent = specularCoefficient * vec3(1.0,1.0,1.0);

    Ambient  += vec4(Lights[i].Ambient,1.0);
    Diffuse  += vec4(Lights[i].Diffuse * difffuse_intensity, 1.0);
    Specular += vec4(Lights[i].Specular * specularCoefficient,1.0);
}

// fonction d'éclairage pour l'ambiant et le diffus
vec4 lighting(in vec3 normal, in vec3 eyeDir)
{
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    // Propriétés de la surface
    vec4 matAmbiant    = Material.Ambient;
    vec4 matDiffuse    = Material.Diffuse;
    vec4 matSpecular   = Material.Specular;
    float matShininess = 10.0;


    // Clear the light intensity accumulators
    Ambient  = vec4 (0.0, 0.0, 0.0, 1.0);
    Diffuse  = vec4 (0.0, 0.0, 0.0, 1.0);
    Specular = vec4 (0.0, 0.0, 0.0, 1.0);

    // Calcul des 3 lumières
    if (pointLightOn == 1) {
       pointLight_Phong(0, normal, eyeDir, matShininess);
    }

    if (spotLightOn == 1) {
        spotLight_Phong(1, normal, eyeDir, matShininess);
    }

	if (dirLightOn == 1) {
		directionalLight_Phong(2, normal, eyeDir, matShininess);
	}
    

    // Ajout des contribution lumineuses calculées
    color += Ambient * matAmbiant;
    color += Diffuse * matDiffuse;
    color += Specular * matSpecular;
    color =  clamp(color, 0.0, 1.0);

    return color;
}

// fonction d'éclairage pour l'ambiant et le diffus
vec4 lightingIBL()
{
    // couleur finale calculée par IBL
    vec4 resColor = vec4(0.0);

    // Déclaration des accumulateurs nécessaires
    // On ne tiendra pas compte ici de l'AMBIANT.
    Diffuse  = vec4 (0.0, 0.0, 0.0, 1.0);
    Specular = vec4 (0.0, 0.0, 0.0, 1.0);

    // Propriétés de la surface
    vec4 matDiffuse    = Material.Diffuse;
    vec4 matSpecular   = Material.Specular;

    // on n'utilise PAS le "shininess" en IBL, car 
    // on utilise toujours des réflections parfaitement spéculaires
    // pour aller chercher les contributions spéculaires ou diffuses 
    // à l'intérieur de cartes... Cela étant dit, pour simuler l'effet
    // "glossy", on utilisera plutôt une carte Glossy où l'image à
    // l'intérieur du skybox a été filtrée (léger flou).

    // quelques déclarations relatives à l'IBL
    mat3 normalMatrix_wolrdSpace;    // la matrice de transfo en coord. univ. de la normale
    vec3 normal_worldSpace;          // la normale en coord. universelles
    vec3 vertexPos_worldSpace;       // la position du vertex en coord. universelles.
    vec3 eyeDir_worldSpace;          // la direction vertex-caméra en coord. universelles
    vec3 reflectDir_wordlSpace;      // la direction de réflexion (en coord. univ.) pour le calcul du IBL spéculaire

    // Afin de calculer la normale en coord. universelles (WORLD COORD.)
    // (à ne pas mélanger avec les coordonnées de visualisation ou eyeCoord ! qu'on 
    // utilise habituellement) il faut trouver la normal Matrix en coord universelles
    // C'est facile, la normalMatrix est toujours uniquement la sous-matrice 3X3
    // de la matrice utilisée pour placer les modèles tant que l'on scale uniformément...
    normalMatrix_wolrdSpace = mat3(modelMatrix);

    // Calcul de la normale en coordonnées universelles (world coord)
    normal_worldSpace = normalize(normalMatrix_wolrdSpace * Normal_objectSpace);

    // Lire la contribution diffuse du fragment dans la carte spéculaire.
    Diffuse += textureCube(diffMap,normal_worldSpace);

    // Pour le calcul spéculaire, on veut échantillonner la texture spéculaire 
	// là ou la reflexion du vecteur wcEyedir est reflétée.

    vertexPos_worldSpace = (modelMatrix * Position_objectSpace).xyz;
    eyeDir_worldSpace = EyePos_worldSpace - vertexPos_worldSpace;
	// Il faut fournir le vecteur camera-vertex à reflect()
    reflectDir_wordlSpace = reflect(-eyeDir_worldSpace, normal_worldSpace);
    Specular += textureCube(specMap,reflectDir_wordlSpace);

    // Ajout des contribution lumineuses calculées
    resColor += Diffuse * matDiffuse;
    resColor += Specular * matSpecular;
    resColor =  clamp(resColor, 0.0, 1.0);

    return resColor;
}


void main () {
    //  Normale du fragment dansle référenciel de caméra
	vec3 n = normalize( Normal_cameraSpace );
    // Vecteur allant du fragment à la caméra dans le référentiel de caméra
	vec3 e = normalize(Position_cameraSpace);

	if (IBLon > 0)
		color = lightingIBL().rgb;
	else
		color = lighting(n, e).rgb;
	depth = gl_FragCoord.z;
}
