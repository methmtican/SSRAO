#ifndef GLOBJECT_H_
#define GLOBJECT_H_

#include <GL/glew.h>

class GLObject
{
	bool simple_lazy_state;
	int  num_quads;

        float rotation;
        float translation[3];
        float scale;

        float center[3];
        float size[3];

        void (*select_cb)(void);

	GLuint prog;         // The shader to use
	GLuint vao;          // The geometry to draw
	GLuint uni_texture;
        GLuint uni_rotation;
        GLuint uni_translation;
        GLuint uni_scale;

	GLuint texture_id;

        static float projection[];
        static GLuint uni_proj;

	static const GLint attrib_position = 0;
	static const GLint attrib_texcoord = 1;
	static const GLint attrib_color    = 2;

	char* readShaderFile( const char* filename );
	void printShaderLog( int shader, const char* filename, const char* mesg );
	void printProgramLog( int shader, const char* filename );

public:

        static void setProjectionMatrix( float fovy, float aspect, float z_near, float z_far  );

	GLObject();

	void loadTexture( const char* filename );
	void loadShader( const char* filename );
	void loadGeometry( GLfloat* data, int num_verticies,
                       GLuint* idata,  int num_indicies );

        void setRotation( float rad );
        void setTranslation( float vec[] );
        void setScale( float s );

        void setSelectable( bool sel, void(*cb)(void)=0 );
        void select();
        bool contains( float pt[3] );

	void draw();
};


#endif /* GLOBJECT_H_ */
