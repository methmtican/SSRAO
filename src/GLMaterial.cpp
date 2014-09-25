#include <GL/glew.h>

#include "GLMaterial.h"
#include "config.h"

#ifdef WITH_IL
  #include <IL/il.h>
  #include <IL/ilut.h>
#endif

GLMaterial::GLMaterial()
{
  for( int i=0; i<NUM_TEXTYPE; i++ )
    textures[i] = 0;
}

void GLMaterial::apply()
{
  if( shader )
    shader -> apply();

  if( textures[ TEXTYPE_DIFFUSE ] )
  {
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textures[ TEXTYPE_DIFFUSE ] );
  }
}

GLuint GLTextureBank::loadTexture( const char* filename )
{
    GLuint texture_id;

#ifdef WITH_IL

#ifdef WITH_ILUT
    glActiveTexture( GL_TEXTURE0 );

    texture_id = ilutGLLoadImage( (char*) filename );

    if( !texture_id )
        printf( "Error creating GL image file\n" );
    ILenum error;
    while( ( error = ilGetError()) != IL_NO_ERROR )
      printf( "%d: %s\n", error, iluErrorString(error));

    glBindTexture( GL_TEXTURE_2D, texture_id );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    printf( "Loaded image: %s\n", filename );
    printf( "  resolution: %i x %i\n", ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ));
    printf( "  num channels: %i\n", ilGetInteger( IL_IMAGE_CHANNELS ));
    printf( "  bytes per channel: %i\n", ilGetInteger( IL_IMAGE_BPC ));
    printf( "  size of data: %i\n", ilGetInteger( IL_IMAGE_SIZE_OF_DATA ));
#else
    ILuint image;
    ilGenImages( 1, &image );
    ilBindImage( image );
    ilLoadImage( (char*) filename );

    ILenum error;
    while( ( error = ilGetError()) != IL_NO_ERROR )
      printf( "%d: %s\n", error, iluErrorString(error));

    glGenTextures( 1, &texture_id );
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    printf( "Loaded image: %s\n", filename );
    printf( "  resolution: %i x %i\n", ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ));
    printf( "  num channels: %i\n", ilGetInteger( IL_IMAGE_CHANNELS ));
    printf( "  bytes per channel: %i\n", ilGetInteger( IL_IMAGE_BPC ));
    printf( "  size of data: %i\n", ilGetInteger( IL_IMAGE_SIZE_OF_DATA ));

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8,
                  ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, ilGetData() );

#endif

#else
    glGenTextures( 1, &texture_id );
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );


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
      return 0;
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

    return texture_id;
}


GLTextureBank::~GLTextureBank()
{
  TextureList::iterator itr = bank.begin();

  while( itr != bank.end() )
  {
    glDeleteTextures( 1, &itr -> second );
    itr++;
  }
}

GLuint GLTextureBank::getTexture( std::string filename )
{
  GLuint id = bank[ filename ]; 

  if( !id )
  {
    id = loadTexture( filename.c_str() );
    bank[ filename ] = id;
  }
  
  return id;
}

