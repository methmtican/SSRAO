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
out vec3 light_dir;

void main()
{
  vec4 pos      = vec4( position, 1.0 );
  texcoord_vert = texcoord;
  color_vert    = color;
  normal_vert   = normalize(( modelview * vec4( normal, 0.0 )).xyz );

  // hard coded directional light for now
  light_dir     = ( modelview * normalize(vec4( 1.0, 1.0, 1.0, 0.0 ))).xyz ;

  gl_Position = projection * modelview * pos;
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D texture;

in vec4 color_vert;
in vec2 texcoord_vert;
in vec3 normal_vert;
in vec3 light_dir;

out vec4 color;

void main()
{
  // Simply grab the texel and modify by vertex color
  color = texture2D( texture, texcoord_vert ) ;
  //color.rgb *= color_vert.rgb ;

  color = color * dot( normal_vert, light_dir );
  color = vec4( color ) ;
}

#endif

