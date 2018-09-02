#pragma once

#include <string>

#include <GLFL/glfl.h>

#include "program/errors.h"

namespace Graphics
{
    inline void CheckErrors()
    {
        GLenum err = glGetError();
        if (err)
        {
            std::string msg;
            do
            {
                if (msg.size())
                    msg += ", ";

                switch (err)
                {
                    case GL_INVALID_ENUM:                  msg += "Invalid enum";                  break;
                    case GL_INVALID_VALUE:                 msg += "Invalid value";                 break;
                    case GL_INVALID_OPERATION:             msg += "Invalid operation";             break;
                    case GL_INVALID_FRAMEBUFFER_OPERATION: msg += "Invalid framebuffer operation"; break;
                    case GL_OUT_OF_MEMORY:                 msg += "Out of memory";                 break;
                    #ifdef GL_STACK_UNDERFLOW
                    case GL_STACK_UNDERFLOW:               msg += "Stack underflow";               break;
                    #endif
                    #ifdef GL_STACK_OVERFLOW
                    case GL_STACK_OVERFLOW:                msg += "Stack overflow";                break;
                    #endif
                    default:                               msg += "Unknown error";                 break;
                }
            }
            while ((err = glGetError()));
            Program::Error("OpenGL error: ", msg, ".");
        }
    }
}
