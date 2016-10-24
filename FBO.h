///////////////////////////////////////////////////////////////////////////////
///  @file FBO.h
///  @author  Olivier Dionne 
///  @brief   D�clare une classe impl�mentant un "frame buffer object"
///  @date    2008-10-19
///  @version 1.0
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <iostream>
#include <GL/glew.h>
#include "Var.h"

///////////////////////////////////////////////////////////////////////////////
///  @class CFBO
///  @brief Cette classe encapsule un FBO (Frame Buffer Object) openGL
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
class CFBO
{
public:
   /// Constructeur par d�faut
	CFBO();
   /// destructeur par d�faut
	~CFBO();

	/// Initialise le FBO et cr�e une texture de rendu pour RGB et pour DepthBuffer
	void Init( int w, int h );

	/// Lib�re la m�moire openGL
	void Liberer();
   /// Commencement de la capture des instruction openGL r�alisant le contenu du FBO
   void CommencerCapture(); 
   /// fin de la description du contenu du FBO
   void TerminerCapture();
	/// Retourne la largeur de la texture
   inline int GetWidth()  const { return m_TextureW; }
   /// Retourne la hauteur de la texture
   inline int GetHeight() const { return m_TextureH; }
   /// Retourne l'ID de texture de couleur
   inline GLuint GetRGBTex() const { return m_Texture; }
   /// Retourne la hauteur de la texture
   inline GLuint GetDepthTex() const { return m_Profondeur; }

private:
	
   /// l'ID du FBO
	GLuint m_FBO;
   /// l'ID de la texture RGB
	GLuint m_Texture;
	/// l'ID de la texture profondeur
	GLuint m_Profondeur;
   /// le format interne (commun�ment GL_RGB)
	GLint  m_InternalFormat;
   /// la cible de la texture (commun�ment GL_TEXTURE2D)
	GLenum m_Target;
	
   /// la largeur de la texture
   int m_TextureW;
   /// la hauteur de la texture
   int m_TextureH;
};

