#pragma once

// This file is a part of CGLFL (configurable OpenGL function loader).
// Generated, do no edit!
//
// Version: 1.0.0
// API: gles 2.0
// Extensions: none

#define CGLFL_GL_MAJOR 2
#define CGLFL_GL_MINOR 0
#define CGLFL_GL_API_gles
#define CGLFL_GL_PROFILE_none

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

#define CGLFL_FUNC_COUNT 142

#define glActiveTexture                       CGLFL_CALL(0,glActiveTexture,void,1,(texture),(GLenum texture))
#define glAttachShader                        CGLFL_CALL(1,glAttachShader,void,2,(program,shader),(GLuint program,GLuint shader))
#define glBindAttribLocation                  CGLFL_CALL(2,glBindAttribLocation,void,3,(program,index,name),(GLuint program,GLuint index,const GLchar *name))
#define glBindBuffer                          CGLFL_CALL(3,glBindBuffer,void,2,(target,buffer),(GLenum target,GLuint buffer))
#define glBindFramebuffer                     CGLFL_CALL(4,glBindFramebuffer,void,2,(target,framebuffer),(GLenum target,GLuint framebuffer))
#define glBindRenderbuffer                    CGLFL_CALL(5,glBindRenderbuffer,void,2,(target,renderbuffer),(GLenum target,GLuint renderbuffer))
#define glBindTexture                         CGLFL_CALL(6,glBindTexture,void,2,(target,texture),(GLenum target,GLuint texture))
#define glBlendColor                          CGLFL_CALL(7,glBlendColor,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glBlendEquation                       CGLFL_CALL(8,glBlendEquation,void,1,(mode),(GLenum mode))
#define glBlendEquationSeparate               CGLFL_CALL(9,glBlendEquationSeparate,void,2,(modeRGB,modeAlpha),(GLenum modeRGB,GLenum modeAlpha))
#define glBlendFunc                           CGLFL_CALL(10,glBlendFunc,void,2,(sfactor,dfactor),(GLenum sfactor,GLenum dfactor))
#define glBlendFuncSeparate                   CGLFL_CALL(11,glBlendFuncSeparate,void,4,(sfactorRGB,dfactorRGB,sfactorAlpha,dfactorAlpha),(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha))
#define glBufferData                          CGLFL_CALL(12,glBufferData,void,4,(target,size,data,usage),(GLenum target,GLsizeiptr size,const void *data,GLenum usage))
#define glBufferSubData                       CGLFL_CALL(13,glBufferSubData,void,4,(target,offset,size,data),(GLenum target,GLintptr offset,GLsizeiptr size,const void *data))
#define glCheckFramebufferStatus              CGLFL_CALL(14,glCheckFramebufferStatus,GLenum,1,(target),(GLenum target))
#define glClear                               CGLFL_CALL(15,glClear,void,1,(mask),(GLbitfield mask))
#define glClearColor                          CGLFL_CALL(16,glClearColor,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glClearDepthf                         CGLFL_CALL(17,glClearDepthf,void,1,(d),(GLfloat d))
#define glClearStencil                        CGLFL_CALL(18,glClearStencil,void,1,(s),(GLint s))
#define glColorMask                           CGLFL_CALL(19,glColorMask,void,4,(red,green,blue,alpha),(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha))
#define glCompileShader                       CGLFL_CALL(20,glCompileShader,void,1,(shader),(GLuint shader))
#define glCompressedTexImage2D                CGLFL_CALL(21,glCompressedTexImage2D,void,8,(target,level,internalformat,width,height,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage2D             CGLFL_CALL(22,glCompressedTexSubImage2D,void,9,(target,level,xoffset,yoffset,width,height,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void *data))
#define glCopyTexImage2D                      CGLFL_CALL(23,glCopyTexImage2D,void,8,(target,level,internalformat,x,y,width,height,border),(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border))
#define glCopyTexSubImage2D                   CGLFL_CALL(24,glCopyTexSubImage2D,void,8,(target,level,xoffset,yoffset,x,y,width,height),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height))
#define glCreateProgram                       CGLFL_CALL(25,glCreateProgram,GLuint,0,(),())
#define glCreateShader                        CGLFL_CALL(26,glCreateShader,GLuint,1,(type),(GLenum type))
#define glCullFace                            CGLFL_CALL(27,glCullFace,void,1,(mode),(GLenum mode))
#define glDeleteBuffers                       CGLFL_CALL(28,glDeleteBuffers,void,2,(n,buffers),(GLsizei n,const GLuint *buffers))
#define glDeleteFramebuffers                  CGLFL_CALL(29,glDeleteFramebuffers,void,2,(n,framebuffers),(GLsizei n,const GLuint *framebuffers))
#define glDeleteProgram                       CGLFL_CALL(30,glDeleteProgram,void,1,(program),(GLuint program))
#define glDeleteRenderbuffers                 CGLFL_CALL(31,glDeleteRenderbuffers,void,2,(n,renderbuffers),(GLsizei n,const GLuint *renderbuffers))
#define glDeleteShader                        CGLFL_CALL(32,glDeleteShader,void,1,(shader),(GLuint shader))
#define glDeleteTextures                      CGLFL_CALL(33,glDeleteTextures,void,2,(n,textures),(GLsizei n,const GLuint *textures))
#define glDepthFunc                           CGLFL_CALL(34,glDepthFunc,void,1,(func),(GLenum func))
#define glDepthMask                           CGLFL_CALL(35,glDepthMask,void,1,(flag),(GLboolean flag))
#define glDepthRangef                         CGLFL_CALL(36,glDepthRangef,void,2,(n,f),(GLfloat n,GLfloat f))
#define glDetachShader                        CGLFL_CALL(37,glDetachShader,void,2,(program,shader),(GLuint program,GLuint shader))
#define glDisable                             CGLFL_CALL(38,glDisable,void,1,(cap),(GLenum cap))
#define glDisableVertexAttribArray            CGLFL_CALL(39,glDisableVertexAttribArray,void,1,(index),(GLuint index))
#define glDrawArrays                          CGLFL_CALL(40,glDrawArrays,void,3,(mode,first,count),(GLenum mode,GLint first,GLsizei count))
#define glDrawElements                        CGLFL_CALL(41,glDrawElements,void,4,(mode,count,type,indices),(GLenum mode,GLsizei count,GLenum type,const void *indices))
#define glEnable                              CGLFL_CALL(42,glEnable,void,1,(cap),(GLenum cap))
#define glEnableVertexAttribArray             CGLFL_CALL(43,glEnableVertexAttribArray,void,1,(index),(GLuint index))
#define glFinish                              CGLFL_CALL(44,glFinish,void,0,(),())
#define glFlush                               CGLFL_CALL(45,glFlush,void,0,(),())
#define glFramebufferRenderbuffer             CGLFL_CALL(46,glFramebufferRenderbuffer,void,4,(target,attachment,renderbuffertarget,renderbuffer),(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer))
#define glFramebufferTexture2D                CGLFL_CALL(47,glFramebufferTexture2D,void,5,(target,attachment,textarget,texture,level),(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level))
#define glFrontFace                           CGLFL_CALL(48,glFrontFace,void,1,(mode),(GLenum mode))
#define glGenBuffers                          CGLFL_CALL(49,glGenBuffers,void,2,(n,buffers),(GLsizei n,GLuint *buffers))
#define glGenFramebuffers                     CGLFL_CALL(50,glGenFramebuffers,void,2,(n,framebuffers),(GLsizei n,GLuint *framebuffers))
#define glGenRenderbuffers                    CGLFL_CALL(51,glGenRenderbuffers,void,2,(n,renderbuffers),(GLsizei n,GLuint *renderbuffers))
#define glGenTextures                         CGLFL_CALL(52,glGenTextures,void,2,(n,textures),(GLsizei n,GLuint *textures))
#define glGenerateMipmap                      CGLFL_CALL(53,glGenerateMipmap,void,1,(target),(GLenum target))
#define glGetActiveAttrib                     CGLFL_CALL(54,glGetActiveAttrib,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLint *size,GLenum *type,GLchar *name))
#define glGetActiveUniform                    CGLFL_CALL(55,glGetActiveUniform,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLint *size,GLenum *type,GLchar *name))
#define glGetAttachedShaders                  CGLFL_CALL(56,glGetAttachedShaders,void,4,(program,maxCount,count,shaders),(GLuint program,GLsizei maxCount,GLsizei *count,GLuint *shaders))
#define glGetAttribLocation                   CGLFL_CALL(57,glGetAttribLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetBooleanv                         CGLFL_CALL(58,glGetBooleanv,void,2,(pname,data),(GLenum pname,GLboolean *data))
#define glGetBufferParameteriv                CGLFL_CALL(59,glGetBufferParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetError                            CGLFL_CALL(60,glGetError,GLenum,0,(),())
#define glGetFloatv                           CGLFL_CALL(61,glGetFloatv,void,2,(pname,data),(GLenum pname,GLfloat *data))
#define glGetFramebufferAttachmentParameteriv CGLFL_CALL(62,glGetFramebufferAttachmentParameteriv,void,4,(target,attachment,pname,params),(GLenum target,GLenum attachment,GLenum pname,GLint *params))
#define glGetIntegerv                         CGLFL_CALL(63,glGetIntegerv,void,2,(pname,data),(GLenum pname,GLint *data))
#define glGetProgramInfoLog                   CGLFL_CALL(64,glGetProgramInfoLog,void,4,(program,bufSize,length,infoLog),(GLuint program,GLsizei bufSize,GLsizei *length,GLchar *infoLog))
#define glGetProgramiv                        CGLFL_CALL(65,glGetProgramiv,void,3,(program,pname,params),(GLuint program,GLenum pname,GLint *params))
#define glGetRenderbufferParameteriv          CGLFL_CALL(66,glGetRenderbufferParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetShaderInfoLog                    CGLFL_CALL(67,glGetShaderInfoLog,void,4,(shader,bufSize,length,infoLog),(GLuint shader,GLsizei bufSize,GLsizei *length,GLchar *infoLog))
#define glGetShaderPrecisionFormat            CGLFL_CALL(68,glGetShaderPrecisionFormat,void,4,(shadertype,precisiontype,range,precision),(GLenum shadertype,GLenum precisiontype,GLint *range,GLint *precision))
#define glGetShaderSource                     CGLFL_CALL(69,glGetShaderSource,void,4,(shader,bufSize,length,source),(GLuint shader,GLsizei bufSize,GLsizei *length,GLchar *source))
#define glGetShaderiv                         CGLFL_CALL(70,glGetShaderiv,void,3,(shader,pname,params),(GLuint shader,GLenum pname,GLint *params))
#define glGetString                           CGLFL_CALL(71,glGetString,const GLubyte *,1,(name),(GLenum name))
#define glGetTexParameterfv                   CGLFL_CALL(72,glGetTexParameterfv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLfloat *params))
#define glGetTexParameteriv                   CGLFL_CALL(73,glGetTexParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetUniformLocation                  CGLFL_CALL(74,glGetUniformLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetUniformfv                        CGLFL_CALL(75,glGetUniformfv,void,3,(program,location,params),(GLuint program,GLint location,GLfloat *params))
#define glGetUniformiv                        CGLFL_CALL(76,glGetUniformiv,void,3,(program,location,params),(GLuint program,GLint location,GLint *params))
#define glGetVertexAttribPointerv             CGLFL_CALL(77,glGetVertexAttribPointerv,void,3,(index,pname,pointer),(GLuint index,GLenum pname,void **pointer))
#define glGetVertexAttribfv                   CGLFL_CALL(78,glGetVertexAttribfv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLfloat *params))
#define glGetVertexAttribiv                   CGLFL_CALL(79,glGetVertexAttribiv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLint *params))
#define glHint                                CGLFL_CALL(80,glHint,void,2,(target,mode),(GLenum target,GLenum mode))
#define glIsBuffer                            CGLFL_CALL(81,glIsBuffer,GLboolean,1,(buffer),(GLuint buffer))
#define glIsEnabled                           CGLFL_CALL(82,glIsEnabled,GLboolean,1,(cap),(GLenum cap))
#define glIsFramebuffer                       CGLFL_CALL(83,glIsFramebuffer,GLboolean,1,(framebuffer),(GLuint framebuffer))
#define glIsProgram                           CGLFL_CALL(84,glIsProgram,GLboolean,1,(program),(GLuint program))
#define glIsRenderbuffer                      CGLFL_CALL(85,glIsRenderbuffer,GLboolean,1,(renderbuffer),(GLuint renderbuffer))
#define glIsShader                            CGLFL_CALL(86,glIsShader,GLboolean,1,(shader),(GLuint shader))
#define glIsTexture                           CGLFL_CALL(87,glIsTexture,GLboolean,1,(texture),(GLuint texture))
#define glLineWidth                           CGLFL_CALL(88,glLineWidth,void,1,(width),(GLfloat width))
#define glLinkProgram                         CGLFL_CALL(89,glLinkProgram,void,1,(program),(GLuint program))
#define glPixelStorei                         CGLFL_CALL(90,glPixelStorei,void,2,(pname,param),(GLenum pname,GLint param))
#define glPolygonOffset                       CGLFL_CALL(91,glPolygonOffset,void,2,(factor,units),(GLfloat factor,GLfloat units))
#define glReadPixels                          CGLFL_CALL(92,glReadPixels,void,7,(x,y,width,height,format,type,pixels),(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void *pixels))
#define glReleaseShaderCompiler               CGLFL_CALL(93,glReleaseShaderCompiler,void,0,(),())
#define glRenderbufferStorage                 CGLFL_CALL(94,glRenderbufferStorage,void,4,(target,internalformat,width,height),(GLenum target,GLenum internalformat,GLsizei width,GLsizei height))
#define glSampleCoverage                      CGLFL_CALL(95,glSampleCoverage,void,2,(value,invert),(GLfloat value,GLboolean invert))
#define glScissor                             CGLFL_CALL(96,glScissor,void,4,(x,y,width,height),(GLint x,GLint y,GLsizei width,GLsizei height))
#define glShaderBinary                        CGLFL_CALL(97,glShaderBinary,void,5,(count,shaders,binaryformat,binary,length),(GLsizei count,const GLuint *shaders,GLenum binaryformat,const void *binary,GLsizei length))
#define glShaderSource                        CGLFL_CALL(98,glShaderSource,void,4,(shader,count,string,length),(GLuint shader,GLsizei count,const GLchar *const*string,const GLint *length))
#define glStencilFunc                         CGLFL_CALL(99,glStencilFunc,void,3,(func,ref,mask),(GLenum func,GLint ref,GLuint mask))
#define glStencilFuncSeparate                 CGLFL_CALL(100,glStencilFuncSeparate,void,4,(face,func,ref,mask),(GLenum face,GLenum func,GLint ref,GLuint mask))
#define glStencilMask                         CGLFL_CALL(101,glStencilMask,void,1,(mask),(GLuint mask))
#define glStencilMaskSeparate                 CGLFL_CALL(102,glStencilMaskSeparate,void,2,(face,mask),(GLenum face,GLuint mask))
#define glStencilOp                           CGLFL_CALL(103,glStencilOp,void,3,(fail,zfail,zpass),(GLenum fail,GLenum zfail,GLenum zpass))
#define glStencilOpSeparate                   CGLFL_CALL(104,glStencilOpSeparate,void,4,(face,sfail,dpfail,dppass),(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass))
#define glTexImage2D                          CGLFL_CALL(105,glTexImage2D,void,9,(target,level,internalformat,width,height,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexParameterf                       CGLFL_CALL(106,glTexParameterf,void,3,(target,pname,param),(GLenum target,GLenum pname,GLfloat param))
#define glTexParameterfv                      CGLFL_CALL(107,glTexParameterfv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLfloat *params))
#define glTexParameteri                       CGLFL_CALL(108,glTexParameteri,void,3,(target,pname,param),(GLenum target,GLenum pname,GLint param))
#define glTexParameteriv                      CGLFL_CALL(109,glTexParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLint *params))
#define glTexSubImage2D                       CGLFL_CALL(110,glTexSubImage2D,void,9,(target,level,xoffset,yoffset,width,height,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void *pixels))
#define glUniform1f                           CGLFL_CALL(111,glUniform1f,void,2,(location,v0),(GLint location,GLfloat v0))
#define glUniform1fv                          CGLFL_CALL(112,glUniform1fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform1i                           CGLFL_CALL(113,glUniform1i,void,2,(location,v0),(GLint location,GLint v0))
#define glUniform1iv                          CGLFL_CALL(114,glUniform1iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform2f                           CGLFL_CALL(115,glUniform2f,void,3,(location,v0,v1),(GLint location,GLfloat v0,GLfloat v1))
#define glUniform2fv                          CGLFL_CALL(116,glUniform2fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform2i                           CGLFL_CALL(117,glUniform2i,void,3,(location,v0,v1),(GLint location,GLint v0,GLint v1))
#define glUniform2iv                          CGLFL_CALL(118,glUniform2iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform3f                           CGLFL_CALL(119,glUniform3f,void,4,(location,v0,v1,v2),(GLint location,GLfloat v0,GLfloat v1,GLfloat v2))
#define glUniform3fv                          CGLFL_CALL(120,glUniform3fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform3i                           CGLFL_CALL(121,glUniform3i,void,4,(location,v0,v1,v2),(GLint location,GLint v0,GLint v1,GLint v2))
#define glUniform3iv                          CGLFL_CALL(122,glUniform3iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform4f                           CGLFL_CALL(123,glUniform4f,void,5,(location,v0,v1,v2,v3),(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3))
#define glUniform4fv                          CGLFL_CALL(124,glUniform4fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform4i                           CGLFL_CALL(125,glUniform4i,void,5,(location,v0,v1,v2,v3),(GLint location,GLint v0,GLint v1,GLint v2,GLint v3))
#define glUniform4iv                          CGLFL_CALL(126,glUniform4iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniformMatrix2fv                    CGLFL_CALL(127,glUniformMatrix2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3fv                    CGLFL_CALL(128,glUniformMatrix3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4fv                    CGLFL_CALL(129,glUniformMatrix4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUseProgram                          CGLFL_CALL(130,glUseProgram,void,1,(program),(GLuint program))
#define glValidateProgram                     CGLFL_CALL(131,glValidateProgram,void,1,(program),(GLuint program))
#define glVertexAttrib1f                      CGLFL_CALL(132,glVertexAttrib1f,void,2,(index,x),(GLuint index,GLfloat x))
#define glVertexAttrib1fv                     CGLFL_CALL(133,glVertexAttrib1fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib2f                      CGLFL_CALL(134,glVertexAttrib2f,void,3,(index,x,y),(GLuint index,GLfloat x,GLfloat y))
#define glVertexAttrib2fv                     CGLFL_CALL(135,glVertexAttrib2fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib3f                      CGLFL_CALL(136,glVertexAttrib3f,void,4,(index,x,y,z),(GLuint index,GLfloat x,GLfloat y,GLfloat z))
#define glVertexAttrib3fv                     CGLFL_CALL(137,glVertexAttrib3fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib4f                      CGLFL_CALL(138,glVertexAttrib4f,void,5,(index,x,y,z,w),(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w))
#define glVertexAttrib4fv                     CGLFL_CALL(139,glVertexAttrib4fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttribPointer                 CGLFL_CALL(140,glVertexAttribPointer,void,6,(index,size,type,normalized,stride,pointer),(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void *pointer))
#define glViewport                            CGLFL_CALL(141,glViewport,void,4,(x,y,width,height),(GLint x,GLint y,GLsizei width,GLsizei height))

#define GL_ACTIVE_ATTRIBUTES                            0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                  0x8B8A
#define GL_ACTIVE_TEXTURE                               0x84E0
#define GL_ACTIVE_UNIFORMS                              0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                    0x8B87
#define GL_ALIASED_LINE_WIDTH_RANGE                     0x846E
#define GL_ALIASED_POINT_SIZE_RANGE                     0x846D
#define GL_ALPHA                                        0x1906
#define GL_ALPHA_BITS                                   0x0D55
#define GL_ALWAYS                                       0x0207
#define GL_ARRAY_BUFFER                                 0x8892
#define GL_ARRAY_BUFFER_BINDING                         0x8894
#define GL_ATTACHED_SHADERS                             0x8B85
#define GL_BACK                                         0x0405
#define GL_BLEND                                        0x0BE2
#define GL_BLEND_COLOR                                  0x8005
#define GL_BLEND_DST_ALPHA                              0x80CA
#define GL_BLEND_DST_RGB                                0x80C8
#define GL_BLEND_EQUATION                               0x8009
#define GL_BLEND_EQUATION_ALPHA                         0x883D
#define GL_BLEND_EQUATION_RGB                           0x8009
#define GL_BLEND_SRC_ALPHA                              0x80CB
#define GL_BLEND_SRC_RGB                                0x80C9
#define GL_BLUE_BITS                                    0x0D54
#define GL_BOOL                                         0x8B56
#define GL_BOOL_VEC2                                    0x8B57
#define GL_BOOL_VEC3                                    0x8B58
#define GL_BOOL_VEC4                                    0x8B59
#define GL_BUFFER_SIZE                                  0x8764
#define GL_BUFFER_USAGE                                 0x8765
#define GL_BYTE                                         0x1400
#define GL_CCW                                          0x0901
#define GL_CLAMP_TO_EDGE                                0x812F
#define GL_COLOR_ATTACHMENT0                            0x8CE0
#define GL_COLOR_BUFFER_BIT                             0x00004000
#define GL_COLOR_CLEAR_VALUE                            0x0C22
#define GL_COLOR_WRITEMASK                              0x0C23
#define GL_COMPILE_STATUS                               0x8B81
#define GL_COMPRESSED_TEXTURE_FORMATS                   0x86A3
#define GL_CONSTANT_ALPHA                               0x8003
#define GL_CONSTANT_COLOR                               0x8001
#define GL_CULL_FACE                                    0x0B44
#define GL_CULL_FACE_MODE                               0x0B45
#define GL_CURRENT_PROGRAM                              0x8B8D
#define GL_CURRENT_VERTEX_ATTRIB                        0x8626
#define GL_CW                                           0x0900
#define GL_DECR                                         0x1E03
#define GL_DECR_WRAP                                    0x8508
#define GL_DELETE_STATUS                                0x8B80
#define GL_DEPTH_ATTACHMENT                             0x8D00
#define GL_DEPTH_BITS                                   0x0D56
#define GL_DEPTH_BUFFER_BIT                             0x00000100
#define GL_DEPTH_CLEAR_VALUE                            0x0B73
#define GL_DEPTH_COMPONENT                              0x1902
#define GL_DEPTH_COMPONENT16                            0x81A5
#define GL_DEPTH_FUNC                                   0x0B74
#define GL_DEPTH_RANGE                                  0x0B70
#define GL_DEPTH_TEST                                   0x0B71
#define GL_DEPTH_WRITEMASK                              0x0B72
#define GL_DITHER                                       0x0BD0
#define GL_DONT_CARE                                    0x1100
#define GL_DST_ALPHA                                    0x0304
#define GL_DST_COLOR                                    0x0306
#define GL_DYNAMIC_DRAW                                 0x88E8
#define GL_ELEMENT_ARRAY_BUFFER                         0x8893
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                 0x8895
#define GL_EQUAL                                        0x0202
#define GL_EXTENSIONS                                   0x1F03
#define GL_FALSE                                        0
#define GL_FASTEST                                      0x1101
#define GL_FIXED                                        0x140C
#define GL_FLOAT                                        0x1406
#define GL_FLOAT_MAT2                                   0x8B5A
#define GL_FLOAT_MAT3                                   0x8B5B
#define GL_FLOAT_MAT4                                   0x8B5C
#define GL_FLOAT_VEC2                                   0x8B50
#define GL_FLOAT_VEC3                                   0x8B51
#define GL_FLOAT_VEC4                                   0x8B52
#define GL_FRAGMENT_SHADER                              0x8B30
#define GL_FRAMEBUFFER                                  0x8D40
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         0x8CD2
#define GL_FRAMEBUFFER_BINDING                          0x8CA6
#define GL_FRAMEBUFFER_COMPLETE                         0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT            0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS            0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT    0x8CD7
#define GL_FRAMEBUFFER_UNSUPPORTED                      0x8CDD
#define GL_FRONT                                        0x0404
#define GL_FRONT_AND_BACK                               0x0408
#define GL_FRONT_FACE                                   0x0B46
#define GL_FUNC_ADD                                     0x8006
#define GL_FUNC_REVERSE_SUBTRACT                        0x800B
#define GL_FUNC_SUBTRACT                                0x800A
#define GL_GENERATE_MIPMAP_HINT                         0x8192
#define GL_GEQUAL                                       0x0206
#define GL_GREATER                                      0x0204
#define GL_GREEN_BITS                                   0x0D53
#define GL_HIGH_FLOAT                                   0x8DF2
#define GL_HIGH_INT                                     0x8DF5
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT             0x8B9B
#define GL_IMPLEMENTATION_COLOR_READ_TYPE               0x8B9A
#define GL_INCR                                         0x1E02
#define GL_INCR_WRAP                                    0x8507
#define GL_INFO_LOG_LENGTH                              0x8B84
#define GL_INT                                          0x1404
#define GL_INT_VEC2                                     0x8B53
#define GL_INT_VEC3                                     0x8B54
#define GL_INT_VEC4                                     0x8B55
#define GL_INVALID_ENUM                                 0x0500
#define GL_INVALID_FRAMEBUFFER_OPERATION                0x0506
#define GL_INVALID_OPERATION                            0x0502
#define GL_INVALID_VALUE                                0x0501
#define GL_INVERT                                       0x150A
#define GL_KEEP                                         0x1E00
#define GL_LEQUAL                                       0x0203
#define GL_LESS                                         0x0201
#define GL_LINEAR                                       0x2601
#define GL_LINEAR_MIPMAP_LINEAR                         0x2703
#define GL_LINEAR_MIPMAP_NEAREST                        0x2701
#define GL_LINES                                        0x0001
#define GL_LINE_LOOP                                    0x0002
#define GL_LINE_STRIP                                   0x0003
#define GL_LINE_WIDTH                                   0x0B21
#define GL_LINK_STATUS                                  0x8B82
#define GL_LOW_FLOAT                                    0x8DF0
#define GL_LOW_INT                                      0x8DF3
#define GL_LUMINANCE                                    0x1909
#define GL_LUMINANCE_ALPHA                              0x190A
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS             0x8B4D
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                    0x851C
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS                 0x8DFD
#define GL_MAX_RENDERBUFFER_SIZE                        0x84E8
#define GL_MAX_TEXTURE_IMAGE_UNITS                      0x8872
#define GL_MAX_TEXTURE_SIZE                             0x0D33
#define GL_MAX_VARYING_VECTORS                          0x8DFC
#define GL_MAX_VERTEX_ATTRIBS                           0x8869
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS               0x8B4C
#define GL_MAX_VERTEX_UNIFORM_VECTORS                   0x8DFB
#define GL_MAX_VIEWPORT_DIMS                            0x0D3A
#define GL_MEDIUM_FLOAT                                 0x8DF1
#define GL_MEDIUM_INT                                   0x8DF4
#define GL_MIRRORED_REPEAT                              0x8370
#define GL_NEAREST                                      0x2600
#define GL_NEAREST_MIPMAP_LINEAR                        0x2702
#define GL_NEAREST_MIPMAP_NEAREST                       0x2700
#define GL_NEVER                                        0x0200
#define GL_NICEST                                       0x1102
#define GL_NONE                                         0
#define GL_NOTEQUAL                                     0x0205
#define GL_NO_ERROR                                     0
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS               0x86A2
#define GL_NUM_SHADER_BINARY_FORMATS                    0x8DF9
#define GL_ONE                                          1
#define GL_ONE_MINUS_CONSTANT_ALPHA                     0x8004
#define GL_ONE_MINUS_CONSTANT_COLOR                     0x8002
#define GL_ONE_MINUS_DST_ALPHA                          0x0305
#define GL_ONE_MINUS_DST_COLOR                          0x0307
#define GL_ONE_MINUS_SRC_ALPHA                          0x0303
#define GL_ONE_MINUS_SRC_COLOR                          0x0301
#define GL_OUT_OF_MEMORY                                0x0505
#define GL_PACK_ALIGNMENT                               0x0D05
#define GL_POINTS                                       0x0000
#define GL_POLYGON_OFFSET_FACTOR                        0x8038
#define GL_POLYGON_OFFSET_FILL                          0x8037
#define GL_POLYGON_OFFSET_UNITS                         0x2A00
#define GL_RED_BITS                                     0x0D52
#define GL_RENDERBUFFER                                 0x8D41
#define GL_RENDERBUFFER_ALPHA_SIZE                      0x8D53
#define GL_RENDERBUFFER_BINDING                         0x8CA7
#define GL_RENDERBUFFER_BLUE_SIZE                       0x8D52
#define GL_RENDERBUFFER_DEPTH_SIZE                      0x8D54
#define GL_RENDERBUFFER_GREEN_SIZE                      0x8D51
#define GL_RENDERBUFFER_HEIGHT                          0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT                 0x8D44
#define GL_RENDERBUFFER_RED_SIZE                        0x8D50
#define GL_RENDERBUFFER_STENCIL_SIZE                    0x8D55
#define GL_RENDERBUFFER_WIDTH                           0x8D42
#define GL_RENDERER                                     0x1F01
#define GL_REPEAT                                       0x2901
#define GL_REPLACE                                      0x1E01
#define GL_RGB                                          0x1907
#define GL_RGB565                                       0x8D62
#define GL_RGB5_A1                                      0x8057
#define GL_RGBA                                         0x1908
#define GL_RGBA4                                        0x8056
#define GL_SAMPLER_2D                                   0x8B5E
#define GL_SAMPLER_CUBE                                 0x8B60
#define GL_SAMPLES                                      0x80A9
#define GL_SAMPLE_ALPHA_TO_COVERAGE                     0x809E
#define GL_SAMPLE_BUFFERS                               0x80A8
#define GL_SAMPLE_COVERAGE                              0x80A0
#define GL_SAMPLE_COVERAGE_INVERT                       0x80AB
#define GL_SAMPLE_COVERAGE_VALUE                        0x80AA
#define GL_SCISSOR_BOX                                  0x0C10
#define GL_SCISSOR_TEST                                 0x0C11
#define GL_SHADER_BINARY_FORMATS                        0x8DF8
#define GL_SHADER_COMPILER                              0x8DFA
#define GL_SHADER_SOURCE_LENGTH                         0x8B88
#define GL_SHADER_TYPE                                  0x8B4F
#define GL_SHADING_LANGUAGE_VERSION                     0x8B8C
#define GL_SHORT                                        0x1402
#define GL_SRC_ALPHA                                    0x0302
#define GL_SRC_ALPHA_SATURATE                           0x0308
#define GL_SRC_COLOR                                    0x0300
#define GL_STATIC_DRAW                                  0x88E4
#define GL_STENCIL_ATTACHMENT                           0x8D20
#define GL_STENCIL_BACK_FAIL                            0x8801
#define GL_STENCIL_BACK_FUNC                            0x8800
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                 0x8803
#define GL_STENCIL_BACK_REF                             0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK                      0x8CA4
#define GL_STENCIL_BACK_WRITEMASK                       0x8CA5
#define GL_STENCIL_BITS                                 0x0D57
#define GL_STENCIL_BUFFER_BIT                           0x00000400
#define GL_STENCIL_CLEAR_VALUE                          0x0B91
#define GL_STENCIL_FAIL                                 0x0B94
#define GL_STENCIL_FUNC                                 0x0B92
#define GL_STENCIL_INDEX8                               0x8D48
#define GL_STENCIL_PASS_DEPTH_FAIL                      0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS                      0x0B96
#define GL_STENCIL_REF                                  0x0B97
#define GL_STENCIL_TEST                                 0x0B90
#define GL_STENCIL_VALUE_MASK                           0x0B93
#define GL_STENCIL_WRITEMASK                            0x0B98
#define GL_STREAM_DRAW                                  0x88E0
#define GL_SUBPIXEL_BITS                                0x0D50
#define GL_TEXTURE                                      0x1702
#define GL_TEXTURE0                                     0x84C0
#define GL_TEXTURE1                                     0x84C1
#define GL_TEXTURE10                                    0x84CA
#define GL_TEXTURE11                                    0x84CB
#define GL_TEXTURE12                                    0x84CC
#define GL_TEXTURE13                                    0x84CD
#define GL_TEXTURE14                                    0x84CE
#define GL_TEXTURE15                                    0x84CF
#define GL_TEXTURE16                                    0x84D0
#define GL_TEXTURE17                                    0x84D1
#define GL_TEXTURE18                                    0x84D2
#define GL_TEXTURE19                                    0x84D3
#define GL_TEXTURE2                                     0x84C2
#define GL_TEXTURE20                                    0x84D4
#define GL_TEXTURE21                                    0x84D5
#define GL_TEXTURE22                                    0x84D6
#define GL_TEXTURE23                                    0x84D7
#define GL_TEXTURE24                                    0x84D8
#define GL_TEXTURE25                                    0x84D9
#define GL_TEXTURE26                                    0x84DA
#define GL_TEXTURE27                                    0x84DB
#define GL_TEXTURE28                                    0x84DC
#define GL_TEXTURE29                                    0x84DD
#define GL_TEXTURE3                                     0x84C3
#define GL_TEXTURE30                                    0x84DE
#define GL_TEXTURE31                                    0x84DF
#define GL_TEXTURE4                                     0x84C4
#define GL_TEXTURE5                                     0x84C5
#define GL_TEXTURE6                                     0x84C6
#define GL_TEXTURE7                                     0x84C7
#define GL_TEXTURE8                                     0x84C8
#define GL_TEXTURE9                                     0x84C9
#define GL_TEXTURE_2D                                   0x0DE1
#define GL_TEXTURE_BINDING_2D                           0x8069
#define GL_TEXTURE_BINDING_CUBE_MAP                     0x8514
#define GL_TEXTURE_CUBE_MAP                             0x8513
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                  0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                  0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                  0x851A
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                  0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                  0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                  0x8519
#define GL_TEXTURE_MAG_FILTER                           0x2800
#define GL_TEXTURE_MIN_FILTER                           0x2801
#define GL_TEXTURE_WRAP_S                               0x2802
#define GL_TEXTURE_WRAP_T                               0x2803
#define GL_TRIANGLES                                    0x0004
#define GL_TRIANGLE_FAN                                 0x0006
#define GL_TRIANGLE_STRIP                               0x0005
#define GL_TRUE                                         1
#define GL_UNPACK_ALIGNMENT                             0x0CF5
#define GL_UNSIGNED_BYTE                                0x1401
#define GL_UNSIGNED_INT                                 0x1405
#define GL_UNSIGNED_SHORT                               0x1403
#define GL_UNSIGNED_SHORT_4_4_4_4                       0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1                       0x8034
#define GL_UNSIGNED_SHORT_5_6_5                         0x8363
#define GL_VALIDATE_STATUS                              0x8B83
#define GL_VENDOR                                       0x1F00
#define GL_VERSION                                      0x1F02
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING           0x889F
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                  0x8622
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED               0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                  0x8645
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                     0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                   0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                     0x8625
#define GL_VERTEX_SHADER                                0x8B31
#define GL_VIEWPORT                                     0x0BA2
#define GL_ZERO                                         0
