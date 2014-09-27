#version 330

#ifdef VERTEX_SHADER 

in vec3 position;
in vec2 texcoord;
in vec4 color;
in vec3 normal;

uniform mat4 projection;
uniform mat4 modelview;

out vec4 color_vert;
out vec2 texcoord_vert;
out vec3 normal_vert;
out vec4 pos_vert;

void main()
{
  vec4 pos      = vec4( position, 1.0 );
  texcoord_vert = texcoord;
  color_vert    = color;
  normal_vert   = normalize(( modelview * vec4( normal, 0.0 )).xyz );
  pos_vert      = modelview * pos;


  gl_Position = projection * modelview * pos;
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D texture;

in vec4 color_vert;
in vec2 texcoord_vert;
in vec3 normal_vert;
in vec4 pos_vert;

out vec4 color;
out vec4 gbuff1;
out vec4 gbuff2;

void main()
{
  // Simply grab the texel and modify by vertex color
  color = texture2D( texture, texcoord_vert ) ;
  //color.rgb *= color_vert.rgb ;
  color = vec4( color ) ;
  gbuff1 = vec4( normal_vert, 1.0 );
  gbuff2 = pos_vert; 
}

#endif

