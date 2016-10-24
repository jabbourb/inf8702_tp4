#version 430 core
uniform sampler2D colorMap;
uniform sampler2D depthMap;

in vec2 fragTexCoord;
out vec4 color;

float LineariserProfondeur(float d)
{
	float n = 0.1; //
	float f = 200.0;
	
	return(2.0 * n) / (f + n - d * (f - n));	
}

void main (void) 
{ 
	color = vec4(LineariserProfondeur(texture(depthMap, fragTexCoord.xy).x));
	color = clamp(color, 0.0, 1.0);
}
