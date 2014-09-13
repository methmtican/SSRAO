#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>
#include "window.h"
#include "GLObject.h"

GLObject* model;

void draw()
{

}

void loadModel()
{
  model = new GLObject();
  model -> loadShader( "basic.glsl" );
  
  const aiScene* scene = aiImportFile( "", aiProcessPreset_TargetRealtime_MaxQuality );   
}

void init()
{
  glewInit();

}

int main()
{
  createWindow( "SSR and SSAO Demo", 800, 800 );

  loadModel();

  while(1)
    draw();
}
