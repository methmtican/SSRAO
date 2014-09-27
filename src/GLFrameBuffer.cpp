#include "GLFrameBuffer.h"

#include <stdio.h>
#include <string.h>

GLFrameBuffer::GLFrameBuffer( int* _res )
{
  res[0] = _res[0];
  res[1] = _res[1];

  num_color_attachments = 0;
  has_depth = false;
  has_stencil = false;

  glGenFramebuffers( 1, &fbo_id );
}

GLFrameBuffer::~GLFrameBuffer()
{
  glDeleteFramebuffers( 1, &fbo_id );

  // FIXME: Should clean up texture and renderbuffer attachments as well!!
}

void GLFrameBuffer::checkState()
{
  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER ) ;
  switch( status )
  {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      printf( "Framebuffer is incomplete: attachments are incomplete!\n" );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      printf( "Framebuffer is incomplete: missing an attachment!\n" );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      printf( "Framebuffer is incomplete: incomplete draw buffer!\n" );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      printf( "Framebuffer is incomplete: incomplete read buffer!\n" );
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      printf( "Framebuffer is unsupported!\n" );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      printf( "Framebuffer is incomplete: bad mix of multisampled attachments!\n" );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      printf( "Framebuffer is incomplete: layer targets!\n" );
      break;
    case GL_FRAMEBUFFER_COMPLETE:
      // yay good framebuffer - dont print
      break;
  }
}

void GLFrameBuffer::pushColorAttachment( GLenum iformat )
{
  if( num_color_attachments == GL_MAX_COLOR_ATTACHMENTS-1 )
  {
    printf( "ERROR: Trying to attach a new color attachment to fbo but hit max color attachments!\n" );
    return;
  }

  bind();

  glGenTextures( 1, &texture_id[num_color_attachments] );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, texture_id[num_color_attachments] );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D( GL_TEXTURE_2D, 0, iformat, res[0], res[1], 0,
                //GL_RGB, GL_UNSIGNED_BYTE, 0 );
                GL_RGB, GL_FLOAT, 0 );

  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+num_color_attachments,
                          GL_TEXTURE_2D, texture_id[num_color_attachments], 0 );
  num_color_attachments++;

  checkState();  
}

void GLFrameBuffer::popColorAttachment()
{
  if( num_color_attachments > 0 )
  {
    num_color_attachments--;

    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+num_color_attachments,
                            GL_TEXTURE_2D, 0, 0 );
  }
}

void GLFrameBuffer::pushDepthStencilAttachment( GLenum iformat )
{
  if( has_depth || has_stencil ) 
  {
    printf( "ERROR: Trying to attach a depth/stencil attachment, but fbo already has one!\n" );
    return;
  }

  bind();

  GLuint rb_id;
  glGenRenderbuffers( 1, &rb_id );
  glBindRenderbuffer( GL_RENDERBUFFER, rb_id );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, res[0], res[1] );

  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_id );

  has_depth = true;

  checkState();
}

void GLFrameBuffer::popDepthStencilAttachment()
{
  if( has_depth || has_stencil )
  {
    has_depth = false;
    has_stencil = false;

    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
  }

}

GLuint GLFrameBuffer::getColorAttachmentTexture( int attachment )
{
  if( attachment < GL_MAX_COLOR_ATTACHMENTS )
    return texture_id[ attachment ];
  else
    return 0;
}

void GLFrameBuffer::bind()
{
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo_id );

  GLenum enable[] = { GL_COLOR_ATTACHMENT0,
                      GL_COLOR_ATTACHMENT1,
                      GL_COLOR_ATTACHMENT2 };
  glDrawBuffers( 3, enable );
}

void GLFrameBuffer::unbind()
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}
