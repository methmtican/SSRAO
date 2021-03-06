// internal
#include "GLObject.h"
#include "GLFrameBuffer.h"
#include "GLMaterial.h"
#include "GLShader.h"
#include "config.h"

// core libraries
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

// assimp libraries
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// GL libraries
#include <GL/glew.h>

#include <GL/freeglut.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

// Conditional DevIL libraries
#ifdef WITH_IL
  #include <IL/il.h>
  #include <IL/ilu.h>
  #ifdef WITH_ILUT
    #include <IL/ilut.h>
  #endif
#endif

int num_models;
GLObject** models;
GLObject*  screen_quad;
GLMaterial* materials;
GLFrameBuffer** fbos;
GLTextureBank texture_bank;

typedef std::vector<glm::mat4> MatrixStack;
MatrixStack matrix_stack;

glm::vec3 eye( 1.5, 7.0, 0.4 );
glm::vec3 center( 0.3, 7.5, 0.4 );
glm::vec3 up( 0.0, 1.0, 0.0 );
glm::vec3 dir;

int res[] = { 800, 800 };

int mouse_button_state;
glm::vec2 mouse_pos;
int ignore_mesh = -1;

void draw()
{
  // GBuffer Pass
  fbos[0] -> bind();

  glEnable( GL_DEPTH_TEST );
  glDepthMask( 1 );

  glClearColor( 0.0, 0.0, 0.0, 0.0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  for( int i=0; i<num_models; i++ )
  {
    if( i == ignore_mesh ) 
      continue;
    models[i] -> draw();
  }

  GLFrameBuffer::unbind();

  // Lighting Pass
  glClearColor( 0.4, 0.4, 1.0, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glDepthMask( 0 );
  glDisable( GL_DEPTH_TEST );

  screen_quad -> draw();

  glutSwapBuffers();
  glutPostRedisplay();
}

void setModelMats( aiNode* node, int level=0 )
{
  glm::mat4 mat = glm::make_mat4( (float*)(&node -> mTransformation) );
  mat = glm::transpose( mat );
  mat = matrix_stack.back() * mat;

  matrix_stack . push_back( mat );

  for( int i=0; i< node -> mNumMeshes; i++ )
    models[ node -> mMeshes[i] ] -> setModelMatrix( mat );

  //printf( "%*s* - %i\n", level, "", node -> mNumChildren );
  for( int i=0; i<node->mNumChildren; i++ )
    setModelMats( node -> mChildren[i], level+1 );

  matrix_stack . pop_back();
}

void loadModels()
{

  const char* path_model  = "sponza.3DS" ;
  
  std::string path  = SSRAO_MODEL_PATH;
              path += path_model ;

  const aiScene* scene = aiImportFile( path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality );   

  printf( "Loaded %s\n", path.c_str() );

  if( !scene )
  {
    printf( "ERROR reading %s!\n", path.c_str() );
    exit(1);
  }

  num_models = scene -> mNumMeshes;
  models = new GLObject*[ num_models ];

  path  = SSRAO_SHADER_PATH;
  //path += "basic.glsl";
  path += "g-buffer.glsl";
  GLShader* scene_shader = new GLShader( path.c_str() );

  // Load all scene materials first
  if( scene -> HasTextures() )
    printf( "Model '%s' has embeded textures. Embeded textures are currently not supported, ignoring textures.\n", path.c_str() );

  materials = new GLMaterial[ scene -> mNumMaterials ];
  printf( "num materials = %i\n", scene -> mNumMaterials );

  for( unsigned int i=0; i < scene -> mNumMaterials; i++ )
  {
    printf( "Loading material %i...\n", i );
    materials[i] . setShader( scene_shader );

    aiString path_mat; 
    aiTextureMapping mapping;
    unsigned int uv_idx;
   
    // FIXME: currently only support a single diffuse texture
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_DIFFUSE, 0, &path_mat, &mapping, &uv_idx ))
    {
      path  = SSRAO_TEXTURE_PATH;
      path += path_mat.data;

      //printf( "  Loading DIFFUSE texture: %s\n", path.c_str() );
      //printf( "    uv index = %u\n", uv_idx );
      //printf( "    mapping mode = %s\n", mapping == aiTextureMapping_UV ? "UV": "NOT UV" );
      materials[i] . setTexture( GLMaterial::TEXTYPE_DIFFUSE,
                                 texture_bank.getTexture( path ),
                                 uv_idx );
    }
    int idx = 1;
    while( idx < 8 )
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_DIFFUSE, idx++, &path_mat, &mapping, &uv_idx ))
      printf( "Material %i has more than one diffuse texture which is unsupported!\n", i );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_SPECULAR, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: SPECULAR( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_AMBIENT, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: AMBIENT( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_EMISSIVE, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: EMISSIVE( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_HEIGHT, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: HEIGHT( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_NORMALS, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: NORMALS( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_SHININESS, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: SHININESS( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_OPACITY, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: OPACITY( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_DISPLACEMENT, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: DISPLACEMENT( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_LIGHTMAP, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: LIGHTMAP( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_REFLECTION, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: REFLECTION( %s )\n", i, path_mat.data );
    if( AI_SUCCESS == scene->mMaterials[i]->GetTexture( aiTextureType_UNKNOWN, 0, &path_mat ))
      printf( "Material %i has unsupported texture type: UNKNOWN( %s )\n", i, path_mat.data );
  }

  // Load all scene meshes second
  printf( "  num meshes: %i\n", num_models );
  for( int i=0; i< num_models; i++ )
  {
    const aiMesh* mesh = scene -> mMeshes[i];

    models[i] = new GLObject();
    models[i] ->  setMaterial( &materials[ mesh -> mMaterialIndex ] );
    
    //printf( "  mesh %i - \"%s\":\n", i, mesh -> mName . C_Str() );
    //printf( "    num faces: %i\n", mesh -> mNumFaces );
    //printf( "    num verts: %i\n", mesh -> mNumVertices );
    //printf( "    material index: %i\n", mesh -> mMaterialIndex );

    // create face array
    GLuint* faces = new GLuint[ 3*mesh -> mNumFaces ];
    for( int f=0; f < mesh -> mNumFaces; f++ )
    {
      aiFace* face = &mesh -> mFaces[f];
      memcpy( &faces[f*3], face -> mIndices, 3*sizeof(GLuint));
    }
 
    models[i] -> loadFaces( faces, mesh -> mNumFaces );
    delete[] faces;

    if( mesh -> HasPositions() )
    {
      //printf( "    has positions\n" );
      models[i] -> loadVertexAttribute( (float*)mesh -> mVertices, mesh -> mNumVertices,
                                        GLShader::ATTRIBUTE_POSITION );
    }

    if( mesh -> HasNormals() )
    {
      //printf( "    has normals\n" );
      models[i] -> loadVertexAttribute( (float*)mesh -> mNormals, mesh -> mNumVertices,
                                        GLShader::ATTRIBUTE_NORMAL );
    }

    int uvidx = 0;
    while( mesh -> HasTextureCoords(uvidx))
    {
      //printf( "    has tex coord %i\n", uvidx );
      float* data = new float[ 2*mesh -> mNumVertices ];
      for( int tc=0; tc < mesh -> mNumVertices; tc++ )
      {
        data[2*tc]   = mesh -> mTextureCoords[uvidx][tc].x;
        data[2*tc+1] = mesh -> mTextureCoords[uvidx][tc].y;
      }
      models[i] -> loadVertexAttribute( data, mesh -> mNumVertices,
                                        (GLShader::Attribute)(GLShader::ATTRIBUTE_TEXCOORD0+uvidx) );
      delete[] data;
      uvidx++;
    }
    

    if( mesh -> HasVertexColors(0))
    {
      //printf( "    has colors\n" );
      float* data = new float[ 4*mesh -> mNumVertices ];
      for( int c=0; c < mesh -> mNumVertices; c++ )
      {
        data[2*c]   = mesh -> mColors[0][c].r;
        data[2*c+1] = mesh -> mColors[1][c].g;
        data[2*c+2] = mesh -> mColors[2][c].b;
        data[2*c+3] = mesh -> mColors[2][c].a;
      }
      models[i] -> loadVertexAttribute( data, mesh -> mNumVertices,
                                        GLShader::ATTRIBUTE_COLOR );
    }
  }

  setModelMats( scene -> mRootNode );

  // Create Screen Quad Model
  GLfloat positions[] = { -1.0, -1.0, 0.0,
                          -1.0,  1.0, 0.0,
                           1.0,  1.0, 0.0,
                           1.0, -1.0, 0.0 };
  screen_quad = new GLObject();
  screen_quad -> loadVertexAttribute( positions, 4, GLShader::ATTRIBUTE_POSITION );

  GLuint faces[] = { 0, 2, 1, 0, 3, 2 };
  screen_quad -> loadFaces( faces, 2 );

  GLMaterial* quad_mat = new GLMaterial();
  quad_mat -> setTexture( GLMaterial::TEXTYPE_GBUFFER0, fbos[0] -> getColorAttachmentTexture( 0 ) );
  quad_mat -> setTexture( GLMaterial::TEXTYPE_GBUFFER1, fbos[0] -> getColorAttachmentTexture( 1 ) );
  quad_mat -> setTexture( GLMaterial::TEXTYPE_GBUFFER2, fbos[0] -> getColorAttachmentTexture( 2 ) ); 

  path  = SSRAO_SHADER_PATH;
  //path += "basic.glsl";
  path += "lighting.glsl";
  GLShader* quad_shader = new GLShader( path.c_str() );
  quad_mat -> setShader( quad_shader );
  screen_quad -> setMaterial( quad_mat );

}

void debugOutput( GLenum source, GLenum type, GLuint id,
                  GLenum severity, GLsizei length, const GLchar* message, void* userParam )
{
   if( severity == GL_DEBUG_SEVERITY_HIGH ||
       severity == GL_DEBUG_SEVERITY_MEDIUM )
     printf( "GL DEBUG: %s\n", message );
}

void init()
{
  GLenum error = glewInit();
  if( GLEW_OK != error )
    printf( "ERROR initializing glew: %s\n", glewGetErrorString( error ));

#ifdef WITH_IL
  ilInit();
  iluInit();
#ifdef WITH_ILUT
  ilutInit();
  ilutRenderer( ILUT_OPENGL );
  ilutEnable( ILUT_OPENGL_CONV );
#endif
#endif

  // initialize the matrix stack with an identity matrix
  matrix_stack.push_back( glm::mat4() );

  glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC) glutGetProcAddress("glGetUniformBlockIndex");
  glUniformBlockBinding  = (PFNGLUNIFORMBLOCKBINDINGPROC)  glutGetProcAddress("glUniformBlockBinding");
  glGenVertexArrays      = (PFNGLGENVERTEXARRAYSPROC)      glutGetProcAddress("glGenVertexArrays");
  glBindVertexArray      = (PFNGLBINDVERTEXARRAYPROC)      glutGetProcAddress("glBindVertexArray");
  glBindBufferRange      = (PFNGLBINDBUFFERRANGEPROC)      glutGetProcAddress("glBindBufferRange");
  glDeleteVertexArrays   = (PFNGLDELETEVERTEXARRAYSPROC)   glutGetProcAddress("glDeleteVertexArrays");
  glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) glutGetProcAddress("glDebugMessageCallback");
  glGenFramebuffers      = (PFNGLGENFRAMEBUFFERSPROC)      glutGetProcAddress("glGenFramebuffers");
  glDeleteFramebuffers   = (PFNGLDELETEFRAMEBUFFERSPROC)   glutGetProcAddress("glDeleteFramebuffers");
  glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) glutGetProcAddress("glFramebufferTexture2D");
  glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) glutGetProcAddress("glCheckFramebufferStatus");
  glGenRenderbuffers     = (PFNGLGENRENDERBUFFERSPROC)     glutGetProcAddress("glGenRenderbuffers");
  glBindRenderbuffer     = (PFNGLBINDRENDERBUFFERPROC)     glutGetProcAddress("glBindRenderbuffer");
  glRenderbufferStorage  = (PFNGLRENDERBUFFERSTORAGEPROC)  glutGetProcAddress("glRenderbufferStorage");
  glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) glutGetProcAddress("glFramebufferRenderbuffer"); 
  glBindFramebuffer      = (PFNGLBINDFRAMEBUFFERPROC)      glutGetProcAddress("glBindFramebuffer");
  
  GLObject::setProjectionMatrix( 60.0 * M_PI / 180.0 , 1.0, 0.1, 100.0 );
  GLObject::setViewMatrix( eye, center, up );
 
  glViewport( 0, 0, res[0], res[1] );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_MULTISAMPLE );
 
  glEnable( GL_CULL_FACE );
  glDisable( GL_BLEND );

#ifdef WITH_GL_DEBUG
  glEnable( GL_DEBUG_OUTPUT );
  glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
  glDebugMessageCallback( debugOutput, 0 );
#endif

  // initialize the look direction;
  dir = glm::normalize( center - eye );
  center = eye + dir;

  // initialize the g-buffer
  fbos    = new GLFrameBuffer*[1];
  fbos[0] = new GLFrameBuffer( res );
  fbos[0] -> pushColorAttachment( GL_RGBA ); // COLORS
  fbos[0] -> pushColorAttachment( GL_RGBA32F ); // NORMALS (xyz) / UNUSED (a)
  fbos[0] -> pushColorAttachment( GL_RGBA32F ); // POSITIONS
  fbos[0] -> pushDepthStencilAttachment( GL_DEPTH_COMPONENT24 ); // Z buffer

  // bind the system fbo
  GLFrameBuffer::unbind();
}

void mouseButtons( int button, int state, int x, int y )
{
  if( button == GLUT_LEFT_BUTTON )
  {
    mouse_button_state = state;

    if( state == GLUT_DOWN )
      mouse_pos = glm::vec2( x, y );
  }
}

void mouseMotion( int x, int y )
{
  if( mouse_button_state == GLUT_DOWN )
  {
    glm::vec2 new_pos( x, y );
    glm::vec2 delta = new_pos - mouse_pos;
    delta /= 1000;
   
    glm::vec3 side_dir = glm::normalize( glm::cross( up, dir ) );
    glm::vec3 up_dir   = glm::normalize( glm::cross( side_dir, dir ) );

    center -= eye;
    center = glm::rotate( center, delta.x, up_dir );
    center = glm::rotate( center, -delta.y, side_dir );
    center += eye;

    dir = glm::normalize( center - eye );
    center = eye + dir;

    GLObject::setViewMatrix( eye, center, up );


    mouse_pos = new_pos;
  }
}

void keyboard( unsigned char key, int x, int y )
{
  glm::vec3 side_dir;
  switch( key )
  {
    case 'w':
      eye += dir * 0.2f;
      break;
    case 's':
      eye -= dir * 0.2f;
      break;
    case 'a':
      side_dir = glm::normalize( glm::cross( up, -dir ));
      eye -= side_dir * 0.2f;
      break;
    case 'd':
      side_dir = glm::normalize( glm::cross( up, -dir ));
      eye += side_dir * 0.2f;
      break;
    case 'p':
      ignore_mesh++;
      printf( "Ignoring mesh %i\n", ignore_mesh );
      break;
    case 'o':
      if( ignore_mesh > -1 )
      {
        ignore_mesh--;
        printf( "Ignoring mesh %i\n", ignore_mesh );
      }
      break;
  };
  center = eye + dir;
  GLObject::setViewMatrix( eye, center, up );
}

void segFaultHandler( int sig )
{
  void *array[10];
  size_t size;

  size = backtrace( array, 10 );
  fprintf( stderr, "Error: signal %d\n", sig );
  backtrace_symbols_fd( array, size, STDERR_FILENO );

  exit(1);
}

int main( int argc, char **argv )
{
  glutInit(&argc, argv);

  glutInitDisplayMode( GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE );

  //glutInitContextVersion (3, 3);
  glutInitContextFlags (GLUT_COMPATIBILITY_PROFILE );

  glutInitWindowPosition(100,100);
  glutInitWindowSize( res[0], res[1] );
  glutCreateWindow( "SSR and SSAO Demo");
        
  glutDisplayFunc( draw );
  //glutReshapeFunc( );
  glutIdleFunc( draw );

  glutKeyboardFunc( keyboard );
  glutMouseFunc( mouseButtons );
  glutMotionFunc( mouseMotion );
  
  signal( SIGSEGV, segFaultHandler );

  printf( "Initializing Renderer...\n" );
  init();

  glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS );

  printf( "Loading Model...\n" );
  loadModels();

  printf( "Drawing...\n" );
  glutMainLoop();
}
