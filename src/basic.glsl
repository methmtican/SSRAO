#version 120

#ifdef VERTEX_SHADER 

attribute vec3 position;
attribute vec2 texcoord;
attribute vec3 color;

varying vec3 color_vert;
varying vec2 texcoord_vert;

void main()
{
  vec4 pos      = vec4( position, 1.0 );
  texcoord_vert = texcoord;
  color_vert    = color;

  gl_Position = pos;
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D texture;

varying vec3 color_vert;
varying vec2 texcoord_vert;

void main()
{
  // Simply grab the texel and modify by vertex color
  vec4 color = texture2D( texture, texcoord_vert ) ;
  color.rgb *= color_vert ;
  gl_FragColor = color ;
}

#endif

