#version 430 core
layout(location = 0) in vec3 vp;

out vec2 fragTexCoord;

void main (void)
{
   gl_Position = vec4(vp,1.0);
   fragTexCoord = (vp.xy+vec2(1.0,1.0))/2.0;
}
