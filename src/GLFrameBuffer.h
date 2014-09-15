#ifndef __GL_FRAME_BUFFER_H__
#define __GL_FRAME_BUFFER_H__

#include <GL/glew.h>

class GLFrameBuffer
{
  int res[2];
  GLuint fbo_id;
  GLuint texture_id[ GL_MAX_COLOR_ATTACHMENTS ];

  int num_color_attachments;
  bool has_depth;
  bool has_stencil;

  void checkState();

public:
  GLFrameBuffer( int* _res );
  ~GLFrameBuffer();

  void pushColorAttachment( GLenum iformat );
  void popColorAttachment();

  void pushDepthStencilAttachment( GLenum iformat );
  void popDepthStencilAttachment();

  GLuint getColorAttachmentTexture( int attachment );

  void bind();
  static void unbind();
};

#endif
