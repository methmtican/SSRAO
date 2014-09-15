#ifndef GLOBJECT_H_
#define GLOBJECT_H_

#include <GL/glew.h>
#include <glm/glm.hpp>

class GLObject
{
public:
        enum Attribute { ATTRIBUTE_POSITION=0, ATTRIBUTE_NORMAL=1, ATTRIBUTE_COLOR=2, ATTRIBUTE_TEXCOORD=3 };

private:
	bool simple_lazy_state;
	int  num_faces;

        void (*select_cb)(void);

	GLuint prog;         // The shader to use
	GLuint vao;          // The geometry to draw
	GLuint uni_texture;

	GLuint texture_id;

        glm::mat4 model_mat;
        static glm::mat4 proj_mat;
        static glm::mat4 view_mat;

        static GLuint uni_tex_color;
        static GLuint uni_tex_normal;
        static GLuint uni_tex_pos;
        static GLuint uni_proj;
        static GLuint uni_mv;

	static const GLint attrib_position = (GLint)ATTRIBUTE_POSITION;
        static const GLint attrib_normal   = (GLint)ATTRIBUTE_NORMAL;
	static const GLint attrib_color    = (GLint)ATTRIBUTE_COLOR;
	static const GLint attrib_texcoord = (GLint)ATTRIBUTE_TEXCOORD;

	char* readShaderFile( const char* filename );
	void printShaderLog( int shader, const char* filename, const char* mesg );
	void printProgramLog( int shader, const char* filename );

public:
        static void setProjectionMatrix( float fovy, float aspect, float z_near, float z_far  );
        static void setViewMatrix( glm::vec3& eye, glm::vec3& center, glm::vec3& up );

	GLObject();

	void loadTexture( const char* filename );
        void setTexture( GLuint tex_id, GLuint unit );
	void loadShader( const char* filename );
	void loadInterleaved( GLfloat* data, int num_verticies,
                              GLuint* idata,  int num_indicies );

        void loadVertexAttribute( GLfloat* data, int num_vertices, Attribute attribute );
        void loadFaces( GLuint* data, int num_faces );

        void setModelMatrix( glm::mat4& mat );

        void setSelectable( bool sel, void(*cb)(void)=0 );
        void select();


	void draw();
};


#endif /* GLOBJECT_H_ */
