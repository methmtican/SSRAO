#include "GLObject.h"
#include "config.h"

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>

#ifdef WITH_IL
  #include <IL/il.h>
  #define ILUT_USE_OPENGL 1
  #include <IL/ilut.h>
#endif

// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

glm::mat4 GLObject::proj_mat;
glm::mat4 GLObject::view_mat;
GLuint    GLObject::uni_proj;
GLuint    GLObject::uni_mv;
GLuint    GLObject::uni_tex_color;
GLuint    GLObject::uni_tex_normal;
GLuint    GLObject::uni_tex_pos;

void GLObject::setProjectionMatrix( float fovy, float aspect, float z_near, float z_far )
{
  proj_mat = glm::perspective( fovy, aspect, z_near, z_far );
/*
    for( int i=0; i<16; i++ )
      projection[i] = 0.0;

    float f = 1.0 / tan( fovy * (M_PI / 360.0 ));
    projection[0] = f / aspect;
    projection[5] = f;
    projection[10] = (z_far + z_near) / (z_near - z_far);
    projection[14] = (2.0 * z_far  * z_near ) / (z_near - z_far);
    projection[11] = -1.0;
    projection[15] = 0.0;
*/
}

void GLObject::setViewMatrix( glm::vec3& eye, glm::vec3& center, glm::vec3& up )
{
  view_mat = glm::lookAt( eye, center, up );
} 

char* GLObject::readShaderFile( const char* filename )
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

void GLObject::printShaderLog( int shader, const char* filename, const char* mesg )
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

void GLObject::printProgramLog( int shader, const char* filename )
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

GLObject::GLObject()
{
    simple_lazy_state = false;

    texture_id = 0;
    num_faces = 0;

    vao = 0;
}

void GLObject::loadTexture( const char* filename )
{

#ifdef WITH_IL

#define WITH_ILUT 1
#ifdef WITH_ILUT
    glActiveTexture( GL_TEXTURE0 );
    texture_id = ilutGLLoadImage( (char*)filename );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    if( !texture_id )
        printf ("Error reading image file\n" );

    printf( "Loaded image: %s\n", filename );
    printf( "  resolution: %i x %i\n", ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ));
    printf( "  num channels: %i\n", ilGetInteger( IL_IMAGE_CHANNELS ));
    printf( "  bytes per channel: %i\n", ilGetInteger( IL_IMAGE_BPC ));
    printf( "  size of data: %i\n", ilGetInteger( IL_IMAGE_SIZE_OF_DATA ));
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
#else
    ILuint image;
    ilGenImages( 1, &image );
    ilBindImage( image );
    ilLoadImage( (char*) filename );

    glGenTextures( 1, &texture_id );
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8,
                  ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, ilGetData() );

    printf( "Loaded image: \n" );
    printf( "  resolution: %i x %i\n", ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ));
    printf( "  num channels: %i\n", ilGetInteger( IL_IMAGE_CHANNELS ));
    printf( "  bytes per channel: %i\n", ilGetInteger( IL_IMAGE_BPC ));
    printf( "  size of data: %i\n", ilGetInteger( IL_IMAGE_SIZE_OF_DATA ));
#endif

#else
    glGenTextures( 1, &texture_id );
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );


    FILE *fd = fopen( filename, "rb" ) ;

    unsigned int dim_u ;
    unsigned int dim_v ;
    unsigned int max_value ;

    char line[128] ;

    // first line is the "magic" header
    fgets( line, 128, fd ) ;

    if( strcmp( line, "P6\n" ) != 0 )
    {
      printf( "Error: Loading PPM Texture: %s, first line is not magic identifier\n", filename ) ;
      return ;
    }

    // second line is just a comment
    fgets( line, 128, fd ) ;

    // third line is the dimensions of the texture
    fgets( line, 128, fd ) ;
    sscanf( line, "%u %u\n", &dim_u, &dim_v ) ;

    //fourth line is the max color component value
    fgets( line, 128, fd ) ;
    sscanf( line, "%u\n", &max_value ) ;

    //rest of file is the pixel data

    unsigned char* data = (unsigned char*)malloc( dim_u * dim_v * 3 * sizeof( unsigned char )) ;
    fread( data, sizeof( unsigned char ) * dim_u * dim_v * 3, 1, fd ) ;

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8,
                  dim_u, dim_v, 0,
                  GL_RGB, GL_UNSIGNED_BYTE, data );

#endif
}

void GLObject::setTexture( GLuint tex_id, GLuint unit )
{
  //glActiveTexture( GL_TEXTURE0+unit );
  //glBindTexture( GL_TEXTURE_2D, tex_id );
}

void GLObject::loadShader( const char* filename )
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

    glBindFragDataLocation( prog, 0, "color" );
    glBindFragDataLocation( prog, 1, "gbuff1" );
    glBindFragDataLocation( prog, 2, "gbuff2" );

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

    simple_lazy_state = true ;
}

void GLObject::loadInterleaved( GLfloat* data, int num_verticies,
                                GLuint* idata,  int num_indicies )
{
    // FIXME: this currently assumes the data is interleaved:
    //          3 component position
    //          2 component texture coordinate
    //          3 component color
    int num_components = 3 + 2 + 3;
    int data_size  = num_verticies * num_components;
    int idata_size = num_indicies;

    if( !vao )
      glGenVertexArrays(1, &vao );
    glBindVertexArray( vao );

    GLuint vbuffer;
    glGenBuffers( 1, &vbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vbuffer );
    glBufferData( GL_ARRAY_BUFFER, data_size*sizeof(float), data, GL_STATIC_DRAW );

    GLuint ibuffer;
    glGenBuffers( 1, &ibuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, idata_size*sizeof(GLuint), idata, GL_STATIC_DRAW );

    glVertexAttribPointer( attrib_position, 3, GL_FLOAT, 0, 8*sizeof(float), (void*)0 );
    glEnableVertexAttribArray( attrib_position );

    glVertexAttribPointer( attrib_texcoord, 2, GL_FLOAT, 0, 8*sizeof(float), (void*)(3*sizeof(float)) );
    glEnableVertexAttribArray( attrib_texcoord );

    glVertexAttribPointer( attrib_color, 3, GL_FLOAT, 0, 8*sizeof(float), (void*)(5*sizeof(float)) );
    glEnableVertexAttribArray( attrib_color );

    glBindVertexArray( 0 );
}

void GLObject::loadVertexAttribute( GLfloat* data, int num_vertices, Attribute attribute )
{
    if( !vao )
      glGenVertexArrays(1, &vao );
    glBindVertexArray( vao );

    GLint num_comps = 0;
    GLint attrib = 0;
    switch( attribute )
    {
    case ATTRIBUTE_POSITION:
      num_comps = 3;
      attrib = attrib_position;
      break;
    case ATTRIBUTE_NORMAL:
      num_comps = 3;
      attrib = attrib_normal;
      break;
    case ATTRIBUTE_COLOR:
      num_comps = 4;
      attrib = attrib_color;
      break;
    case ATTRIBUTE_TEXCOORD:
      num_comps = 2;
      attrib = attrib_texcoord;
      break;
    };

    // FIXME: if loadVertexAttribute was called already for an attribute, we should reuse the previous buffer
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, num_comps*num_vertices*sizeof(float), data, GL_STREAM_DRAW );

    glEnableVertexAttribArray( attrib );
    glVertexAttribPointer( attrib, num_comps, GL_FLOAT, 0, 0, (void*)0 );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void GLObject::loadFaces( GLuint* faces, int _num_faces )
{
    if( !vao )
      glGenVertexArrays(1, &vao );
    glBindVertexArray( vao );

    GLuint buffer;
    // FIXME: this currently assumes triangle faces
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 3*_num_faces*sizeof(GLuint), faces, GL_STREAM_DRAW );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    num_faces = _num_faces;
}

void GLObject::setModelMatrix( glm::mat4& mat )
{
    model_mat = mat;
}

void GLObject::setSelectable( bool sel, void (*cb)(void) )
{
    select_cb = sel ? cb : 0 ;
}

void GLObject::select()
{
  if( select_cb ) select_cb();
}

void GLObject::draw()
{
    glUseProgram( prog );

    if( uni_proj != -1 )       glUniformMatrix4fv( uni_proj, 1, false, glm::value_ptr( proj_mat ));
    if( uni_mv != -1 )         glUniformMatrix4fv( uni_mv, 1, false, glm::value_ptr( view_mat * model_mat ));
    //glUniform1i( uni_texture, 0 );
    //if( uni_tex_color != -1 )  glUniform1i( uni_tex_color, 0 );
    //if( uni_tex_normal != -1 ) glUniform1i( uni_tex_normal,1 );
    //if( uni_tex_pos != -1 )    glUniform1i( uni_tex_pos,   2 );

    glBindVertexArray( vao );

    glDrawElements( GL_TRIANGLES, num_faces*3, GL_UNSIGNED_INT, (void*)0 );
}
