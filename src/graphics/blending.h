#pragma once

#include <GLFL/glfl.h>

namespace Graphics::Blending
{
    enum Factors
    {
        zero                 = GL_ZERO,
        one                  = GL_ONE,
        src                  = GL_SRC_COLOR,
        one_minus_src        = GL_ONE_MINUS_SRC_COLOR,
        dst                  = GL_DST_COLOR,
        one_minus_dst        = GL_ONE_MINUS_DST_COLOR,
        src_a                = GL_SRC_ALPHA,
        one_minus_src_a      = GL_ONE_MINUS_SRC_ALPHA,
        dst_a                = GL_DST_ALPHA,
        one_minus_dst_a      = GL_ONE_MINUS_DST_ALPHA,
        constant             = GL_CONSTANT_COLOR,
        one_minus_constant   = GL_ONE_MINUS_CONSTANT_COLOR,
        constant_a           = GL_CONSTANT_ALPHA,
        one_minus_constant_a = GL_ONE_MINUS_CONSTANT_ALPHA,
        src_a_saturate       = GL_SRC_ALPHA_SATURATE,
        OnPlatform(PC)
        (
            src1             = GL_SRC1_COLOR,
            one_minus_src1   = GL_ONE_MINUS_SRC1_COLOR,
            src1_a           = GL_SRC1_ALPHA,
            one_minus_src1_a = GL_ONE_MINUS_SRC1_ALPHA,
        )
    };
    enum Equations
    {
        eq_add              = GL_FUNC_ADD,
        eq_subtract         = GL_FUNC_SUBTRACT,
        eq_reverse_subtract = GL_FUNC_REVERSE_SUBTRACT,
        OnPlatform(PC)
        (
            eq_min          = GL_MIN,
            eq_max          = GL_MAX,
        )
    };

    // Func(a,b) and Equation(a) set same parameters for both color and alpha.
    // Func(a,b,c,d) and Equation(a,b) set separete parameters for color and alpha.
    inline void Enable()  {glEnable(GL_BLEND);}
    inline void Disable() {glDisable(GL_BLEND);}
    inline void Func(Factors src, Factors dst)                             {glBlendFunc(src, dst);}
    inline void Func(Factors src, Factors dst, Factors srca, Factors dsta) {glBlendFuncSeparate(src, dst, srca, dsta);}
    inline void Equation(Equations eq)                {glBlendEquation(eq);}
    inline void Equation(Equations eq, Equations eqa) {glBlendEquationSeparate(eq, eqa);}

    inline void FuncOverwrite        () {Func(one, zero);}
    inline void FuncAdd              () {Func(one, one);}
    inline void FuncNormalSimple     () {Func(src_a, one_minus_src_a);} // Resulting alpha values are incorrect and should be ignored.
    inline void FuncNormalSimpleToPre() {Func(src_a, one_minus_src_a, one, one_minus_src_a);} // Destination and output are premultiplied, source is not.
    inline void FuncNormalPre        () {Func(one, one_minus_src_a);} // Source, destination and output are premultiplited.
}
