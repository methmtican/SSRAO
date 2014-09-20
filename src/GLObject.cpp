#include "GLObject.h"
#include "GLMaterial.h"
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

GLObject::GLObject()
{
    simple_lazy_state = false;

    texture_id = 0;
    num_faces = 0;

    vao = 0;
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
      glGenVertexArraysAPPLE(1, &vao );
    glBindVertexArrayAPPLE( vao );

    GLuint vbuffer;
    glGenBuffers( 1, &vbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vbuffer );
    glBufferData( GL_ARRAY_BUFFER, data_size*sizeof(float), data, GL_STATIC_DRAW );

    GLuint ibuffer;
    glGenBuffers( 1, &ibuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, idata_size*sizeof(GLuint), idata, GL_STATIC_DRAW );

    glVertexAttribPointer( GLShader::attrib_position, 3, GL_FLOAT, 0, 8*sizeof(float), (void*)0 );
    glEnableVertexAttribArray( GLShader::attrib_position );

    glVertexAttribPointer( GLShader::attrib_texcoord, 2, GL_FLOAT, 0, 8*sizeof(float), (void*)(3*sizeof(float)) );
    glEnableVertexAttribArray( GLShader::attrib_texcoord );

    glVertexAttribPointer( GLShader::attrib_color, 3, GL_FLOAT, 0, 8*sizeof(float), (void*)(5*sizeof(float)) );
    glEnableVertexAttribArray( GLShader::attrib_color );

    glBindVertexArrayAPPLE( 0 );
}

void GLObject::loadVertexAttribute( GLfloat* data, int num_vertices, GLShader::Attribute attribute )
{
    if( !vao )
      glGenVertexArraysAPPLE(1, &vao );
    glBindVertexArrayAPPLE( vao );

    GLint num_comps = 0;
    GLint attrib = 0;
    switch( attribute )
    {
    case GLShader::ATTRIBUTE_POSITION:
      num_comps = 3;
      attrib = GLShader::attrib_position;
      break;
    case GLShader::ATTRIBUTE_NORMAL:
      num_comps = 3;
      attrib = GLShader::attrib_normal;
      break;
    case GLShader::ATTRIBUTE_COLOR:
      num_comps = 4;
      attrib = GLShader::attrib_color;
      break;
    case GLShader::ATTRIBUTE_TEXCOORD:
      num_comps = 2;
      attrib = GLShader::attrib_texcoord;
      break;
    };

    // FIXME: if loadVertexAttribute was called already for an attribute, we should reuse the previous buffer
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, num_comps*num_vertices*sizeof(float), data, GL_STREAM_DRAW );

    glEnableVertexAttribArray( attrib );
    glVertexAttribPointer( attrib, num_comps, GL_FLOAT, 0, 0, (void*)0 );

    glBindVertexArrayAPPLE( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void GLObject::loadFaces( GLuint* faces, int _num_faces )
{
    if( !vao )
      glGenVertexArraysAPPLE(1, &vao );
    glBindVertexArrayAPPLE( vao );

    GLuint buffer;
    // FIXME: this currently assumes triangle faces
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 3*_num_faces*sizeof(GLuint), faces, GL_STREAM_DRAW );

    glBindVertexArrayAPPLE( 0 );
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
    if( mat )
    {
      mat -> apply();
      if( mat -> getShader() -> uni_proj != -1 )  
        glUniformMatrix4fv( mat -> getShader() -> uni_proj, 1, false, glm::value_ptr( proj_mat ));
      if( mat -> getShader() -> uni_mv != -1 )    
        glUniformMatrix4fv( mat -> getShader() -> uni_mv, 1, false, glm::value_ptr( view_mat * model_mat ));
      //glUniform1i( uni_texture, 0 );
      //if( uni_tex_color != -1 )  glUniform1i( uni_tex_color, 0 );
      //if( uni_tex_normal != -1 ) glUniform1i( uni_tex_normal,1 );
      //if( uni_tex_pos != -1 )    glUniform1i( uni_tex_pos,   2 );
    }

    glBindVertexArrayAPPLE( vao );

    glDrawElements( GL_TRIANGLES, num_faces*3, GL_UNSIGNED_INT, (void*)0 );
}
