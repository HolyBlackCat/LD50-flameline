#pragma once

// This file is a part of CGLFL (configurable OpenGL function loader).
// Generated, do no edit!
//
// Version: 1.0.0
// API: gl 3.2 (core profile)
// Extensions: none

#include <cstddef>
#include <cstdint>

#ifndef CGLFL_API
#  if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#    define CGLFL_API __stdcall
#  else
#    define CGLFL_API
#  endif
#endif

typedef std::uint32_t GLenum;
typedef std::uint8_t GLboolean;
typedef std::uint32_t GLbitfield;
typedef void GLvoid;
typedef std::int8_t GLbyte;
typedef std::uint8_t GLubyte;
typedef std::int16_t GLshort;
typedef std::uint16_t GLushort;
typedef std::int32_t GLint;
typedef std::uint32_t GLuint;
typedef std::int32_t GLclampx;
typedef std::int32_t GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
#ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef std::uint32_t GLhandleARB;
#endif
typedef std::uint16_t GLhalf;
typedef std::uint16_t GLhalfARB;
typedef std::int32_t GLfixed;
typedef std::intptr_t GLintptr;
typedef std::intptr_t GLintptrARB;
typedef std::ptrdiff_t GLsizeiptr;
typedef std::ptrdiff_t GLsizeiptrARB;
typedef std::int64_t GLint64;
typedef std::int64_t GLint64EXT;
typedef std::uint64_t GLuint64;
typedef std::uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;
struct _cl_context;
struct _cl_event;
typedef void (CGLFL_API *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (CGLFL_API *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (CGLFL_API *GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (CGLFL_API *GLDEBUGPROCAMD)(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, void *userParam);
typedef std::uint16_t GLhalfNV;
typedef GLintptr GLvdpauSurfaceNV;
typedef void (CGLFL_API *GLVULKANPROCNV)(void);
