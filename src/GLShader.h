#ifndef GLSHADER_H_
#define GLSHADER_H_

#include <GL/gl.h>
#include <glm/glm.hpp>

class GLShader
{
public:
  enum Attribute { ATTRIBUTE_POSITION=0, ATTRIBUTE_NORMAL=1, ATTRIBUTE_COLOR=2, ATTRIBUTE_TEXCOORD=3 };

  // FIXME: These members shouldn't be public
  static const GLint attrib_position = (GLint)ATTRIBUTE_POSITION;
  static const GLint attrib_normal   = (GLint)ATTRIBUTE_NORMAL;
  static const GLint attrib_color    = (GLint)ATTRIBUTE_COLOR;
  static const GLint attrib_texcoord = (GLint)ATTRIBUTE_TEXCOORD;

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
