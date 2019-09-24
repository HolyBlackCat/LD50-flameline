#pragma once

// This file is a part of CGLFL (configurable OpenGL function loader).
// Generated, do no edit!
//
// Version: 1.0.0
// API: gl 2.1
// Extensions: none

#define CGLFL_GL_MAJOR 2
#define CGLFL_GL_MINOR 1
#define CGLFL_GL_API_gl
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
#define CGLFL_IMPL_FOR_EACH_10(m, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7) m(p8) m(p9) m(p10)
#define CGLFL_IMPL_FOR_EACH_11(m, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11) m(p1) m(p2) m(p3) m(p4) m(p5) m(p6) m(p7) m(p8) m(p9) m(p10) m(p11)

#define CGLFL_FUNC_COUNT 551

#define glAccum                    CGLFL_CALL(0,glAccum,void,2,(op,value),(GLenum op,GLfloat value))
#define glActiveTexture            CGLFL_CALL(1,glActiveTexture,void,1,(texture),(GLenum texture))
#define glAlphaFunc                CGLFL_CALL(2,glAlphaFunc,void,2,(func,ref),(GLenum func,GLfloat ref))
#define glAreTexturesResident      CGLFL_CALL(3,glAreTexturesResident,GLboolean,3,(n,textures,residences),(GLsizei n,const GLuint *textures,GLboolean *residences))
#define glArrayElement             CGLFL_CALL(4,glArrayElement,void,1,(i),(GLint i))
#define glAttachShader             CGLFL_CALL(5,glAttachShader,void,2,(program,shader),(GLuint program,GLuint shader))
#define glBegin                    CGLFL_CALL(6,glBegin,void,1,(mode),(GLenum mode))
#define glBeginQuery               CGLFL_CALL(7,glBeginQuery,void,2,(target,id),(GLenum target,GLuint id))
#define glBindAttribLocation       CGLFL_CALL(8,glBindAttribLocation,void,3,(program,index,name),(GLuint program,GLuint index,const GLchar *name))
#define glBindBuffer               CGLFL_CALL(9,glBindBuffer,void,2,(target,buffer),(GLenum target,GLuint buffer))
#define glBindTexture              CGLFL_CALL(10,glBindTexture,void,2,(target,texture),(GLenum target,GLuint texture))
#define glBitmap                   CGLFL_CALL(11,glBitmap,void,7,(width,height,xorig,yorig,xmove,ymove,bitmap),(GLsizei width,GLsizei height,GLfloat xorig,GLfloat yorig,GLfloat xmove,GLfloat ymove,const GLubyte *bitmap))
#define glBlendColor               CGLFL_CALL(12,glBlendColor,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glBlendEquation            CGLFL_CALL(13,glBlendEquation,void,1,(mode),(GLenum mode))
#define glBlendEquationSeparate    CGLFL_CALL(14,glBlendEquationSeparate,void,2,(modeRGB,modeAlpha),(GLenum modeRGB,GLenum modeAlpha))
#define glBlendFunc                CGLFL_CALL(15,glBlendFunc,void,2,(sfactor,dfactor),(GLenum sfactor,GLenum dfactor))
#define glBlendFuncSeparate        CGLFL_CALL(16,glBlendFuncSeparate,void,4,(sfactorRGB,dfactorRGB,sfactorAlpha,dfactorAlpha),(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha))
#define glBufferData               CGLFL_CALL(17,glBufferData,void,4,(target,size,data,usage),(GLenum target,GLsizeiptr size,const void *data,GLenum usage))
#define glBufferSubData            CGLFL_CALL(18,glBufferSubData,void,4,(target,offset,size,data),(GLenum target,GLintptr offset,GLsizeiptr size,const void *data))
#define glCallList                 CGLFL_CALL(19,glCallList,void,1,(list),(GLuint list))
#define glCallLists                CGLFL_CALL(20,glCallLists,void,3,(n,type,lists),(GLsizei n,GLenum type,const void *lists))
#define glClear                    CGLFL_CALL(21,glClear,void,1,(mask),(GLbitfield mask))
#define glClearAccum               CGLFL_CALL(22,glClearAccum,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glClearColor               CGLFL_CALL(23,glClearColor,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glClearDepth               CGLFL_CALL(24,glClearDepth,void,1,(depth),(GLdouble depth))
#define glClearIndex               CGLFL_CALL(25,glClearIndex,void,1,(c),(GLfloat c))
#define glClearStencil             CGLFL_CALL(26,glClearStencil,void,1,(s),(GLint s))
#define glClientActiveTexture      CGLFL_CALL(27,glClientActiveTexture,void,1,(texture),(GLenum texture))
#define glClipPlane                CGLFL_CALL(28,glClipPlane,void,2,(plane,equation),(GLenum plane,const GLdouble *equation))
#define glColor3b                  CGLFL_CALL(29,glColor3b,void,3,(red,green,blue),(GLbyte red,GLbyte green,GLbyte blue))
#define glColor3bv                 CGLFL_CALL(30,glColor3bv,void,1,(v),(const GLbyte *v))
#define glColor3d                  CGLFL_CALL(31,glColor3d,void,3,(red,green,blue),(GLdouble red,GLdouble green,GLdouble blue))
#define glColor3dv                 CGLFL_CALL(32,glColor3dv,void,1,(v),(const GLdouble *v))
#define glColor3f                  CGLFL_CALL(33,glColor3f,void,3,(red,green,blue),(GLfloat red,GLfloat green,GLfloat blue))
#define glColor3fv                 CGLFL_CALL(34,glColor3fv,void,1,(v),(const GLfloat *v))
#define glColor3i                  CGLFL_CALL(35,glColor3i,void,3,(red,green,blue),(GLint red,GLint green,GLint blue))
#define glColor3iv                 CGLFL_CALL(36,glColor3iv,void,1,(v),(const GLint *v))
#define glColor3s                  CGLFL_CALL(37,glColor3s,void,3,(red,green,blue),(GLshort red,GLshort green,GLshort blue))
#define glColor3sv                 CGLFL_CALL(38,glColor3sv,void,1,(v),(const GLshort *v))
#define glColor3ub                 CGLFL_CALL(39,glColor3ub,void,3,(red,green,blue),(GLubyte red,GLubyte green,GLubyte blue))
#define glColor3ubv                CGLFL_CALL(40,glColor3ubv,void,1,(v),(const GLubyte *v))
#define glColor3ui                 CGLFL_CALL(41,glColor3ui,void,3,(red,green,blue),(GLuint red,GLuint green,GLuint blue))
#define glColor3uiv                CGLFL_CALL(42,glColor3uiv,void,1,(v),(const GLuint *v))
#define glColor3us                 CGLFL_CALL(43,glColor3us,void,3,(red,green,blue),(GLushort red,GLushort green,GLushort blue))
#define glColor3usv                CGLFL_CALL(44,glColor3usv,void,1,(v),(const GLushort *v))
#define glColor4b                  CGLFL_CALL(45,glColor4b,void,4,(red,green,blue,alpha),(GLbyte red,GLbyte green,GLbyte blue,GLbyte alpha))
#define glColor4bv                 CGLFL_CALL(46,glColor4bv,void,1,(v),(const GLbyte *v))
#define glColor4d                  CGLFL_CALL(47,glColor4d,void,4,(red,green,blue,alpha),(GLdouble red,GLdouble green,GLdouble blue,GLdouble alpha))
#define glColor4dv                 CGLFL_CALL(48,glColor4dv,void,1,(v),(const GLdouble *v))
#define glColor4f                  CGLFL_CALL(49,glColor4f,void,4,(red,green,blue,alpha),(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha))
#define glColor4fv                 CGLFL_CALL(50,glColor4fv,void,1,(v),(const GLfloat *v))
#define glColor4i                  CGLFL_CALL(51,glColor4i,void,4,(red,green,blue,alpha),(GLint red,GLint green,GLint blue,GLint alpha))
#define glColor4iv                 CGLFL_CALL(52,glColor4iv,void,1,(v),(const GLint *v))
#define glColor4s                  CGLFL_CALL(53,glColor4s,void,4,(red,green,blue,alpha),(GLshort red,GLshort green,GLshort blue,GLshort alpha))
#define glColor4sv                 CGLFL_CALL(54,glColor4sv,void,1,(v),(const GLshort *v))
#define glColor4ub                 CGLFL_CALL(55,glColor4ub,void,4,(red,green,blue,alpha),(GLubyte red,GLubyte green,GLubyte blue,GLubyte alpha))
#define glColor4ubv                CGLFL_CALL(56,glColor4ubv,void,1,(v),(const GLubyte *v))
#define glColor4ui                 CGLFL_CALL(57,glColor4ui,void,4,(red,green,blue,alpha),(GLuint red,GLuint green,GLuint blue,GLuint alpha))
#define glColor4uiv                CGLFL_CALL(58,glColor4uiv,void,1,(v),(const GLuint *v))
#define glColor4us                 CGLFL_CALL(59,glColor4us,void,4,(red,green,blue,alpha),(GLushort red,GLushort green,GLushort blue,GLushort alpha))
#define glColor4usv                CGLFL_CALL(60,glColor4usv,void,1,(v),(const GLushort *v))
#define glColorMask                CGLFL_CALL(61,glColorMask,void,4,(red,green,blue,alpha),(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha))
#define glColorMaterial            CGLFL_CALL(62,glColorMaterial,void,2,(face,mode),(GLenum face,GLenum mode))
#define glColorPointer             CGLFL_CALL(63,glColorPointer,void,4,(size,type,stride,pointer),(GLint size,GLenum type,GLsizei stride,const void *pointer))
#define glCompileShader            CGLFL_CALL(64,glCompileShader,void,1,(shader),(GLuint shader))
#define glCompressedTexImage1D     CGLFL_CALL(65,glCompressedTexImage1D,void,7,(target,level,internalformat,width,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexImage2D     CGLFL_CALL(66,glCompressedTexImage2D,void,8,(target,level,internalformat,width,height,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexImage3D     CGLFL_CALL(67,glCompressedTexImage3D,void,9,(target,level,internalformat,width,height,depth,border,imageSize,data),(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage1D  CGLFL_CALL(68,glCompressedTexSubImage1D,void,7,(target,level,xoffset,width,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage2D  CGLFL_CALL(69,glCompressedTexSubImage2D,void,9,(target,level,xoffset,yoffset,width,height,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void *data))
#define glCompressedTexSubImage3D  CGLFL_CALL(70,glCompressedTexSubImage3D,void,11,(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void *data))
#define glCopyPixels               CGLFL_CALL(71,glCopyPixels,void,5,(x,y,width,height,type),(GLint x,GLint y,GLsizei width,GLsizei height,GLenum type))
#define glCopyTexImage1D           CGLFL_CALL(72,glCopyTexImage1D,void,7,(target,level,internalformat,x,y,width,border),(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border))
#define glCopyTexImage2D           CGLFL_CALL(73,glCopyTexImage2D,void,8,(target,level,internalformat,x,y,width,height,border),(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border))
#define glCopyTexSubImage1D        CGLFL_CALL(74,glCopyTexSubImage1D,void,6,(target,level,xoffset,x,y,width),(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width))
#define glCopyTexSubImage2D        CGLFL_CALL(75,glCopyTexSubImage2D,void,8,(target,level,xoffset,yoffset,x,y,width,height),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height))
#define glCopyTexSubImage3D        CGLFL_CALL(76,glCopyTexSubImage3D,void,9,(target,level,xoffset,yoffset,zoffset,x,y,width,height),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height))
#define glCreateProgram            CGLFL_CALL(77,glCreateProgram,GLuint,0,(),())
#define glCreateShader             CGLFL_CALL(78,glCreateShader,GLuint,1,(type),(GLenum type))
#define glCullFace                 CGLFL_CALL(79,glCullFace,void,1,(mode),(GLenum mode))
#define glDeleteBuffers            CGLFL_CALL(80,glDeleteBuffers,void,2,(n,buffers),(GLsizei n,const GLuint *buffers))
#define glDeleteLists              CGLFL_CALL(81,glDeleteLists,void,2,(list,range),(GLuint list,GLsizei range))
#define glDeleteProgram            CGLFL_CALL(82,glDeleteProgram,void,1,(program),(GLuint program))
#define glDeleteQueries            CGLFL_CALL(83,glDeleteQueries,void,2,(n,ids),(GLsizei n,const GLuint *ids))
#define glDeleteShader             CGLFL_CALL(84,glDeleteShader,void,1,(shader),(GLuint shader))
#define glDeleteTextures           CGLFL_CALL(85,glDeleteTextures,void,2,(n,textures),(GLsizei n,const GLuint *textures))
#define glDepthFunc                CGLFL_CALL(86,glDepthFunc,void,1,(func),(GLenum func))
#define glDepthMask                CGLFL_CALL(87,glDepthMask,void,1,(flag),(GLboolean flag))
#define glDepthRange               CGLFL_CALL(88,glDepthRange,void,2,(n,f),(GLdouble n,GLdouble f))
#define glDetachShader             CGLFL_CALL(89,glDetachShader,void,2,(program,shader),(GLuint program,GLuint shader))
#define glDisable                  CGLFL_CALL(90,glDisable,void,1,(cap),(GLenum cap))
#define glDisableClientState       CGLFL_CALL(91,glDisableClientState,void,1,(array),(GLenum array))
#define glDisableVertexAttribArray CGLFL_CALL(92,glDisableVertexAttribArray,void,1,(index),(GLuint index))
#define glDrawArrays               CGLFL_CALL(93,glDrawArrays,void,3,(mode,first,count),(GLenum mode,GLint first,GLsizei count))
#define glDrawBuffer               CGLFL_CALL(94,glDrawBuffer,void,1,(buf),(GLenum buf))
#define glDrawBuffers              CGLFL_CALL(95,glDrawBuffers,void,2,(n,bufs),(GLsizei n,const GLenum *bufs))
#define glDrawElements             CGLFL_CALL(96,glDrawElements,void,4,(mode,count,type,indices),(GLenum mode,GLsizei count,GLenum type,const void *indices))
#define glDrawPixels               CGLFL_CALL(97,glDrawPixels,void,5,(width,height,format,type,pixels),(GLsizei width,GLsizei height,GLenum format,GLenum type,const void *pixels))
#define glDrawRangeElements        CGLFL_CALL(98,glDrawRangeElements,void,6,(mode,start,end,count,type,indices),(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void *indices))
#define glEdgeFlag                 CGLFL_CALL(99,glEdgeFlag,void,1,(flag),(GLboolean flag))
#define glEdgeFlagPointer          CGLFL_CALL(100,glEdgeFlagPointer,void,2,(stride,pointer),(GLsizei stride,const void *pointer))
#define glEdgeFlagv                CGLFL_CALL(101,glEdgeFlagv,void,1,(flag),(const GLboolean *flag))
#define glEnable                   CGLFL_CALL(102,glEnable,void,1,(cap),(GLenum cap))
#define glEnableClientState        CGLFL_CALL(103,glEnableClientState,void,1,(array),(GLenum array))
#define glEnableVertexAttribArray  CGLFL_CALL(104,glEnableVertexAttribArray,void,1,(index),(GLuint index))
#define glEnd                      CGLFL_CALL(105,glEnd,void,0,(),())
#define glEndList                  CGLFL_CALL(106,glEndList,void,0,(),())
#define glEndQuery                 CGLFL_CALL(107,glEndQuery,void,1,(target),(GLenum target))
#define glEvalCoord1d              CGLFL_CALL(108,glEvalCoord1d,void,1,(u),(GLdouble u))
#define glEvalCoord1dv             CGLFL_CALL(109,glEvalCoord1dv,void,1,(u),(const GLdouble *u))
#define glEvalCoord1f              CGLFL_CALL(110,glEvalCoord1f,void,1,(u),(GLfloat u))
#define glEvalCoord1fv             CGLFL_CALL(111,glEvalCoord1fv,void,1,(u),(const GLfloat *u))
#define glEvalCoord2d              CGLFL_CALL(112,glEvalCoord2d,void,2,(u,v),(GLdouble u,GLdouble v))
#define glEvalCoord2dv             CGLFL_CALL(113,glEvalCoord2dv,void,1,(u),(const GLdouble *u))
#define glEvalCoord2f              CGLFL_CALL(114,glEvalCoord2f,void,2,(u,v),(GLfloat u,GLfloat v))
#define glEvalCoord2fv             CGLFL_CALL(115,glEvalCoord2fv,void,1,(u),(const GLfloat *u))
#define glEvalMesh1                CGLFL_CALL(116,glEvalMesh1,void,3,(mode,i1,i2),(GLenum mode,GLint i1,GLint i2))
#define glEvalMesh2                CGLFL_CALL(117,glEvalMesh2,void,5,(mode,i1,i2,j1,j2),(GLenum mode,GLint i1,GLint i2,GLint j1,GLint j2))
#define glEvalPoint1               CGLFL_CALL(118,glEvalPoint1,void,1,(i),(GLint i))
#define glEvalPoint2               CGLFL_CALL(119,glEvalPoint2,void,2,(i,j),(GLint i,GLint j))
#define glFeedbackBuffer           CGLFL_CALL(120,glFeedbackBuffer,void,3,(size,type,buffer),(GLsizei size,GLenum type,GLfloat *buffer))
#define glFinish                   CGLFL_CALL(121,glFinish,void,0,(),())
#define glFlush                    CGLFL_CALL(122,glFlush,void,0,(),())
#define glFogCoordPointer          CGLFL_CALL(123,glFogCoordPointer,void,3,(type,stride,pointer),(GLenum type,GLsizei stride,const void *pointer))
#define glFogCoordd                CGLFL_CALL(124,glFogCoordd,void,1,(coord),(GLdouble coord))
#define glFogCoorddv               CGLFL_CALL(125,glFogCoorddv,void,1,(coord),(const GLdouble *coord))
#define glFogCoordf                CGLFL_CALL(126,glFogCoordf,void,1,(coord),(GLfloat coord))
#define glFogCoordfv               CGLFL_CALL(127,glFogCoordfv,void,1,(coord),(const GLfloat *coord))
#define glFogf                     CGLFL_CALL(128,glFogf,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glFogfv                    CGLFL_CALL(129,glFogfv,void,2,(pname,params),(GLenum pname,const GLfloat *params))
#define glFogi                     CGLFL_CALL(130,glFogi,void,2,(pname,param),(GLenum pname,GLint param))
#define glFogiv                    CGLFL_CALL(131,glFogiv,void,2,(pname,params),(GLenum pname,const GLint *params))
#define glFrontFace                CGLFL_CALL(132,glFrontFace,void,1,(mode),(GLenum mode))
#define glFrustum                  CGLFL_CALL(133,glFrustum,void,6,(left,right,bottom,top,zNear,zFar),(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble zNear,GLdouble zFar))
#define glGenBuffers               CGLFL_CALL(134,glGenBuffers,void,2,(n,buffers),(GLsizei n,GLuint *buffers))
#define glGenLists                 CGLFL_CALL(135,glGenLists,GLuint,1,(range),(GLsizei range))
#define glGenQueries               CGLFL_CALL(136,glGenQueries,void,2,(n,ids),(GLsizei n,GLuint *ids))
#define glGenTextures              CGLFL_CALL(137,glGenTextures,void,2,(n,textures),(GLsizei n,GLuint *textures))
#define glGetActiveAttrib          CGLFL_CALL(138,glGetActiveAttrib,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLint *size,GLenum *type,GLchar *name))
#define glGetActiveUniform         CGLFL_CALL(139,glGetActiveUniform,void,7,(program,index,bufSize,length,size,type,name),(GLuint program,GLuint index,GLsizei bufSize,GLsizei *length,GLint *size,GLenum *type,GLchar *name))
#define glGetAttachedShaders       CGLFL_CALL(140,glGetAttachedShaders,void,4,(program,maxCount,count,shaders),(GLuint program,GLsizei maxCount,GLsizei *count,GLuint *shaders))
#define glGetAttribLocation        CGLFL_CALL(141,glGetAttribLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetBooleanv              CGLFL_CALL(142,glGetBooleanv,void,2,(pname,data),(GLenum pname,GLboolean *data))
#define glGetBufferParameteriv     CGLFL_CALL(143,glGetBufferParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetBufferPointerv        CGLFL_CALL(144,glGetBufferPointerv,void,3,(target,pname,params),(GLenum target,GLenum pname,void **params))
#define glGetBufferSubData         CGLFL_CALL(145,glGetBufferSubData,void,4,(target,offset,size,data),(GLenum target,GLintptr offset,GLsizeiptr size,void *data))
#define glGetClipPlane             CGLFL_CALL(146,glGetClipPlane,void,2,(plane,equation),(GLenum plane,GLdouble *equation))
#define glGetCompressedTexImage    CGLFL_CALL(147,glGetCompressedTexImage,void,3,(target,level,img),(GLenum target,GLint level,void *img))
#define glGetDoublev               CGLFL_CALL(148,glGetDoublev,void,2,(pname,data),(GLenum pname,GLdouble *data))
#define glGetError                 CGLFL_CALL(149,glGetError,GLenum,0,(),())
#define glGetFloatv                CGLFL_CALL(150,glGetFloatv,void,2,(pname,data),(GLenum pname,GLfloat *data))
#define glGetIntegerv              CGLFL_CALL(151,glGetIntegerv,void,2,(pname,data),(GLenum pname,GLint *data))
#define glGetLightfv               CGLFL_CALL(152,glGetLightfv,void,3,(light,pname,params),(GLenum light,GLenum pname,GLfloat *params))
#define glGetLightiv               CGLFL_CALL(153,glGetLightiv,void,3,(light,pname,params),(GLenum light,GLenum pname,GLint *params))
#define glGetMapdv                 CGLFL_CALL(154,glGetMapdv,void,3,(target,query,v),(GLenum target,GLenum query,GLdouble *v))
#define glGetMapfv                 CGLFL_CALL(155,glGetMapfv,void,3,(target,query,v),(GLenum target,GLenum query,GLfloat *v))
#define glGetMapiv                 CGLFL_CALL(156,glGetMapiv,void,3,(target,query,v),(GLenum target,GLenum query,GLint *v))
#define glGetMaterialfv            CGLFL_CALL(157,glGetMaterialfv,void,3,(face,pname,params),(GLenum face,GLenum pname,GLfloat *params))
#define glGetMaterialiv            CGLFL_CALL(158,glGetMaterialiv,void,3,(face,pname,params),(GLenum face,GLenum pname,GLint *params))
#define glGetPixelMapfv            CGLFL_CALL(159,glGetPixelMapfv,void,2,(map,values),(GLenum map,GLfloat *values))
#define glGetPixelMapuiv           CGLFL_CALL(160,glGetPixelMapuiv,void,2,(map,values),(GLenum map,GLuint *values))
#define glGetPixelMapusv           CGLFL_CALL(161,glGetPixelMapusv,void,2,(map,values),(GLenum map,GLushort *values))
#define glGetPointerv              CGLFL_CALL(162,glGetPointerv,void,2,(pname,params),(GLenum pname,void **params))
#define glGetPolygonStipple        CGLFL_CALL(163,glGetPolygonStipple,void,1,(mask),(GLubyte *mask))
#define glGetProgramInfoLog        CGLFL_CALL(164,glGetProgramInfoLog,void,4,(program,bufSize,length,infoLog),(GLuint program,GLsizei bufSize,GLsizei *length,GLchar *infoLog))
#define glGetProgramiv             CGLFL_CALL(165,glGetProgramiv,void,3,(program,pname,params),(GLuint program,GLenum pname,GLint *params))
#define glGetQueryObjectiv         CGLFL_CALL(166,glGetQueryObjectiv,void,3,(id,pname,params),(GLuint id,GLenum pname,GLint *params))
#define glGetQueryObjectuiv        CGLFL_CALL(167,glGetQueryObjectuiv,void,3,(id,pname,params),(GLuint id,GLenum pname,GLuint *params))
#define glGetQueryiv               CGLFL_CALL(168,glGetQueryiv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetShaderInfoLog         CGLFL_CALL(169,glGetShaderInfoLog,void,4,(shader,bufSize,length,infoLog),(GLuint shader,GLsizei bufSize,GLsizei *length,GLchar *infoLog))
#define glGetShaderSource          CGLFL_CALL(170,glGetShaderSource,void,4,(shader,bufSize,length,source),(GLuint shader,GLsizei bufSize,GLsizei *length,GLchar *source))
#define glGetShaderiv              CGLFL_CALL(171,glGetShaderiv,void,3,(shader,pname,params),(GLuint shader,GLenum pname,GLint *params))
#define glGetString                CGLFL_CALL(172,glGetString,const GLubyte *,1,(name),(GLenum name))
#define glGetTexEnvfv              CGLFL_CALL(173,glGetTexEnvfv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLfloat *params))
#define glGetTexEnviv              CGLFL_CALL(174,glGetTexEnviv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetTexGendv              CGLFL_CALL(175,glGetTexGendv,void,3,(coord,pname,params),(GLenum coord,GLenum pname,GLdouble *params))
#define glGetTexGenfv              CGLFL_CALL(176,glGetTexGenfv,void,3,(coord,pname,params),(GLenum coord,GLenum pname,GLfloat *params))
#define glGetTexGeniv              CGLFL_CALL(177,glGetTexGeniv,void,3,(coord,pname,params),(GLenum coord,GLenum pname,GLint *params))
#define glGetTexImage              CGLFL_CALL(178,glGetTexImage,void,5,(target,level,format,type,pixels),(GLenum target,GLint level,GLenum format,GLenum type,void *pixels))
#define glGetTexLevelParameterfv   CGLFL_CALL(179,glGetTexLevelParameterfv,void,4,(target,level,pname,params),(GLenum target,GLint level,GLenum pname,GLfloat *params))
#define glGetTexLevelParameteriv   CGLFL_CALL(180,glGetTexLevelParameteriv,void,4,(target,level,pname,params),(GLenum target,GLint level,GLenum pname,GLint *params))
#define glGetTexParameterfv        CGLFL_CALL(181,glGetTexParameterfv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLfloat *params))
#define glGetTexParameteriv        CGLFL_CALL(182,glGetTexParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,GLint *params))
#define glGetUniformLocation       CGLFL_CALL(183,glGetUniformLocation,GLint,2,(program,name),(GLuint program,const GLchar *name))
#define glGetUniformfv             CGLFL_CALL(184,glGetUniformfv,void,3,(program,location,params),(GLuint program,GLint location,GLfloat *params))
#define glGetUniformiv             CGLFL_CALL(185,glGetUniformiv,void,3,(program,location,params),(GLuint program,GLint location,GLint *params))
#define glGetVertexAttribPointerv  CGLFL_CALL(186,glGetVertexAttribPointerv,void,3,(index,pname,pointer),(GLuint index,GLenum pname,void **pointer))
#define glGetVertexAttribdv        CGLFL_CALL(187,glGetVertexAttribdv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLdouble *params))
#define glGetVertexAttribfv        CGLFL_CALL(188,glGetVertexAttribfv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLfloat *params))
#define glGetVertexAttribiv        CGLFL_CALL(189,glGetVertexAttribiv,void,3,(index,pname,params),(GLuint index,GLenum pname,GLint *params))
#define glHint                     CGLFL_CALL(190,glHint,void,2,(target,mode),(GLenum target,GLenum mode))
#define glIndexMask                CGLFL_CALL(191,glIndexMask,void,1,(mask),(GLuint mask))
#define glIndexPointer             CGLFL_CALL(192,glIndexPointer,void,3,(type,stride,pointer),(GLenum type,GLsizei stride,const void *pointer))
#define glIndexd                   CGLFL_CALL(193,glIndexd,void,1,(c),(GLdouble c))
#define glIndexdv                  CGLFL_CALL(194,glIndexdv,void,1,(c),(const GLdouble *c))
#define glIndexf                   CGLFL_CALL(195,glIndexf,void,1,(c),(GLfloat c))
#define glIndexfv                  CGLFL_CALL(196,glIndexfv,void,1,(c),(const GLfloat *c))
#define glIndexi                   CGLFL_CALL(197,glIndexi,void,1,(c),(GLint c))
#define glIndexiv                  CGLFL_CALL(198,glIndexiv,void,1,(c),(const GLint *c))
#define glIndexs                   CGLFL_CALL(199,glIndexs,void,1,(c),(GLshort c))
#define glIndexsv                  CGLFL_CALL(200,glIndexsv,void,1,(c),(const GLshort *c))
#define glIndexub                  CGLFL_CALL(201,glIndexub,void,1,(c),(GLubyte c))
#define glIndexubv                 CGLFL_CALL(202,glIndexubv,void,1,(c),(const GLubyte *c))
#define glInitNames                CGLFL_CALL(203,glInitNames,void,0,(),())
#define glInterleavedArrays        CGLFL_CALL(204,glInterleavedArrays,void,3,(format,stride,pointer),(GLenum format,GLsizei stride,const void *pointer))
#define glIsBuffer                 CGLFL_CALL(205,glIsBuffer,GLboolean,1,(buffer),(GLuint buffer))
#define glIsEnabled                CGLFL_CALL(206,glIsEnabled,GLboolean,1,(cap),(GLenum cap))
#define glIsList                   CGLFL_CALL(207,glIsList,GLboolean,1,(list),(GLuint list))
#define glIsProgram                CGLFL_CALL(208,glIsProgram,GLboolean,1,(program),(GLuint program))
#define glIsQuery                  CGLFL_CALL(209,glIsQuery,GLboolean,1,(id),(GLuint id))
#define glIsShader                 CGLFL_CALL(210,glIsShader,GLboolean,1,(shader),(GLuint shader))
#define glIsTexture                CGLFL_CALL(211,glIsTexture,GLboolean,1,(texture),(GLuint texture))
#define glLightModelf              CGLFL_CALL(212,glLightModelf,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glLightModelfv             CGLFL_CALL(213,glLightModelfv,void,2,(pname,params),(GLenum pname,const GLfloat *params))
#define glLightModeli              CGLFL_CALL(214,glLightModeli,void,2,(pname,param),(GLenum pname,GLint param))
#define glLightModeliv             CGLFL_CALL(215,glLightModeliv,void,2,(pname,params),(GLenum pname,const GLint *params))
#define glLightf                   CGLFL_CALL(216,glLightf,void,3,(light,pname,param),(GLenum light,GLenum pname,GLfloat param))
#define glLightfv                  CGLFL_CALL(217,glLightfv,void,3,(light,pname,params),(GLenum light,GLenum pname,const GLfloat *params))
#define glLighti                   CGLFL_CALL(218,glLighti,void,3,(light,pname,param),(GLenum light,GLenum pname,GLint param))
#define glLightiv                  CGLFL_CALL(219,glLightiv,void,3,(light,pname,params),(GLenum light,GLenum pname,const GLint *params))
#define glLineStipple              CGLFL_CALL(220,glLineStipple,void,2,(factor,pattern),(GLint factor,GLushort pattern))
#define glLineWidth                CGLFL_CALL(221,glLineWidth,void,1,(width),(GLfloat width))
#define glLinkProgram              CGLFL_CALL(222,glLinkProgram,void,1,(program),(GLuint program))
#define glListBase                 CGLFL_CALL(223,glListBase,void,1,(base),(GLuint base))
#define glLoadIdentity             CGLFL_CALL(224,glLoadIdentity,void,0,(),())
#define glLoadMatrixd              CGLFL_CALL(225,glLoadMatrixd,void,1,(m),(const GLdouble *m))
#define glLoadMatrixf              CGLFL_CALL(226,glLoadMatrixf,void,1,(m),(const GLfloat *m))
#define glLoadName                 CGLFL_CALL(227,glLoadName,void,1,(name),(GLuint name))
#define glLoadTransposeMatrixd     CGLFL_CALL(228,glLoadTransposeMatrixd,void,1,(m),(const GLdouble *m))
#define glLoadTransposeMatrixf     CGLFL_CALL(229,glLoadTransposeMatrixf,void,1,(m),(const GLfloat *m))
#define glLogicOp                  CGLFL_CALL(230,glLogicOp,void,1,(opcode),(GLenum opcode))
#define glMap1d                    CGLFL_CALL(231,glMap1d,void,6,(target,u1,u2,stride,order,points),(GLenum target,GLdouble u1,GLdouble u2,GLint stride,GLint order,const GLdouble *points))
#define glMap1f                    CGLFL_CALL(232,glMap1f,void,6,(target,u1,u2,stride,order,points),(GLenum target,GLfloat u1,GLfloat u2,GLint stride,GLint order,const GLfloat *points))
#define glMap2d                    CGLFL_CALL(233,glMap2d,void,10,(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points),(GLenum target,GLdouble u1,GLdouble u2,GLint ustride,GLint uorder,GLdouble v1,GLdouble v2,GLint vstride,GLint vorder,const GLdouble *points))
#define glMap2f                    CGLFL_CALL(234,glMap2f,void,10,(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points),(GLenum target,GLfloat u1,GLfloat u2,GLint ustride,GLint uorder,GLfloat v1,GLfloat v2,GLint vstride,GLint vorder,const GLfloat *points))
#define glMapBuffer                CGLFL_CALL(235,glMapBuffer,void *,2,(target,access),(GLenum target,GLenum access))
#define glMapGrid1d                CGLFL_CALL(236,glMapGrid1d,void,3,(un,u1,u2),(GLint un,GLdouble u1,GLdouble u2))
#define glMapGrid1f                CGLFL_CALL(237,glMapGrid1f,void,3,(un,u1,u2),(GLint un,GLfloat u1,GLfloat u2))
#define glMapGrid2d                CGLFL_CALL(238,glMapGrid2d,void,6,(un,u1,u2,vn,v1,v2),(GLint un,GLdouble u1,GLdouble u2,GLint vn,GLdouble v1,GLdouble v2))
#define glMapGrid2f                CGLFL_CALL(239,glMapGrid2f,void,6,(un,u1,u2,vn,v1,v2),(GLint un,GLfloat u1,GLfloat u2,GLint vn,GLfloat v1,GLfloat v2))
#define glMaterialf                CGLFL_CALL(240,glMaterialf,void,3,(face,pname,param),(GLenum face,GLenum pname,GLfloat param))
#define glMaterialfv               CGLFL_CALL(241,glMaterialfv,void,3,(face,pname,params),(GLenum face,GLenum pname,const GLfloat *params))
#define glMateriali                CGLFL_CALL(242,glMateriali,void,3,(face,pname,param),(GLenum face,GLenum pname,GLint param))
#define glMaterialiv               CGLFL_CALL(243,glMaterialiv,void,3,(face,pname,params),(GLenum face,GLenum pname,const GLint *params))
#define glMatrixMode               CGLFL_CALL(244,glMatrixMode,void,1,(mode),(GLenum mode))
#define glMultMatrixd              CGLFL_CALL(245,glMultMatrixd,void,1,(m),(const GLdouble *m))
#define glMultMatrixf              CGLFL_CALL(246,glMultMatrixf,void,1,(m),(const GLfloat *m))
#define glMultTransposeMatrixd     CGLFL_CALL(247,glMultTransposeMatrixd,void,1,(m),(const GLdouble *m))
#define glMultTransposeMatrixf     CGLFL_CALL(248,glMultTransposeMatrixf,void,1,(m),(const GLfloat *m))
#define glMultiDrawArrays          CGLFL_CALL(249,glMultiDrawArrays,void,4,(mode,first,count,drawcount),(GLenum mode,const GLint *first,const GLsizei *count,GLsizei drawcount))
#define glMultiDrawElements        CGLFL_CALL(250,glMultiDrawElements,void,5,(mode,count,type,indices,drawcount),(GLenum mode,const GLsizei *count,GLenum type,const void *const*indices,GLsizei drawcount))
#define glMultiTexCoord1d          CGLFL_CALL(251,glMultiTexCoord1d,void,2,(target,s),(GLenum target,GLdouble s))
#define glMultiTexCoord1dv         CGLFL_CALL(252,glMultiTexCoord1dv,void,2,(target,v),(GLenum target,const GLdouble *v))
#define glMultiTexCoord1f          CGLFL_CALL(253,glMultiTexCoord1f,void,2,(target,s),(GLenum target,GLfloat s))
#define glMultiTexCoord1fv         CGLFL_CALL(254,glMultiTexCoord1fv,void,2,(target,v),(GLenum target,const GLfloat *v))
#define glMultiTexCoord1i          CGLFL_CALL(255,glMultiTexCoord1i,void,2,(target,s),(GLenum target,GLint s))
#define glMultiTexCoord1iv         CGLFL_CALL(256,glMultiTexCoord1iv,void,2,(target,v),(GLenum target,const GLint *v))
#define glMultiTexCoord1s          CGLFL_CALL(257,glMultiTexCoord1s,void,2,(target,s),(GLenum target,GLshort s))
#define glMultiTexCoord1sv         CGLFL_CALL(258,glMultiTexCoord1sv,void,2,(target,v),(GLenum target,const GLshort *v))
#define glMultiTexCoord2d          CGLFL_CALL(259,glMultiTexCoord2d,void,3,(target,s,t),(GLenum target,GLdouble s,GLdouble t))
#define glMultiTexCoord2dv         CGLFL_CALL(260,glMultiTexCoord2dv,void,2,(target,v),(GLenum target,const GLdouble *v))
#define glMultiTexCoord2f          CGLFL_CALL(261,glMultiTexCoord2f,void,3,(target,s,t),(GLenum target,GLfloat s,GLfloat t))
#define glMultiTexCoord2fv         CGLFL_CALL(262,glMultiTexCoord2fv,void,2,(target,v),(GLenum target,const GLfloat *v))
#define glMultiTexCoord2i          CGLFL_CALL(263,glMultiTexCoord2i,void,3,(target,s,t),(GLenum target,GLint s,GLint t))
#define glMultiTexCoord2iv         CGLFL_CALL(264,glMultiTexCoord2iv,void,2,(target,v),(GLenum target,const GLint *v))
#define glMultiTexCoord2s          CGLFL_CALL(265,glMultiTexCoord2s,void,3,(target,s,t),(GLenum target,GLshort s,GLshort t))
#define glMultiTexCoord2sv         CGLFL_CALL(266,glMultiTexCoord2sv,void,2,(target,v),(GLenum target,const GLshort *v))
#define glMultiTexCoord3d          CGLFL_CALL(267,glMultiTexCoord3d,void,4,(target,s,t,r),(GLenum target,GLdouble s,GLdouble t,GLdouble r))
#define glMultiTexCoord3dv         CGLFL_CALL(268,glMultiTexCoord3dv,void,2,(target,v),(GLenum target,const GLdouble *v))
#define glMultiTexCoord3f          CGLFL_CALL(269,glMultiTexCoord3f,void,4,(target,s,t,r),(GLenum target,GLfloat s,GLfloat t,GLfloat r))
#define glMultiTexCoord3fv         CGLFL_CALL(270,glMultiTexCoord3fv,void,2,(target,v),(GLenum target,const GLfloat *v))
#define glMultiTexCoord3i          CGLFL_CALL(271,glMultiTexCoord3i,void,4,(target,s,t,r),(GLenum target,GLint s,GLint t,GLint r))
#define glMultiTexCoord3iv         CGLFL_CALL(272,glMultiTexCoord3iv,void,2,(target,v),(GLenum target,const GLint *v))
#define glMultiTexCoord3s          CGLFL_CALL(273,glMultiTexCoord3s,void,4,(target,s,t,r),(GLenum target,GLshort s,GLshort t,GLshort r))
#define glMultiTexCoord3sv         CGLFL_CALL(274,glMultiTexCoord3sv,void,2,(target,v),(GLenum target,const GLshort *v))
#define glMultiTexCoord4d          CGLFL_CALL(275,glMultiTexCoord4d,void,5,(target,s,t,r,q),(GLenum target,GLdouble s,GLdouble t,GLdouble r,GLdouble q))
#define glMultiTexCoord4dv         CGLFL_CALL(276,glMultiTexCoord4dv,void,2,(target,v),(GLenum target,const GLdouble *v))
#define glMultiTexCoord4f          CGLFL_CALL(277,glMultiTexCoord4f,void,5,(target,s,t,r,q),(GLenum target,GLfloat s,GLfloat t,GLfloat r,GLfloat q))
#define glMultiTexCoord4fv         CGLFL_CALL(278,glMultiTexCoord4fv,void,2,(target,v),(GLenum target,const GLfloat *v))
#define glMultiTexCoord4i          CGLFL_CALL(279,glMultiTexCoord4i,void,5,(target,s,t,r,q),(GLenum target,GLint s,GLint t,GLint r,GLint q))
#define glMultiTexCoord4iv         CGLFL_CALL(280,glMultiTexCoord4iv,void,2,(target,v),(GLenum target,const GLint *v))
#define glMultiTexCoord4s          CGLFL_CALL(281,glMultiTexCoord4s,void,5,(target,s,t,r,q),(GLenum target,GLshort s,GLshort t,GLshort r,GLshort q))
#define glMultiTexCoord4sv         CGLFL_CALL(282,glMultiTexCoord4sv,void,2,(target,v),(GLenum target,const GLshort *v))
#define glNewList                  CGLFL_CALL(283,glNewList,void,2,(list,mode),(GLuint list,GLenum mode))
#define glNormal3b                 CGLFL_CALL(284,glNormal3b,void,3,(nx,ny,nz),(GLbyte nx,GLbyte ny,GLbyte nz))
#define glNormal3bv                CGLFL_CALL(285,glNormal3bv,void,1,(v),(const GLbyte *v))
#define glNormal3d                 CGLFL_CALL(286,glNormal3d,void,3,(nx,ny,nz),(GLdouble nx,GLdouble ny,GLdouble nz))
#define glNormal3dv                CGLFL_CALL(287,glNormal3dv,void,1,(v),(const GLdouble *v))
#define glNormal3f                 CGLFL_CALL(288,glNormal3f,void,3,(nx,ny,nz),(GLfloat nx,GLfloat ny,GLfloat nz))
#define glNormal3fv                CGLFL_CALL(289,glNormal3fv,void,1,(v),(const GLfloat *v))
#define glNormal3i                 CGLFL_CALL(290,glNormal3i,void,3,(nx,ny,nz),(GLint nx,GLint ny,GLint nz))
#define glNormal3iv                CGLFL_CALL(291,glNormal3iv,void,1,(v),(const GLint *v))
#define glNormal3s                 CGLFL_CALL(292,glNormal3s,void,3,(nx,ny,nz),(GLshort nx,GLshort ny,GLshort nz))
#define glNormal3sv                CGLFL_CALL(293,glNormal3sv,void,1,(v),(const GLshort *v))
#define glNormalPointer            CGLFL_CALL(294,glNormalPointer,void,3,(type,stride,pointer),(GLenum type,GLsizei stride,const void *pointer))
#define glOrtho                    CGLFL_CALL(295,glOrtho,void,6,(left,right,bottom,top,zNear,zFar),(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble zNear,GLdouble zFar))
#define glPassThrough              CGLFL_CALL(296,glPassThrough,void,1,(token),(GLfloat token))
#define glPixelMapfv               CGLFL_CALL(297,glPixelMapfv,void,3,(map,mapsize,values),(GLenum map,GLsizei mapsize,const GLfloat *values))
#define glPixelMapuiv              CGLFL_CALL(298,glPixelMapuiv,void,3,(map,mapsize,values),(GLenum map,GLsizei mapsize,const GLuint *values))
#define glPixelMapusv              CGLFL_CALL(299,glPixelMapusv,void,3,(map,mapsize,values),(GLenum map,GLsizei mapsize,const GLushort *values))
#define glPixelStoref              CGLFL_CALL(300,glPixelStoref,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glPixelStorei              CGLFL_CALL(301,glPixelStorei,void,2,(pname,param),(GLenum pname,GLint param))
#define glPixelTransferf           CGLFL_CALL(302,glPixelTransferf,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glPixelTransferi           CGLFL_CALL(303,glPixelTransferi,void,2,(pname,param),(GLenum pname,GLint param))
#define glPixelZoom                CGLFL_CALL(304,glPixelZoom,void,2,(xfactor,yfactor),(GLfloat xfactor,GLfloat yfactor))
#define glPointParameterf          CGLFL_CALL(305,glPointParameterf,void,2,(pname,param),(GLenum pname,GLfloat param))
#define glPointParameterfv         CGLFL_CALL(306,glPointParameterfv,void,2,(pname,params),(GLenum pname,const GLfloat *params))
#define glPointParameteri          CGLFL_CALL(307,glPointParameteri,void,2,(pname,param),(GLenum pname,GLint param))
#define glPointParameteriv         CGLFL_CALL(308,glPointParameteriv,void,2,(pname,params),(GLenum pname,const GLint *params))
#define glPointSize                CGLFL_CALL(309,glPointSize,void,1,(size),(GLfloat size))
#define glPolygonMode              CGLFL_CALL(310,glPolygonMode,void,2,(face,mode),(GLenum face,GLenum mode))
#define glPolygonOffset            CGLFL_CALL(311,glPolygonOffset,void,2,(factor,units),(GLfloat factor,GLfloat units))
#define glPolygonStipple           CGLFL_CALL(312,glPolygonStipple,void,1,(mask),(const GLubyte *mask))
#define glPopAttrib                CGLFL_CALL(313,glPopAttrib,void,0,(),())
#define glPopClientAttrib          CGLFL_CALL(314,glPopClientAttrib,void,0,(),())
#define glPopMatrix                CGLFL_CALL(315,glPopMatrix,void,0,(),())
#define glPopName                  CGLFL_CALL(316,glPopName,void,0,(),())
#define glPrioritizeTextures       CGLFL_CALL(317,glPrioritizeTextures,void,3,(n,textures,priorities),(GLsizei n,const GLuint *textures,const GLfloat *priorities))
#define glPushAttrib               CGLFL_CALL(318,glPushAttrib,void,1,(mask),(GLbitfield mask))
#define glPushClientAttrib         CGLFL_CALL(319,glPushClientAttrib,void,1,(mask),(GLbitfield mask))
#define glPushMatrix               CGLFL_CALL(320,glPushMatrix,void,0,(),())
#define glPushName                 CGLFL_CALL(321,glPushName,void,1,(name),(GLuint name))
#define glRasterPos2d              CGLFL_CALL(322,glRasterPos2d,void,2,(x,y),(GLdouble x,GLdouble y))
#define glRasterPos2dv             CGLFL_CALL(323,glRasterPos2dv,void,1,(v),(const GLdouble *v))
#define glRasterPos2f              CGLFL_CALL(324,glRasterPos2f,void,2,(x,y),(GLfloat x,GLfloat y))
#define glRasterPos2fv             CGLFL_CALL(325,glRasterPos2fv,void,1,(v),(const GLfloat *v))
#define glRasterPos2i              CGLFL_CALL(326,glRasterPos2i,void,2,(x,y),(GLint x,GLint y))
#define glRasterPos2iv             CGLFL_CALL(327,glRasterPos2iv,void,1,(v),(const GLint *v))
#define glRasterPos2s              CGLFL_CALL(328,glRasterPos2s,void,2,(x,y),(GLshort x,GLshort y))
#define glRasterPos2sv             CGLFL_CALL(329,glRasterPos2sv,void,1,(v),(const GLshort *v))
#define glRasterPos3d              CGLFL_CALL(330,glRasterPos3d,void,3,(x,y,z),(GLdouble x,GLdouble y,GLdouble z))
#define glRasterPos3dv             CGLFL_CALL(331,glRasterPos3dv,void,1,(v),(const GLdouble *v))
#define glRasterPos3f              CGLFL_CALL(332,glRasterPos3f,void,3,(x,y,z),(GLfloat x,GLfloat y,GLfloat z))
#define glRasterPos3fv             CGLFL_CALL(333,glRasterPos3fv,void,1,(v),(const GLfloat *v))
#define glRasterPos3i              CGLFL_CALL(334,glRasterPos3i,void,3,(x,y,z),(GLint x,GLint y,GLint z))
#define glRasterPos3iv             CGLFL_CALL(335,glRasterPos3iv,void,1,(v),(const GLint *v))
#define glRasterPos3s              CGLFL_CALL(336,glRasterPos3s,void,3,(x,y,z),(GLshort x,GLshort y,GLshort z))
#define glRasterPos3sv             CGLFL_CALL(337,glRasterPos3sv,void,1,(v),(const GLshort *v))
#define glRasterPos4d              CGLFL_CALL(338,glRasterPos4d,void,4,(x,y,z,w),(GLdouble x,GLdouble y,GLdouble z,GLdouble w))
#define glRasterPos4dv             CGLFL_CALL(339,glRasterPos4dv,void,1,(v),(const GLdouble *v))
#define glRasterPos4f              CGLFL_CALL(340,glRasterPos4f,void,4,(x,y,z,w),(GLfloat x,GLfloat y,GLfloat z,GLfloat w))
#define glRasterPos4fv             CGLFL_CALL(341,glRasterPos4fv,void,1,(v),(const GLfloat *v))
#define glRasterPos4i              CGLFL_CALL(342,glRasterPos4i,void,4,(x,y,z,w),(GLint x,GLint y,GLint z,GLint w))
#define glRasterPos4iv             CGLFL_CALL(343,glRasterPos4iv,void,1,(v),(const GLint *v))
#define glRasterPos4s              CGLFL_CALL(344,glRasterPos4s,void,4,(x,y,z,w),(GLshort x,GLshort y,GLshort z,GLshort w))
#define glRasterPos4sv             CGLFL_CALL(345,glRasterPos4sv,void,1,(v),(const GLshort *v))
#define glReadBuffer               CGLFL_CALL(346,glReadBuffer,void,1,(src),(GLenum src))
#define glReadPixels               CGLFL_CALL(347,glReadPixels,void,7,(x,y,width,height,format,type,pixels),(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void *pixels))
#define glRectd                    CGLFL_CALL(348,glRectd,void,4,(x1,y1,x2,y2),(GLdouble x1,GLdouble y1,GLdouble x2,GLdouble y2))
#define glRectdv                   CGLFL_CALL(349,glRectdv,void,2,(v1,v2),(const GLdouble *v1,const GLdouble *v2))
#define glRectf                    CGLFL_CALL(350,glRectf,void,4,(x1,y1,x2,y2),(GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2))
#define glRectfv                   CGLFL_CALL(351,glRectfv,void,2,(v1,v2),(const GLfloat *v1,const GLfloat *v2))
#define glRecti                    CGLFL_CALL(352,glRecti,void,4,(x1,y1,x2,y2),(GLint x1,GLint y1,GLint x2,GLint y2))
#define glRectiv                   CGLFL_CALL(353,glRectiv,void,2,(v1,v2),(const GLint *v1,const GLint *v2))
#define glRects                    CGLFL_CALL(354,glRects,void,4,(x1,y1,x2,y2),(GLshort x1,GLshort y1,GLshort x2,GLshort y2))
#define glRectsv                   CGLFL_CALL(355,glRectsv,void,2,(v1,v2),(const GLshort *v1,const GLshort *v2))
#define glRenderMode               CGLFL_CALL(356,glRenderMode,GLint,1,(mode),(GLenum mode))
#define glRotated                  CGLFL_CALL(357,glRotated,void,4,(angle,x,y,z),(GLdouble angle,GLdouble x,GLdouble y,GLdouble z))
#define glRotatef                  CGLFL_CALL(358,glRotatef,void,4,(angle,x,y,z),(GLfloat angle,GLfloat x,GLfloat y,GLfloat z))
#define glSampleCoverage           CGLFL_CALL(359,glSampleCoverage,void,2,(value,invert),(GLfloat value,GLboolean invert))
#define glScaled                   CGLFL_CALL(360,glScaled,void,3,(x,y,z),(GLdouble x,GLdouble y,GLdouble z))
#define glScalef                   CGLFL_CALL(361,glScalef,void,3,(x,y,z),(GLfloat x,GLfloat y,GLfloat z))
#define glScissor                  CGLFL_CALL(362,glScissor,void,4,(x,y,width,height),(GLint x,GLint y,GLsizei width,GLsizei height))
#define glSecondaryColor3b         CGLFL_CALL(363,glSecondaryColor3b,void,3,(red,green,blue),(GLbyte red,GLbyte green,GLbyte blue))
#define glSecondaryColor3bv        CGLFL_CALL(364,glSecondaryColor3bv,void,1,(v),(const GLbyte *v))
#define glSecondaryColor3d         CGLFL_CALL(365,glSecondaryColor3d,void,3,(red,green,blue),(GLdouble red,GLdouble green,GLdouble blue))
#define glSecondaryColor3dv        CGLFL_CALL(366,glSecondaryColor3dv,void,1,(v),(const GLdouble *v))
#define glSecondaryColor3f         CGLFL_CALL(367,glSecondaryColor3f,void,3,(red,green,blue),(GLfloat red,GLfloat green,GLfloat blue))
#define glSecondaryColor3fv        CGLFL_CALL(368,glSecondaryColor3fv,void,1,(v),(const GLfloat *v))
#define glSecondaryColor3i         CGLFL_CALL(369,glSecondaryColor3i,void,3,(red,green,blue),(GLint red,GLint green,GLint blue))
#define glSecondaryColor3iv        CGLFL_CALL(370,glSecondaryColor3iv,void,1,(v),(const GLint *v))
#define glSecondaryColor3s         CGLFL_CALL(371,glSecondaryColor3s,void,3,(red,green,blue),(GLshort red,GLshort green,GLshort blue))
#define glSecondaryColor3sv        CGLFL_CALL(372,glSecondaryColor3sv,void,1,(v),(const GLshort *v))
#define glSecondaryColor3ub        CGLFL_CALL(373,glSecondaryColor3ub,void,3,(red,green,blue),(GLubyte red,GLubyte green,GLubyte blue))
#define glSecondaryColor3ubv       CGLFL_CALL(374,glSecondaryColor3ubv,void,1,(v),(const GLubyte *v))
#define glSecondaryColor3ui        CGLFL_CALL(375,glSecondaryColor3ui,void,3,(red,green,blue),(GLuint red,GLuint green,GLuint blue))
#define glSecondaryColor3uiv       CGLFL_CALL(376,glSecondaryColor3uiv,void,1,(v),(const GLuint *v))
#define glSecondaryColor3us        CGLFL_CALL(377,glSecondaryColor3us,void,3,(red,green,blue),(GLushort red,GLushort green,GLushort blue))
#define glSecondaryColor3usv       CGLFL_CALL(378,glSecondaryColor3usv,void,1,(v),(const GLushort *v))
#define glSecondaryColorPointer    CGLFL_CALL(379,glSecondaryColorPointer,void,4,(size,type,stride,pointer),(GLint size,GLenum type,GLsizei stride,const void *pointer))
#define glSelectBuffer             CGLFL_CALL(380,glSelectBuffer,void,2,(size,buffer),(GLsizei size,GLuint *buffer))
#define glShadeModel               CGLFL_CALL(381,glShadeModel,void,1,(mode),(GLenum mode))
#define glShaderSource             CGLFL_CALL(382,glShaderSource,void,4,(shader,count,string,length),(GLuint shader,GLsizei count,const GLchar *const*string,const GLint *length))
#define glStencilFunc              CGLFL_CALL(383,glStencilFunc,void,3,(func,ref,mask),(GLenum func,GLint ref,GLuint mask))
#define glStencilFuncSeparate      CGLFL_CALL(384,glStencilFuncSeparate,void,4,(face,func,ref,mask),(GLenum face,GLenum func,GLint ref,GLuint mask))
#define glStencilMask              CGLFL_CALL(385,glStencilMask,void,1,(mask),(GLuint mask))
#define glStencilMaskSeparate      CGLFL_CALL(386,glStencilMaskSeparate,void,2,(face,mask),(GLenum face,GLuint mask))
#define glStencilOp                CGLFL_CALL(387,glStencilOp,void,3,(fail,zfail,zpass),(GLenum fail,GLenum zfail,GLenum zpass))
#define glStencilOpSeparate        CGLFL_CALL(388,glStencilOpSeparate,void,4,(face,sfail,dpfail,dppass),(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass))
#define glTexCoord1d               CGLFL_CALL(389,glTexCoord1d,void,1,(s),(GLdouble s))
#define glTexCoord1dv              CGLFL_CALL(390,glTexCoord1dv,void,1,(v),(const GLdouble *v))
#define glTexCoord1f               CGLFL_CALL(391,glTexCoord1f,void,1,(s),(GLfloat s))
#define glTexCoord1fv              CGLFL_CALL(392,glTexCoord1fv,void,1,(v),(const GLfloat *v))
#define glTexCoord1i               CGLFL_CALL(393,glTexCoord1i,void,1,(s),(GLint s))
#define glTexCoord1iv              CGLFL_CALL(394,glTexCoord1iv,void,1,(v),(const GLint *v))
#define glTexCoord1s               CGLFL_CALL(395,glTexCoord1s,void,1,(s),(GLshort s))
#define glTexCoord1sv              CGLFL_CALL(396,glTexCoord1sv,void,1,(v),(const GLshort *v))
#define glTexCoord2d               CGLFL_CALL(397,glTexCoord2d,void,2,(s,t),(GLdouble s,GLdouble t))
#define glTexCoord2dv              CGLFL_CALL(398,glTexCoord2dv,void,1,(v),(const GLdouble *v))
#define glTexCoord2f               CGLFL_CALL(399,glTexCoord2f,void,2,(s,t),(GLfloat s,GLfloat t))
#define glTexCoord2fv              CGLFL_CALL(400,glTexCoord2fv,void,1,(v),(const GLfloat *v))
#define glTexCoord2i               CGLFL_CALL(401,glTexCoord2i,void,2,(s,t),(GLint s,GLint t))
#define glTexCoord2iv              CGLFL_CALL(402,glTexCoord2iv,void,1,(v),(const GLint *v))
#define glTexCoord2s               CGLFL_CALL(403,glTexCoord2s,void,2,(s,t),(GLshort s,GLshort t))
#define glTexCoord2sv              CGLFL_CALL(404,glTexCoord2sv,void,1,(v),(const GLshort *v))
#define glTexCoord3d               CGLFL_CALL(405,glTexCoord3d,void,3,(s,t,r),(GLdouble s,GLdouble t,GLdouble r))
#define glTexCoord3dv              CGLFL_CALL(406,glTexCoord3dv,void,1,(v),(const GLdouble *v))
#define glTexCoord3f               CGLFL_CALL(407,glTexCoord3f,void,3,(s,t,r),(GLfloat s,GLfloat t,GLfloat r))
#define glTexCoord3fv              CGLFL_CALL(408,glTexCoord3fv,void,1,(v),(const GLfloat *v))
#define glTexCoord3i               CGLFL_CALL(409,glTexCoord3i,void,3,(s,t,r),(GLint s,GLint t,GLint r))
#define glTexCoord3iv              CGLFL_CALL(410,glTexCoord3iv,void,1,(v),(const GLint *v))
#define glTexCoord3s               CGLFL_CALL(411,glTexCoord3s,void,3,(s,t,r),(GLshort s,GLshort t,GLshort r))
#define glTexCoord3sv              CGLFL_CALL(412,glTexCoord3sv,void,1,(v),(const GLshort *v))
#define glTexCoord4d               CGLFL_CALL(413,glTexCoord4d,void,4,(s,t,r,q),(GLdouble s,GLdouble t,GLdouble r,GLdouble q))
#define glTexCoord4dv              CGLFL_CALL(414,glTexCoord4dv,void,1,(v),(const GLdouble *v))
#define glTexCoord4f               CGLFL_CALL(415,glTexCoord4f,void,4,(s,t,r,q),(GLfloat s,GLfloat t,GLfloat r,GLfloat q))
#define glTexCoord4fv              CGLFL_CALL(416,glTexCoord4fv,void,1,(v),(const GLfloat *v))
#define glTexCoord4i               CGLFL_CALL(417,glTexCoord4i,void,4,(s,t,r,q),(GLint s,GLint t,GLint r,GLint q))
#define glTexCoord4iv              CGLFL_CALL(418,glTexCoord4iv,void,1,(v),(const GLint *v))
#define glTexCoord4s               CGLFL_CALL(419,glTexCoord4s,void,4,(s,t,r,q),(GLshort s,GLshort t,GLshort r,GLshort q))
#define glTexCoord4sv              CGLFL_CALL(420,glTexCoord4sv,void,1,(v),(const GLshort *v))
#define glTexCoordPointer          CGLFL_CALL(421,glTexCoordPointer,void,4,(size,type,stride,pointer),(GLint size,GLenum type,GLsizei stride,const void *pointer))
#define glTexEnvf                  CGLFL_CALL(422,glTexEnvf,void,3,(target,pname,param),(GLenum target,GLenum pname,GLfloat param))
#define glTexEnvfv                 CGLFL_CALL(423,glTexEnvfv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLfloat *params))
#define glTexEnvi                  CGLFL_CALL(424,glTexEnvi,void,3,(target,pname,param),(GLenum target,GLenum pname,GLint param))
#define glTexEnviv                 CGLFL_CALL(425,glTexEnviv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLint *params))
#define glTexGend                  CGLFL_CALL(426,glTexGend,void,3,(coord,pname,param),(GLenum coord,GLenum pname,GLdouble param))
#define glTexGendv                 CGLFL_CALL(427,glTexGendv,void,3,(coord,pname,params),(GLenum coord,GLenum pname,const GLdouble *params))
#define glTexGenf                  CGLFL_CALL(428,glTexGenf,void,3,(coord,pname,param),(GLenum coord,GLenum pname,GLfloat param))
#define glTexGenfv                 CGLFL_CALL(429,glTexGenfv,void,3,(coord,pname,params),(GLenum coord,GLenum pname,const GLfloat *params))
#define glTexGeni                  CGLFL_CALL(430,glTexGeni,void,3,(coord,pname,param),(GLenum coord,GLenum pname,GLint param))
#define glTexGeniv                 CGLFL_CALL(431,glTexGeniv,void,3,(coord,pname,params),(GLenum coord,GLenum pname,const GLint *params))
#define glTexImage1D               CGLFL_CALL(432,glTexImage1D,void,8,(target,level,internalformat,width,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexImage2D               CGLFL_CALL(433,glTexImage2D,void,9,(target,level,internalformat,width,height,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexImage3D               CGLFL_CALL(434,glTexImage3D,void,10,(target,level,internalformat,width,height,depth,border,format,type,pixels),(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void *pixels))
#define glTexParameterf            CGLFL_CALL(435,glTexParameterf,void,3,(target,pname,param),(GLenum target,GLenum pname,GLfloat param))
#define glTexParameterfv           CGLFL_CALL(436,glTexParameterfv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLfloat *params))
#define glTexParameteri            CGLFL_CALL(437,glTexParameteri,void,3,(target,pname,param),(GLenum target,GLenum pname,GLint param))
#define glTexParameteriv           CGLFL_CALL(438,glTexParameteriv,void,3,(target,pname,params),(GLenum target,GLenum pname,const GLint *params))
#define glTexSubImage1D            CGLFL_CALL(439,glTexSubImage1D,void,7,(target,level,xoffset,width,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void *pixels))
#define glTexSubImage2D            CGLFL_CALL(440,glTexSubImage2D,void,9,(target,level,xoffset,yoffset,width,height,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void *pixels))
#define glTexSubImage3D            CGLFL_CALL(441,glTexSubImage3D,void,11,(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels),(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void *pixels))
#define glTranslated               CGLFL_CALL(442,glTranslated,void,3,(x,y,z),(GLdouble x,GLdouble y,GLdouble z))
#define glTranslatef               CGLFL_CALL(443,glTranslatef,void,3,(x,y,z),(GLfloat x,GLfloat y,GLfloat z))
#define glUniform1f                CGLFL_CALL(444,glUniform1f,void,2,(location,v0),(GLint location,GLfloat v0))
#define glUniform1fv               CGLFL_CALL(445,glUniform1fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform1i                CGLFL_CALL(446,glUniform1i,void,2,(location,v0),(GLint location,GLint v0))
#define glUniform1iv               CGLFL_CALL(447,glUniform1iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform2f                CGLFL_CALL(448,glUniform2f,void,3,(location,v0,v1),(GLint location,GLfloat v0,GLfloat v1))
#define glUniform2fv               CGLFL_CALL(449,glUniform2fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform2i                CGLFL_CALL(450,glUniform2i,void,3,(location,v0,v1),(GLint location,GLint v0,GLint v1))
#define glUniform2iv               CGLFL_CALL(451,glUniform2iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform3f                CGLFL_CALL(452,glUniform3f,void,4,(location,v0,v1,v2),(GLint location,GLfloat v0,GLfloat v1,GLfloat v2))
#define glUniform3fv               CGLFL_CALL(453,glUniform3fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform3i                CGLFL_CALL(454,glUniform3i,void,4,(location,v0,v1,v2),(GLint location,GLint v0,GLint v1,GLint v2))
#define glUniform3iv               CGLFL_CALL(455,glUniform3iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniform4f                CGLFL_CALL(456,glUniform4f,void,5,(location,v0,v1,v2,v3),(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3))
#define glUniform4fv               CGLFL_CALL(457,glUniform4fv,void,3,(location,count,value),(GLint location,GLsizei count,const GLfloat *value))
#define glUniform4i                CGLFL_CALL(458,glUniform4i,void,5,(location,v0,v1,v2,v3),(GLint location,GLint v0,GLint v1,GLint v2,GLint v3))
#define glUniform4iv               CGLFL_CALL(459,glUniform4iv,void,3,(location,count,value),(GLint location,GLsizei count,const GLint *value))
#define glUniformMatrix2fv         CGLFL_CALL(460,glUniformMatrix2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix2x3fv       CGLFL_CALL(461,glUniformMatrix2x3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix2x4fv       CGLFL_CALL(462,glUniformMatrix2x4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3fv         CGLFL_CALL(463,glUniformMatrix3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3x2fv       CGLFL_CALL(464,glUniformMatrix3x2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix3x4fv       CGLFL_CALL(465,glUniformMatrix3x4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4fv         CGLFL_CALL(466,glUniformMatrix4fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4x2fv       CGLFL_CALL(467,glUniformMatrix4x2fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUniformMatrix4x3fv       CGLFL_CALL(468,glUniformMatrix4x3fv,void,4,(location,count,transpose,value),(GLint location,GLsizei count,GLboolean transpose,const GLfloat *value))
#define glUnmapBuffer              CGLFL_CALL(469,glUnmapBuffer,GLboolean,1,(target),(GLenum target))
#define glUseProgram               CGLFL_CALL(470,glUseProgram,void,1,(program),(GLuint program))
#define glValidateProgram          CGLFL_CALL(471,glValidateProgram,void,1,(program),(GLuint program))
#define glVertex2d                 CGLFL_CALL(472,glVertex2d,void,2,(x,y),(GLdouble x,GLdouble y))
#define glVertex2dv                CGLFL_CALL(473,glVertex2dv,void,1,(v),(const GLdouble *v))
#define glVertex2f                 CGLFL_CALL(474,glVertex2f,void,2,(x,y),(GLfloat x,GLfloat y))
#define glVertex2fv                CGLFL_CALL(475,glVertex2fv,void,1,(v),(const GLfloat *v))
#define glVertex2i                 CGLFL_CALL(476,glVertex2i,void,2,(x,y),(GLint x,GLint y))
#define glVertex2iv                CGLFL_CALL(477,glVertex2iv,void,1,(v),(const GLint *v))
#define glVertex2s                 CGLFL_CALL(478,glVertex2s,void,2,(x,y),(GLshort x,GLshort y))
#define glVertex2sv                CGLFL_CALL(479,glVertex2sv,void,1,(v),(const GLshort *v))
#define glVertex3d                 CGLFL_CALL(480,glVertex3d,void,3,(x,y,z),(GLdouble x,GLdouble y,GLdouble z))
#define glVertex3dv                CGLFL_CALL(481,glVertex3dv,void,1,(v),(const GLdouble *v))
#define glVertex3f                 CGLFL_CALL(482,glVertex3f,void,3,(x,y,z),(GLfloat x,GLfloat y,GLfloat z))
#define glVertex3fv                CGLFL_CALL(483,glVertex3fv,void,1,(v),(const GLfloat *v))
#define glVertex3i                 CGLFL_CALL(484,glVertex3i,void,3,(x,y,z),(GLint x,GLint y,GLint z))
#define glVertex3iv                CGLFL_CALL(485,glVertex3iv,void,1,(v),(const GLint *v))
#define glVertex3s                 CGLFL_CALL(486,glVertex3s,void,3,(x,y,z),(GLshort x,GLshort y,GLshort z))
#define glVertex3sv                CGLFL_CALL(487,glVertex3sv,void,1,(v),(const GLshort *v))
#define glVertex4d                 CGLFL_CALL(488,glVertex4d,void,4,(x,y,z,w),(GLdouble x,GLdouble y,GLdouble z,GLdouble w))
#define glVertex4dv                CGLFL_CALL(489,glVertex4dv,void,1,(v),(const GLdouble *v))
#define glVertex4f                 CGLFL_CALL(490,glVertex4f,void,4,(x,y,z,w),(GLfloat x,GLfloat y,GLfloat z,GLfloat w))
#define glVertex4fv                CGLFL_CALL(491,glVertex4fv,void,1,(v),(const GLfloat *v))
#define glVertex4i                 CGLFL_CALL(492,glVertex4i,void,4,(x,y,z,w),(GLint x,GLint y,GLint z,GLint w))
#define glVertex4iv                CGLFL_CALL(493,glVertex4iv,void,1,(v),(const GLint *v))
#define glVertex4s                 CGLFL_CALL(494,glVertex4s,void,4,(x,y,z,w),(GLshort x,GLshort y,GLshort z,GLshort w))
#define glVertex4sv                CGLFL_CALL(495,glVertex4sv,void,1,(v),(const GLshort *v))
#define glVertexAttrib1d           CGLFL_CALL(496,glVertexAttrib1d,void,2,(index,x),(GLuint index,GLdouble x))
#define glVertexAttrib1dv          CGLFL_CALL(497,glVertexAttrib1dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib1f           CGLFL_CALL(498,glVertexAttrib1f,void,2,(index,x),(GLuint index,GLfloat x))
#define glVertexAttrib1fv          CGLFL_CALL(499,glVertexAttrib1fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib1s           CGLFL_CALL(500,glVertexAttrib1s,void,2,(index,x),(GLuint index,GLshort x))
#define glVertexAttrib1sv          CGLFL_CALL(501,glVertexAttrib1sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib2d           CGLFL_CALL(502,glVertexAttrib2d,void,3,(index,x,y),(GLuint index,GLdouble x,GLdouble y))
#define glVertexAttrib2dv          CGLFL_CALL(503,glVertexAttrib2dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib2f           CGLFL_CALL(504,glVertexAttrib2f,void,3,(index,x,y),(GLuint index,GLfloat x,GLfloat y))
#define glVertexAttrib2fv          CGLFL_CALL(505,glVertexAttrib2fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib2s           CGLFL_CALL(506,glVertexAttrib2s,void,3,(index,x,y),(GLuint index,GLshort x,GLshort y))
#define glVertexAttrib2sv          CGLFL_CALL(507,glVertexAttrib2sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib3d           CGLFL_CALL(508,glVertexAttrib3d,void,4,(index,x,y,z),(GLuint index,GLdouble x,GLdouble y,GLdouble z))
#define glVertexAttrib3dv          CGLFL_CALL(509,glVertexAttrib3dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib3f           CGLFL_CALL(510,glVertexAttrib3f,void,4,(index,x,y,z),(GLuint index,GLfloat x,GLfloat y,GLfloat z))
#define glVertexAttrib3fv          CGLFL_CALL(511,glVertexAttrib3fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib3s           CGLFL_CALL(512,glVertexAttrib3s,void,4,(index,x,y,z),(GLuint index,GLshort x,GLshort y,GLshort z))
#define glVertexAttrib3sv          CGLFL_CALL(513,glVertexAttrib3sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib4Nbv         CGLFL_CALL(514,glVertexAttrib4Nbv,void,2,(index,v),(GLuint index,const GLbyte *v))
#define glVertexAttrib4Niv         CGLFL_CALL(515,glVertexAttrib4Niv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttrib4Nsv         CGLFL_CALL(516,glVertexAttrib4Nsv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib4Nub         CGLFL_CALL(517,glVertexAttrib4Nub,void,5,(index,x,y,z,w),(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w))
#define glVertexAttrib4Nubv        CGLFL_CALL(518,glVertexAttrib4Nubv,void,2,(index,v),(GLuint index,const GLubyte *v))
#define glVertexAttrib4Nuiv        CGLFL_CALL(519,glVertexAttrib4Nuiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttrib4Nusv        CGLFL_CALL(520,glVertexAttrib4Nusv,void,2,(index,v),(GLuint index,const GLushort *v))
#define glVertexAttrib4bv          CGLFL_CALL(521,glVertexAttrib4bv,void,2,(index,v),(GLuint index,const GLbyte *v))
#define glVertexAttrib4d           CGLFL_CALL(522,glVertexAttrib4d,void,5,(index,x,y,z,w),(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w))
#define glVertexAttrib4dv          CGLFL_CALL(523,glVertexAttrib4dv,void,2,(index,v),(GLuint index,const GLdouble *v))
#define glVertexAttrib4f           CGLFL_CALL(524,glVertexAttrib4f,void,5,(index,x,y,z,w),(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w))
#define glVertexAttrib4fv          CGLFL_CALL(525,glVertexAttrib4fv,void,2,(index,v),(GLuint index,const GLfloat *v))
#define glVertexAttrib4iv          CGLFL_CALL(526,glVertexAttrib4iv,void,2,(index,v),(GLuint index,const GLint *v))
#define glVertexAttrib4s           CGLFL_CALL(527,glVertexAttrib4s,void,5,(index,x,y,z,w),(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w))
#define glVertexAttrib4sv          CGLFL_CALL(528,glVertexAttrib4sv,void,2,(index,v),(GLuint index,const GLshort *v))
#define glVertexAttrib4ubv         CGLFL_CALL(529,glVertexAttrib4ubv,void,2,(index,v),(GLuint index,const GLubyte *v))
#define glVertexAttrib4uiv         CGLFL_CALL(530,glVertexAttrib4uiv,void,2,(index,v),(GLuint index,const GLuint *v))
#define glVertexAttrib4usv         CGLFL_CALL(531,glVertexAttrib4usv,void,2,(index,v),(GLuint index,const GLushort *v))
#define glVertexAttribPointer      CGLFL_CALL(532,glVertexAttribPointer,void,6,(index,size,type,normalized,stride,pointer),(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void *pointer))
#define glVertexPointer            CGLFL_CALL(533,glVertexPointer,void,4,(size,type,stride,pointer),(GLint size,GLenum type,GLsizei stride,const void *pointer))
#define glViewport                 CGLFL_CALL(534,glViewport,void,4,(x,y,width,height),(GLint x,GLint y,GLsizei width,GLsizei height))
#define glWindowPos2d              CGLFL_CALL(535,glWindowPos2d,void,2,(x,y),(GLdouble x,GLdouble y))
#define glWindowPos2dv             CGLFL_CALL(536,glWindowPos2dv,void,1,(v),(const GLdouble *v))
#define glWindowPos2f              CGLFL_CALL(537,glWindowPos2f,void,2,(x,y),(GLfloat x,GLfloat y))
#define glWindowPos2fv             CGLFL_CALL(538,glWindowPos2fv,void,1,(v),(const GLfloat *v))
#define glWindowPos2i              CGLFL_CALL(539,glWindowPos2i,void,2,(x,y),(GLint x,GLint y))
#define glWindowPos2iv             CGLFL_CALL(540,glWindowPos2iv,void,1,(v),(const GLint *v))
#define glWindowPos2s              CGLFL_CALL(541,glWindowPos2s,void,2,(x,y),(GLshort x,GLshort y))
#define glWindowPos2sv             CGLFL_CALL(542,glWindowPos2sv,void,1,(v),(const GLshort *v))
#define glWindowPos3d              CGLFL_CALL(543,glWindowPos3d,void,3,(x,y,z),(GLdouble x,GLdouble y,GLdouble z))
#define glWindowPos3dv             CGLFL_CALL(544,glWindowPos3dv,void,1,(v),(const GLdouble *v))
#define glWindowPos3f              CGLFL_CALL(545,glWindowPos3f,void,3,(x,y,z),(GLfloat x,GLfloat y,GLfloat z))
#define glWindowPos3fv             CGLFL_CALL(546,glWindowPos3fv,void,1,(v),(const GLfloat *v))
#define glWindowPos3i              CGLFL_CALL(547,glWindowPos3i,void,3,(x,y,z),(GLint x,GLint y,GLint z))
#define glWindowPos3iv             CGLFL_CALL(548,glWindowPos3iv,void,1,(v),(const GLint *v))
#define glWindowPos3s              CGLFL_CALL(549,glWindowPos3s,void,3,(x,y,z),(GLshort x,GLshort y,GLshort z))
#define glWindowPos3sv             CGLFL_CALL(550,glWindowPos3sv,void,1,(v),(const GLshort *v))

#define GL_2D                                   0x0600
#define GL_2_BYTES                              0x1407
#define GL_3D                                   0x0601
#define GL_3D_COLOR                             0x0602
#define GL_3D_COLOR_TEXTURE                     0x0603
#define GL_3_BYTES                              0x1408
#define GL_4D_COLOR_TEXTURE                     0x0604
#define GL_4_BYTES                              0x1409
#define GL_ACCUM                                0x0100
#define GL_ACCUM_ALPHA_BITS                     0x0D5B
#define GL_ACCUM_BLUE_BITS                      0x0D5A
#define GL_ACCUM_BUFFER_BIT                     0x00000200
#define GL_ACCUM_CLEAR_VALUE                    0x0B80
#define GL_ACCUM_GREEN_BITS                     0x0D59
#define GL_ACCUM_RED_BITS                       0x0D58
#define GL_ACTIVE_ATTRIBUTES                    0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH          0x8B8A
#define GL_ACTIVE_TEXTURE                       0x84E0
#define GL_ACTIVE_UNIFORMS                      0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH            0x8B87
#define GL_ADD                                  0x0104
#define GL_ADD_SIGNED                           0x8574
#define GL_ALIASED_LINE_WIDTH_RANGE             0x846E
#define GL_ALIASED_POINT_SIZE_RANGE             0x846D
#define GL_ALL_ATTRIB_BITS                      0xFFFFFFFF
#define GL_ALPHA                                0x1906
#define GL_ALPHA12                              0x803D
#define GL_ALPHA16                              0x803E
#define GL_ALPHA4                               0x803B
#define GL_ALPHA8                               0x803C
#define GL_ALPHA_BIAS                           0x0D1D
#define GL_ALPHA_BITS                           0x0D55
#define GL_ALPHA_SCALE                          0x0D1C
#define GL_ALPHA_TEST                           0x0BC0
#define GL_ALPHA_TEST_FUNC                      0x0BC1
#define GL_ALPHA_TEST_REF                       0x0BC2
#define GL_ALWAYS                               0x0207
#define GL_AMBIENT                              0x1200
#define GL_AMBIENT_AND_DIFFUSE                  0x1602
#define GL_AND                                  0x1501
#define GL_AND_INVERTED                         0x1504
#define GL_AND_REVERSE                          0x1502
#define GL_ARRAY_BUFFER                         0x8892
#define GL_ARRAY_BUFFER_BINDING                 0x8894
#define GL_ATTACHED_SHADERS                     0x8B85
#define GL_ATTRIB_STACK_DEPTH                   0x0BB0
#define GL_AUTO_NORMAL                          0x0D80
#define GL_AUX0                                 0x0409
#define GL_AUX1                                 0x040A
#define GL_AUX2                                 0x040B
#define GL_AUX3                                 0x040C
#define GL_AUX_BUFFERS                          0x0C00
#define GL_BACK                                 0x0405
#define GL_BACK_LEFT                            0x0402
#define GL_BACK_RIGHT                           0x0403
#define GL_BGR                                  0x80E0
#define GL_BGRA                                 0x80E1
#define GL_BITMAP                               0x1A00
#define GL_BITMAP_TOKEN                         0x0704
#define GL_BLEND                                0x0BE2
#define GL_BLEND_COLOR                          0x8005
#define GL_BLEND_DST                            0x0BE0
#define GL_BLEND_DST_ALPHA                      0x80CA
#define GL_BLEND_DST_RGB                        0x80C8
#define GL_BLEND_EQUATION                       0x8009
#define GL_BLEND_EQUATION_ALPHA                 0x883D
#define GL_BLEND_EQUATION_RGB                   0x8009
#define GL_BLEND_SRC                            0x0BE1
#define GL_BLEND_SRC_ALPHA                      0x80CB
#define GL_BLEND_SRC_RGB                        0x80C9
#define GL_BLUE                                 0x1905
#define GL_BLUE_BIAS                            0x0D1B
#define GL_BLUE_BITS                            0x0D54
#define GL_BLUE_SCALE                           0x0D1A
#define GL_BOOL                                 0x8B56
#define GL_BOOL_VEC2                            0x8B57
#define GL_BOOL_VEC3                            0x8B58
#define GL_BOOL_VEC4                            0x8B59
#define GL_BUFFER_ACCESS                        0x88BB
#define GL_BUFFER_MAPPED                        0x88BC
#define GL_BUFFER_MAP_POINTER                   0x88BD
#define GL_BUFFER_SIZE                          0x8764
#define GL_BUFFER_USAGE                         0x8765
#define GL_BYTE                                 0x1400
#define GL_C3F_V3F                              0x2A24
#define GL_C4F_N3F_V3F                          0x2A26
#define GL_C4UB_V2F                             0x2A22
#define GL_C4UB_V3F                             0x2A23
#define GL_CCW                                  0x0901
#define GL_CLAMP                                0x2900
#define GL_CLAMP_TO_BORDER                      0x812D
#define GL_CLAMP_TO_EDGE                        0x812F
#define GL_CLEAR                                0x1500
#define GL_CLIENT_ACTIVE_TEXTURE                0x84E1
#define GL_CLIENT_ALL_ATTRIB_BITS               0xFFFFFFFF
#define GL_CLIENT_ATTRIB_STACK_DEPTH            0x0BB1
#define GL_CLIENT_PIXEL_STORE_BIT               0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT              0x00000002
#define GL_CLIP_PLANE0                          0x3000
#define GL_CLIP_PLANE1                          0x3001
#define GL_CLIP_PLANE2                          0x3002
#define GL_CLIP_PLANE3                          0x3003
#define GL_CLIP_PLANE4                          0x3004
#define GL_CLIP_PLANE5                          0x3005
#define GL_COEFF                                0x0A00
#define GL_COLOR                                0x1800
#define GL_COLOR_ARRAY                          0x8076
#define GL_COLOR_ARRAY_BUFFER_BINDING           0x8898
#define GL_COLOR_ARRAY_POINTER                  0x8090
#define GL_COLOR_ARRAY_SIZE                     0x8081
#define GL_COLOR_ARRAY_STRIDE                   0x8083
#define GL_COLOR_ARRAY_TYPE                     0x8082
#define GL_COLOR_BUFFER_BIT                     0x00004000
#define GL_COLOR_CLEAR_VALUE                    0x0C22
#define GL_COLOR_INDEX                          0x1900
#define GL_COLOR_INDEXES                        0x1603
#define GL_COLOR_LOGIC_OP                       0x0BF2
#define GL_COLOR_MATERIAL                       0x0B57
#define GL_COLOR_MATERIAL_FACE                  0x0B55
#define GL_COLOR_MATERIAL_PARAMETER             0x0B56
#define GL_COLOR_SUM                            0x8458
#define GL_COLOR_WRITEMASK                      0x0C23
#define GL_COMBINE                              0x8570
#define GL_COMBINE_ALPHA                        0x8572
#define GL_COMBINE_RGB                          0x8571
#define GL_COMPARE_R_TO_TEXTURE                 0x884E
#define GL_COMPILE                              0x1300
#define GL_COMPILE_AND_EXECUTE                  0x1301
#define GL_COMPILE_STATUS                       0x8B81
#define GL_COMPRESSED_ALPHA                     0x84E9
#define GL_COMPRESSED_INTENSITY                 0x84EC
#define GL_COMPRESSED_LUMINANCE                 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA           0x84EB
#define GL_COMPRESSED_RGB                       0x84ED
#define GL_COMPRESSED_RGBA                      0x84EE
#define GL_COMPRESSED_SLUMINANCE                0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA          0x8C4B
#define GL_COMPRESSED_SRGB                      0x8C48
#define GL_COMPRESSED_SRGB_ALPHA                0x8C49
#define GL_COMPRESSED_TEXTURE_FORMATS           0x86A3
#define GL_CONSTANT                             0x8576
#define GL_CONSTANT_ALPHA                       0x8003
#define GL_CONSTANT_ATTENUATION                 0x1207
#define GL_CONSTANT_COLOR                       0x8001
#define GL_COORD_REPLACE                        0x8862
#define GL_COPY                                 0x1503
#define GL_COPY_INVERTED                        0x150C
#define GL_COPY_PIXEL_TOKEN                     0x0706
#define GL_CULL_FACE                            0x0B44
#define GL_CULL_FACE_MODE                       0x0B45
#define GL_CURRENT_BIT                          0x00000001
#define GL_CURRENT_COLOR                        0x0B00
#define GL_CURRENT_FOG_COORD                    0x8453
#define GL_CURRENT_FOG_COORDINATE               0x8453
#define GL_CURRENT_INDEX                        0x0B01
#define GL_CURRENT_NORMAL                       0x0B02
#define GL_CURRENT_PROGRAM                      0x8B8D
#define GL_CURRENT_QUERY                        0x8865
#define GL_CURRENT_RASTER_COLOR                 0x0B04
#define GL_CURRENT_RASTER_DISTANCE              0x0B09
#define GL_CURRENT_RASTER_INDEX                 0x0B05
#define GL_CURRENT_RASTER_POSITION              0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID        0x0B08
#define GL_CURRENT_RASTER_SECONDARY_COLOR       0x845F
#define GL_CURRENT_RASTER_TEXTURE_COORDS        0x0B06
#define GL_CURRENT_SECONDARY_COLOR              0x8459
#define GL_CURRENT_TEXTURE_COORDS               0x0B03
#define GL_CURRENT_VERTEX_ATTRIB                0x8626
#define GL_CW                                   0x0900
#define GL_DECAL                                0x2101
#define GL_DECR                                 0x1E03
#define GL_DECR_WRAP                            0x8508
#define GL_DELETE_STATUS                        0x8B80
#define GL_DEPTH                                0x1801
#define GL_DEPTH_BIAS                           0x0D1F
#define GL_DEPTH_BITS                           0x0D56
#define GL_DEPTH_BUFFER_BIT                     0x00000100
#define GL_DEPTH_CLEAR_VALUE                    0x0B73
#define GL_DEPTH_COMPONENT                      0x1902
#define GL_DEPTH_COMPONENT16                    0x81A5
#define GL_DEPTH_COMPONENT24                    0x81A6
#define GL_DEPTH_COMPONENT32                    0x81A7
#define GL_DEPTH_FUNC                           0x0B74
#define GL_DEPTH_RANGE                          0x0B70
#define GL_DEPTH_SCALE                          0x0D1E
#define GL_DEPTH_TEST                           0x0B71
#define GL_DEPTH_TEXTURE_MODE                   0x884B
#define GL_DEPTH_WRITEMASK                      0x0B72
#define GL_DIFFUSE                              0x1201
#define GL_DITHER                               0x0BD0
#define GL_DOMAIN                               0x0A02
#define GL_DONT_CARE                            0x1100
#define GL_DOT3_RGB                             0x86AE
#define GL_DOT3_RGBA                            0x86AF
#define GL_DOUBLE                               0x140A
#define GL_DOUBLEBUFFER                         0x0C32
#define GL_DRAW_BUFFER                          0x0C01
#define GL_DRAW_BUFFER0                         0x8825
#define GL_DRAW_BUFFER1                         0x8826
#define GL_DRAW_BUFFER10                        0x882F
#define GL_DRAW_BUFFER11                        0x8830
#define GL_DRAW_BUFFER12                        0x8831
#define GL_DRAW_BUFFER13                        0x8832
#define GL_DRAW_BUFFER14                        0x8833
#define GL_DRAW_BUFFER15                        0x8834
#define GL_DRAW_BUFFER2                         0x8827
#define GL_DRAW_BUFFER3                         0x8828
#define GL_DRAW_BUFFER4                         0x8829
#define GL_DRAW_BUFFER5                         0x882A
#define GL_DRAW_BUFFER6                         0x882B
#define GL_DRAW_BUFFER7                         0x882C
#define GL_DRAW_BUFFER8                         0x882D
#define GL_DRAW_BUFFER9                         0x882E
#define GL_DRAW_PIXEL_TOKEN                     0x0705
#define GL_DST_ALPHA                            0x0304
#define GL_DST_COLOR                            0x0306
#define GL_DYNAMIC_COPY                         0x88EA
#define GL_DYNAMIC_DRAW                         0x88E8
#define GL_DYNAMIC_READ                         0x88E9
#define GL_EDGE_FLAG                            0x0B43
#define GL_EDGE_FLAG_ARRAY                      0x8079
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING       0x889B
#define GL_EDGE_FLAG_ARRAY_POINTER              0x8093
#define GL_EDGE_FLAG_ARRAY_STRIDE               0x808C
#define GL_ELEMENT_ARRAY_BUFFER                 0x8893
#define GL_ELEMENT_ARRAY_BUFFER_BINDING         0x8895
#define GL_EMISSION                             0x1600
#define GL_ENABLE_BIT                           0x00002000
#define GL_EQUAL                                0x0202
#define GL_EQUIV                                0x1509
#define GL_EVAL_BIT                             0x00010000
#define GL_EXP                                  0x0800
#define GL_EXP2                                 0x0801
#define GL_EXTENSIONS                           0x1F03
#define GL_EYE_LINEAR                           0x2400
#define GL_EYE_PLANE                            0x2502
#define GL_FALSE                                0
#define GL_FASTEST                              0x1101
#define GL_FEEDBACK                             0x1C01
#define GL_FEEDBACK_BUFFER_POINTER              0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE                 0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE                 0x0DF2
#define GL_FILL                                 0x1B02
#define GL_FLAT                                 0x1D00
#define GL_FLOAT                                0x1406
#define GL_FLOAT_MAT2                           0x8B5A
#define GL_FLOAT_MAT2x3                         0x8B65
#define GL_FLOAT_MAT2x4                         0x8B66
#define GL_FLOAT_MAT3                           0x8B5B
#define GL_FLOAT_MAT3x2                         0x8B67
#define GL_FLOAT_MAT3x4                         0x8B68
#define GL_FLOAT_MAT4                           0x8B5C
#define GL_FLOAT_MAT4x2                         0x8B69
#define GL_FLOAT_MAT4x3                         0x8B6A
#define GL_FLOAT_VEC2                           0x8B50
#define GL_FLOAT_VEC3                           0x8B51
#define GL_FLOAT_VEC4                           0x8B52
#define GL_FOG                                  0x0B60
#define GL_FOG_BIT                              0x00000080
#define GL_FOG_COLOR                            0x0B66
#define GL_FOG_COORD                            0x8451
#define GL_FOG_COORDINATE                       0x8451
#define GL_FOG_COORDINATE_ARRAY                 0x8457
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING  0x889D
#define GL_FOG_COORDINATE_ARRAY_POINTER         0x8456
#define GL_FOG_COORDINATE_ARRAY_STRIDE          0x8455
#define GL_FOG_COORDINATE_ARRAY_TYPE            0x8454
#define GL_FOG_COORDINATE_SOURCE                0x8450
#define GL_FOG_COORD_ARRAY                      0x8457
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING       0x889D
#define GL_FOG_COORD_ARRAY_POINTER              0x8456
#define GL_FOG_COORD_ARRAY_STRIDE               0x8455
#define GL_FOG_COORD_ARRAY_TYPE                 0x8454
#define GL_FOG_COORD_SRC                        0x8450
#define GL_FOG_DENSITY                          0x0B62
#define GL_FOG_END                              0x0B64
#define GL_FOG_HINT                             0x0C54
#define GL_FOG_INDEX                            0x0B61
#define GL_FOG_MODE                             0x0B65
#define GL_FOG_START                            0x0B63
#define GL_FRAGMENT_DEPTH                       0x8452
#define GL_FRAGMENT_SHADER                      0x8B30
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT      0x8B8B
#define GL_FRONT                                0x0404
#define GL_FRONT_AND_BACK                       0x0408
#define GL_FRONT_FACE                           0x0B46
#define GL_FRONT_LEFT                           0x0400
#define GL_FRONT_RIGHT                          0x0401
#define GL_FUNC_ADD                             0x8006
#define GL_FUNC_REVERSE_SUBTRACT                0x800B
#define GL_FUNC_SUBTRACT                        0x800A
#define GL_GENERATE_MIPMAP                      0x8191
#define GL_GENERATE_MIPMAP_HINT                 0x8192
#define GL_GEQUAL                               0x0206
#define GL_GREATER                              0x0204
#define GL_GREEN                                0x1904
#define GL_GREEN_BIAS                           0x0D19
#define GL_GREEN_BITS                           0x0D53
#define GL_GREEN_SCALE                          0x0D18
#define GL_HINT_BIT                             0x00008000
#define GL_INCR                                 0x1E02
#define GL_INCR_WRAP                            0x8507
#define GL_INDEX_ARRAY                          0x8077
#define GL_INDEX_ARRAY_BUFFER_BINDING           0x8899
#define GL_INDEX_ARRAY_POINTER                  0x8091
#define GL_INDEX_ARRAY_STRIDE                   0x8086
#define GL_INDEX_ARRAY_TYPE                     0x8085
#define GL_INDEX_BITS                           0x0D51
#define GL_INDEX_CLEAR_VALUE                    0x0C20
#define GL_INDEX_LOGIC_OP                       0x0BF1
#define GL_INDEX_MODE                           0x0C30
#define GL_INDEX_OFFSET                         0x0D13
#define GL_INDEX_SHIFT                          0x0D12
#define GL_INDEX_WRITEMASK                      0x0C21
#define GL_INFO_LOG_LENGTH                      0x8B84
#define GL_INT                                  0x1404
#define GL_INTENSITY                            0x8049
#define GL_INTENSITY12                          0x804C
#define GL_INTENSITY16                          0x804D
#define GL_INTENSITY4                           0x804A
#define GL_INTENSITY8                           0x804B
#define GL_INTERPOLATE                          0x8575
#define GL_INT_VEC2                             0x8B53
#define GL_INT_VEC3                             0x8B54
#define GL_INT_VEC4                             0x8B55
#define GL_INVALID_ENUM                         0x0500
#define GL_INVALID_OPERATION                    0x0502
#define GL_INVALID_VALUE                        0x0501
#define GL_INVERT                               0x150A
#define GL_KEEP                                 0x1E00
#define GL_LEFT                                 0x0406
#define GL_LEQUAL                               0x0203
#define GL_LESS                                 0x0201
#define GL_LIGHT0                               0x4000
#define GL_LIGHT1                               0x4001
#define GL_LIGHT2                               0x4002
#define GL_LIGHT3                               0x4003
#define GL_LIGHT4                               0x4004
#define GL_LIGHT5                               0x4005
#define GL_LIGHT6                               0x4006
#define GL_LIGHT7                               0x4007
#define GL_LIGHTING                             0x0B50
#define GL_LIGHTING_BIT                         0x00000040
#define GL_LIGHT_MODEL_AMBIENT                  0x0B53
#define GL_LIGHT_MODEL_COLOR_CONTROL            0x81F8
#define GL_LIGHT_MODEL_LOCAL_VIEWER             0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE                 0x0B52
#define GL_LINE                                 0x1B01
#define GL_LINEAR                               0x2601
#define GL_LINEAR_ATTENUATION                   0x1208
#define GL_LINEAR_MIPMAP_LINEAR                 0x2703
#define GL_LINEAR_MIPMAP_NEAREST                0x2701
#define GL_LINES                                0x0001
#define GL_LINE_BIT                             0x00000004
#define GL_LINE_LOOP                            0x0002
#define GL_LINE_RESET_TOKEN                     0x0707
#define GL_LINE_SMOOTH                          0x0B20
#define GL_LINE_SMOOTH_HINT                     0x0C52
#define GL_LINE_STIPPLE                         0x0B24
#define GL_LINE_STIPPLE_PATTERN                 0x0B25
#define GL_LINE_STIPPLE_REPEAT                  0x0B26
#define GL_LINE_STRIP                           0x0003
#define GL_LINE_TOKEN                           0x0702
#define GL_LINE_WIDTH                           0x0B21
#define GL_LINE_WIDTH_GRANULARITY               0x0B23
#define GL_LINE_WIDTH_RANGE                     0x0B22
#define GL_LINK_STATUS                          0x8B82
#define GL_LIST_BASE                            0x0B32
#define GL_LIST_BIT                             0x00020000
#define GL_LIST_INDEX                           0x0B33
#define GL_LIST_MODE                            0x0B30
#define GL_LOAD                                 0x0101
#define GL_LOGIC_OP                             0x0BF1
#define GL_LOGIC_OP_MODE                        0x0BF0
#define GL_LOWER_LEFT                           0x8CA1
#define GL_LUMINANCE                            0x1909
#define GL_LUMINANCE12                          0x8041
#define GL_LUMINANCE12_ALPHA12                  0x8047
#define GL_LUMINANCE12_ALPHA4                   0x8046
#define GL_LUMINANCE16                          0x8042
#define GL_LUMINANCE16_ALPHA16                  0x8048
#define GL_LUMINANCE4                           0x803F
#define GL_LUMINANCE4_ALPHA4                    0x8043
#define GL_LUMINANCE6_ALPHA2                    0x8044
#define GL_LUMINANCE8                           0x8040
#define GL_LUMINANCE8_ALPHA8                    0x8045
#define GL_LUMINANCE_ALPHA                      0x190A
#define GL_MAP1_COLOR_4                         0x0D90
#define GL_MAP1_GRID_DOMAIN                     0x0DD0
#define GL_MAP1_GRID_SEGMENTS                   0x0DD1
#define GL_MAP1_INDEX                           0x0D91
#define GL_MAP1_NORMAL                          0x0D92
#define GL_MAP1_TEXTURE_COORD_1                 0x0D93
#define GL_MAP1_TEXTURE_COORD_2                 0x0D94
#define GL_MAP1_TEXTURE_COORD_3                 0x0D95
#define GL_MAP1_TEXTURE_COORD_4                 0x0D96
#define GL_MAP1_VERTEX_3                        0x0D97
#define GL_MAP1_VERTEX_4                        0x0D98
#define GL_MAP2_COLOR_4                         0x0DB0
#define GL_MAP2_GRID_DOMAIN                     0x0DD2
#define GL_MAP2_GRID_SEGMENTS                   0x0DD3
#define GL_MAP2_INDEX                           0x0DB1
#define GL_MAP2_NORMAL                          0x0DB2
#define GL_MAP2_TEXTURE_COORD_1                 0x0DB3
#define GL_MAP2_TEXTURE_COORD_2                 0x0DB4
#define GL_MAP2_TEXTURE_COORD_3                 0x0DB5
#define GL_MAP2_TEXTURE_COORD_4                 0x0DB6
#define GL_MAP2_VERTEX_3                        0x0DB7
#define GL_MAP2_VERTEX_4                        0x0DB8
#define GL_MAP_COLOR                            0x0D10
#define GL_MAP_STENCIL                          0x0D11
#define GL_MATRIX_MODE                          0x0BA0
#define GL_MAX                                  0x8008
#define GL_MAX_3D_TEXTURE_SIZE                  0x8073
#define GL_MAX_ATTRIB_STACK_DEPTH               0x0D35
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH        0x0D3B
#define GL_MAX_CLIP_PLANES                      0x0D32
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS     0x8B4D
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE            0x851C
#define GL_MAX_DRAW_BUFFERS                     0x8824
#define GL_MAX_ELEMENTS_INDICES                 0x80E9
#define GL_MAX_ELEMENTS_VERTICES                0x80E8
#define GL_MAX_EVAL_ORDER                       0x0D30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS      0x8B49
#define GL_MAX_LIGHTS                           0x0D31
#define GL_MAX_LIST_NESTING                     0x0B31
#define GL_MAX_MODELVIEW_STACK_DEPTH            0x0D36
#define GL_MAX_NAME_STACK_DEPTH                 0x0D37
#define GL_MAX_PIXEL_MAP_TABLE                  0x0D34
#define GL_MAX_PROJECTION_STACK_DEPTH           0x0D38
#define GL_MAX_TEXTURE_COORDS                   0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS              0x8872
#define GL_MAX_TEXTURE_LOD_BIAS                 0x84FD
#define GL_MAX_TEXTURE_SIZE                     0x0D33
#define GL_MAX_TEXTURE_STACK_DEPTH              0x0D39
#define GL_MAX_TEXTURE_UNITS                    0x84E2
#define GL_MAX_VARYING_FLOATS                   0x8B4B
#define GL_MAX_VERTEX_ATTRIBS                   0x8869
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS       0x8B4C
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS        0x8B4A
#define GL_MAX_VIEWPORT_DIMS                    0x0D3A
#define GL_MIN                                  0x8007
#define GL_MIRRORED_REPEAT                      0x8370
#define GL_MODELVIEW                            0x1700
#define GL_MODELVIEW_MATRIX                     0x0BA6
#define GL_MODELVIEW_STACK_DEPTH                0x0BA3
#define GL_MODULATE                             0x2100
#define GL_MULT                                 0x0103
#define GL_MULTISAMPLE                          0x809D
#define GL_MULTISAMPLE_BIT                      0x20000000
#define GL_N3F_V3F                              0x2A25
#define GL_NAME_STACK_DEPTH                     0x0D70
#define GL_NAND                                 0x150E
#define GL_NEAREST                              0x2600
#define GL_NEAREST_MIPMAP_LINEAR                0x2702
#define GL_NEAREST_MIPMAP_NEAREST               0x2700
#define GL_NEVER                                0x0200
#define GL_NICEST                               0x1102
#define GL_NONE                                 0
#define GL_NOOP                                 0x1505
#define GL_NOR                                  0x1508
#define GL_NORMALIZE                            0x0BA1
#define GL_NORMAL_ARRAY                         0x8075
#define GL_NORMAL_ARRAY_BUFFER_BINDING          0x8897
#define GL_NORMAL_ARRAY_POINTER                 0x808F
#define GL_NORMAL_ARRAY_STRIDE                  0x807F
#define GL_NORMAL_ARRAY_TYPE                    0x807E
#define GL_NORMAL_MAP                           0x8511
#define GL_NOTEQUAL                             0x0205
#define GL_NO_ERROR                             0
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS       0x86A2
#define GL_OBJECT_LINEAR                        0x2401
#define GL_OBJECT_PLANE                         0x2501
#define GL_ONE                                  1
#define GL_ONE_MINUS_CONSTANT_ALPHA             0x8004
#define GL_ONE_MINUS_CONSTANT_COLOR             0x8002
#define GL_ONE_MINUS_DST_ALPHA                  0x0305
#define GL_ONE_MINUS_DST_COLOR                  0x0307
#define GL_ONE_MINUS_SRC_ALPHA                  0x0303
#define GL_ONE_MINUS_SRC_COLOR                  0x0301
#define GL_OPERAND0_ALPHA                       0x8598
#define GL_OPERAND0_RGB                         0x8590
#define GL_OPERAND1_ALPHA                       0x8599
#define GL_OPERAND1_RGB                         0x8591
#define GL_OPERAND2_ALPHA                       0x859A
#define GL_OPERAND2_RGB                         0x8592
#define GL_OR                                   0x1507
#define GL_ORDER                                0x0A01
#define GL_OR_INVERTED                          0x150D
#define GL_OR_REVERSE                           0x150B
#define GL_OUT_OF_MEMORY                        0x0505
#define GL_PACK_ALIGNMENT                       0x0D05
#define GL_PACK_IMAGE_HEIGHT                    0x806C
#define GL_PACK_LSB_FIRST                       0x0D01
#define GL_PACK_ROW_LENGTH                      0x0D02
#define GL_PACK_SKIP_IMAGES                     0x806B
#define GL_PACK_SKIP_PIXELS                     0x0D04
#define GL_PACK_SKIP_ROWS                       0x0D03
#define GL_PACK_SWAP_BYTES                      0x0D00
#define GL_PASS_THROUGH_TOKEN                   0x0700
#define GL_PERSPECTIVE_CORRECTION_HINT          0x0C50
#define GL_PIXEL_MAP_A_TO_A                     0x0C79
#define GL_PIXEL_MAP_A_TO_A_SIZE                0x0CB9
#define GL_PIXEL_MAP_B_TO_B                     0x0C78
#define GL_PIXEL_MAP_B_TO_B_SIZE                0x0CB8
#define GL_PIXEL_MAP_G_TO_G                     0x0C77
#define GL_PIXEL_MAP_G_TO_G_SIZE                0x0CB7
#define GL_PIXEL_MAP_I_TO_A                     0x0C75
#define GL_PIXEL_MAP_I_TO_A_SIZE                0x0CB5
#define GL_PIXEL_MAP_I_TO_B                     0x0C74
#define GL_PIXEL_MAP_I_TO_B_SIZE                0x0CB4
#define GL_PIXEL_MAP_I_TO_G                     0x0C73
#define GL_PIXEL_MAP_I_TO_G_SIZE                0x0CB3
#define GL_PIXEL_MAP_I_TO_I                     0x0C70
#define GL_PIXEL_MAP_I_TO_I_SIZE                0x0CB0
#define GL_PIXEL_MAP_I_TO_R                     0x0C72
#define GL_PIXEL_MAP_I_TO_R_SIZE                0x0CB2
#define GL_PIXEL_MAP_R_TO_R                     0x0C76
#define GL_PIXEL_MAP_R_TO_R_SIZE                0x0CB6
#define GL_PIXEL_MAP_S_TO_S                     0x0C71
#define GL_PIXEL_MAP_S_TO_S_SIZE                0x0CB1
#define GL_PIXEL_MODE_BIT                       0x00000020
#define GL_PIXEL_PACK_BUFFER                    0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING            0x88ED
#define GL_PIXEL_UNPACK_BUFFER                  0x88EC
#define GL_PIXEL_UNPACK_BUFFER_BINDING          0x88EF
#define GL_POINT                                0x1B00
#define GL_POINTS                               0x0000
#define GL_POINT_BIT                            0x00000002
#define GL_POINT_DISTANCE_ATTENUATION           0x8129
#define GL_POINT_FADE_THRESHOLD_SIZE            0x8128
#define GL_POINT_SIZE                           0x0B11
#define GL_POINT_SIZE_GRANULARITY               0x0B13
#define GL_POINT_SIZE_MAX                       0x8127
#define GL_POINT_SIZE_MIN                       0x8126
#define GL_POINT_SIZE_RANGE                     0x0B12
#define GL_POINT_SMOOTH                         0x0B10
#define GL_POINT_SMOOTH_HINT                    0x0C51
#define GL_POINT_SPRITE                         0x8861
#define GL_POINT_SPRITE_COORD_ORIGIN            0x8CA0
#define GL_POINT_TOKEN                          0x0701
#define GL_POLYGON                              0x0009
#define GL_POLYGON_BIT                          0x00000008
#define GL_POLYGON_MODE                         0x0B40
#define GL_POLYGON_OFFSET_FACTOR                0x8038
#define GL_POLYGON_OFFSET_FILL                  0x8037
#define GL_POLYGON_OFFSET_LINE                  0x2A02
#define GL_POLYGON_OFFSET_POINT                 0x2A01
#define GL_POLYGON_OFFSET_UNITS                 0x2A00
#define GL_POLYGON_SMOOTH                       0x0B41
#define GL_POLYGON_SMOOTH_HINT                  0x0C53
#define GL_POLYGON_STIPPLE                      0x0B42
#define GL_POLYGON_STIPPLE_BIT                  0x00000010
#define GL_POLYGON_TOKEN                        0x0703
#define GL_POSITION                             0x1203
#define GL_PREVIOUS                             0x8578
#define GL_PRIMARY_COLOR                        0x8577
#define GL_PROJECTION                           0x1701
#define GL_PROJECTION_MATRIX                    0x0BA7
#define GL_PROJECTION_STACK_DEPTH               0x0BA4
#define GL_PROXY_TEXTURE_1D                     0x8063
#define GL_PROXY_TEXTURE_2D                     0x8064
#define GL_PROXY_TEXTURE_3D                     0x8070
#define GL_PROXY_TEXTURE_CUBE_MAP               0x851B
#define GL_Q                                    0x2003
#define GL_QUADRATIC_ATTENUATION                0x1209
#define GL_QUADS                                0x0007
#define GL_QUAD_STRIP                           0x0008
#define GL_QUERY_COUNTER_BITS                   0x8864
#define GL_QUERY_RESULT                         0x8866
#define GL_QUERY_RESULT_AVAILABLE               0x8867
#define GL_R                                    0x2002
#define GL_R3_G3_B2                             0x2A10
#define GL_READ_BUFFER                          0x0C02
#define GL_READ_ONLY                            0x88B8
#define GL_READ_WRITE                           0x88BA
#define GL_RED                                  0x1903
#define GL_RED_BIAS                             0x0D15
#define GL_RED_BITS                             0x0D52
#define GL_RED_SCALE                            0x0D14
#define GL_REFLECTION_MAP                       0x8512
#define GL_RENDER                               0x1C00
#define GL_RENDERER                             0x1F01
#define GL_RENDER_MODE                          0x0C40
#define GL_REPEAT                               0x2901
#define GL_REPLACE                              0x1E01
#define GL_RESCALE_NORMAL                       0x803A
#define GL_RETURN                               0x0102
#define GL_RGB                                  0x1907
#define GL_RGB10                                0x8052
#define GL_RGB10_A2                             0x8059
#define GL_RGB12                                0x8053
#define GL_RGB16                                0x8054
#define GL_RGB4                                 0x804F
#define GL_RGB5                                 0x8050
#define GL_RGB5_A1                              0x8057
#define GL_RGB8                                 0x8051
#define GL_RGBA                                 0x1908
#define GL_RGBA12                               0x805A
#define GL_RGBA16                               0x805B
#define GL_RGBA2                                0x8055
#define GL_RGBA4                                0x8056
#define GL_RGBA8                                0x8058
#define GL_RGBA_MODE                            0x0C31
#define GL_RGB_SCALE                            0x8573
#define GL_RIGHT                                0x0407
#define GL_S                                    0x2000
#define GL_SAMPLER_1D                           0x8B5D
#define GL_SAMPLER_1D_SHADOW                    0x8B61
#define GL_SAMPLER_2D                           0x8B5E
#define GL_SAMPLER_2D_SHADOW                    0x8B62
#define GL_SAMPLER_3D                           0x8B5F
#define GL_SAMPLER_CUBE                         0x8B60
#define GL_SAMPLES                              0x80A9
#define GL_SAMPLES_PASSED                       0x8914
#define GL_SAMPLE_ALPHA_TO_COVERAGE             0x809E
#define GL_SAMPLE_ALPHA_TO_ONE                  0x809F
#define GL_SAMPLE_BUFFERS                       0x80A8
#define GL_SAMPLE_COVERAGE                      0x80A0
#define GL_SAMPLE_COVERAGE_INVERT               0x80AB
#define GL_SAMPLE_COVERAGE_VALUE                0x80AA
#define GL_SCISSOR_BIT                          0x00080000
#define GL_SCISSOR_BOX                          0x0C10
#define GL_SCISSOR_TEST                         0x0C11
#define GL_SECONDARY_COLOR_ARRAY                0x845E
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_SECONDARY_COLOR_ARRAY_POINTER        0x845D
#define GL_SECONDARY_COLOR_ARRAY_SIZE           0x845A
#define GL_SECONDARY_COLOR_ARRAY_STRIDE         0x845C
#define GL_SECONDARY_COLOR_ARRAY_TYPE           0x845B
#define GL_SELECT                               0x1C02
#define GL_SELECTION_BUFFER_POINTER             0x0DF3
#define GL_SELECTION_BUFFER_SIZE                0x0DF4
#define GL_SEPARATE_SPECULAR_COLOR              0x81FA
#define GL_SET                                  0x150F
#define GL_SHADER_SOURCE_LENGTH                 0x8B88
#define GL_SHADER_TYPE                          0x8B4F
#define GL_SHADE_MODEL                          0x0B54
#define GL_SHADING_LANGUAGE_VERSION             0x8B8C
#define GL_SHININESS                            0x1601
#define GL_SHORT                                0x1402
#define GL_SINGLE_COLOR                         0x81F9
#define GL_SLUMINANCE                           0x8C46
#define GL_SLUMINANCE8                          0x8C47
#define GL_SLUMINANCE8_ALPHA8                   0x8C45
#define GL_SLUMINANCE_ALPHA                     0x8C44
#define GL_SMOOTH                               0x1D01
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY        0x0B23
#define GL_SMOOTH_LINE_WIDTH_RANGE              0x0B22
#define GL_SMOOTH_POINT_SIZE_GRANULARITY        0x0B13
#define GL_SMOOTH_POINT_SIZE_RANGE              0x0B12
#define GL_SOURCE0_ALPHA                        0x8588
#define GL_SOURCE0_RGB                          0x8580
#define GL_SOURCE1_ALPHA                        0x8589
#define GL_SOURCE1_RGB                          0x8581
#define GL_SOURCE2_ALPHA                        0x858A
#define GL_SOURCE2_RGB                          0x8582
#define GL_SPECULAR                             0x1202
#define GL_SPHERE_MAP                           0x2402
#define GL_SPOT_CUTOFF                          0x1206
#define GL_SPOT_DIRECTION                       0x1204
#define GL_SPOT_EXPONENT                        0x1205
#define GL_SRC0_ALPHA                           0x8588
#define GL_SRC0_RGB                             0x8580
#define GL_SRC1_ALPHA                           0x8589
#define GL_SRC1_RGB                             0x8581
#define GL_SRC2_ALPHA                           0x858A
#define GL_SRC2_RGB                             0x8582
#define GL_SRC_ALPHA                            0x0302
#define GL_SRC_ALPHA_SATURATE                   0x0308
#define GL_SRC_COLOR                            0x0300
#define GL_SRGB                                 0x8C40
#define GL_SRGB8                                0x8C41
#define GL_SRGB8_ALPHA8                         0x8C43
#define GL_SRGB_ALPHA                           0x8C42
#define GL_STACK_OVERFLOW                       0x0503
#define GL_STACK_UNDERFLOW                      0x0504
#define GL_STATIC_COPY                          0x88E6
#define GL_STATIC_DRAW                          0x88E4
#define GL_STATIC_READ                          0x88E5
#define GL_STENCIL                              0x1802
#define GL_STENCIL_BACK_FAIL                    0x8801
#define GL_STENCIL_BACK_FUNC                    0x8800
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL         0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS         0x8803
#define GL_STENCIL_BACK_REF                     0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK              0x8CA4
#define GL_STENCIL_BACK_WRITEMASK               0x8CA5
#define GL_STENCIL_BITS                         0x0D57
#define GL_STENCIL_BUFFER_BIT                   0x00000400
#define GL_STENCIL_CLEAR_VALUE                  0x0B91
#define GL_STENCIL_FAIL                         0x0B94
#define GL_STENCIL_FUNC                         0x0B92
#define GL_STENCIL_INDEX                        0x1901
#define GL_STENCIL_PASS_DEPTH_FAIL              0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS              0x0B96
#define GL_STENCIL_REF                          0x0B97
#define GL_STENCIL_TEST                         0x0B90
#define GL_STENCIL_VALUE_MASK                   0x0B93
#define GL_STENCIL_WRITEMASK                    0x0B98
#define GL_STEREO                               0x0C33
#define GL_STREAM_COPY                          0x88E2
#define GL_STREAM_DRAW                          0x88E0
#define GL_STREAM_READ                          0x88E1
#define GL_SUBPIXEL_BITS                        0x0D50
#define GL_SUBTRACT                             0x84E7
#define GL_T                                    0x2001
#define GL_T2F_C3F_V3F                          0x2A2A
#define GL_T2F_C4F_N3F_V3F                      0x2A2C
#define GL_T2F_C4UB_V3F                         0x2A29
#define GL_T2F_N3F_V3F                          0x2A2B
#define GL_T2F_V3F                              0x2A27
#define GL_T4F_C4F_N3F_V4F                      0x2A2D
#define GL_T4F_V4F                              0x2A28
#define GL_TEXTURE                              0x1702
#define GL_TEXTURE0                             0x84C0
#define GL_TEXTURE1                             0x84C1
#define GL_TEXTURE10                            0x84CA
#define GL_TEXTURE11                            0x84CB
#define GL_TEXTURE12                            0x84CC
#define GL_TEXTURE13                            0x84CD
#define GL_TEXTURE14                            0x84CE
#define GL_TEXTURE15                            0x84CF
#define GL_TEXTURE16                            0x84D0
#define GL_TEXTURE17                            0x84D1
#define GL_TEXTURE18                            0x84D2
#define GL_TEXTURE19                            0x84D3
#define GL_TEXTURE2                             0x84C2
#define GL_TEXTURE20                            0x84D4
#define GL_TEXTURE21                            0x84D5
#define GL_TEXTURE22                            0x84D6
#define GL_TEXTURE23                            0x84D7
#define GL_TEXTURE24                            0x84D8
#define GL_TEXTURE25                            0x84D9
#define GL_TEXTURE26                            0x84DA
#define GL_TEXTURE27                            0x84DB
#define GL_TEXTURE28                            0x84DC
#define GL_TEXTURE29                            0x84DD
#define GL_TEXTURE3                             0x84C3
#define GL_TEXTURE30                            0x84DE
#define GL_TEXTURE31                            0x84DF
#define GL_TEXTURE4                             0x84C4
#define GL_TEXTURE5                             0x84C5
#define GL_TEXTURE6                             0x84C6
#define GL_TEXTURE7                             0x84C7
#define GL_TEXTURE8                             0x84C8
#define GL_TEXTURE9                             0x84C9
#define GL_TEXTURE_1D                           0x0DE0
#define GL_TEXTURE_2D                           0x0DE1
#define GL_TEXTURE_3D                           0x806F
#define GL_TEXTURE_ALPHA_SIZE                   0x805F
#define GL_TEXTURE_BASE_LEVEL                   0x813C
#define GL_TEXTURE_BINDING_1D                   0x8068
#define GL_TEXTURE_BINDING_2D                   0x8069
#define GL_TEXTURE_BINDING_3D                   0x806A
#define GL_TEXTURE_BINDING_CUBE_MAP             0x8514
#define GL_TEXTURE_BIT                          0x00040000
#define GL_TEXTURE_BLUE_SIZE                    0x805E
#define GL_TEXTURE_BORDER                       0x1005
#define GL_TEXTURE_BORDER_COLOR                 0x1004
#define GL_TEXTURE_COMPARE_FUNC                 0x884D
#define GL_TEXTURE_COMPARE_MODE                 0x884C
#define GL_TEXTURE_COMPONENTS                   0x1003
#define GL_TEXTURE_COMPRESSED                   0x86A1
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE        0x86A0
#define GL_TEXTURE_COMPRESSION_HINT             0x84EF
#define GL_TEXTURE_COORD_ARRAY                  0x8078
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING   0x889A
#define GL_TEXTURE_COORD_ARRAY_POINTER          0x8092
#define GL_TEXTURE_COORD_ARRAY_SIZE             0x8088
#define GL_TEXTURE_COORD_ARRAY_STRIDE           0x808A
#define GL_TEXTURE_COORD_ARRAY_TYPE             0x8089
#define GL_TEXTURE_CUBE_MAP                     0x8513
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X          0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y          0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z          0x851A
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X          0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y          0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z          0x8519
#define GL_TEXTURE_DEPTH                        0x8071
#define GL_TEXTURE_DEPTH_SIZE                   0x884A
#define GL_TEXTURE_ENV                          0x2300
#define GL_TEXTURE_ENV_COLOR                    0x2201
#define GL_TEXTURE_ENV_MODE                     0x2200
#define GL_TEXTURE_FILTER_CONTROL               0x8500
#define GL_TEXTURE_GEN_MODE                     0x2500
#define GL_TEXTURE_GEN_Q                        0x0C63
#define GL_TEXTURE_GEN_R                        0x0C62
#define GL_TEXTURE_GEN_S                        0x0C60
#define GL_TEXTURE_GEN_T                        0x0C61
#define GL_TEXTURE_GREEN_SIZE                   0x805D
#define GL_TEXTURE_HEIGHT                       0x1001
#define GL_TEXTURE_INTENSITY_SIZE               0x8061
#define GL_TEXTURE_INTERNAL_FORMAT              0x1003
#define GL_TEXTURE_LOD_BIAS                     0x8501
#define GL_TEXTURE_LUMINANCE_SIZE               0x8060
#define GL_TEXTURE_MAG_FILTER                   0x2800
#define GL_TEXTURE_MATRIX                       0x0BA8
#define GL_TEXTURE_MAX_LEVEL                    0x813D
#define GL_TEXTURE_MAX_LOD                      0x813B
#define GL_TEXTURE_MIN_FILTER                   0x2801
#define GL_TEXTURE_MIN_LOD                      0x813A
#define GL_TEXTURE_PRIORITY                     0x8066
#define GL_TEXTURE_RED_SIZE                     0x805C
#define GL_TEXTURE_RESIDENT                     0x8067
#define GL_TEXTURE_STACK_DEPTH                  0x0BA5
#define GL_TEXTURE_WIDTH                        0x1000
#define GL_TEXTURE_WRAP_R                       0x8072
#define GL_TEXTURE_WRAP_S                       0x2802
#define GL_TEXTURE_WRAP_T                       0x2803
#define GL_TRANSFORM_BIT                        0x00001000
#define GL_TRANSPOSE_COLOR_MATRIX               0x84E6
#define GL_TRANSPOSE_MODELVIEW_MATRIX           0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX          0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX             0x84E5
#define GL_TRIANGLES                            0x0004
#define GL_TRIANGLE_FAN                         0x0006
#define GL_TRIANGLE_STRIP                       0x0005
#define GL_TRUE                                 1
#define GL_UNPACK_ALIGNMENT                     0x0CF5
#define GL_UNPACK_IMAGE_HEIGHT                  0x806E
#define GL_UNPACK_LSB_FIRST                     0x0CF1
#define GL_UNPACK_ROW_LENGTH                    0x0CF2
#define GL_UNPACK_SKIP_IMAGES                   0x806D
#define GL_UNPACK_SKIP_PIXELS                   0x0CF4
#define GL_UNPACK_SKIP_ROWS                     0x0CF3
#define GL_UNPACK_SWAP_BYTES                    0x0CF0
#define GL_UNSIGNED_BYTE                        0x1401
#define GL_UNSIGNED_BYTE_2_3_3_REV              0x8362
#define GL_UNSIGNED_BYTE_3_3_2                  0x8032
#define GL_UNSIGNED_INT                         0x1405
#define GL_UNSIGNED_INT_10_10_10_2              0x8036
#define GL_UNSIGNED_INT_2_10_10_10_REV          0x8368
#define GL_UNSIGNED_INT_8_8_8_8                 0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV             0x8367
#define GL_UNSIGNED_SHORT                       0x1403
#define GL_UNSIGNED_SHORT_1_5_5_5_REV           0x8366
#define GL_UNSIGNED_SHORT_4_4_4_4               0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV           0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1               0x8034
#define GL_UNSIGNED_SHORT_5_6_5                 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV             0x8364
#define GL_UPPER_LEFT                           0x8CA2
#define GL_V2F                                  0x2A20
#define GL_V3F                                  0x2A21
#define GL_VALIDATE_STATUS                      0x8B83
#define GL_VENDOR                               0x1F00
#define GL_VERSION                              0x1F02
#define GL_VERTEX_ARRAY                         0x8074
#define GL_VERTEX_ARRAY_BUFFER_BINDING          0x8896
#define GL_VERTEX_ARRAY_POINTER                 0x808E
#define GL_VERTEX_ARRAY_SIZE                    0x807A
#define GL_VERTEX_ARRAY_STRIDE                  0x807C
#define GL_VERTEX_ARRAY_TYPE                    0x807B
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING   0x889F
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED          0x8622
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED       0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER          0x8645
#define GL_VERTEX_ATTRIB_ARRAY_SIZE             0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE           0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE             0x8625
#define GL_VERTEX_PROGRAM_POINT_SIZE            0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE              0x8643
#define GL_VERTEX_SHADER                        0x8B31
#define GL_VIEWPORT                             0x0BA2
#define GL_VIEWPORT_BIT                         0x00000800
#define GL_WEIGHT_ARRAY_BUFFER_BINDING          0x889E
#define GL_WRITE_ONLY                           0x88B9
#define GL_XOR                                  0x1506
#define GL_ZERO                                 0
#define GL_ZOOM_X                               0x0D16
#define GL_ZOOM_Y                               0x0D17
