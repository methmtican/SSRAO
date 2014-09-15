#version 330

#ifdef VERTEX_SHADER 

in vec3 position;

void main()
{
  gl_Position = vec4( position, 1.0 );
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D tex_color;
uniform sampler2D tex_normal;
uniform sampler2D tex_pos;

out vec4 color;

void main()
{
  color = vec4( texture( tex_color, gl_FragCoord.xy )) ;
}

#endif

