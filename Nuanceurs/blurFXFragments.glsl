#version 430 core
uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform int resH;
uniform int resW;

in vec2 fragTexCoord;
out vec4 color;

//Le nb de step dans la texture, bas� sur la r�solution
float h_step = 1.0/resW;
float v_step = 1.0/resH;


float LineariserProfondeur(float d)
{
	float n = 0.1; //
	float f = 200.0;
	
	return(2.0 * n) / (f + n - d * (f - n));	
}

//Filtre gaussien 5x5 : sigma=2.5
//0.028672	0.036333	0.039317	0.036333	0.028672
//0.036333	0.046042	0.049824	0.046042	0.036333
//0.039317	0.049824	0.053916	0.049824	0.039317
//0.036333	0.046042	0.049824	0.046042	0.036333
//0.028672	0.036333	0.039317	0.036333	0.028672
vec4 FiltreGaussien(in float x, in float y, in float etendue)
{
	vec4 sum = vec4(0.0);

	sum += texture2D(colorMap, vec2(x - 2.0*h_step*etendue, y - 2.0*v_step*etendue)) * 0.028672;
	sum += texture2D(colorMap, vec2(x - 2.0*h_step*etendue, y - 1.0*v_step*etendue)) * 0.036333;
	sum += texture2D(colorMap, vec2(x - 2.0*h_step*etendue, y)) * 0.039317;
	sum += texture2D(colorMap, vec2(x - 2.0*h_step*etendue, y + 1.0*v_step*etendue)) * 0.036333;
	sum += texture2D(colorMap, vec2(x - 2.0*h_step*etendue, y + 2.0*v_step*etendue)) * 0.028672;

	sum += texture2D(colorMap, vec2(x - 1.0*h_step*etendue, y - 2.0*v_step*etendue)) * 0.036333;
	sum += texture2D(colorMap, vec2(x - 1.0*h_step*etendue, y - 1.0*v_step*etendue)) * 0.046042;
	sum += texture2D(colorMap, vec2(x - 1.0*h_step*etendue, y)) * 0.049824;
	sum += texture2D(colorMap, vec2(x - 1.0*h_step*etendue, y + 1.0*v_step*etendue)) * 0.046042;
	sum += texture2D(colorMap, vec2(x - 1.0*h_step*etendue, y + 2.0*v_step*etendue)) * 0.036333;

	sum += texture2D(colorMap, vec2(x, y - 2.0*v_step*etendue)) * 0.039317;
	sum += texture2D(colorMap, vec2(x, y - 1.0*v_step*etendue)) * 0.049824;
	sum += texture2D(colorMap, vec2(x, y)) * 0.053916;
	sum += texture2D(colorMap, vec2(x, y + 1.0*v_step*etendue)) * 0.049824;
	sum += texture2D(colorMap, vec2(x, y + 2.0*v_step*etendue)) * 0.039317;

	sum += texture2D(colorMap, vec2(x + 1.0*h_step*etendue, y - 2.0*v_step*etendue)) * 0.036333;
	sum += texture2D(colorMap, vec2(x + 1.0*h_step*etendue, y - 1.0*v_step*etendue)) * 0.046042;
	sum += texture2D(colorMap, vec2(x + 1.0*h_step*etendue, y)) * 0.049824;
	sum += texture2D(colorMap, vec2(x + 1.0*h_step*etendue, y + 1.0*v_step*etendue)) * 0.046042;
	sum += texture2D(colorMap, vec2(x + 1.0*h_step*etendue, y + 2.0*v_step*etendue)) * 0.036333;

	sum += texture2D(colorMap, vec2(x + 2.0*h_step*etendue, y - 2.0*v_step*etendue)) * 0.028672;
	sum += texture2D(colorMap, vec2(x + 2.0*h_step*etendue, y - 1.0*v_step*etendue)) * 0.036333;
	sum += texture2D(colorMap, vec2(x + 2.0*h_step*etendue, y)) * 0.039317;
	sum += texture2D(colorMap, vec2(x + 2.0*h_step*etendue, y + 1.0*v_step*etendue)) * 0.036333;
	sum += texture2D(colorMap, vec2(x + 2.0*h_step*etendue, y + 2.0*v_step*etendue)) * 0.028672;

	return vec4(sum.xyz, 1.0);
}


void main (void) 
{ 
    vec4 clear_color;
	vec4 blurred_col;
	float depth;
	float center_depth;
	float w; //Le poids avec lequel on applique le filtre

	//Colorier un point milieu en noir
	if ( abs(fragTexCoord.x-0.5) < 3*h_step && abs(fragTexCoord.y-0.5) < 3*v_step)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{

		//TODO : Ajouter le flou gaussien:
		//Couleur du FBO:
		clear_color = vec4(texture(colorMap, fragTexCoord.xy).rgb, 1.0);
   
		// Quantifier la diff�rence de profondeur entre le point vis� et le fragment courant:
		// Utilisez LineariserProfondeur() sur les profondeurs �chantillonn�es
		depth = LineariserProfondeur(texture(depthMap, fragTexCoord.xy).x); 
		
		center_depth = LineariserProfondeur(texture(depthMap, vec2(0.5,0.5)).x); 

		// Le poids est simplement la diff�rence absolue entre ces deux valeurs:
		w= abs(depth - center_depth);

		// Appliquer un filtre gaussien (�tendue de 1) :
		blurred_col = FiltreGaussien(fragTexCoord.x,fragTexCoord.y,1);
		// Appliquer un filtre gaussien avec une plus grande �tendue (ex.: 5) :
		blurred_col += FiltreGaussien(fragTexCoord.x,fragTexCoord.y,5);
		// Moyenne des deux filtres :
		blurred_col = blurred_col/2;

		// � modifier :
		// Ajuster la couleur selon la diff�rence de profondeur entre le point vis� et le fragment courant:
		color =  w * blurred_col + (1 - w) * clear_color;
		

		// Afin de d�boguer, on peut affichier simplement les valeurs de profondeurs:
		 //color = vec4(depth, depth, depth, 1.0);
	
	}
    
	color = clamp(color, 0.0, 1.0);
}
