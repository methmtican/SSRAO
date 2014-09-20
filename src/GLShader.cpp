#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <GL/glew.h>

#include "GLShader.h"

using namespace std;

char* GLShader::readShaderFile( const char* filename )
{
  ifstream file( filename ) ;
  if( !file.good() )
    printf( "Error: Couldn't open %s for reading!\n", filename );

  file.seekg( 0, ios::end );
  int length = file.tellg() ;
  file.seekg( 0, ios::beg );

  char* buff = new char[ length+1 ];
  file.read( buff, length );
  file.close();

  buff[length] = 0;
  return buff;
}

void GLShader::printShaderLog( int shader, const char* filename, const char* mesg )
{
  GLint error ;
  glGetShaderiv( shader, GL_COMPILE_STATUS, &error );

  if( !error )
  {
    int logLength;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
    if( logLength > 0 )
    {
      char* buffer = new char[ logLength ];
      int tmp;
      glGetShaderInfoLog( shader, logLength, &tmp, buffer );
      printf( "ERROR: Trying to compile %s shader: %s\n%s\n", mesg, filename, buffer );
      delete[] buffer;
    }
  }
}

void GLShader::printProgramLog( int shader, const char* filename )
{
  GLint error ;
  glGetProgramiv( shader, GL_LINK_STATUS, &error );

  if( !error )
  {
    int logLength;
    glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &logLength );
    if( logLength > 0 )
    {
      char* buffer = new char[ logLength ];
      int tmp;
      glGetProgramInfoLog( shader, logLength, &tmp, buffer );
      printf( "ERROR: Trying to link program: %s\n%s\n", filename, buffer );
      delete[] buffer;
    }
  }
}

GLShader::GLShader( const char* filename )
{

    const int STRING_LENGTH = 32 ;
    char* shader_src[2];

    shader_src[0] = new char[ STRING_LENGTH ];
    //printf( "Reading shader file\n" );
    shader_src[1] = readShaderFile( filename );

    // Create the shaders
    //printf( "Creating Vertex Shader\n" );
    fflush(stdout);
    int shader_vert = glCreateShader( GL_VERTEX_SHADER );
    strncpy( shader_src[0], "#define VERTEX_SHADER\n", STRING_LENGTH );
    glShaderSource( shader_vert, 2, (const char**)shader_src, NULL );
    glCompileShader( shader_vert );
    printShaderLog( shader_vert, filename, "VERTEX" );

    int shader_geom = -1;
    if( strstr( shader_src[1], "GEOMETRY_SHADER" ) != 0 )
    {
        //printf( "Creating Geometry Shader\n" );
        shader_geom = glCreateShader( GL_GEOMETRY_SHADER );
        strncpy( shader_src[0], "#define GEOMETRY_SHADER\n", STRING_LENGTH );
        glShaderSource( shader_geom, 2, (const char**)shader_src, NULL );
        glCompileShader( shader_geom );
        printShaderLog( shader_geom, filename, "GEOMETRY" );
    }

    //printf( "Creating Fragment Shader\n" );
    int shader_frag = glCreateShader( GL_FRAGMENT_SHADER );
    strncpy( shader_src[0], "#define FRAGMENT_SHADER\n", STRING_LENGTH );
    glShaderSource( shader_frag, 2, (const char**)shader_src, NULL );
    glCompileShader( shader_frag );
    printShaderLog( shader_frag, filename, "FRAGMENT" );

    // Create the Program
    //printf( "Creating Program...\n" );
    prog = glCreateProgram();
    glAttachShader( prog, shader_vert );
    if( shader_geom != -1 ) glAttachShader(prog, shader_geom );
    glAttachShader( prog, shader_frag );

    //glBindFragDataLocation( prog, 0, "color" );
    //glBindFragDataLocation( prog, 1, "gbuff1" );
    //glBindFragDataLocation( prog, 2, "gbuff2" );

    //printf( "Binding attribute locations...\n" );
    glBindAttribLocation( prog, attrib_position, "position" );
    glBindAttribLocation( prog, attrib_normal,   "normal" );
    glBindAttribLocation( prog, attrib_color,    "texcoord" );
    glBindAttribLocation( prog, attrib_texcoord, "color" );

    glLinkProgram( prog );
    glValidateProgram( prog );
    //printf( "Program linked....\n" );
    printProgramLog( prog, filename );

    uni_proj        = glGetUniformLocation( prog, "projection" );
    uni_mv          = glGetUniformLocation( prog, "modelview" );
    uni_texture     = glGetUniformLocation( prog, "texture" );
    uni_tex_color   = glGetUniformLocation( prog, "tex_color" );
    uni_tex_pos     = glGetUniformLocation( prog, "tex_pos" );
    uni_tex_normal  = glGetUniformLocation( prog, "tex_normal" );

    delete[] shader_src[0];
    delete[] shader_src[1];
}

void GLShader::apply()
{
  glUseProgram( prog );
}
