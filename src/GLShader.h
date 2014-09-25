#ifndef GLSHADER_H_
#define GLSHADER_H_

#include <GL/gl.h>
#include <glm/glm.hpp>

class GLShader
{
public:
  enum Attribute { ATTRIBUTE_POSITION=0, ATTRIBUTE_NORMAL=1, ATTRIBUTE_COLOR=2, 
                   ATTRIBUTE_TEXCOORD0=3,
                   ATTRIBUTE_TEXCOORD1=4,
                   ATTRIBUTE_TEXCOORD2=5,
                   ATTRIBUTE_TEXCOORD3=6,
                   ATTRIBUTE_TEXCOORD4=7,
                   ATTRIBUTE_TEXCOORD5=8,
                   ATTRIBUTE_TEXCOORD6=9,
                   ATTRIBUTE_TEXCOORD7=10,
 };

  // FIXME: These members shouldn't be public
  static const GLint attrib_position  = (GLint)ATTRIBUTE_POSITION;
  static const GLint attrib_normal    = (GLint)ATTRIBUTE_NORMAL;
  static const GLint attrib_color     = (GLint)ATTRIBUTE_COLOR;
  static const GLint attrib_texcoord0 = (GLint)ATTRIBUTE_TEXCOORD0;
  static const GLint attrib_texcoord1 = (GLint)ATTRIBUTE_TEXCOORD1;
  static const GLint attrib_texcoord2 = (GLint)ATTRIBUTE_TEXCOORD2;
  static const GLint attrib_texcoord3 = (GLint)ATTRIBUTE_TEXCOORD3;
  static const GLint attrib_texcoord4 = (GLint)ATTRIBUTE_TEXCOORD4;
  static const GLint attrib_texcoord5 = (GLint)ATTRIBUTE_TEXCOORD5;
  static const GLint attrib_texcoord6 = (GLint)ATTRIBUTE_TEXCOORD6;
  static const GLint attrib_texcoord7 = (GLint)ATTRIBUTE_TEXCOORD7;

  GLuint uni_texture;
  GLuint uni_tex_color;
  GLuint uni_tex_normal;
  GLuint uni_tex_pos;
  GLuint uni_proj;
  GLuint uni_mv;

private:
  GLuint prog;         // The shader to use

  char* readShaderFile( const char* filename );
  void printShaderLog( int shader, const char* filename, const char* mesg );
  void printProgramLog( int shader, const char* filename );

public:
  GLShader( const char* filename );

  void apply();
};


#endif /* GLSHADER_H_ */
