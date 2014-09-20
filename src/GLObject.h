#ifndef GLOBJECT_H_
#define GLOBJECT_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "GLShader.h"

class GLMaterial;

class GLObject
{
private:
	bool simple_lazy_state;
	int  num_faces;

        void (*select_cb)(void);

        GLMaterial *mat;
	GLuint vao;          // The geometry to draw

	GLuint texture_id;

        glm::mat4 model_mat;
        static glm::mat4 proj_mat;
        static glm::mat4 view_mat;

public:
        static void setProjectionMatrix( float fovy, float aspect, float z_near, float z_far  );
        static void setViewMatrix( glm::vec3& eye, glm::vec3& center, glm::vec3& up );

	GLObject();

        void setMaterial( GLMaterial* _mat ) { mat = _mat; }
        GLMaterial* getMaterial() { return mat; }
 
	void loadInterleaved( GLfloat* data, int num_verticies,
                              GLuint* idata,  int num_indicies );

        void loadVertexAttribute( GLfloat* data, int num_vertices, GLShader::Attribute attribute );
        void loadFaces( GLuint* data, int num_faces );

        void setModelMatrix( glm::mat4& mat );

        void setSelectable( bool sel, void(*cb)(void)=0 );
        void select();


	void draw();
};


#endif /* GLOBJECT_H_ */
