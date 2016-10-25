///////////////////////////////////////////////////////////////////////////////
///  @file FBO.cpp
///  @author  Olivier Dionne
///  @brief   D�finit la classe CFBO impl�mentant un Frame Buffer Object simple
///           pour openGL
///  @date    2008-10-19
///  @version 1.0
///
///////////////////////////////////////////////////////////////////////////////
#include "FBO.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
///  public overloaded constructor  CFBO \n
///
///  Cr�e un FBO tr�s simple o� tous les membres priv�s sont � 0.
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
CFBO::CFBO() :
m_FBO( 0 ),
m_Texture( 0 ),
m_InternalFormat( 0 ),
m_Target( GL_TEXTURE_2D ),
m_TextureW( 0 ),
m_TextureH( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////
///  public destructor  ~CFBO \n
///
///  D�truit un objet FBO
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
CFBO::~CFBO()
{
   Liberer();
}


///////////////////////////////////////////////////////////////////////////////
///  public  Init \n
///
///  Cette m�thode initialise le FBO en cr�ant les noms de texture � l'interne.
///  Pour une mise � jour du FBO � chaque image, cette fonction NE DEVRAIT PAS
///  �tre appel�e constamment, parce qu'elle consomme de la m�moire et est 
///  relativement lente. On conseille donc d'appeler Init() une seule fois au d�but
///  de l'application, puis simplement d'utiliser la paire CommencerCapture() et 
///  TerminerCapture() afin de mettre � jour le contenu interne du FBO en tout temps.
///  �videmment, parce qu'il est important que la taille du FBO soit la m�me que le 
///  viewport courant, si la taille du viewport change en cours d'ex�cution, il est 
///  imp�ratif d'appeler Init() � nouveau (avec les nouvelles valeurs de w et de h), mais
///  en ayant pris soin au pr�alable de LIBERER la m�moire du FBO.
///  
///  @param [in]       w int   La largeur du viewport / de la texture interne
///  @param [in]       h int   La hauteur du viewport / de la texture interne
///  @param [in]       format int   Le format interne de la texture du FBO (commun�ment GL_RGB)
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void CFBO::Init(int w, int h)
{
	// Dimensions
	m_TextureW = w;
	m_TextureH = h;

	// TODO: Remplir la fonction d'initialisation d'un FBO:

	// Cr�er et lier un nouveau frame buffer avec l'ID m_fbo:


	//Generation d'un id
	glGenFramebuffers(1, &m_FBO);
	//Lie le frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	// Cr�er une texture RGB pour les couleurs avec L'ID m_Texture:
	// Pour �chantillionner plus tard des valeurs exactes
	// on veut des filtres de mignification et magnification de tpe NEAREST!

	//Generation du buffer pour les couleurs
	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_TextureW, m_TextureH, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Application des filtres de magnification et mignification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	//On deverouille le buffer de texture
	glBindTexture(GL_TEXTURE_2D, 0);

	//Generation du buffer pour la profondeur
	glGenTextures(1, &m_Profondeur);
	glBindTexture(GL_TEXTURE_2D, m_Profondeur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_TextureW, m_TextureH, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	// Application des filtres de magnification et mignification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//On deverouille le buffer de texture
	glBindTexture(GL_TEXTURE_2D, 0);
	/*
	// Generation du buffer pour la profondeur
	glGenRenderbuffers(1, &m_Profondeur);
	glBindRenderbuffer(GL_RENDERBUFFER, m_Profondeur);

	// Configuration du Render Buffer
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, w, h);
	//On deverouille le budder
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	*/
	// Attacher nos deux textures au frame buffer � des fin d'affichage (DRAW):
	// Association du Color Buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Texture, 0);
	
	// Association du Depth Buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_Profondeur, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_Profondeur);

	
	// V�rification des erreurs FBO
	// Nous vous fournissons cette v�rification d'erreurs
	// pour que vous arriviez plus ais�ment � d�boguer votre code.
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// On v�rifie les erreurs � la suite de la cr�ation du FBO
	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE_EXT:
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT" << endl;
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		cerr << "GL_FRAMEBUFFER_UNSUPPORTED_EXT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
		cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		cerr << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		cerr << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT" << endl;
		break;
	default:
		cerr << "ERREUR INCONNUE" << endl;
	}

	//On deverouille le Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////
///  public  Liberer \n
///
///  Cette fonction lib�re l'espace m�moire interne du FBO
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void CFBO::Liberer()
{
   if ( m_Texture )
   {
      glDeleteTextures( 1, &m_Texture );
      m_Texture = 0;
   }

   if ( m_FBO )
   {
      glDeleteFramebuffers( 1, &m_FBO );
      m_FBO = 0;
   }
   if(m_Profondeur)
   {
	   glDeleteTextures( 1, &m_Profondeur );
      m_Profondeur = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////
///  public  CommencerCapture \n
///
///  Cette fonction d�bute la d�finition du contenu du FBO. Son utilisation est
///  tr�s simple et intuitive. Une fois le FBO construit et initialiser avec new ()
///  et Init(), on n'a qu'� ins�rer les commandes openGL produisant le rendu externe
///  voulu (qui sera enregistr� dans le FBO) entre les commandes CommencerCapture() et
///  TerminerCapture();
///
///  @return Aucune
///
///  @author Olivier Dionne, Fr�d�ric Plourde
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void CFBO::CommencerCapture()
{
    // TODO: 
	// Activer l'utilisation du FBO
	// Attention � la r�solution avec laquelle on veut afficher!
	
	// set rendering destination to FBO
	glBindFramebuffer(GL_FRAMEBUFFER,m_FBO);

	//Set viewport size
	glViewport(0, 0, CVar::currentW, CVar::currentH);
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




}

///////////////////////////////////////////////////////////////////////////////
///  public  TerminerCapture \n
///
///  Cette fonction termine la d�finition du contenu du FBO. Son utilisation est
///  tr�s simple et intuitive. Une fois le FBO construit et initialiser avec new ()
///  et Init(), on n'a qu'� ins�rer les commandes openGL produisant le rendu externe
///  voulu (qui sera enregistr� dans le FBO) entre les commandes CommencerCapture() et
///  TerminerCapture();
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void CFBO::TerminerCapture()
{
	// TODO: 
	// Remettre OpenGL dans l'�tat par d�faut
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Set viewport size
	glViewport(0, 0, CVar::currentW, CVar::currentH);
}