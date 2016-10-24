#version 430 core
uniform samplerCube colorMap;

in vec3 fragTexCoord;
layout(location = 0) out vec3 color;
layout(location = 1) out float depth;

void main (void) 
{
   color = texture(colorMap, fragTexCoord).xyz;
   depth = 1000.0;
}
