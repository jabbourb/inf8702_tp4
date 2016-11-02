///////////////////////////////////////////////////////////////////////////////
///  @file main.cpp
///  @brief   le main du programme ProjetNuanceur pour le cours INF8702 de Polytechnique
///  @author  Frédéric Plourde (2007)
///  @author  Félix Gingras Harvey (2016)
///  @date    2007 / 2016
///  @version 2.0
///
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/matrix_inverse.hpp>
#include <gtx/transform.hpp>
#include <gtx/matrix_cross_product.hpp>
#include "textfile.h"
#include "GrilleQuads.h"
#include "Cst.h"
#include "Var.h"
#include "NuanceurProg.h"
#include "Materiau.h"
#include "Texture2D.h"
#include "Modele3DOBJ.h"
#include "Skybox.h"
#include "Gazon.h"
#include "FBO.h"


///////////////////////////////////////////////
// LES OBJETS                                //
///////////////////////////////////////////////

// les programmes de nuanceurs
CNuanceurProg progNuanceurGaussien("Nuanceurs/blurFXSommets.glsl", "Nuanceurs/blurFXFragments.glsl", false);
CNuanceurProg progNuanceurShadowMap("Nuanceurs/shadowSommets.glsl", "Nuanceurs/shadowFragments.glsl", false);
CNuanceurProg progNuanceurDebug("Nuanceurs/debugSommets.glsl", "Nuanceurs/debugFragments.glsl", false);
CNuanceurProg progNuanceurSkybox("Nuanceurs/skyBoxSommets.glsl", "Nuanceurs/skyBoxFragments.glsl", false);
CNuanceurProg progNuanceurGazon("Nuanceurs/gazonSommets.glsl", "Nuanceurs/gazonFragments.glsl", false);
CNuanceurProg progNuanceurModele3D("Nuanceurs/modele3DSommets.glsl", "Nuanceurs/modele3DFragments.glsl", false);

// les différents matériaux utilisés dans le programme
CMateriau mat_pierre_model(0.0, 0.0, 0.0, 1.0, 0.5, 0.5, 0.5, 1.0, 0.05, 0.05, 0.05, 1.0, 0.0, 0.0, 0.0, 1.0, 10.0);
CMateriau mat_metal_model(0.0, 0.0, 0.0, 1.0, 0.2, 0.2, 0.2, 1.0, 0.8, 0.8, 0.8, 1.0, 0.0, 0.0, 0.0, 1.0, 10.0);
CMateriau mat_cuivre_model(0.0, 0.0, 0.0, 1.0, 0.5, 0.25, 0.15, 1.0, 0.35, 0.3, 0.1, 1.0, 0.0, 0.0, 0.0, 1.0, 50.0);
CMateriau nurbs_mat_ambiant_model(0.3, 0.3, 0.3, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0);

// Les objets 3D graphiques (à instancier plus tard parce que les textures demandent le contexte graphique)
CGazon * gazon;
CSkybox *skybox;
CModele3DOBJ *modele3Dvenus;
CModele3DOBJ *modele3Dbuddha;
CModele3DOBJ *modele3Dsphere;
CFBO *fbo = NULL;
CFBO *shadowMaps[3];
CTextureCubemap *carteDiffuse;

bool afficherShadowMap = false;
bool afficherAutresModeles = false;
unsigned int shadowMapAAfficher = 0;

double sourisX = 0;
double sourisY = 0;
float horizontalAngle = 0.f;// Angle horizontale de la caméra: vers les Z
float verticalAngle = 0.f;// Angle vertical: vers l'horizon
float initialFoV = 45.0f;// "Field of View" initial

float vitesseCamera = 20.0f; // unités / seconde
float vitesseSouris = 0.075f;

// Vecteurs caméra
glm::vec3 cam_position = glm::vec3(0, 0, -25);
glm::vec3 direction;
glm::vec3 cam_right;
glm::vec3 cam_up;

glm::mat4 gazonModelMatrix;
glm::mat4 venusModelMatrix;
glm::mat4 sphereModelMatrix;
glm::mat4 buddhaModelMatrix;

//{ 0.5f, 0.0f, 0.0f, 0.0f,  0.0f, 0.5f, 0.0f, 0.0f,  0.0f, 0.0f, 0.5f, 0.0f,  0.5f, 0.5f, 0.5f, 1.0f };
glm::mat4 scaleAndBiasMatrix = glm::mat4(
	glm::vec4(0.5f, 0.0f, 0.0f, 0.0f), 
	glm::vec4(0.0f, 0.5f, 0.0f, 0.0f), 
	glm::vec4(0.0f, 0.0f, 0.5f, 0.0f), 
	glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

// Matrice de projections de chaque lumière
glm::mat4 lightVP[3];

GLuint quad_vbo;
GLuint quad_ibo;
GLuint quad_vao;

GLuint debug_quad_vbo;
GLuint debug_quad_ibo;
GLuint debug_quad_vao;

GLuint quad_size;


///////////////////////////////////////////////
// PROTOTYPES DES FONCTIONS DU MAIN          //
///////////////////////////////////////////////
void initialisation (void);
void construireMatricesProjectivesEclairage(void);
void construireCartesOmbrage(void);
void dessinerSkybox(void);
void dessinerScene(void);
void dessinerModele3D(CModele3DOBJ* modele, glm::mat4 M, CMateriau mat);
void dessinerGazon(void);
void dessinerQuad(void);
void initQuad(GLuint vao, GLuint vbo, GLuint ibo);
glm::mat4 getModelMatrixVenus(void);
glm::mat4 getModelMatrixBuddha(void);
glm::mat4 getModelMatrixSphere(void);
glm::mat4 getModelMatrixGazon(void);
void attribuerValeursLumieres(GLuint progNuanceur);
void clavier(GLFWwindow *fenetre, int touche, int scancode, int action, int mods);
void mouvementSouris(GLFWwindow* window, double deltaT, glm::vec3& direction, glm::vec3& right, glm::vec3& up);
void redimensionnement(GLFWwindow *fenetre, int w, int h);
void rafraichirCamera(GLFWwindow* window, double deltaT);
void compilerNuanceurs();

// le main
int main(int argc,char *argv[])
{
	// start GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERREUR: impossible d'initialiser GLFW3\n");
		return 1;
	}


	GLFWwindow* fenetre = glfwCreateWindow(CVar::currentW , CVar::currentH, "INF8702 - Labo", NULL, NULL);
	if (!fenetre) {
		fprintf(stderr, "ERREUR: impossibe d'initialiser la fenêtre avec GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwSetWindowPos(fenetre, 600, 100);

	// Rendre le contexte openGL courrant celui de la fenêtre
	glfwMakeContextCurrent(fenetre);

	// Combien d'updates d'écran on attend après l'appel à glfwSwapBuffers()
	// pour effectivement échanger les buffers et retourner
	glfwSwapInterval(1);

	// Définir la fonction clavier
	glfwSetKeyCallback(fenetre, clavier);

	// Reset mouse position for next frame
	glfwSetCursorPos(fenetre, CVar::currentW / 2, CVar::currentH / 2);

	// Définire le comportement du curseur
	glfwSetInputMode(fenetre, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Définir la fonction de redimensionnement
	glfwSetWindowSizeCallback(fenetre, redimensionnement);
	
	// vérification de la version 4.X d'openGL
	glewInit();
	if (glewIsSupported("GL_VERSION_4_5"))
		printf("Pret pour OpenGL 4.5\n\n");
	else {
		printf("\nOpenGL 4.5 n'est pas supporte! \n");
		exit(1);
	}

	// Specifier le context openGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // recueillir des informations sur le système de rendu
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Materiel de rendu graphique: %s\n", renderer);
	printf("Plus récente vversion d'OpenGL supportee: %s\n\n", version);

    GLint max;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max);
    printf("GL_MAX_TEXTURE_UNITS = %d\n", max);

    glGetIntegerv(GL_MAX_VARYING_FLOATS, &max);
    printf("GL_MAX_VARYING_FLOATS = %d\n\n", max);


	if (!glfwExtensionSupported ("GL_EXT_framebuffer_object") ){
        printf("Objets 'Frame Buffer' NON supportes! :( ... Je quitte !\n");
        exit (1);
    } else {
        printf("Objets 'Frame Buffer' supportes :)\n\n");
    }


    // compiler et lier les nuanceurs
    compilerNuanceurs();

    // initialisation de variables d'état openGL et création des listes
    initialisation();
	
	double dernierTemps = glfwGetTime();
	int nbFrames = 0;
	
    // boucle principale de gestion des evenements
	while (!glfwWindowShouldClose(fenetre))
	{
		//Temps ecoule en secondes depuis l'initialisation de GLFW
		double temps = glfwGetTime();
		double deltaT = temps - CVar::temps;
		CVar::temps = temps;

		nbFrames++;
		// Si ça fait une seconde que l'on a pas affiché les infos
		if (temps - dernierTemps >= 1.0){
			if (CVar::showDebugInfo){
				printf("%f ms/frame\n", 1000.0 / double(nbFrames));
				printf("Position: (%f,%f,%f)\n", cam_position.x, cam_position.y, cam_position.z);
			}
			nbFrames = 0;
			dernierTemps += 1.0;
		}
		
		// Rafraichir le point de vue selon les input clavier et souris
		rafraichirCamera(fenetre, deltaT);
		
		// Afficher nos modèlests
		dessinerScene();

		// Swap buffers
		glfwSwapBuffers(fenetre);
		glfwPollEvents();

	}
	// close GL context and any other GLFW resources
	glfwTerminate();

    // on doit faire le ménage... !
	delete gazon;
	delete CVar::lumieres[ENUM_LUM::LumPonctuelle];
	delete CVar::lumieres[ENUM_LUM::LumDirectionnelle];
	delete CVar::lumieres[ENUM_LUM::LumSpot];
	delete modele3Dvenus;
	delete skybox;

	if (modele3Dbuddha)
		delete modele3Dbuddha;

	if (modele3Dsphere)
		delete modele3Dsphere;

	if (fbo)
		delete fbo;

	if (shadowMaps) {
		for (int i = 0; i<3; i++) {
			delete shadowMaps[i];
		}
	}
	

    // le programme n'arrivera jamais jusqu'ici
    return EXIT_SUCCESS;
}



// initialisation d'openGL
void initialisation (void) {
	////////////////////////////////////////////////////
	// CONSTRUCTION DES LUMIÈRES
	////////////////////////////////////////////////////

	// LUMIÈRE PONCTUELLE (enum : LumPonctuelle - 0)
	CVar::lumieres[ENUM_LUM::LumPonctuelle] = new CLumiere(0.1f, 0.1f, 0.1f, 0.5f, 0.5f, 1.0f, 0.7f, 0.7f, 0.7f, 0.0f, 20.0f, -20.0f, 1.0f, true);
	CVar::lumieres[ENUM_LUM::LumPonctuelle]->modifierConstAtt(1.1);
	CVar::lumieres[ENUM_LUM::LumPonctuelle]->modifierLinAtt(0.0);
	CVar::lumieres[ENUM_LUM::LumPonctuelle]->modifierQuadAtt(0.0);

	// LUMIÈRE SPOT (enum : LumSpot - 1)
	CVar::lumieres[ENUM_LUM::LumSpot] = new CLumiere(0.2f, 0.2f, 0.2f, 0.9f, 0.8f, 0.4f, 1.0f, 1.0f, 1.0f, 10.0f, 10.0f, -10.0f, 1.0f, true, -0.5f, -1.0f, 1.0f, 5.f, 60.0);

	// LUMIÈRE DIRECTIONNELLE (enum : LumDirectionnelle - 2)
	CVar::lumieres[ENUM_LUM::LumDirectionnelle] = new CLumiere(0.1f, 0.1f, 0.1f, 0.8f, 0.8f, 0.8f, 0.4f, 0.4f, 0.4f, 5.0f, -10.0f, -5.0f, 0.0f, true);

	// construire le skybox avec les textures
	skybox = new CSkybox("textures/uffizi_cross_LDR.bmp", CCst::grandeurSkybox);

	gazon = new CGazon("textures/gazon.bmp", 1.0f, 1.0f);
	gazonModelMatrix = getModelMatrixGazon();
	
	char* modele3DTexture = NULL;  // on ne désire pas de texture pour l'instant
	
	modele3Dvenus = new CModele3DOBJ("Modeles/venus-low.obj", modele3DTexture, 1.0);
	modele3Dvenus->attribuerNuanceur(progNuanceurModele3D);
	venusModelMatrix = getModelMatrixVenus();

	// On ne load pas les modèles inutilement pour accélérer le lancement
	if (afficherAutresModeles)
	{
		modele3Dsphere = new CModele3DOBJ("Modeles/sphere.obj", modele3DTexture, 1.0);
		modele3Dsphere->attribuerNuanceur(progNuanceurModele3D);
		sphereModelMatrix = getModelMatrixSphere();

		modele3Dbuddha = new CModele3DOBJ("Modeles/buddha.obj", modele3DTexture, 1.0);
		modele3Dbuddha->attribuerNuanceur(progNuanceurModele3D);
		buddhaModelMatrix = getModelMatrixBuddha();
	}

	// créer une carte de diffuse pour L'IBL
	carteDiffuse = new CTextureCubemap("textures/uffizi_cross_LDR_diffuse.bmp");

	// fixer la couleur de fond
	glClearColor(0.0, 0.0, 0.5, 1.0);

	// activer les etats openGL
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// TODO :
	// Création du frame buffer object pour pré-rendu de la scène:
	// Quelle taille devrait avoir nos textures?
	fbo =  new CFBO();
	fbo->Init(CVar::currentW,CVar::currentH);

	// TODO 
	// Création des trois FBOs pour cartes d'ombres:
    // Utilisez CCst::tailleShadowMap
	shadowMaps[0] = new CFBO();
	shadowMaps[0]->Init(CCst::tailleShadowMap, CCst::tailleShadowMap);
	shadowMaps[1] = new CFBO();
	shadowMaps[1]->Init(CCst::tailleShadowMap, CCst::tailleShadowMap);
	shadowMaps[2] = new CFBO();
	shadowMaps[2]->Init(CCst::tailleShadowMap, CCst::tailleShadowMap);

	construireMatricesProjectivesEclairage();

	// Le quad d'affichage principal
	initQuad(quad_vao, quad_vbo, quad_ibo);

	// Un autre quad pour fins de déboguage.
	initQuad(debug_quad_vao, debug_quad_vbo, debug_quad_ibo);

}


//////////////////////////////////////////////////////////
//////////  FONCTIONS POUR LE SHADOWMAPPING //////////////
//////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///  global public  construireCartesOmbrage \n
///
///  fonction de construction des cartes d'ombrage
///
///
///  @return Aucune
///
///  @author Frédéric Plourde, Félix G. Harvey
///  @date   2008-10-29
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
void construireCartesOmbrage(void)
{
	GLuint handle;
	glm::mat4 lightMVP;
	
	// Construire les trois cartes d'ombrage
	// Vous devez, pour chaque lumière, créer des shadowMaps en utilisant les matrices projectives
	// contenues dans LightVP[i]
	// Il vous faut donc afficher dans le FBO adéquat pour chaque lumière, en utilisant le bon nuanceur.
	// Comme il n'y a que le modèle 3D de la vénus qui peu créer des ombres, vous pouvez déssiner que ce modèle,
	// avec la bonne matrice modèle!
	for (unsigned int i = 0; i < CVar::lumieres.size(); i++)
	{
		// TODO :
		// ...
	}
}

///////////////////////////////////////////////////////////////////////////////
///  global public  construireMatricesProjectivesEclairage \n
///
///  fonction de construction des cartes d'ombrage
///
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
void construireMatricesProjectivesEclairage(void)
{
	// TODO:
	// Compléter la fonction "construireMatricesProjectivesEclairage"
	// On doit ici construire les matrice vue-projection (VP dans MVP) pour chaque lumière.
	// On donne des indices sur quelles valeurs utiliser, mais dans certains cas, d'autres valeurs 
	// peuvent fonctionner. Pour des shadow maps identiques aux images dans l'énoncé,
	// prendre les valeurs indiquées ici.
	// Les foncitons glm::lookAt(), glm::perspective() et glm::ortho() vous seront utiles...
	
	// Variables temporaires:
	float fov;
	float const K = 100.0f;
	float const ortho_width=20.f;
	GLfloat pos[4];
	GLfloat dir[3];
	glm::vec3 point_vise;
	glm::mat4 lumVueMat;
	glm::mat4 lumProjMat;

	//Parametre pour la perspective

	//On prend un intervalle entre near et far assez grand pour ne pas couper l'ombre
	float near = 0.1f;
	float far = 300.0f;
	//On prend un ratio a 1 car pour notre FBO shadowmap la longeur et largeur de la texture sont identique (512)
	float aspect = 1.0f;

	/// LUM0 : PONCTUELLE : sauvegarder dans lightVP[0]
	// position = position lumière
	// point visé : centre de l'objet (on triche avec la lumière ponctuelle)
	// fov = Assez pour voir completement le moèdle (~90 est OK).

	//On recupere la position de la lumiere
	CLumiere *lumPon = CVar::lumieres[ENUM_LUM::LumPonctuelle];
	(*lumPon).obtenirPos(pos);

	//On recupere la position du centre de la Venus
	point_vise = modele3Dvenus->obtenirCentroid();

	//Matrice de vue pour la lumiere ponctuelle (du centre de la camera vers le centre du model 3D)
	lumVueMat = glm::lookAt(glm::vec3(pos[0],pos[1],pos[2]), point_vise, cam_up);

	fov = 90.0f;
	
	//Matrice de projection pour la lumière ponctuelle
	lumProjMat = glm::perspective(fov, aspect, near, far);

	lightVP[0] = lumProjMat * lumVueMat;

	/// LUM1 : SPOT : sauvegarder dans lightVP[1]
	//	position = position lumière
	//	direction = spot_dir (attention != point visé)
	//  fov = angle du spot

	
	CLumiere *lumSpot = CVar::lumieres[ENUM_LUM::LumSpot];

	//On recupere la position de la lumiere
	(*lumSpot).obtenirPos(pos);
	//On recupere la direction de la lumiere
	(*lumSpot).obtenirSpotDir(dir);
	//On recupere l'angle du spot
	fov = (*lumSpot).obtenirSpotCutOff();

	//Construction de la matrice vue
	lumVueMat = glm::lookAt(glm::vec3(pos[0], pos[1], pos[2]), glm::vec3(pos[0], pos[1], pos[2]) + glm::vec3(dir[0], dir[1], dir[2]), cam_up);
	//Construction de la matrice projection
	lumProjMat = glm::perspective(fov, aspect, near, far);

	lightVP[1] = lumProjMat * lumVueMat;

	//LUM2 : DIRECTIONNELLE : sauvegarder dans lightVP[2]
	//	position = -dir * K | K=constante assez grande pour ne pas être dans le modèle
	//	direction = 0,0,0
	//  projection orthogonale, assez large pour voir le modèle (ortho_width)
	CLumiere *lumDir = CVar::lumieres[ENUM_LUM::LumDirectionnelle];

	//Dans le cas de la lumiere directionnel la direction est stocker dans la position
	(*lumDir).obtenirPos(pos);

	glm::vec3 positionDir  =  -glm::vec3(pos[0], pos[1], pos[2]) * K;
	glm::vec3 directionDir = glm::vec3(0, 0, 0);

	//Projection matrix
	lumProjMat = glm::ortho<float>(-ortho_width, ortho_width, -ortho_width, ortho_width, -ortho_width, 2 * ortho_width);
	lumVueMat = glm::lookAt(positionDir, directionDir,cam_up);

	lightVP[2] = lumProjMat * lumVueMat;

}


///////////////////////////////////////////////////////////////////////////////
///  global public  getModelMatrixVenus \n
///
///  fonction de construction de la matrice modèle pour la Venus
///
///
///  @return glm::mat4 : la matrice modèle
///
///  @author Félix G. Harvey
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
glm::mat4 getModelMatrixVenus(void)
{
	// Création d'une matrice-modèle.
	// Défini la translation/rotaion/grandeur du modèle.
	glm::mat4 scalingMatrix = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));

	glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
	float deg1 = DEG2RAD(180.0f);
	glm::mat4 rotationMatrix = glm::rotate(deg1, rotationAxis);

	glm::mat4 translationMatrix = glm::translate(-modele3Dvenus->obtenirCentroid());

	return translationMatrix * rotationMatrix * scalingMatrix;
}

///////////////////////////////////////////////////////////////////////////////
///  global public  getModelMatrixSphere \n
///
///  fonction de construction de la matrice modèle pour la sphere métallique
///
///
///  @return glm::mat4 : la matrice modèle
///
///  @author Félix G. Harvey
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
glm::mat4 getModelMatrixSphere(void)
{
	// Création d'une matrice-modèle.
	// Défini la translation/rotaion/grandeur du modèle.
	glm::mat4 scalingMatrix = glm::scale(glm::vec3(5.0f, 5.0f, 5.0f));

	glm::mat4 translationMatrix = glm::translate(glm::vec3(-20.0f, -5.0f, 0.0f) -modele3Dsphere->obtenirCentroid());

	return translationMatrix * scalingMatrix;
}

///////////////////////////////////////////////////////////////////////////////
///  global public  getModelMatrixBuddha \n
///
///  fonction de construction de la matrice modèle pour le buddha
///
///
///  @return glm::mat4 : la matrice modèle
///
///  @author Félix G. Harvey
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
glm::mat4 getModelMatrixBuddha(void)
{
	// Création d'une matrice-modèle.
	// Défini la translation/rotaion/grandeur du modèle.
	glm::mat4 scalingMatrix = glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));

	glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
	float deg = DEG2RAD(180.0f);
	glm::mat4 rotationMatrix = glm::rotate(deg, rotationAxis);
	
	rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	deg = DEG2RAD(- 90.0f);
	rotationMatrix = glm::rotate(deg, rotationAxis) *rotationMatrix;

	glm::mat4 translationMatrix = glm::translate(glm::vec3(20.0f, -5.0f, 0.0f) - modele3Dbuddha->obtenirCentroid());

	return translationMatrix * rotationMatrix * scalingMatrix;
}

///////////////////////////////////////////////////////////////////////////////
///  global public  getModelMatrixGazon\n
///
///  fonction de construction de la matrice modèle pour le gazon
///
///
///  @return glm::mat4 : la matrice modèle
///
///  @author Félix G. Harvey
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
glm::mat4 getModelMatrixGazon(void)
{
	// Création d'une matrice-modèle.
	glm::vec3 t1(-3.5f, -1.5f, 0.f);
	glm::mat4 translationMatrix1 = glm::translate(t1);

	glm::vec3 s(CCst::largeurGazon, CCst::longueurGazon, CCst::hauteurGazon);
	glm::mat4 scalingMatrix = glm::scale(s);

	glm::mat4 rotationMatrix;

	glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
	float a = glm::radians(-90.0f);
	rotationMatrix = glm::rotate(a, rotationAxis);

	glm::vec3 t2(0.f, -20.f, 0.f);
	glm::mat4 translationMatrix2 = glm::translate(t2);

	return translationMatrix2 * rotationMatrix * scalingMatrix * translationMatrix1;
}


///////////////////////////////////////////////////////////////////////////////
///  global public  dessinerModele3D \n
///
///  fonction de d'affichage pour un modèle 3D
///
/// @param modele CModele3DOBJ* pointeur vers le modèle 3D
/// @param M glm::mat4 la matrice modèle du modèle 3D
/// @param mat CMateriau la matériel a appliquer au modèle 3D
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016-10
///
///////////////////////////////////////////////////////////////////////////////
void dessinerModele3D(CModele3DOBJ* modele, glm::mat4 M, CMateriau mat)
{
	progNuanceurModele3D.activer();

	// activer les textures supplémentaires pour le calcul du IBL
	// On parle ici de la carte diffuse (chargée initialement dans 
	// la texture 2D carteDiffuse...et la carte spéculaire (que 
	// l'on va rechercher à l'intérieur de l'objet skybox), car 
	// elle a déjà été générée pour générer le graphisme du skybox.
	carteDiffuse->appliquer(
		WRAP_S | WRAP_T | WRAP_R | MIN_FILTER | MAG_FILTER,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR,
		0, 0,
		0, 0,
		0, 0,
		false, GL_TEXTURE0);

	(*(skybox->obtenirTextures()))[0]->appliquer(
		WRAP_S | WRAP_T | WRAP_R | MIN_FILTER | MAG_FILTER,
		GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		GL_LINEAR, GL_LINEAR,
		0, 0,
		0, 0,
		0, 0,
		false, GL_TEXTURE1);

	glm::mat4 mv = CVar::vue * M;
	glm::mat4 mvp = CVar::projection * CVar::vue * M;
	glm::mat3 mv_n = glm::inverseTranspose(glm::mat3(CVar::vue * M));

	GLuint m_handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "MVP");
	glUniformMatrix4fv(m_handle, 1, GL_FALSE, &mvp[0][0]);

	GLuint v_handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "MV");
	glUniformMatrix4fv(v_handle, 1, GL_FALSE, &mv[0][0]);

	GLuint p_handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "MV_N");
	glUniformMatrix3fv(p_handle, 1, GL_FALSE, &mv_n[0][0]);

	attribuerValeursLumieres(progNuanceurModele3D.getProg());

	m_handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "modelMatrix");
	glUniformMatrix4fv(m_handle, 1, GL_FALSE, &M[0][0]);

	m_handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "EyePos_worldSpace");
	glUniform3fv(m_handle, 1, &cam_position[0]);

	m_handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "IBLon");
	glUniform1i(m_handle, CVar::IBLon);

	////////////////    Fournir les valeurs de matériaux: //////////////////////////
	GLfloat component[4];
	GLuint handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "Material.Ambient");
	mat.obtenirKA(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "Material.Diffuse");
	mat.obtenirKD(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "Material.Specular");
	mat.obtenirKS(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "Material.Exponent");
	mat.obtenirKE(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurModele3D.getProg(), "Material.Shininess");
	glUniform1f(handle, GLfloat(mat.obtenirShininess()));

	///////////////////////////////////////////////////////////////////////////////////

	// ajouts d'autres uniforms
	if (CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee()) 
		progNuanceurModele3D.uniform1("pointLightOn", 1);
	else 
		progNuanceurModele3D.uniform1("pointLightOn", 0);

	if (CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee()) 
		progNuanceurModele3D.uniform1("dirLightOn", 1);
	else 
		progNuanceurModele3D.uniform1("dirLightOn", 0);
	
	if (CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee()) 
		progNuanceurModele3D.uniform1("spotLightOn", 1);
	else 
		progNuanceurModele3D.uniform1("spotLightOn", 0);

	modele->dessiner();
}



///////////////////////////////////////////////////////////////////////////////
///  global public  dessinerGazon \n
///
///  fonction de d'affichage pour le modele du Gazon
///
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016
///
///////////////////////////////////////////////////////////////////////////////
void dessinerGazon()
{
	progNuanceurGazon.activer();

	// Matrice Model-Vue-Projection:
	glm::mat4 mvp = CVar::projection * CVar::vue * gazonModelMatrix;

	glm::mat4 mv =  CVar::vue * gazonModelMatrix;

	// Matrice pour normales (world matrix):
	glm::mat3 mv_n = glm::inverseTranspose(glm::mat3(CVar::vue * gazonModelMatrix));

	GLuint handle;
	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "M");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &gazonModelMatrix[0][0]);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "MV");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &mv[0][0]);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "MVP");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &mvp[0][0]);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "MV_N");
	glUniformMatrix3fv(handle, 1, GL_FALSE, &mv_n[0][0]);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Light0VP");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &lightVP[0][0][0]);
	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Light1VP");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &lightVP[1][0][0]);
	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Light2VP");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &lightVP[2][0][0]);

	////////////////    Fournir les valeurs de matériaux: //////////////////////////
	GLfloat component[4];
	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Material.Ambient");
	nurbs_mat_ambiant_model.obtenirKA(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Material.Diffuse");
	nurbs_mat_ambiant_model.obtenirKD(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Material.Specular");
	nurbs_mat_ambiant_model.obtenirKS(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Material.Exponent");
	nurbs_mat_ambiant_model.obtenirKE(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurGazon.getProg(), "Material.Shininess");
	glUniform1f(handle, nurbs_mat_ambiant_model.obtenirShininess());

	///////////////////////////////////////////////////////////////////////////////////

	attribuerValeursLumieres(progNuanceurGazon.getProg());
	
	// ajouts d'autres uniforms
	if (CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee())
		progNuanceurGazon.uniform1("pointLightOn", 1);
	else
		progNuanceurGazon.uniform1("pointLightOn", 0);

	if (CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee())
		progNuanceurGazon.uniform1("dirLightOn", 1);
	else
		progNuanceurGazon.uniform1("dirLightOn", 0);

	if (CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee())
		progNuanceurGazon.uniform1("spotLightOn", 1);
	else
		progNuanceurGazon.uniform1("spotLightOn", 0);


	//On fourni les shadow maps:
	if (shadowMaps[0]) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowMaps[0]->GetDepthTex());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadowMaps[1]->GetDepthTex());

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowMaps[2]->GetDepthTex());
	}

	gazon->dessiner();
}

///////////////////////////////////////////////////////////////////////////////
///  global public  dessinerSkybox \n
///
///  fonction de d'affichage pour le modele du Skybox
///
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016
///
///////////////////////////////////////////////////////////////////////////////
void dessinerSkybox()
{
	GLenum err = glGetError();
	progNuanceurSkybox.activer();
	err = glGetError();
	glm::vec3 s(CCst::grandeurSkybox, CCst::grandeurSkybox, CCst::grandeurSkybox);
	glm::mat4 scalingMatrix = glm::scale(s);

	glm::mat4 rotationMatrix;
	glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
	float a = glm::radians(0.f);
	rotationMatrix = glm::rotate(a, rotationAxis);

	glm::mat4 translationMatrix = glm::translate(cam_position);
	//glm::mat4 translationMatrix = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;

	// Matrice Model-Vue-Projection:
	glm::mat4 mvp = CVar::projection * CVar::vue * modelMatrix;

	GLuint handle;

	handle = glGetUniformLocation(progNuanceurSkybox.getProg(), "MVP");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &mvp[0][0]);
	err = glGetError();
	//translate
	skybox->dessiner();
	
}

///////////////////////////////////////////////////////////////////////////////
///  global public  dessinerQuadx \n
///
///  fonction de d'affichage pour un quad plein écran
///
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016
///
///////////////////////////////////////////////////////////////////////////////
void dessinerQuad(void)
{
	progNuanceurGaussien.activer();

	GLint handle = glGetUniformLocation(progNuanceurGaussien.getProg(), "resH");
	glUniform1i(handle, CVar::currentH);

	handle = glGetUniformLocation(progNuanceurGaussien.getProg(), "resW");
	glUniform1i(handle, CVar::currentW);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo->GetRGBTex());

	// Bind our texture in Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbo->GetDepthTex());

	
	glBindVertexArray(quad_vao);
	glEnableVertexAttribArray(0);

	glDrawElements(
		GL_QUADS,       // mode
		quad_size,					// count
		GL_UNSIGNED_INT,    // type
		(void*)0            // element array buffer offset
	);

	glDisableVertexAttribArray(0);
}

///////////////////////////////////////////////////////////////////////////////
///  global public  dessinerScene \n
///
///  fonction de d'affichage pour la scène au complet. 
///	 Appelle la fonction dessiner de chaque modèle présent
///
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016
///
///////////////////////////////////////////////////////////////////////////////
void dessinerScene()
{
	

	//////////////////	 Préparer l'affichage:	//////////////////
	
	// Peupler nos textures de profondeur selon la géométrie courante de la scène:
	construireCartesOmbrage();

	// TODO Décommenter les conditions:

	if (CVar::FBOon) {
		// TODO : 
		// Activer le FBO pour l'affichage
		fbo->CommencerCapture();
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, CVar::currentW, CVar::currentH);
	}
	
	//////////////////     Afficher les objets:  ///////////////////////////
	glDisable(GL_DEPTH_TEST);
	GLenum err = glGetError();
	dessinerSkybox();
	err = glGetError();
	glEnable(GL_DEPTH_TEST);
	
	dessinerGazon();
	
	dessinerModele3D(modele3Dvenus, venusModelMatrix, mat_pierre_model);
	
	// Si on veut plus de variété.
	// Surtout utile pour bien voir l'IBL.
	if (afficherAutresModeles)
	{
		dessinerModele3D(modele3Dsphere, sphereModelMatrix, mat_metal_model);
		dessinerModele3D(modele3Dbuddha, buddhaModelMatrix, mat_cuivre_model);
	}

	// TODO Décommenter les conditions:
	if (CVar::FBOon){
		//TODO :
		//Si on utilisait le FBO, le désactiver et dessiner le quad d'écran:
		fbo->TerminerCapture();
		dessinerQuad();
	}
	

	// Fonction d'aide pour mieux visualiser le contenu des shadowMaps
	// afficherShadowMap et shadowMapAAfficher sont déclarés globaux au fichier main.
	if (afficherShadowMap)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		GLenum err = glGetError();
		// On ne veut pas tout cahcher:
		glViewport(0, 0, CVar::currentW/4, CVar::currentH/4);

		// activer le shader
		progNuanceurDebug.activer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowMaps[shadowMapAAfficher]->GetRGBTex());
		err = glGetError();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowMaps[shadowMapAAfficher]->GetDepthTex());
		err = glGetError();

		glBindVertexArray(debug_quad_vao);
		glEnableVertexAttribArray(0);

		glDrawElements(
			GL_QUADS,			// mode
			quad_size,			// count
			GL_UNSIGNED_INT,    // type
			(void*)0            // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		err = glGetError();
		
		glEnable(GL_DEPTH_TEST);
	
	}

    // Flush les derniers vertex du pipeline graphique
    glFlush();
}

///////////////////////////////////////////////////////////////////////////////
///  global public  initQuad \n
///
///  Initialisation d'un quad à afficher sur une portion de l'écran
///
///  @param vao GLuint identifiant le VAO
///  @param vbo GLuint identifiant le VBO
///  @param ibo GLuint identifiant le IBO
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016
///
///////////////////////////////////////////////////////////////////////////////
void initQuad(GLuint vao, GLuint vbo, GLuint ibo)
{
	float sommets[] = { -1.f, -1.f, 0.f,   1.f, -1.f, 0.f,   1.f, 1.f, 0.f,   -1.f, 1.f, 0.f };
	unsigned int indices[] = { 0,1,2,3 };

	// Notre Vertex Array Object - VAO
	glGenVertexArrays(1, &vao);
	// Un buffer pour les sommets
	glGenBuffers(1, &vbo);
	// Un buffer pour les indices
	glGenBuffers(1, &ibo);

	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	int s_byte_size = sizeof(sommets);
	int i_byte_size = sizeof(indices);
	glBufferData(GL_ARRAY_BUFFER, s_byte_size, &sommets[0], GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_byte_size, &indices[0], GL_STATIC_DRAW);

	quad_size = i_byte_size / sizeof(unsigned int);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (char *)NULL + (0));//texcoords

}

///////////////////////////////////////////////////////////////////////////////
///  global public  clavier \n
///
///  fonction de rappel pour la gestion du clavier
///
///  @param [in]       pointeur GLFWwindow	Référence à la fenetre GLFW
///  @param [in]       touche	int			ID de la touche
///  @param [in]       scancode int			Code spécifique à la plateforme et à l'ID
///  @param [in]       action	int			Action appliquée à la touche
///  @param [in]       mods		int			Modifier bits
///
///  @return Aucune
///
///  @author Félix G. Harvey 
///  @date   2016-06-03
///
///////////////////////////////////////////////////////////////////////////////
void clavier(GLFWwindow* fenetre, int touche, int scancode, int action, int mods)
{

	switch (touche) {
	case GLFW_KEY_Q:{
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(fenetre, GL_TRUE);
		break;
	}
	case  GLFW_KEY_ESCAPE:{
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(fenetre, GL_TRUE);
		break;
	}
	case  GLFW_KEY_P:{
		if (action == GLFW_PRESS){
			if (CVar::isPerspective)
				CVar::isPerspective = false;
			else
				CVar::isPerspective = true;
		}
		break;
	}
	case  GLFW_KEY_I:{
		if (action == GLFW_PRESS){
			if (CVar::showDebugInfo)
				CVar::showDebugInfo = false;
			else
				CVar::showDebugInfo = true;
		}
		break;
	}
	case  GLFW_KEY_B: {
		if (action == GLFW_PRESS) {
			if (CVar::IBLon)
				CVar::IBLon = false;
			else
				CVar::IBLon = true;
		}
		break;
	}

	case  GLFW_KEY_F: {
		if (action == GLFW_PRESS) {
			if (CVar::FBOon)
				CVar::FBOon = false;
			else
				CVar::FBOon = true;
		}
		break;
	}

	case  GLFW_KEY_C:{
		if (action == GLFW_PRESS){
			if (CVar::mouseControl){
				CVar::mouseControl = false;
				glfwSetInputMode(fenetre, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}	
			else{
				CVar::mouseControl = true;
				glfwSetInputMode(fenetre, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}		
		}

		break;
	}
	case  GLFW_KEY_1:{
		if (action == GLFW_PRESS){
			if (CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee())
				CVar::lumieres[ENUM_LUM::LumDirectionnelle]->eteindre();
			else
				CVar::lumieres[ENUM_LUM::LumDirectionnelle]->allumer();
		}

		break;
	}
	case  GLFW_KEY_2:{
		if (action == GLFW_PRESS){
			if (CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee())
				CVar::lumieres[ENUM_LUM::LumPonctuelle]->eteindre();
			else
				CVar::lumieres[ENUM_LUM::LumPonctuelle]->allumer();
		}
		break;
	}
	case  GLFW_KEY_3:{
		if (action == GLFW_PRESS){
			if (CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee())
				CVar::lumieres[ENUM_LUM::LumSpot]->eteindre();
		else
			CVar::lumieres[ENUM_LUM::LumSpot]->allumer();
		}
		break;
	}

	// permuter le minFilter
	case GLFW_KEY_N:
	{
		if (action == GLFW_PRESS) {
			if (CVar::minFilter >= 5)
				CVar::minFilter = 0;
			else
				CVar::minFilter++;
		}
		break;
	}

	// permuter le magFilter
	case GLFW_KEY_M:
	{
		if (action == GLFW_PRESS) {
			if (CVar::magFilter >= 1)
				CVar::magFilter = 0;
			else
				CVar::magFilter++;
		}
		break;
	}

	}

}




///////////////////////////////////////////////////////////////////////////////
///  global public  redimensionnement \n
///
///  fonction de rappel pour le redimensionnement de la fenêtre graphique
///
///  @param [in]       w GLsizei    nouvelle largeur "w" en pixels
///  @param [in]       h GLsizei    nouvelle hauteur "h" en pixels
///
///  @return Aucune
///
///  @author Frédéric Plourde 
///  @date   2007-12-14
///
///////////////////////////////////////////////////////////////////////////////
void redimensionnement(GLFWwindow* fenetre, int w, int h)
{
    CVar::currentW = w;
    CVar::currentH = h;
    glViewport(0, 0, w, h);
	fbo->Liberer();
	fbo->Init(w, h);
    dessinerScene();
}

void attribuerValeursLumieres(GLuint progNuanceur)
{
	GLenum error = glGetError();

	//Handle pour attribut de lumiere
	GLuint li_handle;

	li_handle = glGetUniformLocation(progNuanceur, "dirLightOn");
	error = glGetError();
	glUniform1i(li_handle, CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee());
	error = glGetError();
	li_handle = glGetUniformLocation(progNuanceur, "pointLightOn");
	glUniform1i(li_handle, CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee());
	li_handle = glGetUniformLocation(progNuanceur, "spotLightOn");
	glUniform1i(li_handle, CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee());
	error = glGetError();

	// Fournir les valeurs d'éclairage au nuanceur.
	// Les directions et positions doivent être en référenciel de caméra. 
	for (int i = 0; i < CVar::lumieres.size(); i++)
	{
		//Placeholders pour contenir les valeurs
		GLfloat temp3[3];
		GLfloat temp4[4];
		glm::vec4 pos;
		glm::vec4 pos_cam;

		//Creer un descripteur basé sur l'index de lumière
		std::string begin = "Lights[";
		int l_idx = i;
		std::string end = "]";
		std::string light_desc = begin + std::to_string(l_idx) + end;

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Ambient").c_str());
		CVar::lumieres[i]->obtenirKA(temp3);
		glUniform3fv(li_handle, 1, &temp3[0]);
		error = glGetError();

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Diffuse").c_str());
		CVar::lumieres[i]->obtenirKD(temp3);
		glUniform3fv(li_handle, 1, &temp3[0]);

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Specular").c_str());
		CVar::lumieres[i]->obtenirKS(temp3);
		glUniform3fv(li_handle, 1, &temp3[0]);

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Position").c_str());
		CVar::lumieres[i]->obtenirPos(temp4);

		// Transformer ici la direction/position de la lumière vers un référenciel de caméra
		pos = glm::vec4(temp4[0], temp4[1], temp4[2], temp4[3]);
		pos = CVar::vue * pos;

		temp4[0] = pos.x;
		temp4[1] = pos.y;
		temp4[2] = pos.z;
		temp4[3] = pos.w;
		glUniform4fv(li_handle, 1, &temp4[0]);

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".SpotDir").c_str());
		CVar::lumieres[i]->obtenirSpotDir(temp3);
		//Transformer ici la direction du spot
		pos = glm::vec4(temp3[0], temp3[1], temp3[2], 0.0f);
		pos = CVar::vue * pos;
		temp3[0] = pos.x;
		temp3[1] = pos.y;
		temp3[2] = pos.z;
		glUniform3fv(li_handle, 1, &temp3[0]);

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".SpotExp").c_str());
		glUniform1f(li_handle, CVar::lumieres[i]->obtenirSpotExp());

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".SpotCutoff").c_str());
		glUniform1f(li_handle, CVar::lumieres[i]->obtenirSpotCutOff());

		li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Attenuation").c_str());
		glUniform3f(li_handle,
			CVar::lumieres[i]->obtenirConsAtt(),
			CVar::lumieres[i]->obtenirLinAtt(),
			CVar::lumieres[i]->obtenirQuadAtt());
	}

}


//////////////////////////////////////////////////////////
////////////  FONCTIONS POUR LA SOURIS ///////////////////
//////////////////////////////////////////////////////////

void mouvementSouris(GLFWwindow* window, double deltaT, glm::vec3& direction, glm::vec3& right, glm::vec3& up)
{
	if (CVar::mouseControl)
	{
		// Déplacement de la souris:
		// Taille actuelle de la fenetre
		int mid_width, mid_height;
		glfwGetWindowSize(window, &mid_width, &mid_height);
		mid_width /= 2;
		mid_height /= 2;

		// Get mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// Reset mouse position for next frame
		glfwSetCursorPos(window, mid_width, mid_height);

		// Nouvelle orientation
		horizontalAngle += vitesseSouris * deltaT * float(mid_width - xpos);
		verticalAngle += vitesseSouris * deltaT * float(mid_height - ypos);
	}
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	// Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	// Up vector : perpendicular to both direction and right
	up = glm::cross(right, direction);
}

//////////////////////////////////////////////////////////
////////////  FONCTIONS POUR LA CAMÉRA ///////////////////
//////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///  global public  rafraichirCamera \n
///
///  Fonction de gestion de la position de la caméra en coordonnées sphériques.
///  Elle s'occuper de trouver les coordonnées x et y de la caméra à partir 
///  des theta et phi courants, puis fixe dans openGL la position de la caméra
///  à l'aide de gluLookAt().
///
///  @return Aucune
///
///  @author Frédéric Plourde 
///  @date   2007-12-14
///
///////////////////////////////////////////////////////////////////////////////
void rafraichirCamera(GLFWwindow* fenetre, double deltaT)
{
	mouvementSouris(fenetre, deltaT, direction, cam_right, cam_up);

	// Move forward
	if (glfwGetKey(fenetre, GLFW_KEY_W) == GLFW_PRESS){
		cam_position += direction * (float)deltaT * vitesseCamera;
	}
	// Move backward
	if (glfwGetKey(fenetre, GLFW_KEY_S) == GLFW_PRESS){
		cam_position -= direction * (float)deltaT * vitesseCamera;
	}
	// Strafe right
	if (glfwGetKey(fenetre, GLFW_KEY_D) == GLFW_PRESS){
		cam_position += cam_right * (float)deltaT * vitesseCamera;
		}
	// Strafe left
	if (glfwGetKey(fenetre, GLFW_KEY_A) == GLFW_PRESS){
		cam_position -= cam_right * (float)deltaT * vitesseCamera;
	}

	//Matrice de projection:
	float ratio = (float)CVar::currentW / (float)CVar::currentH;
	if (CVar::isPerspective){
		// Caméra perspective: 
		CVar::projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 2000.0f);
	}
	else
	{
		// Caméra orthographique :
		CVar::projection = glm::ortho(-5.0f*ratio, 5.0f*ratio,-5.0f, 5.0f, 0.001f, 3000.0f); // In world coordinates
	}

	// Matrice de vue:
	CVar::vue = glm::lookAt(
		cam_position,				// Position de la caméra
		cam_position + direction,   // regarde vers position + direction
		cam_up                  // Vecteur "haut"
		);
}


//////////////////////////////////////////////////////////
//////////  FONCTIONS POUR LES NUANCEURS /////////////////
//////////////////////////////////////////////////////////
void compilerNuanceurs () 
{
    // on compiler ici les programmes de nuanceurs qui furent prédéfinis

	progNuanceurModele3D.compilerEtLier();
	progNuanceurModele3D.enregistrerUniformInteger("diffMap", CCst::texUnit_0);
	progNuanceurModele3D.enregistrerUniformInteger("specMap", CCst::texUnit_1);

	progNuanceurShadowMap.compilerEtLier();
	progNuanceurShadowMap.enregistrerUniformInteger("colorMap", CCst::texUnit_0);
	progNuanceurShadowMap.enregistrerUniformInteger("depthMap", CCst::texUnit_1);

	progNuanceurGaussien.compilerEtLier();
	progNuanceurGaussien.enregistrerUniformInteger("colorMap", CCst::texUnit_0);
	progNuanceurGaussien.enregistrerUniformInteger("depthMap", CCst::texUnit_1);

	progNuanceurDebug.compilerEtLier();
	progNuanceurDebug.enregistrerUniformInteger("colorMap", CCst::texUnit_0);
	progNuanceurDebug.enregistrerUniformInteger("depthMap", CCst::texUnit_1);

	progNuanceurSkybox.compilerEtLier();
	progNuanceurSkybox.enregistrerUniformInteger("colorMap", CCst::texUnit_0);

	progNuanceurGazon.compilerEtLier();
	progNuanceurGazon.enregistrerUniformInteger("colorMap", CCst::texUnit_0);
	progNuanceurGazon.enregistrerUniformInteger("shadowMap0", CCst::texUnit_1);
	progNuanceurGazon.enregistrerUniformInteger("shadowMap1", CCst::texUnit_2);
	progNuanceurGazon.enregistrerUniformInteger("shadowMap2", CCst::texUnit_3);
}
