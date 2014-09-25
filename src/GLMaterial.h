#ifndef __GL_MATERIAL_H__
#define __GL_MATERIAL_H__

#include <glm/glm.hpp>
#include <GL/gl.h>
#include <map>
#include <string>
#include "GLShader.h"

class GLShader;

class GLMaterial
{
public:
  
  enum TexType { TEXTYPE_DIFFUSE  = 0, 
                 TEXTYPE_GBUFFER0 = 1, 
                 TEXTYPE_GBUFFER1 = 2,
                 TEXTYPE_GBUFFER2 = 3,
                 NUM_TEXTYPE      = 4 };

private:

  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 emission;
  glm::vec4 specular;

  float shininess;
  float shininess_strength;

  GLuint textures[ NUM_TEXTYPE ];
  GLuint texture_uvs[ NUM_TEXTYPE ];

  GLShader* shader;
  

public:

  GLMaterial();
  ~GLMaterial(){}

  void apply();

  void setShader( GLShader* _shader ){ shader = _shader; }
  GLShader* getShader(){ return shader; }

  void setTexture( TexType type, GLuint id, GLuint uvidx=0 )
  { 
    textures[type] = id; 
    texture_uvs[type] = uvidx;
  }
  GLuint getTexture( TexType type ){ return textures[type]; }
 
  GLuint getUVIndex( TexType type){ return texture_uvs[type]; }

  void setAmbient( glm::vec4& color ){ ambient = color; }
  glm::vec4 getAmbient(){ return ambient; }

  void setDiffuse( glm::vec4& color ){ diffuse = color; }
  glm::vec4 getDiffuse(){ return diffuse; }

  void setEmission( glm::vec4& color ){ emission = color; }
  glm::vec4 getEmission(){ return emission; }

  void setShininess( float& val ){ shininess = val; }
  float getShininess(){ return shininess; }

  void setShininessStrength( float& val ){ shininess_strength = val; }
  float getShininessStrength(){ return shininess_strength; }
   
};

class GLTextureBank
{
  typedef std::map< std::string, GLuint> TextureList;

  TextureList bank;

  GLuint loadTexture( const char* file );

public:

  GLTextureBank(){};
  ~GLTextureBank();

  /** getTexture() - load a texture if not in the bank,
   *     otherwise, retrieve the previously loaded texture;
   **/
  GLuint getTexture( std::string file );
};
#endif
