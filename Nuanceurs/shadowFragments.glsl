#version 430 core

// Ouput data
layout(location = 0) out vec3 color;
layout(location = 1) out float fragmentdepth;

void main(){
    // Not really needed, OpenGL does it anyway
	color = vec3(gl_FragCoord.z);
    fragmentdepth = gl_FragCoord.z;
}