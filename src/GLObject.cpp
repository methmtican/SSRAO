#include "GLObject.h"
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

using namespace std;

float  GLObject::projection[16];
GLuint GLObject::uni_proj;

void GLObject::setProjectionMatrix( float fovy, float aspect, float z_near, float z_far )
{
    for( int i=0; i<16; i++ )
      projection[i] = 0.0;

    float f = 1.0 / tan( fovy * (M_PI / 360.0 ));
    projection[0] = f / aspect;
    projection[5] = f;
    projection[10] = (z_far + z_near) / (z_near - z_far);
    projection[14] = (2.0 * z_far  * z_near ) / (z_near - z_far);
    projection[11] = -1.0;
    projection[15] = 0.0;
}

char* GLObject::readShaderFile( const char* filename )
{
  ifstream file( filename ) ;
  if( !file.good() )
    printf( "Error: Couldn't open %s for reading!\n", filename );

  printf( "seek end\n" );
  file.seekg( 0, ios::end );
  int length = file.tellg() ;
  printf( "seek beg\n" );
  file.seekg( 0, ios::beg );

  char* buff = new char[ length+1 ];
  printf( "read\n" );
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
    num_quads = 0;

    rotation = 0.0;
    translation[0] = 0.0;
    translation[1] = 0.0;
    translation[2] = 0.0;
    scale = 1.0;
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

    //for( int i=240*800*3; i<241*800*3; i+=3 )
    //    printf( "  image[%i] = ( %i, %i, %i )\n", i, data[i], data[i+1], data[i+2] );
#endif
}

void GLObject::loadShader( const char* filename )
{

    const int STRING_LENGTH = 32 ;
    char* shader_src[2];

    shader_src[0] = new char[ STRING_LENGTH ];
    printf( "Reading shader file\n" );
    shader_src[1] = readShaderFile( filename );

    // Create the shaders
    printf( "Creating Vertex Shader" );
    int shader_vert = glCreateShader( GL_VERTEX_SHADER );
    strncpy( shader_src[0], "#define VERTEX_SHADER\n", STRING_LENGTH );
    glShaderSource( shader_vert, 2, (const char**)shader_src, NULL );
    glCompileShader( shader_vert );
    printShaderLog( shader_vert, filename, "VERTEX" );

    int shader_geom = -1;
    if( strstr( shader_src[1], "GEOMETRY_SHADER" ) != 0 )
    {
        printf( "Creating Geometry Shader" );
        shader_geom = glCreateShader( GL_GEOMETRY_SHADER );
        strncpy( shader_src[0], "#define GEOMETRY_SHADER\n", STRING_LENGTH );
        glShaderSource( shader_geom, 2, (const char**)shader_src, NULL );
        glCompileShader( shader_geom );
        printShaderLog( shader_geom, filename, "GEOMETRY" );
    }

    printf( "Creating Fragment Shader" );
    int shader_frag = glCreateShader( GL_FRAGMENT_SHADER );
    strncpy( shader_src[0], "#define FRAGMENT_SHADER\n", STRING_LENGTH );
    glShaderSource( shader_frag, 2, (const char**)shader_src, NULL );
    glCompileShader( shader_frag );
    printShaderLog( shader_frag, filename, "FRAGMENT" );

    // Create the Program
    printf( "Creating Program...\n" );
    prog = glCreateProgram();
    glAttachShader( prog, shader_vert );
    if( shader_geom != -1 ) glAttachShader(prog, shader_geom );
    glAttachShader( prog, shader_frag );

    //glBindFragDataLocation( prog, 0, "fragData" );
        printf( "Binding attribute locations...\n" );
    glBindAttribLocation( prog, 0, "position" );
    glBindAttribLocation( prog, 1, "texcoord" );
    glBindAttribLocation( prog, 2, "color" );

    glLinkProgram( prog );
    printf( "Program linked....\n" );
    printProgramLog( prog, filename );

    uni_proj        = glGetUniformLocation( prog, "projection" );
    uni_texture     = glGetUniformLocation( prog, "texture" );
    uni_rotation    = glGetUniformLocation( prog, "rotation" );
    uni_translation = glGetUniformLocation( prog, "translation" );
    uni_scale       = glGetUniformLocation( prog, "scale" );

    delete[] shader_src[0];
    delete[] shader_src[1];

    simple_lazy_state = true ;
}

void GLObject::loadGeometry( GLfloat* data, int num_verticies,
                             GLuint* idata,  int num_indicies )
{
    int num_components = 3 + 2 + 3;
    int data_size  = num_verticies * num_components;
    int idata_size = num_indicies;

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

    num_quads = num_indicies / 4;

    // Create an axis aligned bounding box
    center[0] = 0.0;
    center[1] = 0.0;
    center[2] = 0.0;

    for( int i=0; i<data_size; i+=num_components )
    {
      center[0] += data[i+0];
      center[1] += data[i+1];
      center[2] += data[i+2];
    }

    center[0] /= num_verticies;
    center[1] /= num_verticies;
    center[2] /= num_verticies;
  
    size[0] = 0.0;
    size[1] = 0.0;
    size[2] = 0.0;

    for( int i=0; i<data_size; i+=num_components )
    {
      size[0] = max( size[0], fabsf( data[i+0] - center[0] ));
      size[1] = max( size[1], fabsf( data[i+1] - center[1] ));
      size[2] = max( size[2], fabsf( data[i+2] - center[2] ));
    }

    // Don't allow a size less than 0.01
    size[0] = fmaxf( size[0], 0.01 );
    size[1] = fmaxf( size[1], 0.01 );
    size[2] = fmaxf( size[2], 0.01 );
}

void GLObject::setRotation( float rad )
{
    rotation = rad;
}

void GLObject::setTranslation( float vec[] )
{
    translation[0] = vec[0];
    translation[1] = vec[1];
    translation[2] = vec[2];
}

void GLObject::setScale( float s )
{
    scale = s;
}

void GLObject::setSelectable( bool sel, void (*cb)(void) )
{
    select_cb = sel ? cb : 0 ;
}

void GLObject::select()
{
  if( select_cb ) select_cb();
}

bool GLObject::contains( float pt[3] )
{
/*
  printf( "contains()\n" );
  printf( "  pt     = ( %f, %f, %f )\n", pt[0], pt[1], pt[2] );
  printf( "  center = ( %f, %f, %f )\n", center[0], center[1], center[2] );
  printf( "  size   = ( %f, %f, %f )\n", size[0], size[1], size[2] );
  printf( "  contains = ( %s, %s, %s )\n", fabsf( pt[0] - center[0] ) <= size[0] ? "TRUE" : "FALSE",
                                           fabsf( pt[1] - center[1] ) <= size[1] ? "TRUE" : "FALSE",
                                           fabsf( pt[2] - center[2] ) <= size[2] ? "TRUE" : "FALSE" );
*/
  return ( fabsf( pt[0] - center[0] ) <= size[0] ) &&
         ( fabsf( pt[1] - center[1] ) <= size[1] ) &&
         ( fabsf( pt[2] - center[2] ) <= size[2] );
}

void GLObject::draw()
{
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glUseProgram( prog );

    glUniformMatrix4fv( uni_proj, 1, false, projection );
    glUniform1i( uni_texture, 0 );
    glUniform1f( uni_rotation, rotation );
    glUniform3fv( uni_translation, 3, translation );
    glUniform1f( uni_scale, scale );

    glBindVertexArray( vao );

    glDrawElements( GL_QUADS, num_quads*4, GL_UNSIGNED_INT, (void*)0 );
}
