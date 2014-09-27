#version 330

#ifdef VERTEX_SHADER 

in vec3 position;
out vec3 light_dir;

uniform mat4 modelview;

void main()
{
  light_dir     = ( modelview * normalize(vec4( 1.0, 1.0, 1.0, 0.0 ))).xyz ;
  gl_Position = vec4( position, 1.0 );
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D tex_color;
uniform sampler2D tex_normal;
uniform sampler2D tex_pos;

in vec3 light_dir;
out vec4 color;

void main()
{
  vec4 diffuse  = texture( tex_color, gl_FragCoord.xy/800.0 ) ;
  vec4 position = texture( tex_pos, gl_FragCoord.xy/800.0 );
  vec4 normal   = texture( tex_normal, gl_FragCoord.xy/800.0 );
  if( normal.a == 0.0 )
    discard;

  color.rgb = diffuse.rgb * dot( normal.xyz, light_dir );
  color.a   = 1.0;
}

#endif

