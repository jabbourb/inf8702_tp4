#version 430 core
layout(location = 0) in vec3 vp;

uniform samplerCube colorMap;
uniform mat4 MVP;

out vec3 fragTexCoord;

void main (void)
{
   // effectuons la base : la transformation du vertex
   gl_Position = (MVP * vec4(vp, 1.0));
   fragTexCoord = vp;
}
