#version 430 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec2 vt;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec3 vp;

// Values that stay constant for the whole mesh.
uniform mat4 shadowMVP;

void main(){
 gl_Position =  shadowMVP * vec4(vp,1);
}