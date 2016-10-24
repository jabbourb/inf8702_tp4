#version 430 core
layout(location = 0) in vec3 vp;

uniform sampler2D colorMap;

out vec2 fragTexCoord;

void main (void)
{
   gl_Position = vec4(vp,1.0);
   // TODO :
   // Passer les bonnes coordonées de textures.
   // Attention les sommets sont dans le domaine [-1, 1]
   // et des coordonées de textures devrait être en [0, 1]
   // fragTexCoord = ...
}
