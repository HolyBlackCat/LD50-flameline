#pragma once

// This file is a part of CGLFL (configurable OpenGL function loader).
// Generated, do no edit!
//
// Version: 1.0.0
// API: gl 3.2 (core profile)
// Extensions: none

#define CGLFL_GL_MAJOR 3
#define CGLFL_GL_MINOR 2
#define CGLFL_GL_API_gl
#define CGLFL_GL_PROFILE_core

#define CGLFL_IMPL_FOR_EACH(m, n, ...) CGLFL_IMPL_CAT(CGLFL_IMPL_FOR_EACH_, n)(m, __VA_ARGS__)
#define CGLFL_IMPL_FOR_EACH_0(m, ...)
#define CGLFL_IMPL_FOR_EACH_1(m, p1) m(p1)
#define CGLFL_IMPL_FOR_EACH_2(m, p1, p2) m(p1) m(p2)
#define CGLFL_IMPL_FOR_EACH_3(m, p1, p2, p3) m(p1) m(p2) m(p3)
#define CGLFL_IMPL_FOR_EACH_4(m, p1, p2, p3, p4) m(p1) m(p2) m(p3) m(p4)
#define CGLFL_IMPL_FOR_EACH_5(m, p1, p2, p3, p4, p5) m(p1) m(p2) m(p3) m(p4) m(p5)
#define CGLFL_IMPL_FOR_EACH_6(m, p1, p2, p3, p4, p5, p6) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6)
#define CGLFL_IMPL_FOR_EACH_7(m, p1, p2, p3, p4, p5, p6, p7) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7)
#define CGLFL_IMPL_FOR_EACH_8(m, p1, p2, p3, p4, p5, p6, p7, p8) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7) m(p8)
#define CGLFL_IMPL_FOR_EACH_9(m, p1, p2, p3, p4, p5, p6, p7, p8, p9) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7) m(p8) m(p9)
#define CGLFL_IMPL_FOR_EACH_10(m, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7) m(p8) m(p9) m(p10)
#define CGLFL_IMPL_FOR_EACH_11(m, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7) m(p8) m(p9) m(p10) m(p11)

#define CGLFL_FUNC_COUNT 316

#define glActiveTexture                       CGLFL_CALL(0,glActiveTexture,void,1,(texture),(GLenum texture))
#define glAttachShader                        CGLFL_CALL(1,glAttachShader,void,2,(program,shader),(GLuint program,GLuint shader))
#define glBeginConditionalRender              CGLFL_CALL(2,glBeginConditionalRender,void,2,(id,mode),(GLuint id,GLenum mode))
#define glBeginQuery                          CGLFL_CALL(3,glBeginQuery,void,2,(target,id),(GLenum target,GLuint id))
#define glBeginTransformFeedback              CGLFL_CALL(4,glBeginTransformFeedback,void,1,(primitiveMode),(GLenum primitiveMode))
#define glBindAttribLocation                  CGLFL_CALL(5,glBindAttribLocation,void,3,(program,index,name),(GLuint program,GLuint index,const GLchar *name))
#define glBindBuffer                          CGLFL_CALL(6,glBindBuffer,void,2,(target,buffer),(GLenum target,GLuint buffer))
#define glBindBufferBase                      CGLFL_CALL(7,glBindBufferBase,void,3,(target,index,buffer),(GLenum target,GLuint index,GLuint buffer))
#define glBindBufferRange                     CGLFL_CALL(8,glBindBufferRange,void,5,(target,index,buffer,offset,size),(GLenum target,GLuint index,GLuint buffer,GLintptr offset,GLsizeiptr size))
#define glBindFragDataLocation                CGLFL_CALL(9,glBindFragDataLocation,void,3,(program,color,name),(GLuint program,GLuint color,const GLchar *name))
#define glBindFramebuffer                     CGLFL_CALL(10,glBindFramebuffer,void,2,(target,framebuffer),(GLenum target,GLuint framebuffer))
#define glBindRenderbuffer                    CGLFL_CALL(11,glBindRenderbuffer,void,2,(target,renderbuffer),(GLenum target,GLuint renderbuffer))
#define glBindTexture                         CGLFL_CALL(12,glBindTexture,void,2,(target,texture),(GLenum target,GLuint texture))
#define glBindVertexArray                     CGLFL_CALL(13,glBindVertexArray,void,1,(array),(GLuint array))
#define glBlendColor                          CGLFL_CALL(14,glBlendColor,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glBlendEquation                       CGLFL_CALL(15,glBlendEquation,void,1,(mode),(GLenum mode))
#define glBlendEquationSeparate               CGLFL_CALL(16,glBlendEquationSeparate,void,2,(modeRGB,modeAlpha),(GLenum modeRGB,GLenum modeAlpha))
#define glBlendFunc                           CGLFL_CALL(17,glBlendFunc,void,2,(sfactor,dfactor),(GLenum sfactor,GLenum dfactor))
#define glBlendFuncSeparate                   CGLFL_CALL(18,glBlendFuncSeparate,void,4,(sfactorRGB,dfactorRGB,sfactorAlpha,dfactorAlpha),(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha))
#define glBlitFramebuffer                     CGLFL_CALL(19,glBlitFramebuffer,void,10,(srcX0,srcY0,srcX1,srcY1,dstX0,dstY0,dstX1,dstY1,mask,filter),(GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter))
#define glBufferData                          CGLFL_CALL(20,glBufferData,void,4,(target,size,data,usage),(GLenum target,GLsizeiptr size,const void *data,GLenum usage))
#define glBufferSubData                       CGLFL_CALL(21,glBufferSubData,void,4,(target,offset,size,data),(GLenum target,GLintptr offset,GLsizeiptr size,const void *data))
#define glCheckFramebufferStatus              CGLFL_CALL(22,glCheckFramebufferStatus,GLenum,1,(target),(GLenum target))
#define glClampColor                          CGLFL_CALL(23,glClampColor,void,2,(target,clamp),(GLenum target,GLenum clamp))
#define glClear                               CGLFL_CALL(24,glClear,void,1,(mask),(GLbitfield mask))
#define glClearBufferfi                       CGLFL_CALL(25,glClearBufferfi,void,4,(buffer,drawbuffer,depth,stencil),(GLenum buffer,GLint drawbuffer,GLfloat depth,GLint stencil))
#define glClearBufferfv                       CGLFL_CALL(26,glClearBufferfv,void,3,(buffer,drawbuffer,value),(GLenum buffer,GLint drawbuffer,const GLfloat *value))
#define glClearBufferiv                       CGLFL_CALL(27,glClearBufferiv,void,3,(buffer,drawbuffer,value),(GLenum buffer,GLint drawbuffer,const GLint *value))
#define glClearBufferuiv                      CGLFL_CALL(28,glClearBufferuiv,void,3,(buffer,drawbuffer,value),(GLenum buffer,GLint drawbuffer,const GLuint *value))
#define glClearColor                          CGLFL_CALL(29,glClearColor,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glClearDepth                          CGLFL_CALL(30,glClearDepth,void,1,(depth),(GLdouble depth))
#define glClearStencil                        CGLFL_CALL(31,glClearStencil,void,1,(s),(GLint s))
#define glClientWaitSync                      CGLFL_CALL(32,glClientWaitSync,GLenum,3,(sync,flags,timeout),(GLsync sync,GLbitfield flags,GLuint64 timeout))
#define glColorMask                           CGLFL_CALL(33,glColorMask,void,4,(red,green,blue,alpha),(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha))
#define glColorMaski                          CGLFL_CALL(34,glColorMaski,void,5,(index,r,g,b,a),(GLuint index,GLboolean r,GLboolean g,GLboolean b,GLboolean a))
#define glCompileShader                       CGLFL_CALL(35,glCompileShader,void,1,(shader),(GLuint shader))
#define glCompressedTexImage1D                CGLFL_CALL(36,glCompressedTexImage1D,void,7,(target,level,internalformat,width,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexImage2D                CGLFL_CALL(37,glCompressedTexImage2D,void,8,(target,level,internalformat,width,height,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexImage3D                CGLFL_CALL(38,glCompressedTexImage3D,void,9,(target,level,internalformat,width,height,depth,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage1D             CGLFL_CALL(39,glCompressedTexSubImage1D,void,7,(target,level,xoffset,width,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage2D             CGLFL_CALL(40,glCompressedTexSubImage2D,void,9,(target,level,xoffset,yoffset,width,height,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage3D             CGLFL_CALL(41,glCompressedTexSubImage3D,void,11,(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void *data))
#define glCopyBufferSubData                   CGLFL_CALL(42,glCopyBufferSubData,void,5,(readTarget,writeTarget,readOffset,writeOffset,size),(GLenum readTarget,GLenum writeTarget,GLintptr readOffset,GLintptr writeOffset,GLsizeiptr size))
#define glCopyTexImage1D                      CGLFL_CALL(43,glCopyTexImage1D,void,7,(target,level,internalformat,x,y,width,border),(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border))
#define glCopyTexImage2D                      CGLFL_CALL(44,glCopyTexImage2D,void,8,(target,level,internalformat,x,y,width,height,border),(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border))
#define glCopyTexSubImage1D                   CGLFL_CALL(45,glCopyTexSubImage1D,void,6,(target,level,xoffset,x,y,width),(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width))
#define glCopyTexSubImage2D                   CGLFL_CALL(46,glCopyTexSubImage2D,void,8,(target,level,xoffset,yoffset,x,y,width,height),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height))
#define glCopyTexSubImage3D                   CGLFL_CALL(47,glCopyTexSubImage3D,void,9,(target,level,xoffset,yoffset,zoffset,x,y,width,height),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height))
#define glCreateProgram                       CGLFL_CALL(48,glCreateProgram,GLuint,0,(),())
#define glCreateShader                        CGLFL_CALL(49,glCreateShader,GLuint,1,(type),(GLenum type))
#define glCullFace                            CGLFL_CALL(50,glCullFace,void,1,(mode),(GLenum mode))
#define glDeleteBuffers                       CGLFL_CALL(51,glDeleteBuffers,void,2,(n,buffers),(GLsizei n,const GLuint *buffers))
#define glDeleteFramebuffers                  CGLFL_CALL(52,glDeleteFramebuffers,void,2,(n,framebuffers),(GLsizei n,const GLuint *framebuffers))
#define glDeleteProgram                       CGLFL_CALL(53,glDeleteProgram,void,1,(program),(GLuint program))
#define glDeleteQueries                       CGLFL_CALL(54,glDeleteQueries,void,2,(n,ids),(GLsizei n,const GLuint *ids))
#define glDeleteRenderbuffers                 CGLFL_CALL(55,glDeleteRenderbuffers,void,2,(n,renderbuffers),(GLsizei n,const GLuint *renderbuffers))
#define glDeleteShader                        CGLFL_CALL(56,glDeleteShader,void,1,(shader),(GLuint shader))
#define glDeleteSync                          CGLFL_CALL(57,glDeleteSync,void,1,(sync),(GLsync sync))
#define glDeleteTextures                      CGLFL_CALL(58,glDeleteTextures,void,2,(n,textures),(GLsizei n,const GLuint *textures))
#define glDeleteVertexArrays                  CGLFL_CALL(59,glDeleteVertexArrays,void,2,(n,arrays),(GLsizei n,const GLuint *arrays))
#define glDepthFunc                           CGLFL_CALL(60,glDepthFunc,void,1,(func),(GLenum func))
#define glDepthMask                           CGLFL_CALL(61,glDepthMask,void,1,(flag),(GLboolean flag))
#define glDepthRange                          CGLFL_CALL(62,glDepthRange,void,2,(n,f),(GLdouble n,GLdouble f))
#define glDetachShader                        CGLFL_CALL(63,glDetachShader,void,2,(program,shader),(GLuint program,GLuint shader))
#define glDisable                             CGLFL_CALL(64,glDisable,void,1,(cap),(GLenum cap))
#define glDisableVertexAttribArray            CGLFL_CALL(65,glDisableVertexAttribArray,void,1,(index),(GLuint index))
#define glDisablei                            CGLFL_CALL(66,glDisablei,void,2,(target,index),(GLenum target,GLuint index))
#define glDrawArrays                          CGLFL_CALL(67,glDrawArrays,void,3,(mode,first,count),(GLenum mode,GLint first,GLsizei count))
#define glDrawArraysInstanced                 CGLFL_CALL(68,glDrawArraysInstanced,void,4,(mode,first,count,instancecount),(GLenum mode,GLint first,GLsizei count,GLsizei instancecount))
#define glDrawBuffer                          CGLFL_CALL(69,glDrawBuffer,void,1,(buf),(GLenum buf))
#define glDrawBuffers                         CGLFL_CALL(70,glDrawBuffers,void,2,(n,bufs),(GLsizei n,const GLenum *bufs))
#define glDrawElements                        CGLFL_CALL(71,glDrawElements,void,4,(mode,count,type,indices),(GLenum mode,GLsizei count,GLenum type,const void *indices))
#define glDrawElementsBaseVertex              CGLFL_CALL(72,glDrawElementsBaseVertex,void,5,(mode,count,type,indices,basevertex),(GLenum mode,GLsizei count,GLenum type,const void *indices,GLint basevertex))
#define glDrawElementsInstanced               CGLFL_CALL(73,glDrawElementsInstanced,void,5,(mode,count,type,indices,instancecount),(GLenum mode,GLsizei count,GLenum type,const void *indices,GLsizei instancecount))
#define glDrawElementsInstancedBaseVertex     CGLFL_CALL(74,glDrawElementsInstancedBaseVertex,void,6,(mode,count,type,indices,instancecount,basevertex),(GLenum mode,GLsizei count,GLenum type,const void *indices,GLsizei instancecount,GLint basevertex))
#define glDrawRangeElements                   CGLFL_CALL(75,glDrawRangeElements,void,6,(mode,start,end,count,type,indices),(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void *indices))
#define glDrawRangeElementsBaseVertex         CGLFL_CALL(76,glDrawRangeElementsBaseVertex,void,7,(mode,start,end,count,type,indices,basevertex),(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void *indices,GLint basevertex))
#define glEnable                              CGLFL_CALL(77,glEnable,void,1,(cap),(GLenum cap))
#define glEnableVertexAttribArray             CGLFL_CALL(78,glEnableVertexAttribArray,void,1,(index),(GLuint index))
#define glEnablei                             CGLFL_CALL(79,glEnablei,void,2,(target,index),(GLenum target,GLuint index))
#define glEndConditionalRender                CGLFL_CALL(80,glEndConditionalRender,void,0,(),())
#define glEndQuery                            CGLFL_CALL(81,glEndQuery,void,1,(target),(GLenum target))
#define glEndTransformFeedback                CGLFL_CALL(82,glEndTransformFeedback,void,0,(),())
#define glFenceSync                           CGLFL_CALL(83,glFenceSync,GLsync,2,(condition,flags),(GLenum condition,GLbitfield flags))
#define glFinish                              CGLFL_CALL(84,glFinish,void,0,(),())
#define glFlush                               CGLFL_CALL(85,glFlush,void,0,(),())
#define glFlushMappedBufferRange              CGLFL_CALL(86,glFlushMappedBufferRange,void,3,(target,offset,length),(GLenum target,GLintptr offset,GLsizeiptr length))
#define glFramebufferRenderbuffer             CGLFL_CALL(87,glFramebufferRenderbuffer,void,4,(target,attachment,renderbuffertarget,renderbuffer),(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer))
#define glFramebufferTexture                  CGLFL_CALL(88,glFramebufferTexture,void,4,(target,attachment,texture,level),(GLenum target,GLenum attachment,GLuint texture,GLint level))
#define glFramebufferTexture1D                CGLFL_CALL(89,glFramebufferTexture1D,void,5,(target,attachment,textarget,texture,level),(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level))
#define glFramebufferTexture2D                CGLFL_CALL(90,glFramebufferTexture2D,void,5,(target,attachment,textarget,texture,level),(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level))
#define glFramebufferTexture3D                CGLFL_CALL(91,glFramebufferTexture3D,void,6,(target,attachment,textarget,texture,level,zoffset),(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset))
#define glFramebufferTextureLayer             CGLFL_CALL(92,glFramebufferTextureLayer,void,5,(target,attachment,texture,level,layer),(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer))
#define glFrontFace                           CGLFL_CALL(93,glFrontFace,void,1,(mode),(GLenum mode))
#define glGenBuffers                          CGLFL_CALL(94,glGenBuffers,void,2,(n,buffers),(GLsizei n,GLuint *buffers))
#define glGenFramebuffers                     CGLFL_CALL(95,glGenFramebuffers,void,2,(n,framebuffers),(GLsizei n,GLuint *framebuffers))
#define glGenQueries                          CGLFL_CALL(96,glGenQueries,void,2,(n,ids),(GLsizei n,GLuint *ids))
#define glGenRenderbuffers                    CGLFL_CALL(97,glGenRenderbuffers,void,2,(n,renderbuffers),(GLsizei n,GLuint *renderbuffers))
#define glGenTextures                         CGLFL_CALL(98,glGenTextures,void,2,(n,textures),(GLsizei n,GLuint *textures))
#define glGenVertexArrays                     CGLFL_CALL(99,glGenVertexArrays,void,2,(n,arrays),(GLsizei n,GLuint *arrays))
#define glGenerateMipmap                      CGLFL_CALL(100,glGenerateMipmap,void,1,(target),(GLenum target))
#define glGetActiveAttrib                     CGLFL_CALL(101,glGetActiveAttrib,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLint *size,GLenum *type,GLchar *name))
#define glGetActiveUniform                    CGLFL_CALL(102,glGetActiveUniform,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLint *size,GLenum *type,GLchar *name))
#define glGetActiveUniformBlockName           CGLFL_CALL(103,glGetActiveUniformBlockName,void,5,(program,uniformBlockIndex,bufSize,length,uniformBlockName),(GLuint program,GLuint uniformBlockIndex,GLsizei bufSize,GLsizei *length,GLchar *uniformBlockName))
#define glGetActiveUniformBlockiv             CGLFL_CALL(104,glGetActiveUniformBlockiv,void,4,(program,uniformBlockIndex,pname,params),(GLuint program,GLuint uniformBlockIndex,GLenum pname,GLint *params))
#define glGetActiveUniformName                CGLFL_CALL(105,glGetActiveUniformName,void,5,(program,uniformIndex,bufSize,length,uniformName),(GLuint program,GLuint uniformIndex,GLsizei bufSize,GLsizei *length,GLchar *uniformName))
#define glGetActiveUniformsiv                 CGLFL_CALL(106,glGetActiveUniformsiv,void,5,(program,uniformCount,uniformIndices,pname,params),(GLuint program,GLsizei uniformCount,const GLuint *uniformIndices,GLenum pname,GLint *params))
#define glGetAttachedShaders                  CGLFL_CALL(107,glGetAttachedShaders,void,4,(program,maxCount,count,shaders),(GLuint program,GLsizei maxCount,GLsizei *count,GLuint *shaders))
#define glGetAttribLocation                   CGLFL_CALL(108,glGetAttribLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetBooleani_v                       CGLFL_CALL(109,glGetBooleani_v,void,3,(target,index,data),(GLenum target,GLuint index,GLboolean *data))
#define glGetBooleanv                         CGLFL_CALL(110,glGetBooleanv,void,2,(pname,data),(GLenum pname,GLboolean *data))
#define glGetBufferParameteri64v              CGLFL_CALL(111,glGetBufferParameteri64v,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint64 *params))
#define glGetBufferParameteriv                CGLFL_CALL(112,glGetBufferParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetBufferPointerv                   CGLFL_CALL(113,glGetBufferPointerv,void,3,(target,pname,params),(GLenum target,GLenum pname,void **params))
#define glGetBufferSubData                    CGLFL_CALL(114,glGetBufferSubData,void,4,(target,offset,size,data),(GLenum target,GLintptr offset,GLsizeiptr size,void *data))
#define glGetCompressedTexImage               CGLFL_CALL(115,glGetCompressedTexImage,void,3,(target,level,img),(GLenum target,GLint level,void *img))
#define glGetDoublev                          CGLFL_CALL(116,glGetDoublev,void,2,(pname,data),(GLenum pname,GLdouble *data))
#define glGetError                            CGLFL_CALL(117,glGetError,GLenum,0,(),())
#define glGetFloatv                           CGLFL_CALL(118,glGetFloatv,void,2,(pname,data),(GLenum pname,GLfloat *data))
#define glGetFragDataLocation                 CGLFL_CALL(119,glGetFragDataLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetFramebufferAttachmentParameteriv CGLFL_CALL(120,glGetFramebufferAttachmentParameteriv,void,4,(target,attachment,pname,params),(GLenum target,GLenum attachment,GLenum pname,GLint *params))
#define glGetInteger64i_v                     CGLFL_CALL(121,glGetInteger64i_v,void,3,(target,index,data),(GLenum target,GLuint index,GLint64 *data))
#define glGetInteger64v                       CGLFL_CALL(122,glGetInteger64v,void,2,(pname,data),(GLenum pname,GLint64 *data))
#define glGetIntegeri_v                       CGLFL_CALL(123,glGetIntegeri_v,void,3,(target,index,data),(GLenum target,GLuint index,GLint *data))
#define glGetIntegerv                         CGLFL_CALL(124,glGetIntegerv,void,2,(pname,data),(GLenum pname,GLint *data))
#define glGetMultisamplefv                    CGLFL_CALL(125,glGetMultisamplefv,void,3,(pname,index,val),(GLenum pname,GLuint index,GLfloat *val))
#define glGetProgramInfoLog                   CGLFL_CALL(126,glGetProgramInfoLog,void,4,(program,bufSize,length,infoLog),(GLuint program,GLsizei bufSize,GLsizei *length,GLchar *infoLog))
#define glGetProgramiv                        CGLFL_CALL(127,glGetProgramiv,void,3,(program,pname,params),(GLuint program,GLenum pname,GLint *params))
#define glGetQueryObjectiv                    CGLFL_CALL(128,glGetQueryObjectiv,void,3,(id,pname,params),(GLuint id,GLenum pname,GLint *params))
#define glGetQueryObjectuiv                   CGLFL_CALL(129,glGetQueryObjectuiv,void,3,(id,pname,params),(GLuint id,GLenum pname,GLuint *params))
#define glGetQueryiv                          CGLFL_CALL(130,glGetQueryiv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetRenderbufferParameteriv          CGLFL_CALL(131,glGetRenderbufferParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetShaderInfoLog                    CGLFL_CALL(132,glGetShaderInfoLog,void,4,(shader,bufSize,length,infoLog),(GLuint shader,GLsizei bufSize,GLsizei *length,GLchar *infoLog))
#define glGetShaderSource                     CGLFL_CALL(133,glGetShaderSource,void,4,(shader,bufSize,length,source),(GLuint shader,GLsizei bufSize,GLsizei *length,GLchar *source))
#define glGetShaderiv                         CGLFL_CALL(134,glGetShaderiv,void,3,(shader,pname,params),(GLuint shader,GLenum pname,GLint *params))
#define glGetString                           CGLFL_CALL(135,glGetString,const GLubyte *,1,(name),(GLenum name))
#define glGetStringi                          CGLFL_CALL(136,glGetStringi,const GLubyte *,2,(name,index),(GLenum name,GLuint index))
#define glGetSynciv                           CGLFL_CALL(137,glGetSynciv,void,5,(sync,pname,bufSize,length,values),(GLsync sync,GLenum pname,GLsizei bufSize,GLsizei *length,GLint *values))
#define glGetTexImage                         CGLFL_CALL(138,glGetTexImage,void,5,(target,level,format,type,pixels),(GLenum target,GLint level,GLenum format,GLenum type,void *pixels))
#define glGetTexLevelParameterfv              CGLFL_CALL(139,glGetTexLevelParameterfv,void,4,(target,level,pname,params),(GLenum target,GLint level,GLenum pname,GLfloat *params))
#define glGetTexLevelParameteriv              CGLFL_CALL(140,glGetTexLevelParameteriv,void,4,(target,level,pname,params),(GLenum target,GLint level,GLenum pname,GLint *params))
#define glGetTexParameterIiv                  CGLFL_CALL(141,glGetTexParameterIiv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetTexParameterIuiv                 CGLFL_CALL(142,glGetTexParameterIuiv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLuint *params))
#define glGetTexParameterfv                   CGLFL_CALL(143,glGetTexParameterfv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLfloat *params))
#define glGetTexParameteriv                   CGLFL_CALL(144,glGetTexParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetTransformFeedbackVarying         CGLFL_CALL(145,glGetTransformFeedbackVarying,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLsizei *size,GLenum *type,GLchar *name))
#define glGetUniformBlockIndex                CGLFL_CALL(146,glGetUniformBlockIndex,GLuint,2,(program,uniformBlockName),(GLuint program,const GLchar *uniformBlockName))
#define glGetUniformIndices                   CGLFL_CALL(147,glGetUniformIndices,void,4,(program,uniformCount,uniformNames,uniformIndices),(GLuint program,GLsizei uniformCount,const GLchar *const*uniformNames,GLuint *uniformIndices))
#define glGetUniformLocation                  CGLFL_CALL(148,glGetUniformLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetUniformfv                        CGLFL_CALL(149,glGetUniformfv,void,3,(program,location,params),(GLuint program,GLint location,GLfloat *params))
#define glGetUniformiv                        CGLFL_CALL(150,glGetUniformiv,void,3,(program,location,params),(GLuint program,GLint location,GLint *params))
#define glGetUniformuiv                       CGLFL_CALL(151,glGetUniformuiv,void,3,(program,location,params),(GLuint program,GLint location,GLuint *params))
#define glGetVertexAttribIiv                  CGLFL_CALL(152,glGetVertexAttribIiv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLint *params))
#define glGetVertexAttribIuiv                 CGLFL_CALL(153,glGetVertexAttribIuiv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLuint *params))
#define glGetVertexAttribPointerv             CGLFL_CALL(154,glGetVertexAttribPointerv,void,3,(index,pname,pointer),(GLuint index,GLenum pname,void **pointer))
#define glGetVertexAttribdv                   CGLFL_CALL(155,glGetVertexAttribdv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLdouble *params))
#define glGetVertexAttribfv                   CGLFL_CALL(156,glGetVertexAttribfv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLfloat *params))
#define glGetVertexAttribiv                   CGLFL_CALL(157,glGetVertexAttribiv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLint *params))
#define glHint                                CGLFL_CALL(158,glHint,void,2,(target,mode),(GLenum target,GLenum mode))
#define glIsBuffer                            CGLFL_CALL(159,glIsBuffer,GLboolean,1,(buffer),(GLuint buffer))
#define glIsEnabled                           CGLFL_CALL(160,glIsEnabled,GLboolean,1,(cap),(GLenum cap))
#define glIsEnabledi                          CGLFL_CALL(161,glIsEnabledi,GLboolean,2,(target,index),(GLenum target,GLuint index))
#define glIsFramebuffer                       CGLFL_CALL(162,glIsFramebuffer,GLboolean,1,(framebuffer),(GLuint framebuffer))
#define glIsProgram                           CGLFL_CALL(163,glIsProgram,GLboolean,1,(program),(GLuint program))
#define glIsQuery                             CGLFL_CALL(164,glIsQuery,GLboolean,1,(id),(GLuint id))
#define glIsRenderbuffer                      CGLFL_CALL(165,glIsRenderbuffer,GLboolean,1,(renderbuffer),(GLuint renderbuffer))
#define glIsShader                            CGLFL_CALL(166,glIsShader,GLboolean,1,(shader),(GLuint shader))
#define glIsSync                              CGLFL_CALL(167,glIsSync,GLboolean,1,(sync),(GLsync sync))
#define glIsTexture                           CGLFL_CALL(168,glIsTexture,GLboolean,1,(texture),(GLuint texture))
#define glIsVertexArray                       CGLFL_CALL(169,glIsVertexArray,GLboolean,1,(array),(GLuint array))
#define glLineWidth                           CGLFL_CALL(170,glLineWidth,void,1,(width),(GLfloat width))
#define glLinkProgram                         CGLFL_CALL(171,glLinkProgram,void,1,(program),(GLuint program))
#define glLogicOp                             CGLFL_CALL(172,glLogicOp,void,1,(opcode),(GLenum opcode))
#define glMapBuffer                           CGLFL_CALL(173,glMapBuffer,void *,2,(target,access),(GLenum target,GLenum access))
#define glMapBufferRange                      CGLFL_CALL(174,glMapBufferRange,void *,4,(target,offset,length,access),(GLenum target,GLintptr offset,GLsizeiptr length,GLbitfield access))
#define glMultiDrawArrays                     CGLFL_CALL(175,glMultiDrawArrays,void,4,(mode,first,count,drawcount),(GLenum mode,const GLint *first,const GLsizei *count,GLsizei drawcount))
#define glMultiDrawElements                   CGLFL_CALL(176,glMultiDrawElements,void,5,(mode,count,type,indices,drawcount),(GLenum mode,const GLsizei *count,GLenum type,const void *const*indices,GLsizei drawcount))
#define glMultiDrawElementsBaseVertex         CGLFL_CALL(177,glMultiDrawElementsBaseVertex,void,6,(mode,count,type,indices,drawcount,basevertex),(GLenum mode,const GLsizei *count,GLenum type,const void *const*indices,GLsizei drawcount,const GLint *basevertex))
#define glPixelStoref                         CGLFL_CALL(178,glPixelStoref,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glPixelStorei                         CGLFL_CALL(179,glPixelStorei,void,2,(pname,param),(GLenum pname,GLint param))
#define glPointParameterf                     CGLFL_CALL(180,glPointParameterf,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glPointParameterfv                    CGLFL_CALL(181,glPointParameterfv,void,2,(pname,params),(GLenum pname,const GLfloat *params))
#define glPointParameteri                     CGLFL_CALL(182,glPointParameteri,void,2,(pname,param),(GLenum pname,GLint param))
#define glPointParameteriv                    CGLFL_CALL(183,glPointParameteriv,void,2,(pname,params),(GLenum pname,const GLint *params))
#define glPointSize                           CGLFL_CALL(184,glPointSize,void,1,(size),(GLfloat size))
#define glPolygonMode                         CGLFL_CALL(185,glPolygonMode,void,2,(face,mode),(GLenum face,GLenum mode))
#define glPolygonOffset                       CGLFL_CALL(186,glPolygonOffset,void,2,(factor,units),(GLfloat factor,GLfloat units))
#define glPrimitiveRestartIndex               CGLFL_CALL(187,glPrimitiveRestartIndex,void,1,(index),(GLuint index))
#define glProvokingVertex                     CGLFL_CALL(188,glProvokingVertex,void,1,(mode),(GLenum mode))
#define glReadBuffer                          CGLFL_CALL(189,glReadBuffer,void,1,(src),(GLenum src))
#define glReadPixels                          CGLFL_CALL(190,glReadPixels,void,7,(x,y,width,height,format,type,pixels),(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void *pixels))
#define glRenderbufferStorage                 CGLFL_CALL(191,glRenderbufferStorage,void,4,(target,internalformat,width,height),(GLenum target,GLenum internalformat,GLsizei width,GLsizei height))
#define glRenderbufferStorageMultisample      CGLFL_CALL(192,glRenderbufferStorageMultisample,void,5,(target,samples,internalformat,width,height),(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height))
#define glSampleCoverage                      CGLFL_CALL(193,glSampleCoverage,void,2,(value,invert),(GLfloat value,GLboolean invert))
#define glSampleMaski                         CGLFL_CALL(194,glSampleMaski,void,2,(maskNumber,mask),(GLuint maskNumber,GLbitfield mask))
#define glScissor                             CGLFL_CALL(195,glScissor,void,4,(x,y,width,height),(GLint x,GLint y,GLsizei width,GLsizei height))
#define glShaderSource                        CGLFL_CALL(196,glShaderSource,void,4,(shader,count,string,length),(GLuint shader,GLsizei count,const GLchar *const*string,const GLint *length))
#define glStencilFunc                         CGLFL_CALL(197,glStencilFunc,void,3,(func,ref,mask),(GLenum func,GLint ref,GLuint mask))
#define glStencilFuncSeparate                 CGLFL_CALL(198,glStencilFuncSeparate,void,4,(face,func,ref,mask),(GLenum face,GLenum func,GLint ref,GLuint mask))
#define glStencilMask                         CGLFL_CALL(199,glStencilMask,void,1,(mask),(GLuint mask))
#define glStencilMaskSeparate                 CGLFL_CALL(200,glStencilMaskSeparate,void,2,(face,mask),(GLenum face,GLuint mask))
#define glStencilOp                           CGLFL_CALL(201,glStencilOp,void,3,(fail,zfail,zpass),(GLenum fail,GLenum zfail,GLenum zpass))
#define glStencilOpSeparate                   CGLFL_CALL(202,glStencilOpSeparate,void,4,(face,sfail,dpfail,dppass),(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass))
#define glTexBuffer                           CGLFL_CALL(203,glTexBuffer,void,3,(target,internalformat,buffer),(GLenum target,GLenum internalformat,GLuint buffer))
#define glTexImage1D                          CGLFL_CALL(204,glTexImage1D,void,8,(target,level,internalformat,width,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexImage2D                          CGLFL_CALL(205,glTexImage2D,void,9,(target,level,internalformat,width,height,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexImage2DMultisample               CGLFL_CALL(206,glTexImage2DMultisample,void,6,(target,samples,internalformat,width,height,fixedsamplelocations),(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations))
#define glTexImage3D                          CGLFL_CALL(207,glTexImage3D,void,10,(target,level,internalformat,width,height,depth,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexImage3DMultisample               CGLFL_CALL(208,glTexImage3DMultisample,void,7,(target,samples,internalformat,width,height,depth,fixedsamplelocations),(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations))
#define glTexParameterIiv                     CGLFL_CALL(209,glTexParameterIiv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLint *params))
#define glTexParameterIuiv                    CGLFL_CALL(210,glTexParameterIuiv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLuint *params))
#define glTexParameterf                       CGLFL_CALL(211,glTexParameterf,void,3,(target,pname,param),(GLenum target,GLenum pname,GLfloat param))
#define glTexParameterfv                      CGLFL_CALL(212,glTexParameterfv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLfloat *params))
#define glTexParameteri                       CGLFL_CALL(213,glTexParameteri,void,3,(target,pname,param),(GLenum target,GLenum pname,GLint param))
#define glTexParameteriv                      CGLFL_CALL(214,glTexParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLint *params))
#define glTexSubImage1D                       CGLFL_CALL(215,glTexSubImage1D,void,7,(target,level,xoffset,width,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void *pixels))
#define glTexSubImage2D                       CGLFL_CALL(216,glTexSubImage2D,void,9,(target,level,xoffset,yoffset,width,height,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void *pixels))
#define glTexSubImage3D                       CGLFL_CALL(217,glTexSubImage3D,void,11,(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void *pixels))
#define glTransformFeedbackVaryings           CGLFL_CALL(218,glTransformFeedbackVaryings,void,4,(program,count,varyings,bufferMode),(GLuint program,GLsizei count,const GLchar *const*varyings,GLenum bufferMode))
#define glUniform1f                           CGLFL_CALL(219,glUniform1f,void,2,(location,v0),(GLint location,GLfloat v0))
#define glUniform1fv                          CGLFL_CALL(220,glUniform1fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform1i                           CGLFL_CALL(221,glUniform1i,void,2,(location,v0),(GLint location,GLint v0))
#define glUniform1iv                          CGLFL_CALL(222,glUniform1iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform1ui                          CGLFL_CALL(223,glUniform1ui,void,2,(location,v0),(GLint location,GLuint v0))
#define glUniform1uiv                         CGLFL_CALL(224,glUniform1uiv,void,3,(location,count,value),(GLint location,GLsizei count,const GLuint *value))
#define glUniform2f                           CGLFL_CALL(225,glUniform2f,void,3,(location,v0,v1),(GLint location,GLfloat v0,GLfloat v1))
#define glUniform2fv                          CGLFL_CALL(226,glUniform2fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform2i                           CGLFL_CALL(227,glUniform2i,void,3,(location,v0,v1),(GLint location,GLint v0,GLint v1))
#define glUniform2iv                          CGLFL_CALL(228,glUniform2iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform2ui                          CGLFL_CALL(229,glUniform2ui,void,3,(location,v0,v1),(GLint location,GLuint v0,GLuint v1))
#define glUniform2uiv                         CGLFL_CALL(230,glUniform2uiv,void,3,(location,count,value),(GLint location,GLsizei count,const GLuint *value))
#define glUniform3f                           CGLFL_CALL(231,glUniform3f,void,4,(location,v0,v1,v2),(GLint location,GLfloat v0,GLfloat v1,GLfloat v2))
#define glUniform3fv                          CGLFL_CALL(232,glUniform3fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform3i                           CGLFL_CALL(233,glUniform3i,void,4,(location,v0,v1,v2),(GLint location,GLint v0,GLint v1,GLint v2))
#define glUniform3iv                          CGLFL_CALL(234,glUniform3iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform3ui                          CGLFL_CALL(235,glUniform3ui,void,4,(location,v0,v1,v2),(GLint location,GLuint v0,GLuint v1,GLuint v2))
#define glUniform3uiv                         CGLFL_CALL(236,glUniform3uiv,void,3,(location,count,value),(GLint location,GLsizei count,const GLuint *value))
#define glUniform4f                           CGLFL_CALL(237,glUniform4f,void,5,(location,v0,v1,v2,v3),(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3))
#define glUniform4fv                          CGLFL_CALL(238,glUniform4fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform4i                           CGLFL_CALL(239,glUniform4i,void,5,(location,v0,v1,v2,v3),(GLint location,GLint v0,GLint v1,GLint v2,GLint v3))
#define glUniform4iv                          CGLFL_CALL(240,glUniform4iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform4ui                          CGLFL_CALL(241,glUniform4ui,void,5,(location,v0,v1,v2,v3),(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3))
#define glUniform4uiv                         CGLFL_CALL(242,glUniform4uiv,void,3,(location,count,value),(GLint location,GLsizei count,const GLuint *value))
#define glUniformBlockBinding                 CGLFL_CALL(243,glUniformBlockBinding,void,3,(program,uniformBlockIndex,uniformBlockBinding),(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding))
#define glUniformMatrix2fv                    CGLFL_CALL(244,glUniformMatrix2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix2x3fv                  CGLFL_CALL(245,glUniformMatrix2x3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix2x4fv                  CGLFL_CALL(246,glUniformMatrix2x4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3fv                    CGLFL_CALL(247,glUniformMatrix3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3x2fv                  CGLFL_CALL(248,glUniformMatrix3x2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3x4fv                  CGLFL_CALL(249,glUniformMatrix3x4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4fv                    CGLFL_CALL(250,glUniformMatrix4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4x2fv                  CGLFL_CALL(251,glUniformMatrix4x2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4x3fv                  CGLFL_CALL(252,glUniformMatrix4x3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUnmapBuffer                         CGLFL_CALL(253,glUnmapBuffer,GLboolean,1,(target),(GLenum target))
#define glUseProgram                          CGLFL_CALL(254,glUseProgram,void,1,(program),(GLuint program))
#define glValidateProgram                     CGLFL_CALL(255,glValidateProgram,void,1,(program),(GLuint program))
#define glVertexAttrib1d                      CGLFL_CALL(256,glVertexAttrib1d,void,2,(index,x),(GLuint index,GLdouble x))
#define glVertexAttrib1dv                     CGLFL_CALL(257,glVertexAttrib1dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib1f                      CGLFL_CALL(258,glVertexAttrib1f,void,2,(index,x),(GLuint index,GLfloat x))
#define glVertexAttrib1fv                     CGLFL_CALL(259,glVertexAttrib1fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib1s                      CGLFL_CALL(260,glVertexAttrib1s,void,2,(index,x),(GLuint index,GLshort x))
#define glVertexAttrib1sv                     CGLFL_CALL(261,glVertexAttrib1sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib2d                      CGLFL_CALL(262,glVertexAttrib2d,void,3,(index,x,y),(GLuint index,GLdouble x,GLdouble y))
#define glVertexAttrib2dv                     CGLFL_CALL(263,glVertexAttrib2dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib2f                      CGLFL_CALL(264,glVertexAttrib2f,void,3,(index,x,y),(GLuint index,GLfloat x,GLfloat y))
#define glVertexAttrib2fv                     CGLFL_CALL(265,glVertexAttrib2fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib2s                      CGLFL_CALL(266,glVertexAttrib2s,void,3,(index,x,y),(GLuint index,GLshort x,GLshort y))
#define glVertexAttrib2sv                     CGLFL_CALL(267,glVertexAttrib2sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib3d                      CGLFL_CALL(268,glVertexAttrib3d,void,4,(index,x,y,z),(GLuint index,GLdouble x,GLdouble y,GLdouble z))
#define glVertexAttrib3dv                     CGLFL_CALL(269,glVertexAttrib3dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib3f                      CGLFL_CALL(270,glVertexAttrib3f,void,4,(index,x,y,z),(GLuint index,GLfloat x,GLfloat y,GLfloat z))
#define glVertexAttrib3fv                     CGLFL_CALL(271,glVertexAttrib3fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib3s                      CGLFL_CALL(272,glVertexAttrib3s,void,4,(index,x,y,z),(GLuint index,GLshort x,GLshort y,GLshort z))
#define glVertexAttrib3sv                     CGLFL_CALL(273,glVertexAttrib3sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib4Nbv                    CGLFL_CALL(274,glVertexAttrib4Nbv,void,2,(index,v),(GLuint index,const GLbyte *v))
#define glVertexAttrib4Niv                    CGLFL_CALL(275,glVertexAttrib4Niv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttrib4Nsv                    CGLFL_CALL(276,glVertexAttrib4Nsv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib4Nub                    CGLFL_CALL(277,glVertexAttrib4Nub,void,5,(index,x,y,z,w),(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w))
#define glVertexAttrib4Nubv                   CGLFL_CALL(278,glVertexAttrib4Nubv,void,2,(index,v),(GLuint index,const GLubyte *v))
#define glVertexAttrib4Nuiv                   CGLFL_CALL(279,glVertexAttrib4Nuiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttrib4Nusv                   CGLFL_CALL(280,glVertexAttrib4Nusv,void,2,(index,v),(GLuint index,const GLushort *v))
#define glVertexAttrib4bv                     CGLFL_CALL(281,glVertexAttrib4bv,void,2,(index,v),(GLuint index,const GLbyte *v))
#define glVertexAttrib4d                      CGLFL_CALL(282,glVertexAttrib4d,void,5,(index,x,y,z,w),(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w))
#define glVertexAttrib4dv                     CGLFL_CALL(283,glVertexAttrib4dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib4f                      CGLFL_CALL(284,glVertexAttrib4f,void,5,(index,x,y,z,w),(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w))
#define glVertexAttrib4fv                     CGLFL_CALL(285,glVertexAttrib4fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib4iv                     CGLFL_CALL(286,glVertexAttrib4iv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttrib4s                      CGLFL_CALL(287,glVertexAttrib4s,void,5,(index,x,y,z,w),(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w))
#define glVertexAttrib4sv                     CGLFL_CALL(288,glVertexAttrib4sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib4ubv                    CGLFL_CALL(289,glVertexAttrib4ubv,void,2,(index,v),(GLuint index,const GLubyte *v))
#define glVertexAttrib4uiv                    CGLFL_CALL(290,glVertexAttrib4uiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttrib4usv                    CGLFL_CALL(291,glVertexAttrib4usv,void,2,(index,v),(GLuint index,const GLushort *v))
#define glVertexAttribI1i                     CGLFL_CALL(292,glVertexAttribI1i,void,2,(index,x),(GLuint index,GLint x))
#define glVertexAttribI1iv                    CGLFL_CALL(293,glVertexAttribI1iv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttribI1ui                    CGLFL_CALL(294,glVertexAttribI1ui,void,2,(index,x),(GLuint index,GLuint x))
#define glVertexAttribI1uiv                   CGLFL_CALL(295,glVertexAttribI1uiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttribI2i                     CGLFL_CALL(296,glVertexAttribI2i,void,3,(index,x,y),(GLuint index,GLint x,GLint y))
#define glVertexAttribI2iv                    CGLFL_CALL(297,glVertexAttribI2iv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttribI2ui                    CGLFL_CALL(298,glVertexAttribI2ui,void,3,(index,x,y),(GLuint index,GLuint x,GLuint y))
#define glVertexAttribI2uiv                   CGLFL_CALL(299,glVertexAttribI2uiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttribI3i                     CGLFL_CALL(300,glVertexAttribI3i,void,4,(index,x,y,z),(GLuint index,GLint x,GLint y,GLint z))
#define glVertexAttribI3iv                    CGLFL_CALL(301,glVertexAttribI3iv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttribI3ui                    CGLFL_CALL(302,glVertexAttribI3ui,void,4,(index,x,y,z),(GLuint index,GLuint x,GLuint y,GLuint z))
#define glVertexAttribI3uiv                   CGLFL_CALL(303,glVertexAttribI3uiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttribI4bv                    CGLFL_CALL(304,glVertexAttribI4bv,void,2,(index,v),(GLuint index,const GLbyte *v))
#define glVertexAttribI4i                     CGLFL_CALL(305,glVertexAttribI4i,void,5,(index,x,y,z,w),(GLuint index,GLint x,GLint y,GLint z,GLint w))
#define glVertexAttribI4iv                    CGLFL_CALL(306,glVertexAttribI4iv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttribI4sv                    CGLFL_CALL(307,glVertexAttribI4sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttribI4ubv                   CGLFL_CALL(308,glVertexAttribI4ubv,void,2,(index,v),(GLuint index,const GLubyte *v))
#define glVertexAttribI4ui                    CGLFL_CALL(309,glVertexAttribI4ui,void,5,(index,x,y,z,w),(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w))
#define glVertexAttribI4uiv                   CGLFL_CALL(310,glVertexAttribI4uiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttribI4usv                   CGLFL_CALL(311,glVertexAttribI4usv,void,2,(index,v),(GLuint index,const GLushort *v))
#define glVertexAttribIPointer                CGLFL_CALL(312,glVertexAttribIPointer,void,5,(index,size,type,stride,pointer),(GLuint index,GLint size,GLenum type,GLsizei stride,const void *pointer))
#define glVertexAttribPointer                 CGLFL_CALL(313,glVertexAttribPointer,void,6,(index,size,type,normalized,stride,pointer),(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void *pointer))
#define glViewport                            CGLFL_CALL(314,glViewport,void,4,(x,y,width,height),(GLint x,GLint y,GLsizei width,GLsizei height))
#define glWaitSync                            CGLFL_CALL(315,glWaitSync,void,3,(sync,flags,timeout),(GLsync sync,GLbitfield flags,GLuint64 timeout))

#define GL_ACTIVE_ATTRIBUTES                             0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                   0x8B8A
#define GL_ACTIVE_TEXTURE                                0x84E0
#define GL_ACTIVE_UNIFORMS                               0x8B86
#define GL_ACTIVE_UNIFORM_BLOCKS                         0x8A36
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH          0x8A35
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                     0x8B87
#define GL_ALIASED_LINE_WIDTH_RANGE                      0x846E
#define GL_ALPHA                                         0x1906
#define GL_ALREADY_SIGNALED                              0x911A
#define GL_ALWAYS                                        0x0207
#define GL_AND                                           0x1501
#define GL_AND_INVERTED                                  0x1504
#define GL_AND_REVERSE                                   0x1502
#define GL_ARRAY_BUFFER                                  0x8892
#define GL_ARRAY_BUFFER_BINDING                          0x8894
#define GL_ATTACHED_SHADERS                              0x8B85
#define GL_BACK                                          0x0405
#define GL_BACK_LEFT                                     0x0402
#define GL_BACK_RIGHT                                    0x0403
#define GL_BGR                                           0x80E0
#define GL_BGRA                                          0x80E1
#define GL_BGRA_INTEGER                                  0x8D9B
#define GL_BGR_INTEGER                                   0x8D9A
#define GL_BLEND                                         0x0BE2
#define GL_BLEND_COLOR                                   0x8005
#define GL_BLEND_DST                                     0x0BE0
#define GL_BLEND_DST_ALPHA                               0x80CA
#define GL_BLEND_DST_RGB                                 0x80C8
#define GL_BLEND_EQUATION                                0x8009
#define GL_BLEND_EQUATION_ALPHA                          0x883D
#define GL_BLEND_EQUATION_RGB                            0x8009
#define GL_BLEND_SRC                                     0x0BE1
#define GL_BLEND_SRC_ALPHA                               0x80CB
#define GL_BLEND_SRC_RGB                                 0x80C9
#define GL_BLUE                                          0x1905
#define GL_BLUE_INTEGER                                  0x8D96
#define GL_BOOL                                          0x8B56
#define GL_BOOL_VEC2                                     0x8B57
#define GL_BOOL_VEC3                                     0x8B58
#define GL_BOOL_VEC4                                     0x8B59
#define GL_BUFFER_ACCESS                                 0x88BB
#define GL_BUFFER_ACCESS_FLAGS                           0x911F
#define GL_BUFFER_MAPPED                                 0x88BC
#define GL_BUFFER_MAP_LENGTH                             0x9120
#define GL_BUFFER_MAP_OFFSET                             0x9121
#define GL_BUFFER_MAP_POINTER                            0x88BD
#define GL_BUFFER_SIZE                                   0x8764
#define GL_BUFFER_USAGE                                  0x8765
#define GL_BYTE                                          0x1400
#define GL_CCW                                           0x0901
#define GL_CLAMP_READ_COLOR                              0x891C
#define GL_CLAMP_TO_BORDER                               0x812D
#define GL_CLAMP_TO_EDGE                                 0x812F
#define GL_CLEAR                                         0x1500
#define GL_CLIP_DISTANCE0                                0x3000
#define GL_CLIP_DISTANCE1                                0x3001
#define GL_CLIP_DISTANCE2                                0x3002
#define GL_CLIP_DISTANCE3                                0x3003
#define GL_CLIP_DISTANCE4                                0x3004
#define GL_CLIP_DISTANCE5                                0x3005
#define GL_CLIP_DISTANCE6                                0x3006
#define GL_CLIP_DISTANCE7                                0x3007
#define GL_COLOR                                         0x1800
#define GL_COLOR_ATTACHMENT0                             0x8CE0
#define GL_COLOR_ATTACHMENT1                             0x8CE1
#define GL_COLOR_ATTACHMENT10                            0x8CEA
#define GL_COLOR_ATTACHMENT11                            0x8CEB
#define GL_COLOR_ATTACHMENT12                            0x8CEC
#define GL_COLOR_ATTACHMENT13                            0x8CED
#define GL_COLOR_ATTACHMENT14                            0x8CEE
#define GL_COLOR_ATTACHMENT15                            0x8CEF
#define GL_COLOR_ATTACHMENT16                            0x8CF0
#define GL_COLOR_ATTACHMENT17                            0x8CF1
#define GL_COLOR_ATTACHMENT18                            0x8CF2
#define GL_COLOR_ATTACHMENT19                            0x8CF3
#define GL_COLOR_ATTACHMENT2                             0x8CE2
#define GL_COLOR_ATTACHMENT20                            0x8CF4
#define GL_COLOR_ATTACHMENT21                            0x8CF5
#define GL_COLOR_ATTACHMENT22                            0x8CF6
#define GL_COLOR_ATTACHMENT23                            0x8CF7
#define GL_COLOR_ATTACHMENT24                            0x8CF8
#define GL_COLOR_ATTACHMENT25                            0x8CF9
#define GL_COLOR_ATTACHMENT26                            0x8CFA
#define GL_COLOR_ATTACHMENT27                            0x8CFB
#define GL_COLOR_ATTACHMENT28                            0x8CFC
#define GL_COLOR_ATTACHMENT29                            0x8CFD
#define GL_COLOR_ATTACHMENT3                             0x8CE3
#define GL_COLOR_ATTACHMENT30                            0x8CFE
#define GL_COLOR_ATTACHMENT31                            0x8CFF
#define GL_COLOR_ATTACHMENT4                             0x8CE4
#define GL_COLOR_ATTACHMENT5                             0x8CE5
#define GL_COLOR_ATTACHMENT6                             0x8CE6
#define GL_COLOR_ATTACHMENT7                             0x8CE7
#define GL_COLOR_ATTACHMENT8                             0x8CE8
#define GL_COLOR_ATTACHMENT9                             0x8CE9
#define GL_COLOR_BUFFER_BIT                              0x00004000
#define GL_COLOR_CLEAR_VALUE                             0x0C22
#define GL_COLOR_LOGIC_OP                                0x0BF2
#define GL_COLOR_WRITEMASK                               0x0C23
#define GL_COMPARE_REF_TO_TEXTURE                        0x884E
#define GL_COMPILE_STATUS                                0x8B81
#define GL_COMPRESSED_RED                                0x8225
#define GL_COMPRESSED_RED_RGTC1                          0x8DBB
#define GL_COMPRESSED_RG                                 0x8226
#define GL_COMPRESSED_RGB                                0x84ED
#define GL_COMPRESSED_RGBA                               0x84EE
#define GL_COMPRESSED_RG_RGTC2                           0x8DBD
#define GL_COMPRESSED_SIGNED_RED_RGTC1                   0x8DBC
#define GL_COMPRESSED_SIGNED_RG_RGTC2                    0x8DBE
#define GL_COMPRESSED_SRGB                               0x8C48
#define GL_COMPRESSED_SRGB_ALPHA                         0x8C49
#define GL_COMPRESSED_TEXTURE_FORMATS                    0x86A3
#define GL_CONDITION_SATISFIED                           0x911C
#define GL_CONSTANT_ALPHA                                0x8003
#define GL_CONSTANT_COLOR                                0x8001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT             0x00000002
#define GL_CONTEXT_CORE_PROFILE_BIT                      0x00000001
#define GL_CONTEXT_FLAGS                                 0x821E
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT           0x00000001
#define GL_CONTEXT_PROFILE_MASK                          0x9126
#define GL_COPY                                          0x1503
#define GL_COPY_INVERTED                                 0x150C
#define GL_COPY_READ_BUFFER                              0x8F36
#define GL_COPY_WRITE_BUFFER                             0x8F37
#define GL_CULL_FACE                                     0x0B44
#define GL_CULL_FACE_MODE                                0x0B45
#define GL_CURRENT_PROGRAM                               0x8B8D
#define GL_CURRENT_QUERY                                 0x8865
#define GL_CURRENT_VERTEX_ATTRIB                         0x8626
#define GL_CW                                            0x0900
#define GL_DECR                                          0x1E03
#define GL_DECR_WRAP                                     0x8508
#define GL_DELETE_STATUS                                 0x8B80
#define GL_DEPTH                                         0x1801
#define GL_DEPTH24_STENCIL8                              0x88F0
#define GL_DEPTH32F_STENCIL8                             0x8CAD
#define GL_DEPTH_ATTACHMENT                              0x8D00
#define GL_DEPTH_BUFFER_BIT                              0x00000100
#define GL_DEPTH_CLAMP                                   0x864F
#define GL_DEPTH_CLEAR_VALUE                             0x0B73
#define GL_DEPTH_COMPONENT                               0x1902
#define GL_DEPTH_COMPONENT16                             0x81A5
#define GL_DEPTH_COMPONENT24                             0x81A6
#define GL_DEPTH_COMPONENT32                             0x81A7
#define GL_DEPTH_COMPONENT32F                            0x8CAC
#define GL_DEPTH_FUNC                                    0x0B74
#define GL_DEPTH_RANGE                                   0x0B70
#define GL_DEPTH_STENCIL                                 0x84F9
#define GL_DEPTH_STENCIL_ATTACHMENT                      0x821A
#define GL_DEPTH_TEST                                    0x0B71
#define GL_DEPTH_WRITEMASK                               0x0B72
#define GL_DITHER                                        0x0BD0
#define GL_DONT_CARE                                     0x1100
#define GL_DOUBLE                                        0x140A
#define GL_DOUBLEBUFFER                                  0x0C32
#define GL_DRAW_BUFFER                                   0x0C01
#define GL_DRAW_BUFFER0                                  0x8825
#define GL_DRAW_BUFFER1                                  0x8826
#define GL_DRAW_BUFFER10                                 0x882F
#define GL_DRAW_BUFFER11                                 0x8830
#define GL_DRAW_BUFFER12                                 0x8831
#define GL_DRAW_BUFFER13                                 0x8832
#define GL_DRAW_BUFFER14                                 0x8833
#define GL_DRAW_BUFFER15                                 0x8834
#define GL_DRAW_BUFFER2                                  0x8827
#define GL_DRAW_BUFFER3                                  0x8828
#define GL_DRAW_BUFFER4                                  0x8829
#define GL_DRAW_BUFFER5                                  0x882A
#define GL_DRAW_BUFFER6                                  0x882B
#define GL_DRAW_BUFFER7                                  0x882C
#define GL_DRAW_BUFFER8                                  0x882D
#define GL_DRAW_BUFFER9                                  0x882E
#define GL_DRAW_FRAMEBUFFER                              0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING                      0x8CA6
#define GL_DST_ALPHA                                     0x0304
#define GL_DST_COLOR                                     0x0306
#define GL_DYNAMIC_COPY                                  0x88EA
#define GL_DYNAMIC_DRAW                                  0x88E8
#define GL_DYNAMIC_READ                                  0x88E9
#define GL_ELEMENT_ARRAY_BUFFER                          0x8893
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                  0x8895
#define GL_EQUAL                                         0x0202
#define GL_EQUIV                                         0x1509
#define GL_EXTENSIONS                                    0x1F03
#define GL_FALSE                                         0
#define GL_FASTEST                                       0x1101
#define GL_FILL                                          0x1B02
#define GL_FIRST_VERTEX_CONVENTION                       0x8E4D
#define GL_FIXED_ONLY                                    0x891D
#define GL_FLOAT                                         0x1406
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                0x8DAD
#define GL_FLOAT_MAT2                                    0x8B5A
#define GL_FLOAT_MAT2x3                                  0x8B65
#define GL_FLOAT_MAT2x4                                  0x8B66
#define GL_FLOAT_MAT3                                    0x8B5B
#define GL_FLOAT_MAT3x2                                  0x8B67
#define GL_FLOAT_MAT3x4                                  0x8B68
#define GL_FLOAT_MAT4                                    0x8B5C
#define GL_FLOAT_MAT4x2                                  0x8B69
#define GL_FLOAT_MAT4x3                                  0x8B6A
#define GL_FLOAT_VEC2                                    0x8B50
#define GL_FLOAT_VEC3                                    0x8B51
#define GL_FLOAT_VEC4                                    0x8B52
#define GL_FRAGMENT_SHADER                               0x8B30
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT               0x8B8B
#define GL_FRAMEBUFFER                                   0x8D40
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE             0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE              0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING         0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE         0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE             0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE             0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED                0x8DA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME            0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE            0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE               0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE           0x8217
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER          0x8CD4
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL          0x8CD2
#define GL_FRAMEBUFFER_BINDING                           0x8CA6
#define GL_FRAMEBUFFER_COMPLETE                          0x8CD5
#define GL_FRAMEBUFFER_DEFAULT                           0x8218
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT             0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER            0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS          0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT     0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE            0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER            0x8CDC
#define GL_FRAMEBUFFER_SRGB                              0x8DB9
#define GL_FRAMEBUFFER_UNDEFINED                         0x8219
#define GL_FRAMEBUFFER_UNSUPPORTED                       0x8CDD
#define GL_FRONT                                         0x0404
#define GL_FRONT_AND_BACK                                0x0408
#define GL_FRONT_FACE                                    0x0B46
#define GL_FRONT_LEFT                                    0x0400
#define GL_FRONT_RIGHT                                   0x0401
#define GL_FUNC_ADD                                      0x8006
#define GL_FUNC_REVERSE_SUBTRACT                         0x800B
#define GL_FUNC_SUBTRACT                                 0x800A
#define GL_GEOMETRY_INPUT_TYPE                           0x8917
#define GL_GEOMETRY_OUTPUT_TYPE                          0x8918
#define GL_GEOMETRY_SHADER                               0x8DD9
#define GL_GEOMETRY_VERTICES_OUT                         0x8916
#define GL_GEQUAL                                        0x0206
#define GL_GREATER                                       0x0204
#define GL_GREEN                                         0x1904
#define GL_GREEN_INTEGER                                 0x8D95
#define GL_HALF_FLOAT                                    0x140B
#define GL_INCR                                          0x1E02
#define GL_INCR_WRAP                                     0x8507
#define GL_INFO_LOG_LENGTH                               0x8B84
#define GL_INT                                           0x1404
#define GL_INTERLEAVED_ATTRIBS                           0x8C8C
#define GL_INT_SAMPLER_1D                                0x8DC9
#define GL_INT_SAMPLER_1D_ARRAY                          0x8DCE
#define GL_INT_SAMPLER_2D                                0x8DCA
#define GL_INT_SAMPLER_2D_ARRAY                          0x8DCF
#define GL_INT_SAMPLER_2D_MULTISAMPLE                    0x9109
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY              0x910C
#define GL_INT_SAMPLER_2D_RECT                           0x8DCD
#define GL_INT_SAMPLER_3D                                0x8DCB
#define GL_INT_SAMPLER_BUFFER                            0x8DD0
#define GL_INT_SAMPLER_CUBE                              0x8DCC
#define GL_INT_VEC2                                      0x8B53
#define GL_INT_VEC3                                      0x8B54
#define GL_INT_VEC4                                      0x8B55
#define GL_INVALID_ENUM                                  0x0500
#define GL_INVALID_FRAMEBUFFER_OPERATION                 0x0506
#define GL_INVALID_INDEX                                 0xFFFFFFFF
#define GL_INVALID_OPERATION                             0x0502
#define GL_INVALID_VALUE                                 0x0501
#define GL_INVERT                                        0x150A
#define GL_KEEP                                          0x1E00
#define GL_LAST_VERTEX_CONVENTION                        0x8E4E
#define GL_LEFT                                          0x0406
#define GL_LEQUAL                                        0x0203
#define GL_LESS                                          0x0201
#define GL_LINE                                          0x1B01
#define GL_LINEAR                                        0x2601
#define GL_LINEAR_MIPMAP_LINEAR                          0x2703
#define GL_LINEAR_MIPMAP_NEAREST                         0x2701
#define GL_LINES                                         0x0001
#define GL_LINES_ADJACENCY                               0x000A
#define GL_LINE_LOOP                                     0x0002
#define GL_LINE_SMOOTH                                   0x0B20
#define GL_LINE_SMOOTH_HINT                              0x0C52
#define GL_LINE_STRIP                                    0x0003
#define GL_LINE_STRIP_ADJACENCY                          0x000B
#define GL_LINE_WIDTH                                    0x0B21
#define GL_LINE_WIDTH_GRANULARITY                        0x0B23
#define GL_LINE_WIDTH_RANGE                              0x0B22
#define GL_LINK_STATUS                                   0x8B82
#define GL_LOGIC_OP_MODE                                 0x0BF0
#define GL_LOWER_LEFT                                    0x8CA1
#define GL_MAJOR_VERSION                                 0x821B
#define GL_MAP_FLUSH_EXPLICIT_BIT                        0x0010
#define GL_MAP_INVALIDATE_BUFFER_BIT                     0x0008
#define GL_MAP_INVALIDATE_RANGE_BIT                      0x0004
#define GL_MAP_READ_BIT                                  0x0001
#define GL_MAP_UNSYNCHRONIZED_BIT                        0x0020
#define GL_MAP_WRITE_BIT                                 0x0002
#define GL_MAX                                           0x8008
#define GL_MAX_3D_TEXTURE_SIZE                           0x8073
#define GL_MAX_ARRAY_TEXTURE_LAYERS                      0x88FF
#define GL_MAX_CLIP_DISTANCES                            0x0D32
#define GL_MAX_COLOR_ATTACHMENTS                         0x8CDF
#define GL_MAX_COLOR_TEXTURE_SAMPLES                     0x910E
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS      0x8A32
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS              0x8B4D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                   0x8A2E
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        0x8A31
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                     0x851C
#define GL_MAX_DEPTH_TEXTURE_SAMPLES                     0x910F
#define GL_MAX_DRAW_BUFFERS                              0x8824
#define GL_MAX_ELEMENTS_INDICES                          0x80E9
#define GL_MAX_ELEMENTS_VERTICES                         0x80E8
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   0x8A2D
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS               0x8B49
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS                 0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS                0x9124
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES                  0x8DE0
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS              0x8C29
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS          0x8DE1
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS                   0x8A2C
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS               0x8DDF
#define GL_MAX_INTEGER_SAMPLES                           0x9110
#define GL_MAX_PROGRAM_TEXEL_OFFSET                      0x8905
#define GL_MAX_RECTANGLE_TEXTURE_SIZE                    0x84F8
#define GL_MAX_RENDERBUFFER_SIZE                         0x84E8
#define GL_MAX_SAMPLES                                   0x8D57
#define GL_MAX_SAMPLE_MASK_WORDS                         0x8E59
#define GL_MAX_SERVER_WAIT_TIMEOUT                       0x9111
#define GL_MAX_TEXTURE_BUFFER_SIZE                       0x8C2B
#define GL_MAX_TEXTURE_IMAGE_UNITS                       0x8872
#define GL_MAX_TEXTURE_LOD_BIAS                          0x84FD
#define GL_MAX_TEXTURE_SIZE                              0x0D33
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       0x8C8B
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    0x8C80
#define GL_MAX_UNIFORM_BLOCK_SIZE                        0x8A30
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                   0x8A2F
#define GL_MAX_VARYING_COMPONENTS                        0x8B4B
#define GL_MAX_VARYING_FLOATS                            0x8B4B
#define GL_MAX_VERTEX_ATTRIBS                            0x8869
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                  0x9122
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                0x8B4C
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                     0x8A2B
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                 0x8B4A
#define GL_MAX_VIEWPORT_DIMS                             0x0D3A
#define GL_MIN                                           0x8007
#define GL_MINOR_VERSION                                 0x821C
#define GL_MIN_PROGRAM_TEXEL_OFFSET                      0x8904
#define GL_MIRRORED_REPEAT                               0x8370
#define GL_MULTISAMPLE                                   0x809D
#define GL_NAND                                          0x150E
#define GL_NEAREST                                       0x2600
#define GL_NEAREST_MIPMAP_LINEAR                         0x2702
#define GL_NEAREST_MIPMAP_NEAREST                        0x2700
#define GL_NEVER                                         0x0200
#define GL_NICEST                                        0x1102
#define GL_NONE                                          0
#define GL_NOOP                                          0x1505
#define GL_NOR                                           0x1508
#define GL_NOTEQUAL                                      0x0205
#define GL_NO_ERROR                                      0
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS                0x86A2
#define GL_NUM_EXTENSIONS                                0x821D
#define GL_OBJECT_TYPE                                   0x9112
#define GL_ONE                                           1
#define GL_ONE_MINUS_CONSTANT_ALPHA                      0x8004
#define GL_ONE_MINUS_CONSTANT_COLOR                      0x8002
#define GL_ONE_MINUS_DST_ALPHA                           0x0305
#define GL_ONE_MINUS_DST_COLOR                           0x0307
#define GL_ONE_MINUS_SRC_ALPHA                           0x0303
#define GL_ONE_MINUS_SRC_COLOR                           0x0301
#define GL_OR                                            0x1507
#define GL_OR_INVERTED                                   0x150D
#define GL_OR_REVERSE                                    0x150B
#define GL_OUT_OF_MEMORY                                 0x0505
#define GL_PACK_ALIGNMENT                                0x0D05
#define GL_PACK_IMAGE_HEIGHT                             0x806C
#define GL_PACK_LSB_FIRST                                0x0D01
#define GL_PACK_ROW_LENGTH                               0x0D02
#define GL_PACK_SKIP_IMAGES                              0x806B
#define GL_PACK_SKIP_PIXELS                              0x0D04
#define GL_PACK_SKIP_ROWS                                0x0D03
#define GL_PACK_SWAP_BYTES                               0x0D00
#define GL_PIXEL_PACK_BUFFER                             0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING                     0x88ED
#define GL_PIXEL_UNPACK_BUFFER                           0x88EC
#define GL_PIXEL_UNPACK_BUFFER_BINDING                   0x88EF
#define GL_POINT                                         0x1B00
#define GL_POINTS                                        0x0000
#define GL_POINT_FADE_THRESHOLD_SIZE                     0x8128
#define GL_POINT_SIZE                                    0x0B11
#define GL_POINT_SIZE_GRANULARITY                        0x0B13
#define GL_POINT_SIZE_RANGE                              0x0B12
#define GL_POINT_SPRITE_COORD_ORIGIN                     0x8CA0
#define GL_POLYGON_MODE                                  0x0B40
#define GL_POLYGON_OFFSET_FACTOR                         0x8038
#define GL_POLYGON_OFFSET_FILL                           0x8037
#define GL_POLYGON_OFFSET_LINE                           0x2A02
#define GL_POLYGON_OFFSET_POINT                          0x2A01
#define GL_POLYGON_OFFSET_UNITS                          0x2A00
#define GL_POLYGON_SMOOTH                                0x0B41
#define GL_POLYGON_SMOOTH_HINT                           0x0C53
#define GL_PRIMITIVES_GENERATED                          0x8C87
#define GL_PRIMITIVE_RESTART                             0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX                       0x8F9E
#define GL_PROGRAM_POINT_SIZE                            0x8642
#define GL_PROVOKING_VERTEX                              0x8E4F
#define GL_PROXY_TEXTURE_1D                              0x8063
#define GL_PROXY_TEXTURE_1D_ARRAY                        0x8C19
#define GL_PROXY_TEXTURE_2D                              0x8064
#define GL_PROXY_TEXTURE_2D_ARRAY                        0x8C1B
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE                  0x9101
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY            0x9103
#define GL_PROXY_TEXTURE_3D                              0x8070
#define GL_PROXY_TEXTURE_CUBE_MAP                        0x851B
#define GL_PROXY_TEXTURE_RECTANGLE                       0x84F7
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION      0x8E4C
#define GL_QUERY_BY_REGION_NO_WAIT                       0x8E16
#define GL_QUERY_BY_REGION_WAIT                          0x8E15
#define GL_QUERY_COUNTER_BITS                            0x8864
#define GL_QUERY_NO_WAIT                                 0x8E14
#define GL_QUERY_RESULT                                  0x8866
#define GL_QUERY_RESULT_AVAILABLE                        0x8867
#define GL_QUERY_WAIT                                    0x8E13
#define GL_R11F_G11F_B10F                                0x8C3A
#define GL_R16                                           0x822A
#define GL_R16F                                          0x822D
#define GL_R16I                                          0x8233
#define GL_R16UI                                         0x8234
#define GL_R16_SNORM                                     0x8F98
#define GL_R32F                                          0x822E
#define GL_R32I                                          0x8235
#define GL_R32UI                                         0x8236
#define GL_R3_G3_B2                                      0x2A10
#define GL_R8                                            0x8229
#define GL_R8I                                           0x8231
#define GL_R8UI                                          0x8232
#define GL_R8_SNORM                                      0x8F94
#define GL_RASTERIZER_DISCARD                            0x8C89
#define GL_READ_BUFFER                                   0x0C02
#define GL_READ_FRAMEBUFFER                              0x8CA8
#define GL_READ_FRAMEBUFFER_BINDING                      0x8CAA
#define GL_READ_ONLY                                     0x88B8
#define GL_READ_WRITE                                    0x88BA
#define GL_RED                                           0x1903
#define GL_RED_INTEGER                                   0x8D94
#define GL_RENDERBUFFER                                  0x8D41
#define GL_RENDERBUFFER_ALPHA_SIZE                       0x8D53
#define GL_RENDERBUFFER_BINDING                          0x8CA7
#define GL_RENDERBUFFER_BLUE_SIZE                        0x8D52
#define GL_RENDERBUFFER_DEPTH_SIZE                       0x8D54
#define GL_RENDERBUFFER_GREEN_SIZE                       0x8D51
#define GL_RENDERBUFFER_HEIGHT                           0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT                  0x8D44
#define GL_RENDERBUFFER_RED_SIZE                         0x8D50
#define GL_RENDERBUFFER_SAMPLES                          0x8CAB
#define GL_RENDERBUFFER_STENCIL_SIZE                     0x8D55
#define GL_RENDERBUFFER_WIDTH                            0x8D42
#define GL_RENDERER                                      0x1F01
#define GL_REPEAT                                        0x2901
#define GL_REPLACE                                       0x1E01
#define GL_RG                                            0x8227
#define GL_RG16                                          0x822C
#define GL_RG16F                                         0x822F
#define GL_RG16I                                         0x8239
#define GL_RG16UI                                        0x823A
#define GL_RG16_SNORM                                    0x8F99
#define GL_RG32F                                         0x8230
#define GL_RG32I                                         0x823B
#define GL_RG32UI                                        0x823C
#define GL_RG8                                           0x822B
#define GL_RG8I                                          0x8237
#define GL_RG8UI                                         0x8238
#define GL_RG8_SNORM                                     0x8F95
#define GL_RGB                                           0x1907
#define GL_RGB10                                         0x8052
#define GL_RGB10_A2                                      0x8059
#define GL_RGB12                                         0x8053
#define GL_RGB16                                         0x8054
#define GL_RGB16F                                        0x881B
#define GL_RGB16I                                        0x8D89
#define GL_RGB16UI                                       0x8D77
#define GL_RGB16_SNORM                                   0x8F9A
#define GL_RGB32F                                        0x8815
#define GL_RGB32I                                        0x8D83
#define GL_RGB32UI                                       0x8D71
#define GL_RGB4                                          0x804F
#define GL_RGB5                                          0x8050
#define GL_RGB5_A1                                       0x8057
#define GL_RGB8                                          0x8051
#define GL_RGB8I                                         0x8D8F
#define GL_RGB8UI                                        0x8D7D
#define GL_RGB8_SNORM                                    0x8F96
#define GL_RGB9_E5                                       0x8C3D
#define GL_RGBA                                          0x1908
#define GL_RGBA12                                        0x805A
#define GL_RGBA16                                        0x805B
#define GL_RGBA16F                                       0x881A
#define GL_RGBA16I                                       0x8D88
#define GL_RGBA16UI                                      0x8D76
#define GL_RGBA16_SNORM                                  0x8F9B
#define GL_RGBA2                                         0x8055
#define GL_RGBA32F                                       0x8814
#define GL_RGBA32I                                       0x8D82
#define GL_RGBA32UI                                      0x8D70
#define GL_RGBA4                                         0x8056
#define GL_RGBA8                                         0x8058
#define GL_RGBA8I                                        0x8D8E
#define GL_RGBA8UI                                       0x8D7C
#define GL_RGBA8_SNORM                                   0x8F97
#define GL_RGBA_INTEGER                                  0x8D99
#define GL_RGB_INTEGER                                   0x8D98
#define GL_RG_INTEGER                                    0x8228
#define GL_RIGHT                                         0x0407
#define GL_SAMPLER_1D                                    0x8B5D
#define GL_SAMPLER_1D_ARRAY                              0x8DC0
#define GL_SAMPLER_1D_ARRAY_SHADOW                       0x8DC3
#define GL_SAMPLER_1D_SHADOW                             0x8B61
#define GL_SAMPLER_2D                                    0x8B5E
#define GL_SAMPLER_2D_ARRAY                              0x8DC1
#define GL_SAMPLER_2D_ARRAY_SHADOW                       0x8DC4
#define GL_SAMPLER_2D_MULTISAMPLE                        0x9108
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY                  0x910B
#define GL_SAMPLER_2D_RECT                               0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW                        0x8B64
#define GL_SAMPLER_2D_SHADOW                             0x8B62
#define GL_SAMPLER_3D                                    0x8B5F
#define GL_SAMPLER_BUFFER                                0x8DC2
#define GL_SAMPLER_CUBE                                  0x8B60
#define GL_SAMPLER_CUBE_SHADOW                           0x8DC5
#define GL_SAMPLES                                       0x80A9
#define GL_SAMPLES_PASSED                                0x8914
#define GL_SAMPLE_ALPHA_TO_COVERAGE                      0x809E
#define GL_SAMPLE_ALPHA_TO_ONE                           0x809F
#define GL_SAMPLE_BUFFERS                                0x80A8
#define GL_SAMPLE_COVERAGE                               0x80A0
#define GL_SAMPLE_COVERAGE_INVERT                        0x80AB
#define GL_SAMPLE_COVERAGE_VALUE                         0x80AA
#define GL_SAMPLE_MASK                                   0x8E51
#define GL_SAMPLE_MASK_VALUE                             0x8E52
#define GL_SAMPLE_POSITION                               0x8E50
#define GL_SCISSOR_BOX                                   0x0C10
#define GL_SCISSOR_TEST                                  0x0C11
#define GL_SEPARATE_ATTRIBS                              0x8C8D
#define GL_SET                                           0x150F
#define GL_SHADER_SOURCE_LENGTH                          0x8B88
#define GL_SHADER_TYPE                                   0x8B4F
#define GL_SHADING_LANGUAGE_VERSION                      0x8B8C
#define GL_SHORT                                         0x1402
#define GL_SIGNALED                                      0x9119
#define GL_SIGNED_NORMALIZED                             0x8F9C
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY                 0x0B23
#define GL_SMOOTH_LINE_WIDTH_RANGE                       0x0B22
#define GL_SMOOTH_POINT_SIZE_GRANULARITY                 0x0B13
#define GL_SMOOTH_POINT_SIZE_RANGE                       0x0B12
#define GL_SRC1_ALPHA                                    0x8589
#define GL_SRC_ALPHA                                     0x0302
#define GL_SRC_ALPHA_SATURATE                            0x0308
#define GL_SRC_COLOR                                     0x0300
#define GL_SRGB                                          0x8C40
#define GL_SRGB8                                         0x8C41
#define GL_SRGB8_ALPHA8                                  0x8C43
#define GL_SRGB_ALPHA                                    0x8C42
#define GL_STATIC_COPY                                   0x88E6
#define GL_STATIC_DRAW                                   0x88E4
#define GL_STATIC_READ                                   0x88E5
#define GL_STENCIL                                       0x1802
#define GL_STENCIL_ATTACHMENT                            0x8D20
#define GL_STENCIL_BACK_FAIL                             0x8801
#define GL_STENCIL_BACK_FUNC                             0x8800
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                  0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                  0x8803
#define GL_STENCIL_BACK_REF                              0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK                       0x8CA4
#define GL_STENCIL_BACK_WRITEMASK                        0x8CA5
#define GL_STENCIL_BUFFER_BIT                            0x00000400
#define GL_STENCIL_CLEAR_VALUE                           0x0B91
#define GL_STENCIL_FAIL                                  0x0B94
#define GL_STENCIL_FUNC                                  0x0B92
#define GL_STENCIL_INDEX                                 0x1901
#define GL_STENCIL_INDEX1                                0x8D46
#define GL_STENCIL_INDEX16                               0x8D49
#define GL_STENCIL_INDEX4                                0x8D47
#define GL_STENCIL_INDEX8                                0x8D48
#define GL_STENCIL_PASS_DEPTH_FAIL                       0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS                       0x0B96
#define GL_STENCIL_REF                                   0x0B97
#define GL_STENCIL_TEST                                  0x0B90
#define GL_STENCIL_VALUE_MASK                            0x0B93
#define GL_STENCIL_WRITEMASK                             0x0B98
#define GL_STEREO                                        0x0C33
#define GL_STREAM_COPY                                   0x88E2
#define GL_STREAM_DRAW                                   0x88E0
#define GL_STREAM_READ                                   0x88E1
#define GL_SUBPIXEL_BITS                                 0x0D50
#define GL_SYNC_CONDITION                                0x9113
#define GL_SYNC_FENCE                                    0x9116
#define GL_SYNC_FLAGS                                    0x9115
#define GL_SYNC_FLUSH_COMMANDS_BIT                       0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE                    0x9117
#define GL_SYNC_STATUS                                   0x9114
#define GL_TEXTURE                                       0x1702
#define GL_TEXTURE0                                      0x84C0
#define GL_TEXTURE1                                      0x84C1
#define GL_TEXTURE10                                     0x84CA
#define GL_TEXTURE11                                     0x84CB
#define GL_TEXTURE12                                     0x84CC
#define GL_TEXTURE13                                     0x84CD
#define GL_TEXTURE14                                     0x84CE
#define GL_TEXTURE15                                     0x84CF
#define GL_TEXTURE16                                     0x84D0
#define GL_TEXTURE17                                     0x84D1
#define GL_TEXTURE18                                     0x84D2
#define GL_TEXTURE19                                     0x84D3
#define GL_TEXTURE2                                      0x84C2
#define GL_TEXTURE20                                     0x84D4
#define GL_TEXTURE21                                     0x84D5
#define GL_TEXTURE22                                     0x84D6
#define GL_TEXTURE23                                     0x84D7
#define GL_TEXTURE24                                     0x84D8
#define GL_TEXTURE25                                     0x84D9
#define GL_TEXTURE26                                     0x84DA
#define GL_TEXTURE27                                     0x84DB
#define GL_TEXTURE28                                     0x84DC
#define GL_TEXTURE29                                     0x84DD
#define GL_TEXTURE3                                      0x84C3
#define GL_TEXTURE30                                     0x84DE
#define GL_TEXTURE31                                     0x84DF
#define GL_TEXTURE4                                      0x84C4
#define GL_TEXTURE5                                      0x84C5
#define GL_TEXTURE6                                      0x84C6
#define GL_TEXTURE7                                      0x84C7
#define GL_TEXTURE8                                      0x84C8
#define GL_TEXTURE9                                      0x84C9
#define GL_TEXTURE_1D                                    0x0DE0
#define GL_TEXTURE_1D_ARRAY                              0x8C18
#define GL_TEXTURE_2D                                    0x0DE1
#define GL_TEXTURE_2D_ARRAY                              0x8C1A
#define GL_TEXTURE_2D_MULTISAMPLE                        0x9100
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY                  0x9102
#define GL_TEXTURE_3D                                    0x806F
#define GL_TEXTURE_ALPHA_SIZE                            0x805F
#define GL_TEXTURE_ALPHA_TYPE                            0x8C13
#define GL_TEXTURE_BASE_LEVEL                            0x813C
#define GL_TEXTURE_BINDING_1D                            0x8068
#define GL_TEXTURE_BINDING_1D_ARRAY                      0x8C1C
#define GL_TEXTURE_BINDING_2D                            0x8069
#define GL_TEXTURE_BINDING_2D_ARRAY                      0x8C1D
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE                0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY          0x9105
#define GL_TEXTURE_BINDING_3D                            0x806A
#define GL_TEXTURE_BINDING_BUFFER                        0x8C2C
#define GL_TEXTURE_BINDING_CUBE_MAP                      0x8514
#define GL_TEXTURE_BINDING_RECTANGLE                     0x84F6
#define GL_TEXTURE_BLUE_SIZE                             0x805E
#define GL_TEXTURE_BLUE_TYPE                             0x8C12
#define GL_TEXTURE_BORDER_COLOR                          0x1004
#define GL_TEXTURE_BUFFER                                0x8C2A
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING             0x8C2D
#define GL_TEXTURE_COMPARE_FUNC                          0x884D
#define GL_TEXTURE_COMPARE_MODE                          0x884C
#define GL_TEXTURE_COMPRESSED                            0x86A1
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE                 0x86A0
#define GL_TEXTURE_COMPRESSION_HINT                      0x84EF
#define GL_TEXTURE_CUBE_MAP                              0x8513
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                   0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                   0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                   0x851A
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                   0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                   0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                   0x8519
#define GL_TEXTURE_CUBE_MAP_SEAMLESS                     0x884F
#define GL_TEXTURE_DEPTH                                 0x8071
#define GL_TEXTURE_DEPTH_SIZE                            0x884A
#define GL_TEXTURE_DEPTH_TYPE                            0x8C16
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS                0x9107
#define GL_TEXTURE_GREEN_SIZE                            0x805D
#define GL_TEXTURE_GREEN_TYPE                            0x8C11
#define GL_TEXTURE_HEIGHT                                0x1001
#define GL_TEXTURE_INTERNAL_FORMAT                       0x1003
#define GL_TEXTURE_LOD_BIAS                              0x8501
#define GL_TEXTURE_MAG_FILTER                            0x2800
#define GL_TEXTURE_MAX_LEVEL                             0x813D
#define GL_TEXTURE_MAX_LOD                               0x813B
#define GL_TEXTURE_MIN_FILTER                            0x2801
#define GL_TEXTURE_MIN_LOD                               0x813A
#define GL_TEXTURE_RECTANGLE                             0x84F5
#define GL_TEXTURE_RED_SIZE                              0x805C
#define GL_TEXTURE_RED_TYPE                              0x8C10
#define GL_TEXTURE_SAMPLES                               0x9106
#define GL_TEXTURE_SHARED_SIZE                           0x8C3F
#define GL_TEXTURE_STENCIL_SIZE                          0x88F1
#define GL_TEXTURE_WIDTH                                 0x1000
#define GL_TEXTURE_WRAP_R                                0x8072
#define GL_TEXTURE_WRAP_S                                0x2802
#define GL_TEXTURE_WRAP_T                                0x2803
#define GL_TIMEOUT_EXPIRED                               0x911B
#define GL_TIMEOUT_IGNORED                               0xFFFFFFFFFFFFFFFF
#define GL_TRANSFORM_FEEDBACK_BUFFER                     0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING             0x8C8F
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE                0x8C7F
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                0x8C85
#define GL_TRANSFORM_FEEDBACK_BUFFER_START               0x8C84
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN         0x8C88
#define GL_TRANSFORM_FEEDBACK_VARYINGS                   0x8C83
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH         0x8C76
#define GL_TRIANGLES                                     0x0004
#define GL_TRIANGLES_ADJACENCY                           0x000C
#define GL_TRIANGLE_FAN                                  0x0006
#define GL_TRIANGLE_STRIP                                0x0005
#define GL_TRIANGLE_STRIP_ADJACENCY                      0x000D
#define GL_TRUE                                          1
#define GL_UNIFORM_ARRAY_STRIDE                          0x8A3C
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                 0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES          0x8A43
#define GL_UNIFORM_BLOCK_BINDING                         0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                       0x8A40
#define GL_UNIFORM_BLOCK_INDEX                           0x8A3A
#define GL_UNIFORM_BLOCK_NAME_LENGTH                     0x8A41
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER   0x8A46
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER   0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER     0x8A44
#define GL_UNIFORM_BUFFER                                0x8A11
#define GL_UNIFORM_BUFFER_BINDING                        0x8A28
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT               0x8A34
#define GL_UNIFORM_BUFFER_SIZE                           0x8A2A
#define GL_UNIFORM_BUFFER_START                          0x8A29
#define GL_UNIFORM_IS_ROW_MAJOR                          0x8A3E
#define GL_UNIFORM_MATRIX_STRIDE                         0x8A3D
#define GL_UNIFORM_NAME_LENGTH                           0x8A39
#define GL_UNIFORM_OFFSET                                0x8A3B
#define GL_UNIFORM_SIZE                                  0x8A38
#define GL_UNIFORM_TYPE                                  0x8A37
#define GL_UNPACK_ALIGNMENT                              0x0CF5
#define GL_UNPACK_IMAGE_HEIGHT                           0x806E
#define GL_UNPACK_LSB_FIRST                              0x0CF1
#define GL_UNPACK_ROW_LENGTH                             0x0CF2
#define GL_UNPACK_SKIP_IMAGES                            0x806D
#define GL_UNPACK_SKIP_PIXELS                            0x0CF4
#define GL_UNPACK_SKIP_ROWS                              0x0CF3
#define GL_UNPACK_SWAP_BYTES                             0x0CF0
#define GL_UNSIGNALED                                    0x9118
#define GL_UNSIGNED_BYTE                                 0x1401
#define GL_UNSIGNED_BYTE_2_3_3_REV                       0x8362
#define GL_UNSIGNED_BYTE_3_3_2                           0x8032
#define GL_UNSIGNED_INT                                  0x1405
#define GL_UNSIGNED_INT_10F_11F_11F_REV                  0x8C3B
#define GL_UNSIGNED_INT_10_10_10_2                       0x8036
#define GL_UNSIGNED_INT_24_8                             0x84FA
#define GL_UNSIGNED_INT_2_10_10_10_REV                   0x8368
#define GL_UNSIGNED_INT_5_9_9_9_REV                      0x8C3E
#define GL_UNSIGNED_INT_8_8_8_8                          0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV                      0x8367
#define GL_UNSIGNED_INT_SAMPLER_1D                       0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY                 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D                       0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                 0x8DD7
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE           0x910A
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY     0x910D
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT                  0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_3D                       0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_BUFFER                   0x8DD8
#define GL_UNSIGNED_INT_SAMPLER_CUBE                     0x8DD4
#define GL_UNSIGNED_INT_VEC2                             0x8DC6
#define GL_UNSIGNED_INT_VEC3                             0x8DC7
#define GL_UNSIGNED_INT_VEC4                             0x8DC8
#define GL_UNSIGNED_NORMALIZED                           0x8C17
#define GL_UNSIGNED_SHORT                                0x1403
#define GL_UNSIGNED_SHORT_1_5_5_5_REV                    0x8366
#define GL_UNSIGNED_SHORT_4_4_4_4                        0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV                    0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1                        0x8034
#define GL_UNSIGNED_SHORT_5_6_5                          0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV                      0x8364
#define GL_UPPER_LEFT                                    0x8CA2
#define GL_VALIDATE_STATUS                               0x8B83
#define GL_VENDOR                                        0x1F00
#define GL_VERSION                                       0x1F02
#define GL_VERTEX_ARRAY_BINDING                          0x85B5
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING            0x889F
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                   0x8622
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                   0x88FD
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                   0x8645
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                      0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                    0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                      0x8625
#define GL_VERTEX_PROGRAM_POINT_SIZE                     0x8642
#define GL_VERTEX_SHADER                                 0x8B31
#define GL_VIEWPORT                                      0x0BA2
#define GL_WAIT_FAILED                                   0x911D
#define GL_WRITE_ONLY                                    0x88B9
#define GL_XOR                                           0x1506
#define GL_ZERO                                          0
