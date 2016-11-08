#version 430 core

layout(location = 0) in vec2 vt;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec3 vp;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 MV_N;

//out vec3 LightDirection_cameraSpace;
out vec3 Normal_cameraSpace;
out vec3 Position_cameraSpace;

out vec3 Position_objectSpace;
out vec3 Normal_objectSpace;

void main () {

    gl_Position = MVP * vec4 (vp, 1.0);
	
	
    // Position dans le référenciel de caméra
    Position_cameraSpace = ( MV * vec4(vp,1.0)).xyz;    
	// Normale dans le référenciel de caméra
	Normal_cameraSpace = MV_N * vn; 

	// Passer les variables en coordonnées d'objet pour l'IBL :
	Position_objectSpace = vp;
	Normal_objectSpace = vn;
}
