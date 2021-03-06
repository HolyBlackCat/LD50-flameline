// mat.h
// Vector and matrix math
// Version 3.3.3
// Generated, don't touch.

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

// Vectors and matrices

namespace Math
{
    inline namespace Utility // Scalar concepts
    {
        // Check if a type is a scalar type.
        template <typename T> struct impl_is_scalar : std::is_arithmetic<T> {}; // Not `std::is_scalar`, because that includes pointers.
        template <typename T> concept scalar = impl_is_scalar<T>::value;
        template <typename T> concept cv_unqualified_scalar = scalar<T> && std::is_same_v<T, std::remove_cv_t<T>>;
    }

    inline namespace Vector // Declarations
    {
        template <int D, cv_unqualified_scalar T> struct vec;
        template <int W, int H, cv_unqualified_scalar T> struct mat;
    }

    inline namespace Alias // Short type aliases
    {
        template <typename T> using vec2 = vec<2,T>; template <typename T> using vec3 = vec<3,T>; template <typename T> using vec4 = vec<4,T>;
        template <typename T> using mat2x2 = mat<2,2,T>; template <typename T> using mat3x2 = mat<3,2,T>; template <typename T> using mat4x2 = mat<4,2,T>;
        template <typename T> using mat2x3 = mat<2,3,T>; template <typename T> using mat3x3 = mat<3,3,T>; template <typename T> using mat4x3 = mat<4,3,T>;
        template <typename T> using mat2x4 = mat<2,4,T>; template <typename T> using mat3x4 = mat<3,4,T>; template <typename T> using mat4x4 = mat<4,4,T>;
        template <typename T> using mat2 = mat2x2<T>; template <typename T> using mat3 = mat3x3<T>; template <typename T> using mat4 = mat4x4<T>;

        template <int D> using bvec = vec<D,bool>;
        template <int W, int H> using bmat = mat<W,H,bool>;
        using bvec2 = vec<2,bool>; using bvec3 = vec<3,bool>; using bvec4 = vec<4,bool>;
        using bmat2x2 = mat<2,2,bool>; using bmat3x2 = mat<3,2,bool>; using bmat4x2 = mat<4,2,bool>;
        using bmat2x3 = mat<2,3,bool>; using bmat3x3 = mat<3,3,bool>; using bmat4x3 = mat<4,3,bool>;
        using bmat2x4 = mat<2,4,bool>; using bmat3x4 = mat<3,4,bool>; using bmat4x4 = mat<4,4,bool>;
        using bmat2 = bmat2x2; using bmat3 = bmat3x3; using bmat4 = bmat4x4;

        template <int D> using cvec = vec<D,char>;
        template <int W, int H> using cmat = mat<W,H,char>;
        using cvec2 = vec<2,char>; using cvec3 = vec<3,char>; using cvec4 = vec<4,char>;
        using cmat2x2 = mat<2,2,char>; using cmat3x2 = mat<3,2,char>; using cmat4x2 = mat<4,2,char>;
        using cmat2x3 = mat<2,3,char>; using cmat3x3 = mat<3,3,char>; using cmat4x3 = mat<4,3,char>;
        using cmat2x4 = mat<2,4,char>; using cmat3x4 = mat<3,4,char>; using cmat4x4 = mat<4,4,char>;
        using cmat2 = cmat2x2; using cmat3 = cmat3x3; using cmat4 = cmat4x4;

        template <int D> using ucvec = vec<D,unsigned char>;
        template <int W, int H> using ucmat = mat<W,H,unsigned char>;
        using ucvec2 = vec<2,unsigned char>; using ucvec3 = vec<3,unsigned char>; using ucvec4 = vec<4,unsigned char>;
        using ucmat2x2 = mat<2,2,unsigned char>; using ucmat3x2 = mat<3,2,unsigned char>; using ucmat4x2 = mat<4,2,unsigned char>;
        using ucmat2x3 = mat<2,3,unsigned char>; using ucmat3x3 = mat<3,3,unsigned char>; using ucmat4x3 = mat<4,3,unsigned char>;
        using ucmat2x4 = mat<2,4,unsigned char>; using ucmat3x4 = mat<3,4,unsigned char>; using ucmat4x4 = mat<4,4,unsigned char>;
        using ucmat2 = ucmat2x2; using ucmat3 = ucmat3x3; using ucmat4 = ucmat4x4;

        template <int D> using scvec = vec<D,signed char>;
        template <int W, int H> using scmat = mat<W,H,signed char>;
        using scvec2 = vec<2,signed char>; using scvec3 = vec<3,signed char>; using scvec4 = vec<4,signed char>;
        using scmat2x2 = mat<2,2,signed char>; using scmat3x2 = mat<3,2,signed char>; using scmat4x2 = mat<4,2,signed char>;
        using scmat2x3 = mat<2,3,signed char>; using scmat3x3 = mat<3,3,signed char>; using scmat4x3 = mat<4,3,signed char>;
        using scmat2x4 = mat<2,4,signed char>; using scmat3x4 = mat<3,4,signed char>; using scmat4x4 = mat<4,4,signed char>;
        using scmat2 = scmat2x2; using scmat3 = scmat3x3; using scmat4 = scmat4x4;

        template <int D> using svec = vec<D,short>;
        template <int W, int H> using smat = mat<W,H,short>;
        using svec2 = vec<2,short>; using svec3 = vec<3,short>; using svec4 = vec<4,short>;
        using smat2x2 = mat<2,2,short>; using smat3x2 = mat<3,2,short>; using smat4x2 = mat<4,2,short>;
        using smat2x3 = mat<2,3,short>; using smat3x3 = mat<3,3,short>; using smat4x3 = mat<4,3,short>;
        using smat2x4 = mat<2,4,short>; using smat3x4 = mat<3,4,short>; using smat4x4 = mat<4,4,short>;
        using smat2 = smat2x2; using smat3 = smat3x3; using smat4 = smat4x4;

        template <int D> using usvec = vec<D,unsigned short>;
        template <int W, int H> using usmat = mat<W,H,unsigned short>;
        using usvec2 = vec<2,unsigned short>; using usvec3 = vec<3,unsigned short>; using usvec4 = vec<4,unsigned short>;
        using usmat2x2 = mat<2,2,unsigned short>; using usmat3x2 = mat<3,2,unsigned short>; using usmat4x2 = mat<4,2,unsigned short>;
        using usmat2x3 = mat<2,3,unsigned short>; using usmat3x3 = mat<3,3,unsigned short>; using usmat4x3 = mat<4,3,unsigned short>;
        using usmat2x4 = mat<2,4,unsigned short>; using usmat3x4 = mat<3,4,unsigned short>; using usmat4x4 = mat<4,4,unsigned short>;
        using usmat2 = usmat2x2; using usmat3 = usmat3x3; using usmat4 = usmat4x4;

        template <int D> using ivec = vec<D,int>;
        template <int W, int H> using imat = mat<W,H,int>;
        using ivec2 = vec<2,int>; using ivec3 = vec<3,int>; using ivec4 = vec<4,int>;
        using imat2x2 = mat<2,2,int>; using imat3x2 = mat<3,2,int>; using imat4x2 = mat<4,2,int>;
        using imat2x3 = mat<2,3,int>; using imat3x3 = mat<3,3,int>; using imat4x3 = mat<4,3,int>;
        using imat2x4 = mat<2,4,int>; using imat3x4 = mat<3,4,int>; using imat4x4 = mat<4,4,int>;
        using imat2 = imat2x2; using imat3 = imat3x3; using imat4 = imat4x4;

        template <int D> using uvec = vec<D,unsigned int>;
        template <int W, int H> using umat = mat<W,H,unsigned int>;
        using uvec2 = vec<2,unsigned int>; using uvec3 = vec<3,unsigned int>; using uvec4 = vec<4,unsigned int>;
        using umat2x2 = mat<2,2,unsigned int>; using umat3x2 = mat<3,2,unsigned int>; using umat4x2 = mat<4,2,unsigned int>;
        using umat2x3 = mat<2,3,unsigned int>; using umat3x3 = mat<3,3,unsigned int>; using umat4x3 = mat<4,3,unsigned int>;
        using umat2x4 = mat<2,4,unsigned int>; using umat3x4 = mat<3,4,unsigned int>; using umat4x4 = mat<4,4,unsigned int>;
        using umat2 = umat2x2; using umat3 = umat3x3; using umat4 = umat4x4;

        template <int D> using lvec = vec<D,long>;
        template <int W, int H> using lmat = mat<W,H,long>;
        using lvec2 = vec<2,long>; using lvec3 = vec<3,long>; using lvec4 = vec<4,long>;
        using lmat2x2 = mat<2,2,long>; using lmat3x2 = mat<3,2,long>; using lmat4x2 = mat<4,2,long>;
        using lmat2x3 = mat<2,3,long>; using lmat3x3 = mat<3,3,long>; using lmat4x3 = mat<4,3,long>;
        using lmat2x4 = mat<2,4,long>; using lmat3x4 = mat<3,4,long>; using lmat4x4 = mat<4,4,long>;
        using lmat2 = lmat2x2; using lmat3 = lmat3x3; using lmat4 = lmat4x4;

        template <int D> using ulvec = vec<D,unsigned long>;
        template <int W, int H> using ulmat = mat<W,H,unsigned long>;
        using ulvec2 = vec<2,unsigned long>; using ulvec3 = vec<3,unsigned long>; using ulvec4 = vec<4,unsigned long>;
        using ulmat2x2 = mat<2,2,unsigned long>; using ulmat3x2 = mat<3,2,unsigned long>; using ulmat4x2 = mat<4,2,unsigned long>;
        using ulmat2x3 = mat<2,3,unsigned long>; using ulmat3x3 = mat<3,3,unsigned long>; using ulmat4x3 = mat<4,3,unsigned long>;
        using ulmat2x4 = mat<2,4,unsigned long>; using ulmat3x4 = mat<3,4,unsigned long>; using ulmat4x4 = mat<4,4,unsigned long>;
        using ulmat2 = ulmat2x2; using ulmat3 = ulmat3x3; using ulmat4 = ulmat4x4;

        template <int D> using llvec = vec<D,long long>;
        template <int W, int H> using llmat = mat<W,H,long long>;
        using llvec2 = vec<2,long long>; using llvec3 = vec<3,long long>; using llvec4 = vec<4,long long>;
        using llmat2x2 = mat<2,2,long long>; using llmat3x2 = mat<3,2,long long>; using llmat4x2 = mat<4,2,long long>;
        using llmat2x3 = mat<2,3,long long>; using llmat3x3 = mat<3,3,long long>; using llmat4x3 = mat<4,3,long long>;
        using llmat2x4 = mat<2,4,long long>; using llmat3x4 = mat<3,4,long long>; using llmat4x4 = mat<4,4,long long>;
        using llmat2 = llmat2x2; using llmat3 = llmat3x3; using llmat4 = llmat4x4;

        template <int D> using ullvec = vec<D,unsigned long long>;
        template <int W, int H> using ullmat = mat<W,H,unsigned long long>;
        using ullvec2 = vec<2,unsigned long long>; using ullvec3 = vec<3,unsigned long long>; using ullvec4 = vec<4,unsigned long long>;
        using ullmat2x2 = mat<2,2,unsigned long long>; using ullmat3x2 = mat<3,2,unsigned long long>; using ullmat4x2 = mat<4,2,unsigned long long>;
        using ullmat2x3 = mat<2,3,unsigned long long>; using ullmat3x3 = mat<3,3,unsigned long long>; using ullmat4x3 = mat<4,3,unsigned long long>;
        using ullmat2x4 = mat<2,4,unsigned long long>; using ullmat3x4 = mat<3,4,unsigned long long>; using ullmat4x4 = mat<4,4,unsigned long long>;
        using ullmat2 = ullmat2x2; using ullmat3 = ullmat3x3; using ullmat4 = ullmat4x4;

        template <int D> using fvec = vec<D,float>;
        template <int W, int H> using fmat = mat<W,H,float>;
        using fvec2 = vec<2,float>; using fvec3 = vec<3,float>; using fvec4 = vec<4,float>;
        using fmat2x2 = mat<2,2,float>; using fmat3x2 = mat<3,2,float>; using fmat4x2 = mat<4,2,float>;
        using fmat2x3 = mat<2,3,float>; using fmat3x3 = mat<3,3,float>; using fmat4x3 = mat<4,3,float>;
        using fmat2x4 = mat<2,4,float>; using fmat3x4 = mat<3,4,float>; using fmat4x4 = mat<4,4,float>;
        using fmat2 = fmat2x2; using fmat3 = fmat3x3; using fmat4 = fmat4x4;

        template <int D> using dvec = vec<D,double>;
        template <int W, int H> using dmat = mat<W,H,double>;
        using dvec2 = vec<2,double>; using dvec3 = vec<3,double>; using dvec4 = vec<4,double>;
        using dmat2x2 = mat<2,2,double>; using dmat3x2 = mat<3,2,double>; using dmat4x2 = mat<4,2,double>;
        using dmat2x3 = mat<2,3,double>; using dmat3x3 = mat<3,3,double>; using dmat4x3 = mat<4,3,double>;
        using dmat2x4 = mat<2,4,double>; using dmat3x4 = mat<3,4,double>; using dmat4x4 = mat<4,4,double>;
        using dmat2 = dmat2x2; using dmat3 = dmat3x3; using dmat4 = dmat4x4;

        template <int D> using ldvec = vec<D,long double>;
        template <int W, int H> using ldmat = mat<W,H,long double>;
        using ldvec2 = vec<2,long double>; using ldvec3 = vec<3,long double>; using ldvec4 = vec<4,long double>;
        using ldmat2x2 = mat<2,2,long double>; using ldmat3x2 = mat<3,2,long double>; using ldmat4x2 = mat<4,2,long double>;
        using ldmat2x3 = mat<2,3,long double>; using ldmat3x3 = mat<3,3,long double>; using ldmat4x3 = mat<4,3,long double>;
        using ldmat2x4 = mat<2,4,long double>; using ldmat3x4 = mat<3,4,long double>; using ldmat4x4 = mat<4,4,long double>;
        using ldmat2 = ldmat2x2; using ldmat3 = ldmat3x3; using ldmat4 = ldmat4x4;

        template <int D> using i8vec = vec<D,std::int8_t>;
        template <int W, int H> using i8mat = mat<W,H,std::int8_t>;
        using i8vec2 = vec<2,std::int8_t>; using i8vec3 = vec<3,std::int8_t>; using i8vec4 = vec<4,std::int8_t>;
        using i8mat2x2 = mat<2,2,std::int8_t>; using i8mat3x2 = mat<3,2,std::int8_t>; using i8mat4x2 = mat<4,2,std::int8_t>;
        using i8mat2x3 = mat<2,3,std::int8_t>; using i8mat3x3 = mat<3,3,std::int8_t>; using i8mat4x3 = mat<4,3,std::int8_t>;
        using i8mat2x4 = mat<2,4,std::int8_t>; using i8mat3x4 = mat<3,4,std::int8_t>; using i8mat4x4 = mat<4,4,std::int8_t>;
        using i8mat2 = i8mat2x2; using i8mat3 = i8mat3x3; using i8mat4 = i8mat4x4;

        template <int D> using u8vec = vec<D,std::uint8_t>;
        template <int W, int H> using u8mat = mat<W,H,std::uint8_t>;
        using u8vec2 = vec<2,std::uint8_t>; using u8vec3 = vec<3,std::uint8_t>; using u8vec4 = vec<4,std::uint8_t>;
        using u8mat2x2 = mat<2,2,std::uint8_t>; using u8mat3x2 = mat<3,2,std::uint8_t>; using u8mat4x2 = mat<4,2,std::uint8_t>;
        using u8mat2x3 = mat<2,3,std::uint8_t>; using u8mat3x3 = mat<3,3,std::uint8_t>; using u8mat4x3 = mat<4,3,std::uint8_t>;
        using u8mat2x4 = mat<2,4,std::uint8_t>; using u8mat3x4 = mat<3,4,std::uint8_t>; using u8mat4x4 = mat<4,4,std::uint8_t>;
        using u8mat2 = u8mat2x2; using u8mat3 = u8mat3x3; using u8mat4 = u8mat4x4;

        template <int D> using i16vec = vec<D,std::int16_t>;
        template <int W, int H> using i16mat = mat<W,H,std::int16_t>;
        using i16vec2 = vec<2,std::int16_t>; using i16vec3 = vec<3,std::int16_t>; using i16vec4 = vec<4,std::int16_t>;
        using i16mat2x2 = mat<2,2,std::int16_t>; using i16mat3x2 = mat<3,2,std::int16_t>; using i16mat4x2 = mat<4,2,std::int16_t>;
        using i16mat2x3 = mat<2,3,std::int16_t>; using i16mat3x3 = mat<3,3,std::int16_t>; using i16mat4x3 = mat<4,3,std::int16_t>;
        using i16mat2x4 = mat<2,4,std::int16_t>; using i16mat3x4 = mat<3,4,std::int16_t>; using i16mat4x4 = mat<4,4,std::int16_t>;
        using i16mat2 = i16mat2x2; using i16mat3 = i16mat3x3; using i16mat4 = i16mat4x4;

        template <int D> using u16vec = vec<D,std::uint16_t>;
        template <int W, int H> using u16mat = mat<W,H,std::uint16_t>;
        using u16vec2 = vec<2,std::uint16_t>; using u16vec3 = vec<3,std::uint16_t>; using u16vec4 = vec<4,std::uint16_t>;
        using u16mat2x2 = mat<2,2,std::uint16_t>; using u16mat3x2 = mat<3,2,std::uint16_t>; using u16mat4x2 = mat<4,2,std::uint16_t>;
        using u16mat2x3 = mat<2,3,std::uint16_t>; using u16mat3x3 = mat<3,3,std::uint16_t>; using u16mat4x3 = mat<4,3,std::uint16_t>;
        using u16mat2x4 = mat<2,4,std::uint16_t>; using u16mat3x4 = mat<3,4,std::uint16_t>; using u16mat4x4 = mat<4,4,std::uint16_t>;
        using u16mat2 = u16mat2x2; using u16mat3 = u16mat3x3; using u16mat4 = u16mat4x4;

        template <int D> using i32vec = vec<D,std::int32_t>;
        template <int W, int H> using i32mat = mat<W,H,std::int32_t>;
        using i32vec2 = vec<2,std::int32_t>; using i32vec3 = vec<3,std::int32_t>; using i32vec4 = vec<4,std::int32_t>;
        using i32mat2x2 = mat<2,2,std::int32_t>; using i32mat3x2 = mat<3,2,std::int32_t>; using i32mat4x2 = mat<4,2,std::int32_t>;
        using i32mat2x3 = mat<2,3,std::int32_t>; using i32mat3x3 = mat<3,3,std::int32_t>; using i32mat4x3 = mat<4,3,std::int32_t>;
        using i32mat2x4 = mat<2,4,std::int32_t>; using i32mat3x4 = mat<3,4,std::int32_t>; using i32mat4x4 = mat<4,4,std::int32_t>;
        using i32mat2 = i32mat2x2; using i32mat3 = i32mat3x3; using i32mat4 = i32mat4x4;

        template <int D> using u32vec = vec<D,std::uint32_t>;
        template <int W, int H> using u32mat = mat<W,H,std::uint32_t>;
        using u32vec2 = vec<2,std::uint32_t>; using u32vec3 = vec<3,std::uint32_t>; using u32vec4 = vec<4,std::uint32_t>;
        using u32mat2x2 = mat<2,2,std::uint32_t>; using u32mat3x2 = mat<3,2,std::uint32_t>; using u32mat4x2 = mat<4,2,std::uint32_t>;
        using u32mat2x3 = mat<2,3,std::uint32_t>; using u32mat3x3 = mat<3,3,std::uint32_t>; using u32mat4x3 = mat<4,3,std::uint32_t>;
        using u32mat2x4 = mat<2,4,std::uint32_t>; using u32mat3x4 = mat<3,4,std::uint32_t>; using u32mat4x4 = mat<4,4,std::uint32_t>;
        using u32mat2 = u32mat2x2; using u32mat3 = u32mat3x3; using u32mat4 = u32mat4x4;

        template <int D> using i64vec = vec<D,std::int64_t>;
        template <int W, int H> using i64mat = mat<W,H,std::int64_t>;
        using i64vec2 = vec<2,std::int64_t>; using i64vec3 = vec<3,std::int64_t>; using i64vec4 = vec<4,std::int64_t>;
        using i64mat2x2 = mat<2,2,std::int64_t>; using i64mat3x2 = mat<3,2,std::int64_t>; using i64mat4x2 = mat<4,2,std::int64_t>;
        using i64mat2x3 = mat<2,3,std::int64_t>; using i64mat3x3 = mat<3,3,std::int64_t>; using i64mat4x3 = mat<4,3,std::int64_t>;
        using i64mat2x4 = mat<2,4,std::int64_t>; using i64mat3x4 = mat<3,4,std::int64_t>; using i64mat4x4 = mat<4,4,std::int64_t>;
        using i64mat2 = i64mat2x2; using i64mat3 = i64mat3x3; using i64mat4 = i64mat4x4;

        template <int D> using u64vec = vec<D,std::uint64_t>;
        template <int W, int H> using u64mat = mat<W,H,std::uint64_t>;
        using u64vec2 = vec<2,std::uint64_t>; using u64vec3 = vec<3,std::uint64_t>; using u64vec4 = vec<4,std::uint64_t>;
        using u64mat2x2 = mat<2,2,std::uint64_t>; using u64mat3x2 = mat<3,2,std::uint64_t>; using u64mat4x2 = mat<4,2,std::uint64_t>;
        using u64mat2x3 = mat<2,3,std::uint64_t>; using u64mat3x3 = mat<3,3,std::uint64_t>; using u64mat4x3 = mat<4,3,std::uint64_t>;
        using u64mat2x4 = mat<2,4,std::uint64_t>; using u64mat3x4 = mat<3,4,std::uint64_t>; using u64mat4x4 = mat<4,4,std::uint64_t>;
        using u64mat2 = u64mat2x2; using u64mat3 = u64mat3x3; using u64mat4 = u64mat4x4;

        template <int D> using index_vec = vec<D,std::ptrdiff_t>;
        template <int W, int H> using index_mat = mat<W,H,std::ptrdiff_t>;
        using index_vec2 = vec<2,std::ptrdiff_t>; using index_vec3 = vec<3,std::ptrdiff_t>; using index_vec4 = vec<4,std::ptrdiff_t>;
        using index_mat2x2 = mat<2,2,std::ptrdiff_t>; using index_mat3x2 = mat<3,2,std::ptrdiff_t>; using index_mat4x2 = mat<4,2,std::ptrdiff_t>;
        using index_mat2x3 = mat<2,3,std::ptrdiff_t>; using index_mat3x3 = mat<3,3,std::ptrdiff_t>; using index_mat4x3 = mat<4,3,std::ptrdiff_t>;
        using index_mat2x4 = mat<2,4,std::ptrdiff_t>; using index_mat3x4 = mat<3,4,std::ptrdiff_t>; using index_mat4x4 = mat<4,4,std::ptrdiff_t>;
        using index_mat2 = index_mat2x2; using index_mat3 = index_mat3x3; using index_mat4 = index_mat4x4;

        template <int D> using size_vec = vec<D,std::size_t>;
        template <int W, int H> using size_mat = mat<W,H,std::size_t>;
        using size_vec2 = vec<2,std::size_t>; using size_vec3 = vec<3,std::size_t>; using size_vec4 = vec<4,std::size_t>;
        using size_mat2x2 = mat<2,2,std::size_t>; using size_mat3x2 = mat<3,2,std::size_t>; using size_mat4x2 = mat<4,2,std::size_t>;
        using size_mat2x3 = mat<2,3,std::size_t>; using size_mat3x3 = mat<3,3,std::size_t>; using size_mat4x3 = mat<4,3,std::size_t>;
        using size_mat2x4 = mat<2,4,std::size_t>; using size_mat3x4 = mat<3,4,std::size_t>; using size_mat4x4 = mat<4,4,std::size_t>;
        using size_mat2 = size_mat2x2; using size_mat3 = size_mat3x3; using size_mat4 = size_mat4x4;
    }

    namespace Custom // Customization points.
    {
        // Specializing this adds corresponding constructors and conversion operators to vectors and matrices.
        // The template arguments will never be const.
        template <typename From, typename To>
        struct Convert
        {
            // To operator()(const From &) const {...}
        };

        template <typename From, typename To>
        concept convertible = requires(const Convert<From, To> conv, const From from)
        {
            { conv(from) } -> std::same_as<To>;
        };
    }

    inline namespace Utility // Helper templates
    {
        // Check if `T` is a vector type (possibly const).
        template <typename T> struct impl_is_vector : std::false_type {};
        template <int D, typename T> struct impl_is_vector<      vec<D,T>> : std::true_type {};
        template <int D, typename T> struct impl_is_vector<const vec<D,T>> : std::true_type {};
        template <typename T> concept vector = impl_is_vector<T>::value;

        template <typename T> inline constexpr bool vector_or_scalar = scalar<T> || vector<T>;

        // Checks if none of `P...` are vector types.
        template <typename ...P> inline constexpr bool no_vectors_v = !(vector<P> || ...);

        // Check if `T` is a matrix type (possibly const).
        template <typename T> struct impl_is_matrix : std::false_type {};
        template <int W, int H, typename T> struct impl_is_matrix<      mat<W,H,T>> : std::true_type {};
        template <int W, int H, typename T> struct impl_is_matrix<const mat<W,H,T>> : std::true_type {};
        template <typename T> concept matrix = impl_is_matrix<T>::value;

        // If `T` is a vector (possibly const), returns its element type, with the same cv-qualifier. Otherwise returns `T`.
        template <typename T> struct impl_vec_base {using type = T;};
        template <int D, typename T> struct impl_vec_base<      vec<D,T>> {using type =       T;};
        template <int D, typename T> struct impl_vec_base<const vec<D,T>> {using type = const T;};
        template <typename T> using vec_base_t = typename impl_vec_base<T>::type;

        // If `T` is a vector (possibly const), returns its size. Otherwise returns 1.
        template <typename T> struct impl_vec_size : std::integral_constant<int, 1> {};
        template <int D, typename T> struct impl_vec_size<      vec<D,T>> : std::integral_constant<int, D> {};
        template <int D, typename T> struct impl_vec_size<const vec<D,T>> : std::integral_constant<int, D> {};
        template <typename T> inline constexpr int vec_size_v = impl_vec_size<T>::value;

        // If `D == 1` returns `T`, otherwise returns `vec<D,T>`.
        template <int D, typename T> struct impl_ver_or_scalar {using type = vec<D,T>;};
        template <typename T> struct impl_ver_or_scalar<1,T> {using type = T;};
        template <int D, typename T> using vec_or_scalar_t = typename impl_ver_or_scalar<D,T>::type;

        // If the set {D...} is either {N} or {1,N}, returns `N`.
        // If the set {D...} is empty, returns `1`.
        // Otherwise returns 0.
        template <int ...D> inline constexpr int common_vec_size_or_zero_v = []{
            int ret = 1;
            bool ok = ((D == 1 ? true : ret == 1 || ret == D ? (void(ret = D), true) : false) && ...);
            return ok * ret;
        }();

        template <int ...D> concept have_common_vec_size = common_vec_size_or_zero_v<D...> != 0;

        // If the set {D...} is either {N} or {1,N}, returns `N`.
        // If the set {D...} is empty, returns `1`.
        // Otherwise causes a soft error.
        template <int ...D> requires have_common_vec_size<D...>
        inline constexpr int common_vec_size_v = common_vec_size_or_zero_v<D...>;

        // If `A` is a `[const] vec<D,T>`, returns `[const] vec<D,B>`. Otherwise returns `B`.
        template <typename A, typename B> struct impl_change_vec_base {using type = B;};
        template <int D, typename A, typename B> struct impl_change_vec_base<      vec<D,A>,B> {using type =       vec<D,B>;};
        template <int D, typename A, typename B> struct impl_change_vec_base<const vec<D,A>,B> {using type = const vec<D,B>;};
        template <typename A, typename B> using change_vec_base_t = typename impl_change_vec_base<A,B>::type;

        // Returns a reasonable 'floating-point counterpart' for a type.
        // Currently if the type is not floating-point, returns `double`. Otherwise returns the same type.
        // If `T` is a vector (possibly const), it's base type is changed according to the same rules.
        template <typename T> using floating_point_t = std::conditional_t<std::is_floating_point_v<vec_base_t<T>>, T, change_vec_base_t<T, double>>;

        // 3-way compares two scalar or vector types to determine which one is 'larger' (according to `sizeof`),
        // except floating-point types are always considered to be larger than integral ones.
        // For vector types, examines their base types instead.
        // Considers the types equivalent only if they are the same.
        template <typename A, typename B> inline constexpr std::partial_ordering compare_types_v =
            std::is_same_v<A, B> ? std::partial_ordering::equivalent :
            !vector_or_scalar<A> || !vector_or_scalar<B> ? std::partial_ordering::unordered :
            std::is_floating_point_v<vec_base_t<A>> < std::is_floating_point_v<vec_base_t<B>> ? std::partial_ordering::less    :
            std::is_floating_point_v<vec_base_t<A>> > std::is_floating_point_v<vec_base_t<B>> ? std::partial_ordering::greater :
            sizeof(vec_base_t<A>)                   < sizeof(vec_base_t<B>)                   ? std::partial_ordering::less    :
            sizeof(vec_base_t<A>)                   > sizeof(vec_base_t<B>)                   ? std::partial_ordering::greater : std::partial_ordering::unordered;

        // Internal, see below for the public interface.
        // Given a list of scalar and vector types, determines the "larger' type among them according to `compare_types_v`.
        // Returns `void` on failure.
        // If vector types are present, all of them must have the same size, and the resulting type will also be a vector.
        template <typename ...P> struct impl_larger {};
        template <typename T> struct impl_larger<T> {using type = T;};
        template <typename A, typename B, typename C, typename ...P> requires requires{typename impl_larger<B,C,P...>::type;} struct impl_larger<A,B,C,P...> {using type = typename impl_larger<A, typename impl_larger<B,C,P...>::type>::type;};
        template <typename A, typename B> requires(compare_types_v<A,B> == std::partial_ordering::equivalent) struct impl_larger<A,B> {using type = A;};
        template <typename A, typename B> requires(compare_types_v<A,B> == std::partial_ordering::less      ) struct impl_larger<A,B> {using type = B;};
        template <typename A, typename B> requires(compare_types_v<A,B> == std::partial_ordering::greater   ) struct impl_larger<A,B> {using type = A;};

        template <typename ...P> using larger_t = vec_or_scalar_t<common_vec_size_v<vec_size_v<P>...>, typename impl_larger<std::remove_cv_t<vec_base_t<P>>...>::type>;

        // Checks if it's possible to determine the 'larger' type among `P`.
        template <typename ...P> concept have_larger_type = requires{typename larger_t<P...>;};

        template <typename ...P> struct impl_larger_or_void {using type = void;};
        template <typename ...P> requires have_larger_type<P...> struct impl_larger_or_void<P...> {using type = larger_t<P...>;};

        template <typename ...P> using larger_or_void_t = typename impl_larger_or_void<P...>::type;
    }

    inline namespace Vector // Definitions
    {
        struct uninit {}; // A constructor tag.
        //{ Vectors
        template <typename T> struct vec<2,T> // vec2
        {
            using type = T;
            static constexpr int size = 2;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {type x, r;};
            union {type y, g;};
            constexpr vec() : x{}, y{} {}
            constexpr vec(uninit) {}
            constexpr vec(type x, type y) : x(x), y(y) {}
            explicit constexpr vec(type obj) : x(obj), y(obj) {}
            template <typename TT> constexpr vec(vec2<TT> obj) : x(obj.x), y(obj.y) {}
            template <typename TT> requires Custom::convertible<TT, vec> explicit constexpr vec(const TT &obj) {*this = Custom::Convert<TT, vec>{}(obj);}
            template <typename TT> requires Custom::convertible<vec, TT> explicit operator TT() const {return Custom::Convert<vec, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr vec2<TT> to() const {return vec2<TT>(TT(x), TT(y));}
            [[nodiscard]] constexpr type &operator[](int i) {return *(type *)((char *)this + sizeof(type)*i);}
            [[nodiscard]] constexpr const type &operator[](int i) const {return *(type *)((char *)this + sizeof(type)*i);}
            [[nodiscard]] type *as_array() {return &x;}
            [[nodiscard]] const type *as_array() const {return &x;}
            [[nodiscard]] explicit constexpr operator bool() const {return any(); static_assert(!std::is_same_v<type, bool>, "Use .none(), .any(), or .all() for vectors of bool.");}
            [[nodiscard]] constexpr bool none() const {return !any();}
            [[nodiscard]] constexpr bool any() const {return x || y;}
            [[nodiscard]] constexpr bool all() const {return x && y;}
            [[nodiscard]] constexpr auto sum() const {return x + y;}
            [[nodiscard]] constexpr auto prod() const {return x * y;}
            [[nodiscard]] constexpr auto ratio() const {return floating_point_t<type>(x) / floating_point_t<type>(y);}
            [[nodiscard]] constexpr type min() const {return std::min({x,y});}
            [[nodiscard]] constexpr type max() const {return std::max({x,y});}
            [[nodiscard]] constexpr vec abs() const {return vec(std::abs(x), std::abs(y));}
            [[nodiscard]] constexpr vec3<type> to_vec3(type nz) const {return {x, y, nz};}
            [[nodiscard]] constexpr vec4<type> to_vec4(type nz, type nw) const {return {x, y, nz, nw};}
            [[nodiscard]] constexpr vec3<type> to_vec3() const {return to_vec3(0);}
            [[nodiscard]] constexpr vec4<type> to_vec4() const {return to_vec4(0, 1);}
            [[nodiscard]] constexpr auto len_sqr() const {return x*x + y*y;}
            [[nodiscard]] constexpr auto len() const {return std::sqrt(len_sqr());}
            [[nodiscard]] constexpr auto norm() const -> vec2<decltype(type{}/len())> {if (auto l = len()) return *this / l; else return vec(0);}
            [[nodiscard]] constexpr auto approx_len() const {return floating_point_t<type>(len_sqr() + 1) / 2;} // Accurate only around `len()==1`.
            [[nodiscard]] constexpr auto approx_inv_len() const {return 2 / floating_point_t<type>(len_sqr() + 1);}
            [[nodiscard]] constexpr auto approx_norm() const {return *this * approx_inv_len();} // Guaranteed to converge to `len()==1` eventually, when starting from any finite `len_sqr()`.
            [[nodiscard]] static constexpr vec dir(type angle, type len = 1) {return vec(std::cos(angle) * len, std::sin(angle) * len); static_assert(is_floating_point, "The vector must be floating-point.");}
            template <typename TT = floating_point_t<type>> [[nodiscard]] constexpr TT angle() const {return std::atan2(TT(y), TT(x));}
            [[nodiscard]] constexpr vec rot90(int steps = 1) const {switch (steps & 3) {default: return *this; case 1: return {-y,x}; case 2: return -*this; case 3: return {y,-x};}}
            [[nodiscard]] static constexpr vec dir4(int index) {return vec(1,0).rot90(index);}
            [[nodiscard]] static constexpr vec dir8(int index) {vec array[8]{vec(1,0),vec(1,1),vec(0,1),vec(-1,1),vec(-1,0),vec(-1,-1),vec(0,-1),vec(1,-1)}; return array[index & 7];}
            template <typename TT> [[nodiscard]] constexpr auto dot(const vec2<TT> &o) const {return x * o.x + y * o.y;}
            template <typename TT> [[nodiscard]] constexpr auto cross(const vec2<TT> &o) const {return x * o.y - y * o.x;}
            template <typename TT> [[nodiscard]] constexpr auto delta_to(vec2<TT> v) const {return v - *this;}
            [[nodiscard]] constexpr auto tie() & {return std::tie(x,y);}
            [[nodiscard]] constexpr auto tie() const & {return std::tie(x,y);}
            template <int I> [[nodiscard]] constexpr type &get() & {return std::get<I>(tie());}
            template <int I> [[nodiscard]] constexpr const type &get() const & {return std::get<I>(tie());}
        };

        template <typename T> struct vec<3,T> // vec3
        {
            using type = T;
            static constexpr int size = 3;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {type x, r;};
            union {type y, g;};
            union {type z, b;};
            constexpr vec() : x{}, y{}, z{} {}
            constexpr vec(uninit) {}
            constexpr vec(type x, type y, type z) : x(x), y(y), z(z) {}
            explicit constexpr vec(type obj) : x(obj), y(obj), z(obj) {}
            template <typename TT> constexpr vec(vec3<TT> obj) : x(obj.x), y(obj.y), z(obj.z) {}
            template <typename TT> requires Custom::convertible<TT, vec> explicit constexpr vec(const TT &obj) {*this = Custom::Convert<TT, vec>{}(obj);}
            template <typename TT> requires Custom::convertible<vec, TT> explicit operator TT() const {return Custom::Convert<vec, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr vec3<TT> to() const {return vec3<TT>(TT(x), TT(y), TT(z));}
            [[nodiscard]] constexpr type &operator[](int i) {return *(type *)((char *)this + sizeof(type)*i);}
            [[nodiscard]] constexpr const type &operator[](int i) const {return *(type *)((char *)this + sizeof(type)*i);}
            [[nodiscard]] type *as_array() {return &x;}
            [[nodiscard]] const type *as_array() const {return &x;}
            [[nodiscard]] explicit constexpr operator bool() const {return any(); static_assert(!std::is_same_v<type, bool>, "Use .none(), .any(), or .all() for vectors of bool.");}
            [[nodiscard]] constexpr bool none() const {return !any();}
            [[nodiscard]] constexpr bool any() const {return x || y || z;}
            [[nodiscard]] constexpr bool all() const {return x && y && z;}
            [[nodiscard]] constexpr auto sum() const {return x + y + z;}
            [[nodiscard]] constexpr auto prod() const {return x * y * z;}
            [[nodiscard]] constexpr type min() const {return std::min({x,y,z});}
            [[nodiscard]] constexpr type max() const {return std::max({x,y,z});}
            [[nodiscard]] constexpr vec abs() const {return vec(std::abs(x), std::abs(y), std::abs(z));}
            [[nodiscard]] constexpr vec2<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr vec4<type> to_vec4(type nw) const {return {x, y, z, nw};}
            [[nodiscard]] constexpr vec4<type> to_vec4() const {return to_vec4(1);}
            [[nodiscard]] constexpr auto len_sqr() const {return x*x + y*y + z*z;}
            [[nodiscard]] constexpr auto len() const {return std::sqrt(len_sqr());}
            [[nodiscard]] constexpr auto norm() const -> vec3<decltype(type{}/len())> {if (auto l = len()) return *this / l; else return vec(0);}
            [[nodiscard]] constexpr auto approx_len() const {return floating_point_t<type>(len_sqr() + 1) / 2;} // Accurate only around `len()==1`.
            [[nodiscard]] constexpr auto approx_inv_len() const {return 2 / floating_point_t<type>(len_sqr() + 1);}
            [[nodiscard]] constexpr auto approx_norm() const {return *this * approx_inv_len();} // Guaranteed to converge to `len()==1` eventually, when starting from any finite `len_sqr()`.
            template <typename TT> [[nodiscard]] constexpr auto dot(const vec3<TT> &o) const {return x * o.x + y * o.y + z * o.z;}
            template <typename TT> [[nodiscard]] constexpr auto cross(const vec3<TT> &o) const -> vec3<decltype(x * o.x - x * o.x)> {return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};}
            template <typename TT> [[nodiscard]] constexpr auto delta_to(vec3<TT> v) const {return v - *this;}
            [[nodiscard]] constexpr auto tie() & {return std::tie(x,y,z);}
            [[nodiscard]] constexpr auto tie() const & {return std::tie(x,y,z);}
            template <int I> [[nodiscard]] constexpr type &get() & {return std::get<I>(tie());}
            template <int I> [[nodiscard]] constexpr const type &get() const & {return std::get<I>(tie());}
        };

        template <typename T> struct vec<4,T> // vec4
        {
            using type = T;
            static constexpr int size = 4;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {type x, r;};
            union {type y, g;};
            union {type z, b;};
            union {type w, a;};
            constexpr vec() : x{}, y{}, z{}, w{} {}
            constexpr vec(uninit) {}
            constexpr vec(type x, type y, type z, type w) : x(x), y(y), z(z), w(w) {}
            explicit constexpr vec(type obj) : x(obj), y(obj), z(obj), w(obj) {}
            template <typename TT> constexpr vec(vec4<TT> obj) : x(obj.x), y(obj.y), z(obj.z), w(obj.w) {}
            template <typename TT> requires Custom::convertible<TT, vec> explicit constexpr vec(const TT &obj) {*this = Custom::Convert<TT, vec>{}(obj);}
            template <typename TT> requires Custom::convertible<vec, TT> explicit operator TT() const {return Custom::Convert<vec, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr vec4<TT> to() const {return vec4<TT>(TT(x), TT(y), TT(z), TT(w));}
            [[nodiscard]] constexpr type &operator[](int i) {return *(type *)((char *)this + sizeof(type)*i);}
            [[nodiscard]] constexpr const type &operator[](int i) const {return *(type *)((char *)this + sizeof(type)*i);}
            [[nodiscard]] type *as_array() {return &x;}
            [[nodiscard]] const type *as_array() const {return &x;}
            [[nodiscard]] explicit constexpr operator bool() const {return any(); static_assert(!std::is_same_v<type, bool>, "Use .none(), .any(), or .all() for vectors of bool.");}
            [[nodiscard]] constexpr bool none() const {return !any();}
            [[nodiscard]] constexpr bool any() const {return x || y || z || w;}
            [[nodiscard]] constexpr bool all() const {return x && y && z && w;}
            [[nodiscard]] constexpr auto sum() const {return x + y + z + w;}
            [[nodiscard]] constexpr auto prod() const {return x * y * z * w;}
            [[nodiscard]] constexpr type min() const {return std::min({x,y,z,w});}
            [[nodiscard]] constexpr type max() const {return std::max({x,y,z,w});}
            [[nodiscard]] constexpr vec abs() const {return vec(std::abs(x), std::abs(y), std::abs(z), std::abs(w));}
            [[nodiscard]] constexpr vec2<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr vec3<type> to_vec3() const {return {x, y, z};}
            [[nodiscard]] constexpr auto len_sqr() const {return x*x + y*y + z*z + w*w;}
            [[nodiscard]] constexpr auto len() const {return std::sqrt(len_sqr());}
            [[nodiscard]] constexpr auto norm() const -> vec4<decltype(type{}/len())> {if (auto l = len()) return *this / l; else return vec(0);}
            [[nodiscard]] constexpr auto approx_len() const {return floating_point_t<type>(len_sqr() + 1) / 2;} // Accurate only around `len()==1`.
            [[nodiscard]] constexpr auto approx_inv_len() const {return 2 / floating_point_t<type>(len_sqr() + 1);}
            [[nodiscard]] constexpr auto approx_norm() const {return *this * approx_inv_len();} // Guaranteed to converge to `len()==1` eventually, when starting from any finite `len_sqr()`.
            template <typename TT> [[nodiscard]] constexpr auto dot(const vec4<TT> &o) const {return x * o.x + y * o.y + z * o.z + w * o.w;}
            template <typename TT> [[nodiscard]] constexpr auto delta_to(vec4<TT> v) const {return v - *this;}
            [[nodiscard]] constexpr auto tie() & {return std::tie(x,y,z,w);}
            [[nodiscard]] constexpr auto tie() const & {return std::tie(x,y,z,w);}
            template <int I> [[nodiscard]] constexpr type &get() & {return std::get<I>(tie());}
            template <int I> [[nodiscard]] constexpr const type &get() const & {return std::get<I>(tie());}
        };

        template <typename ...P, typename = std::enable_if_t<sizeof...(P) >= 2 && sizeof...(P) <= 4>> vec(P...) -> vec<sizeof...(P), larger_t<P...>>;
        //} Vectors

        //{ Matrices
        template <typename T> struct mat<2,2,T> // mat2x2
        {
            using type = T;
            using member_type = vec2<T>;
            static constexpr int width = 2, height = 2;
            static constexpr int size = 2;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            constexpr mat() : mat(1,0,0,1) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y) : x(x), y(y) {}
            constexpr mat(type xx, type yx, type xy, type yy) : x(xx,xy), y(yx,yy) {}
            template <typename TT> constexpr mat(const mat2x2<TT> &obj) : x(obj.x), y(obj.y) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat2x2<TT> to() const {return mat2x2<TT>(TT(x.x), TT(y.x), TT(x.y), TT(y.y));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat3x2<type> to_vec3(const member_type &nz) const {return {x, y, nz};}
            [[nodiscard]] constexpr mat4x2<type> to_vec4(const member_type &nz, const member_type &nw) const {return {x, y, nz, nw};}
            [[nodiscard]] constexpr mat3x2<type> to_vec3() const {return to_vec3({});}
            [[nodiscard]] constexpr mat4x2<type> to_vec4() const {return to_vec4({}, {});}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,0,x.y,y.y,0};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,0,0,x.y,y.y,0,0};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,0,0};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,0,x.y,y.y,0,0,0,1};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,0,0,x.y,y.y,0,0,0,0,1,0};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,0,0,0,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,0,x.y,y.y,0,0,0,1,0,0,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,0,0,x.y,y.y,0,0,0,0,1,0,0,0,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat2x2<T> transpose() const {return {x.x,x.y,y.x,y.y};}
            [[nodiscard]] constexpr mat inverse()
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");

                mat ret{};

                ret.x.x =  y.y;
                ret.y.x = -y.x;

                type d = x.x * ret.x.x + x.y * ret.y.x;
                if (d == 0) return {};
                d = 1 / d;
                ret.x.x *= d;
                ret.y.x *= d;

                ret.x.y = (-x.y) * d;
                ret.y.y = ( x.x) * d;

                return ret;
            }
            [[nodiscard]] static constexpr mat scale(vec2<type> v)
            {
                return { v.x , 0   ,
                         0   , v.y };
            }
            [[nodiscard]] static constexpr mat rotate(type angle)
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");
                type c = std::cos(angle);
                type s = std::sin(angle);
                return { c, -s ,
                         s, c  };
            }
        };

        template <typename T> struct mat<2,3,T> // mat2x3
        {
            using type = T;
            using member_type = vec3<T>;
            static constexpr int width = 2, height = 3;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            constexpr mat() : mat(1,0,0,1,0,0) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y) : x(x), y(y) {}
            constexpr mat(type xx, type yx, type xy, type yy, type xz, type yz) : x(xx,xy,xz), y(yx,yy,yz) {}
            template <typename TT> constexpr mat(const mat2x3<TT> &obj) : x(obj.x), y(obj.y) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat2x3<TT> to() const {return mat2x3<TT>(TT(x.x), TT(y.x), TT(x.y), TT(y.y), TT(x.z), TT(y.z));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat3x3<type> to_vec3(const member_type &nz) const {return {x, y, nz};}
            [[nodiscard]] constexpr mat4x3<type> to_vec4(const member_type &nz, const member_type &nw) const {return {x, y, nz, nw};}
            [[nodiscard]] constexpr mat3x3<type> to_vec3() const {return to_vec3({});}
            [[nodiscard]] constexpr mat4x3<type> to_vec4() const {return to_vec4({}, {});}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,0,x.y,y.y,0};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,0,0,x.y,y.y,0,0};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,0,x.y,y.y,0,x.z,y.z,1};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,0,0,x.y,y.y,0,0,x.z,y.z,1,0};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,x.z,y.z,0,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,0,x.y,y.y,0,x.z,y.z,1,0,0,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,0,0,x.y,y.y,0,0,x.z,y.z,1,0,0,0,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat3x2<T> transpose() const {return {x.x,x.y,x.z,y.x,y.y,y.z};}
        };

        template <typename T> struct mat<2,4,T> // mat2x4
        {
            using type = T;
            using member_type = vec4<T>;
            static constexpr int width = 2, height = 4;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            constexpr mat() : mat(1,0,0,1,0,0,0,0) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y) : x(x), y(y) {}
            constexpr mat(type xx, type yx, type xy, type yy, type xz, type yz, type xw, type yw) : x(xx,xy,xz,xw), y(yx,yy,yz,yw) {}
            template <typename TT> constexpr mat(const mat2x4<TT> &obj) : x(obj.x), y(obj.y) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat2x4<TT> to() const {return mat2x4<TT>(TT(x.x), TT(y.x), TT(x.y), TT(y.y), TT(x.z), TT(y.z), TT(x.w), TT(y.w));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat3x4<type> to_vec3(const member_type &nz) const {return {x, y, nz};}
            [[nodiscard]] constexpr mat4x4<type> to_vec4(const member_type &nz, const member_type &nw) const {return {x, y, nz, nw};}
            [[nodiscard]] constexpr mat3x4<type> to_vec3() const {return to_vec3({});}
            [[nodiscard]] constexpr mat4x4<type> to_vec4() const {return to_vec4({}, {});}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,0,x.y,y.y,0};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,0,0,x.y,y.y,0,0};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,x.z,y.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,0,x.y,y.y,0,x.z,y.z,1};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,0,0,x.y,y.y,0,0,x.z,y.z,1,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,0,x.y,y.y,0,x.z,y.z,1,x.w,y.w,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,0,0,x.y,y.y,0,0,x.z,y.z,1,0,x.w,y.w,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat4x2<T> transpose() const {return {x.x,x.y,x.z,x.w,y.x,y.y,y.z,y.w};}
        };

        template <typename T> struct mat<3,2,T> // mat3x2
        {
            using type = T;
            using member_type = vec2<T>;
            static constexpr int width = 3, height = 2;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            union {member_type z, b;};
            constexpr mat() : mat(1,0,0,0,1,0) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}), z(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y, const member_type &z) : x(x), y(y), z(z) {}
            constexpr mat(type xx, type yx, type zx, type xy, type yy, type zy) : x(xx,xy), y(yx,yy), z(zx,zy) {}
            template <typename TT> constexpr mat(const mat3x2<TT> &obj) : x(obj.x), y(obj.y), z(obj.z) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat3x2<TT> to() const {return mat3x2<TT>(TT(x.x), TT(y.x), TT(z.x), TT(x.y), TT(y.y), TT(z.y));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat2x2<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr mat4x2<type> to_vec4(const member_type &nw) const {return {x, y, z, nw};}
            [[nodiscard]] constexpr mat4x2<type> to_vec4() const {return to_vec4({});}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,0,0};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,z.x,x.y,y.y,z.y,0,0,1};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0,0,0,1,0};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,0,0,0,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,z.x,x.y,y.y,z.y,0,0,1,0,0,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0,0,0,1,0,0,0,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat2x3<T> transpose() const {return {x.x,x.y,y.x,y.y,z.x,z.y};}
        };

        template <typename T> struct mat<3,3,T> // mat3x3
        {
            using type = T;
            using member_type = vec3<T>;
            static constexpr int width = 3, height = 3;
            static constexpr int size = 3;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            union {member_type z, b;};
            constexpr mat() : mat(1,0,0,0,1,0,0,0,1) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}), z(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y, const member_type &z) : x(x), y(y), z(z) {}
            constexpr mat(type xx, type yx, type zx, type xy, type yy, type zy, type xz, type yz, type zz) : x(xx,xy,xz), y(yx,yy,yz), z(zx,zy,zz) {}
            template <typename TT> constexpr mat(const mat3x3<TT> &obj) : x(obj.x), y(obj.y), z(obj.z) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat3x3<TT> to() const {return mat3x3<TT>(TT(x.x), TT(y.x), TT(z.x), TT(x.y), TT(y.y), TT(z.y), TT(x.z), TT(y.z), TT(z.z));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat2x3<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr mat4x3<type> to_vec4(const member_type &nw) const {return {x, y, z, nw};}
            [[nodiscard]] constexpr mat4x3<type> to_vec4() const {return to_vec4({});}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,z.x,x.y,y.y,z.y};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,x.z,y.z};}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0,x.z,y.z,z.z,0};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,x.z,y.z,0,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,z.x,x.y,y.y,z.y,x.z,y.z,z.z,0,0,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0,x.z,y.z,z.z,0,0,0,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat3x3<T> transpose() const {return {x.x,x.y,x.z,y.x,y.y,y.z,z.x,z.y,z.z};}
            [[nodiscard]] constexpr mat inverse() const
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");

                mat ret{};

                ret.x.x =  y.y * z.z - z.y * y.z;
                ret.y.x = -y.x * z.z + z.x * y.z;
                ret.z.x =  y.x * z.y - z.x * y.y;

                type d = x.x * ret.x.x + x.y * ret.y.x + x.z * ret.z.x;
                if (d == 0) return {};
                d = 1 / d;
                ret.x.x *= d;
                ret.y.x *= d;
                ret.z.x *= d;

                ret.x.y = (-x.y * z.z + z.y * x.z) * d;
                ret.y.y = ( x.x * z.z - z.x * x.z) * d;
                ret.z.y = (-x.x * z.y + z.x * x.y) * d;
                ret.x.z = ( x.y * y.z - y.y * x.z) * d;
                ret.y.z = (-x.x * y.z + y.x * x.z) * d;
                ret.z.z = ( x.x * y.y - y.x * x.y) * d;

                return ret;
            }
            [[nodiscard]] static constexpr mat scale(vec2<type> v) {return mat2<T>::scale(v).to_mat3();}
            [[nodiscard]] static constexpr mat scale(vec3<type> v)
            {
                return { v.x , 0   , 0   ,
                         0   , v.y , 0   ,
                         0   , 0   , v.z };
            }
            [[nodiscard]] static constexpr mat ortho(vec2<type> min, vec2<type> max)
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");
                return { 2 / (max.x - min.x) , 0                   , (min.x + max.x) / (min.x - max.x) ,
                         0                   , 2 / (max.y - min.y) , (min.y + max.y) / (min.y - max.y) ,
                         0                   , 0                   , 1                                 };
            }
            [[nodiscard]] static constexpr mat translate(vec2<type> v)
            {
                return { 1, 0, v.x ,
                         0, 1, v.y ,
                         0, 0, 1   };
            }
            [[nodiscard]] static constexpr mat rotate(type angle) {return mat2<T>::rotate(angle).to_mat3();}
            [[nodiscard]] static constexpr mat rotate_with_normalized_axis(vec3<type> axis, type angle)
            {
                type c = std::cos(angle);
                type s = std::sin(angle);
                return { axis.x * axis.x * (1 - c) + c          , axis.x * axis.y * (1 - c) - axis.z * s , axis.x * axis.z * (1 - c) + axis.y * s,
                         axis.y * axis.x * (1 - c) + axis.z * s , axis.y * axis.y * (1 - c) + c          , axis.y * axis.z * (1 - c) - axis.x * s,
                         axis.x * axis.z * (1 - c) - axis.y * s , axis.y * axis.z * (1 - c) + axis.x * s , axis.z * axis.z * (1 - c) + c         };
            }
            [[nodiscard]] static constexpr mat rotate(vec3<type> axis, type angle)
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");
                return rotate_with_normalized_axis(axis.norm(), angle);
            }
        };

        template <typename T> struct mat<3,4,T> // mat3x4
        {
            using type = T;
            using member_type = vec4<T>;
            static constexpr int width = 3, height = 4;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            union {member_type z, b;};
            constexpr mat() : mat(1,0,0,0,1,0,0,0,1,0,0,0) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}), z(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y, const member_type &z) : x(x), y(y), z(z) {}
            constexpr mat(type xx, type yx, type zx, type xy, type yy, type zy, type xz, type yz, type zz, type xw, type yw, type zw) : x(xx,xy,xz,xw), y(yx,yy,yz,yw), z(zx,zy,zz,zw) {}
            template <typename TT> constexpr mat(const mat3x4<TT> &obj) : x(obj.x), y(obj.y), z(obj.z) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat3x4<TT> to() const {return mat3x4<TT>(TT(x.x), TT(y.x), TT(z.x), TT(x.y), TT(y.y), TT(z.y), TT(x.z), TT(y.z), TT(z.z), TT(x.w), TT(y.w), TT(z.w));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat2x4<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr mat4x4<type> to_vec4(const member_type &nw) const {return {x, y, z, nw};}
            [[nodiscard]] constexpr mat4x4<type> to_vec4() const {return to_vec4({});}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,z.x,x.y,y.y,z.y};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,x.z,y.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,z.x,x.y,y.y,z.y,x.z,y.z,z.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0,x.z,y.z,z.z,0};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,x.z,y.z,x.w,y.w};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,z.x,0,x.y,y.y,z.y,0,x.z,y.z,z.z,0,x.w,y.w,z.w,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat4x3<T> transpose() const {return {x.x,x.y,x.z,x.w,y.x,y.y,y.z,y.w,z.x,z.y,z.z,z.w};}
        };

        template <typename T> struct mat<4,2,T> // mat4x2
        {
            using type = T;
            using member_type = vec2<T>;
            static constexpr int width = 4, height = 2;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            union {member_type z, b;};
            union {member_type w, a;};
            constexpr mat() : mat(1,0,0,0,0,1,0,0) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}), z(uninit{}), w(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y, const member_type &z, const member_type &w) : x(x), y(y), z(z), w(w) {}
            constexpr mat(type xx, type yx, type zx, type wx, type xy, type yy, type zy, type wy) : x(xx,xy), y(yx,yy), z(zx,zy), w(wx,wy) {}
            template <typename TT> constexpr mat(const mat4x2<TT> &obj) : x(obj.x), y(obj.y), z(obj.z), w(obj.w) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat4x2<TT> to() const {return mat4x2<TT>(TT(x.x), TT(y.x), TT(z.x), TT(w.x), TT(x.y), TT(y.y), TT(z.y), TT(w.y));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat2x2<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr mat3x2<type> to_vec3() const {return {x, y, z};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,z.x,x.y,y.y,z.y};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,0,0};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,z.x,x.y,y.y,z.y,0,0,1};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,z.x,w.x,x.y,y.y,z.y,w.y,0,0,1,0};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,0,0,0,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,z.x,x.y,y.y,z.y,0,0,1,0,0,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,z.x,w.x,x.y,y.y,z.y,w.y,0,0,1,0,0,0,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat2x4<T> transpose() const {return {x.x,x.y,y.x,y.y,z.x,z.y,w.x,w.y};}
        };

        template <typename T> struct mat<4,3,T> // mat4x3
        {
            using type = T;
            using member_type = vec3<T>;
            static constexpr int width = 4, height = 3;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            union {member_type z, b;};
            union {member_type w, a;};
            constexpr mat() : mat(1,0,0,0,0,1,0,0,0,0,1,0) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}), z(uninit{}), w(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y, const member_type &z, const member_type &w) : x(x), y(y), z(z), w(w) {}
            constexpr mat(type xx, type yx, type zx, type wx, type xy, type yy, type zy, type wy, type xz, type yz, type zz, type wz) : x(xx,xy,xz), y(yx,yy,yz), z(zx,zy,zz), w(wx,wy,wz) {}
            template <typename TT> constexpr mat(const mat4x3<TT> &obj) : x(obj.x), y(obj.y), z(obj.z), w(obj.w) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat4x3<TT> to() const {return mat4x3<TT>(TT(x.x), TT(y.x), TT(z.x), TT(w.x), TT(x.y), TT(y.y), TT(z.y), TT(w.y), TT(x.z), TT(y.z), TT(z.z), TT(w.z));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat2x3<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr mat3x3<type> to_vec3() const {return {x, y, z};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,z.x,x.y,y.y,z.y};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,z.x,w.x,x.y,y.y,z.y,w.y};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,x.z,y.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,z.x,x.y,y.y,z.y,x.z,y.z,z.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,x.z,y.z,0,0};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,z.x,x.y,y.y,z.y,x.z,y.z,z.z,0,0,0};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4x4() const {return {x.x,y.x,z.x,w.x,x.y,y.y,z.y,w.y,x.z,y.z,z.z,w.z,0,0,0,1};}
            [[nodiscard]] constexpr mat4x4<type> to_mat4() const {return to_mat4x4();}
            [[nodiscard]] constexpr mat3x4<T> transpose() const {return {x.x,x.y,x.z,y.x,y.y,y.z,z.x,z.y,z.z,w.x,w.y,w.z};}
        };

        template <typename T> struct mat<4,4,T> // mat4x4
        {
            using type = T;
            using member_type = vec4<T>;
            static constexpr int width = 4, height = 4;
            static constexpr int size = 4;
            static constexpr bool is_floating_point = std::is_floating_point_v<type>;
            union {member_type x, r;};
            union {member_type y, g;};
            union {member_type z, b;};
            union {member_type w, a;};
            constexpr mat() : mat(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) {}
            constexpr mat(uninit) : x(uninit{}), y(uninit{}), z(uninit{}), w(uninit{}) {}
            constexpr mat(const member_type &x, const member_type &y, const member_type &z, const member_type &w) : x(x), y(y), z(z), w(w) {}
            constexpr mat(type xx, type yx, type zx, type wx, type xy, type yy, type zy, type wy, type xz, type yz, type zz, type wz, type xw, type yw, type zw, type ww) : x(xx,xy,xz,xw), y(yx,yy,yz,yw), z(zx,zy,zz,zw), w(wx,wy,wz,ww) {}
            template <typename TT> constexpr mat(const mat4x4<TT> &obj) : x(obj.x), y(obj.y), z(obj.z), w(obj.w) {}
            template <typename TT> requires Custom::convertible<TT, mat> explicit constexpr mat(const TT &obj) {*this = Custom::Convert<TT, mat>{}(obj);}
            template <typename TT> requires Custom::convertible<mat, TT> explicit operator TT() const {return Custom::Convert<mat, TT>{}(*this);}
            template <typename TT> [[nodiscard]] constexpr mat4x4<TT> to() const {return mat4x4<TT>(TT(x.x), TT(y.x), TT(z.x), TT(w.x), TT(x.y), TT(y.y), TT(z.y), TT(w.y), TT(x.z), TT(y.z), TT(z.z), TT(w.z), TT(x.w), TT(y.w), TT(z.w), TT(w.w));}
            [[nodiscard]] constexpr member_type &operator[](int i) {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] constexpr const member_type &operator[](int i) const {return *(member_type *)((char *)this + sizeof(member_type)*i);}
            [[nodiscard]] type *as_array() {return &x.x;}
            [[nodiscard]] const type *as_array() const {return &x.x;}
            [[nodiscard]] constexpr mat2x4<type> to_vec2() const {return {x, y};}
            [[nodiscard]] constexpr mat3x4<type> to_vec3() const {return {x, y, z};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2x2() const {return {x.x,y.x,x.y,y.y};}
            [[nodiscard]] constexpr mat2x2<type> to_mat2() const {return to_mat2x2();}
            [[nodiscard]] constexpr mat3x2<type> to_mat3x2() const {return {x.x,y.x,z.x,x.y,y.y,z.y};}
            [[nodiscard]] constexpr mat4x2<type> to_mat4x2() const {return {x.x,y.x,z.x,w.x,x.y,y.y,z.y,w.y};}
            [[nodiscard]] constexpr mat2x3<type> to_mat2x3() const {return {x.x,y.x,x.y,y.y,x.z,y.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3x3() const {return {x.x,y.x,z.x,x.y,y.y,z.y,x.z,y.z,z.z};}
            [[nodiscard]] constexpr mat3x3<type> to_mat3() const {return to_mat3x3();}
            [[nodiscard]] constexpr mat4x3<type> to_mat4x3() const {return {x.x,y.x,z.x,w.x,x.y,y.y,z.y,w.y,x.z,y.z,z.z,w.z};}
            [[nodiscard]] constexpr mat2x4<type> to_mat2x4() const {return {x.x,y.x,x.y,y.y,x.z,y.z,x.w,y.w};}
            [[nodiscard]] constexpr mat3x4<type> to_mat3x4() const {return {x.x,y.x,z.x,x.y,y.y,z.y,x.z,y.z,z.z,x.w,y.w,z.w};}
            [[nodiscard]] constexpr mat4x4<T> transpose() const {return {x.x,x.y,x.z,x.w,y.x,y.y,y.z,y.w,z.x,z.y,z.z,z.w,w.x,w.y,w.z,w.w};}
            [[nodiscard]] constexpr mat inverse() const
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");

                mat ret;

                ret.x.x =  y.y * z.z * w.w - y.y * z.w * w.z - z.y * y.z * w.w + z.y * y.w * w.z + w.y * y.z * z.w - w.y * y.w * z.z;
                ret.y.x = -y.x * z.z * w.w + y.x * z.w * w.z + z.x * y.z * w.w - z.x * y.w * w.z - w.x * y.z * z.w + w.x * y.w * z.z;
                ret.z.x =  y.x * z.y * w.w - y.x * z.w * w.y - z.x * y.y * w.w + z.x * y.w * w.y + w.x * y.y * z.w - w.x * y.w * z.y;
                ret.w.x = -y.x * z.y * w.z + y.x * z.z * w.y + z.x * y.y * w.z - z.x * y.z * w.y - w.x * y.y * z.z + w.x * y.z * z.y;

                type d = x.x * ret.x.x + x.y * ret.y.x + x.z * ret.z.x + x.w * ret.w.x;
                if (d == 0) return {};
                d = 1 / d;
                ret.x.x *= d;
                ret.y.x *= d;
                ret.z.x *= d;
                ret.w.x *= d;

                ret.x.y = (-x.y * z.z * w.w + x.y * z.w * w.z + z.y * x.z * w.w - z.y * x.w * w.z - w.y * x.z * z.w + w.y * x.w * z.z) * d;
                ret.y.y = ( x.x * z.z * w.w - x.x * z.w * w.z - z.x * x.z * w.w + z.x * x.w * w.z + w.x * x.z * z.w - w.x * x.w * z.z) * d;
                ret.z.y = (-x.x * z.y * w.w + x.x * z.w * w.y + z.x * x.y * w.w - z.x * x.w * w.y - w.x * x.y * z.w + w.x * x.w * z.y) * d;
                ret.w.y = ( x.x * z.y * w.z - x.x * z.z * w.y - z.x * x.y * w.z + z.x * x.z * w.y + w.x * x.y * z.z - w.x * x.z * z.y) * d;
                ret.x.z = ( x.y * y.z * w.w - x.y * y.w * w.z - y.y * x.z * w.w + y.y * x.w * w.z + w.y * x.z * y.w - w.y * x.w * y.z) * d;
                ret.y.z = (-x.x * y.z * w.w + x.x * y.w * w.z + y.x * x.z * w.w - y.x * x.w * w.z - w.x * x.z * y.w + w.x * x.w * y.z) * d;
                ret.z.z = ( x.x * y.y * w.w - x.x * y.w * w.y - y.x * x.y * w.w + y.x * x.w * w.y + w.x * x.y * y.w - w.x * x.w * y.y) * d;
                ret.w.z = (-x.x * y.y * w.z + x.x * y.z * w.y + y.x * x.y * w.z - y.x * x.z * w.y - w.x * x.y * y.z + w.x * x.z * y.y) * d;
                ret.x.w = (-x.y * y.z * z.w + x.y * y.w * z.z + y.y * x.z * z.w - y.y * x.w * z.z - z.y * x.z * y.w + z.y * x.w * y.z) * d;
                ret.y.w = ( x.x * y.z * z.w - x.x * y.w * z.z - y.x * x.z * z.w + y.x * x.w * z.z + z.x * x.z * y.w - z.x * x.w * y.z) * d;
                ret.z.w = (-x.x * y.y * z.w + x.x * y.w * z.y + y.x * x.y * z.w - y.x * x.w * z.y - z.x * x.y * y.w + z.x * x.w * y.y) * d;
                ret.w.w = ( x.x * y.y * z.z - x.x * y.z * z.y - y.x * x.y * z.z + y.x * x.z * z.y + z.x * x.y * y.z - z.x * x.z * y.y) * d;

                return ret;
            }
            [[nodiscard]] static constexpr mat scale(vec3<type> v) {return mat3<T>::scale(v).to_mat4();}
            [[nodiscard]] static constexpr mat ortho(vec2<type> min, vec2<type> max, type near, type far)
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");
                return { 2 / (max.x - min.x) , 0                   , 0                , (min.x + max.x) / (min.x - max.x) ,
                         0                   , 2 / (max.y - min.y) , 0                , (min.y + max.y) / (min.y - max.y) ,
                         0                   , 0                   , 2 / (near - far) , (near + far) / (near - far)       ,
                         0                   , 0                   , 0                , 1                                 };
            }
            [[nodiscard]] static constexpr mat look_at(vec3<type> src, vec3<type> dst, vec3<type> local_up)
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");
                vec3<type> v3 = (src-dst).norm();
                vec3<type> v1 = local_up.cross(v3).norm();
                vec3<type> v2 = v3.cross(v1);
                return { v1.x , v1.y , v1.z , -src.x*v1.x-src.y*v1.y-src.z*v1.z ,
                         v2.x , v2.y , v2.z , -src.x*v2.x-src.y*v2.y-src.z*v2.z ,
                         v3.x , v3.y , v3.z , -src.x*v3.x-src.y*v3.y-src.z*v3.z ,
                         0    , 0    , 0    , 1                                 };
            }
            [[nodiscard]] static constexpr mat translate(vec3<type> v)
            {
                return { 1 , 0 , 0 , v.x ,
                         0 , 1 , 0 , v.y ,
                         0 , 0 , 1 , v.z ,
                         0 , 0 , 0 , 1   };
            }
            [[nodiscard]] static constexpr mat rotate_with_normalized_axis(vec3<type> axis, type angle) {return mat3<T>::rotate_with_normalized_axis(axis, angle).to_mat4();}
            [[nodiscard]] static constexpr mat rotate(vec3<type> axis, type angle) {return mat3<T>::rotate(axis, angle).to_mat4();}
            [[nodiscard]] static constexpr mat perspective(type wh_aspect, type y_fov, type near, type far)
            {
                static_assert(is_floating_point, "This function only makes sense for floating-point matrices.");
                y_fov = type(1) / std::tan(y_fov / 2);
                return { y_fov / wh_aspect , 0     , 0                           , 0                             ,
                         0                 , y_fov , 0                           , 0                             ,
                         0                 , 0     , (near + far) / (near - far) , 2 * near * far / (near - far) ,
                         0                 , 0     , -1                          , 0                             };
            }
        };

        template <scalar ...P> requires (sizeof...(P) == 4) mat(P...) -> mat<2, 2, larger_t<P...>>;
        template <scalar ...P> requires (sizeof...(P) == 9) mat(P...) -> mat<3, 3, larger_t<P...>>;
        template <scalar ...P> requires (sizeof...(P) == 16) mat(P...) -> mat<4, 4, larger_t<P...>>;
        template <typename ...P> requires (sizeof...(P) >= 2 && sizeof...(P) <= 4 && ((vec_size_v<P> == 2) && ...)) mat(P...) -> mat<sizeof...(P), 2, larger_t<typename P::type...>>;
        template <typename ...P> requires (sizeof...(P) >= 2 && sizeof...(P) <= 4 && ((vec_size_v<P> == 3) && ...)) mat(P...) -> mat<sizeof...(P), 3, larger_t<typename P::type...>>;
        template <typename ...P> requires (sizeof...(P) >= 2 && sizeof...(P) <= 4 && ((vec_size_v<P> == 4) && ...)) mat(P...) -> mat<sizeof...(P), 4, larger_t<typename P::type...>>;
        //} Matrices

        //{ Operators
        //{  vec2
        template <typename A, typename B> [[nodiscard]] constexpr auto operator+(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x + b.x)> {return {a.x + b.x, a.y + b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator+(const vec2<V> &v, const S &s) {return v + vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator+(const S &s, const vec2<V> &v) {return vec2<S>(s) + v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator-(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x - b.x)> {return {a.x - b.x, a.y - b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator-(const vec2<V> &v, const S &s) {return v - vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator-(const S &s, const vec2<V> &v) {return vec2<S>(s) - v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator*(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x * b.x)> {return {a.x * b.x, a.y * b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator*(const vec2<V> &v, const S &s) {return v * vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator*(const S &s, const vec2<V> &v) {return vec2<S>(s) * v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator/(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x / b.x)> {return {a.x / b.x, a.y / b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator/(const vec2<V> &v, const S &s) {return v / vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator/(const S &s, const vec2<V> &v) {return vec2<S>(s) / v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator%(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x % b.x)> {return {a.x % b.x, a.y % b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator%(const vec2<V> &v, const S &s) {return v % vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator%(const S &s, const vec2<V> &v) {return vec2<S>(s) % v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator^(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x ^ b.x)> {return {a.x ^ b.x, a.y ^ b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator^(const vec2<V> &v, const S &s) {return v ^ vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator^(const S &s, const vec2<V> &v) {return vec2<S>(s) ^ v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator&(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x & b.x)> {return {a.x & b.x, a.y & b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator&(const vec2<V> &v, const S &s) {return v & vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator&(const S &s, const vec2<V> &v) {return vec2<S>(s) & v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator|(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x | b.x)> {return {a.x | b.x, a.y | b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator|(const vec2<V> &v, const S &s) {return v | vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator|(const S &s, const vec2<V> &v) {return vec2<S>(s) | v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<<(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x << b.x)> {return {a.x << b.x, a.y << b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<<(const vec2<V> &v, const S &s) {return v << vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<<(const S &s, const vec2<V> &v) {return vec2<S>(s) << v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>>(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x >> b.x)> {return {a.x >> b.x, a.y >> b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>>(const vec2<V> &v, const S &s) {return v >> vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>>(const S &s, const vec2<V> &v) {return vec2<S>(s) >> v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x < b.x)> {return {a.x < b.x, a.y < b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<(const vec2<V> &v, const S &s) {return v < vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<(const S &s, const vec2<V> &v) {return vec2<S>(s) < v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x > b.x)> {return {a.x > b.x, a.y > b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>(const vec2<V> &v, const S &s) {return v > vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>(const S &s, const vec2<V> &v) {return vec2<S>(s) > v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<=(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x <= b.x)> {return {a.x <= b.x, a.y <= b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<=(const vec2<V> &v, const S &s) {return v <= vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<=(const S &s, const vec2<V> &v) {return vec2<S>(s) <= v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>=(const vec2<A> &a, const vec2<B> &b) -> vec2<decltype(a.x >= b.x)> {return {a.x >= b.x, a.y >= b.y};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>=(const vec2<V> &v, const S &s) {return v >= vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>=(const S &s, const vec2<V> &v) {return vec2<S>(s) >= v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator==(const vec2<A> &a, const vec2<B> &b) {return a.x == b.x && a.y == b.y;}
        template <typename V, scalar S> [[nodiscard]] constexpr bool operator==(const vec2<V> &v, const S &s) {return v == vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr bool operator==(const S &s, const vec2<V> &v) {return vec2<S>(s) == v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator!=(const vec2<A> &a, const vec2<B> &b) {return a.x != b.x || a.y != b.y;}
        template <typename V, scalar S> [[nodiscard]] constexpr bool operator!=(const vec2<V> &v, const S &s) {return v != vec2<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr bool operator!=(const S &s, const vec2<V> &v) {return vec2<S>(s) != v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const vec2<A> &a, const vec2<B> &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const vec2<A> &a, const B &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const A &a, const vec2<B> &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const vec2<A> &a, const vec2<B> &b) {return bool(a) || bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const vec2<A> &a, const B &b) {return bool(a) || bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const A &a, const vec2<B> &b) {return bool(a) || bool(b);}
        template <typename T> [[nodiscard]] constexpr auto operator~(const vec2<T> &v) -> vec2<decltype(~v.x)> {return {~v.x, ~v.y};}
        template <typename T> [[nodiscard]] constexpr auto operator+(const vec2<T> &v) -> vec2<decltype(+v.x)> {return {+v.x, +v.y};}
        template <typename T> [[nodiscard]] constexpr auto operator-(const vec2<T> &v) -> vec2<decltype(-v.x)> {return {-v.x, -v.y};}
        template <typename T> [[nodiscard]] constexpr bool operator!(const vec2<T> &v) {return !bool(v);}
        template <typename T> constexpr vec2<T> &operator++(vec2<T> &v) {++v.x; ++v.y; return v;}
        template <typename T> constexpr vec2<T> operator++(vec2<T> &v, int) {return {v.x++, v.y++};}
        template <typename T> constexpr vec2<T> &operator--(vec2<T> &v) {--v.x; --v.y; return v;}
        template <typename T> constexpr vec2<T> operator--(vec2<T> &v, int) {return {v.x--, v.y--};}
        template <typename A, typename B> constexpr vec2<A> &operator+=(vec2<A> &a, const vec2<B> &b) {a.x += b.x; a.y += b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator+=(vec2<V> &v, const S &s) {return v += vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator-=(vec2<A> &a, const vec2<B> &b) {a.x -= b.x; a.y -= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator-=(vec2<V> &v, const S &s) {return v -= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator*=(vec2<A> &a, const vec2<B> &b) {a.x *= b.x; a.y *= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator*=(vec2<V> &v, const S &s) {return v *= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator/=(vec2<A> &a, const vec2<B> &b) {a.x /= b.x; a.y /= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator/=(vec2<V> &v, const S &s) {return v /= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator%=(vec2<A> &a, const vec2<B> &b) {a.x %= b.x; a.y %= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator%=(vec2<V> &v, const S &s) {return v %= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator^=(vec2<A> &a, const vec2<B> &b) {a.x ^= b.x; a.y ^= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator^=(vec2<V> &v, const S &s) {return v ^= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator&=(vec2<A> &a, const vec2<B> &b) {a.x &= b.x; a.y &= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator&=(vec2<V> &v, const S &s) {return v &= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator|=(vec2<A> &a, const vec2<B> &b) {a.x |= b.x; a.y |= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator|=(vec2<V> &v, const S &s) {return v |= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator<<=(vec2<A> &a, const vec2<B> &b) {a.x <<= b.x; a.y <<= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator<<=(vec2<V> &v, const S &s) {return v <<= vec2<S>(s);}
        template <typename A, typename B> constexpr vec2<A> &operator>>=(vec2<A> &a, const vec2<B> &b) {a.x >>= b.x; a.y >>= b.y; return a;}
        template <typename V, scalar S> constexpr vec2<V> &operator>>=(vec2<V> &v, const S &s) {return v >>= vec2<S>(s);}
        //}  vec2

        //{  vec3
        template <typename A, typename B> [[nodiscard]] constexpr auto operator+(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x + b.x)> {return {a.x + b.x, a.y + b.y, a.z + b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator+(const vec3<V> &v, const S &s) {return v + vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator+(const S &s, const vec3<V> &v) {return vec3<S>(s) + v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator-(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x - b.x)> {return {a.x - b.x, a.y - b.y, a.z - b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator-(const vec3<V> &v, const S &s) {return v - vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator-(const S &s, const vec3<V> &v) {return vec3<S>(s) - v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator*(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x * b.x)> {return {a.x * b.x, a.y * b.y, a.z * b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator*(const vec3<V> &v, const S &s) {return v * vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator*(const S &s, const vec3<V> &v) {return vec3<S>(s) * v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator/(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x / b.x)> {return {a.x / b.x, a.y / b.y, a.z / b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator/(const vec3<V> &v, const S &s) {return v / vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator/(const S &s, const vec3<V> &v) {return vec3<S>(s) / v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator%(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x % b.x)> {return {a.x % b.x, a.y % b.y, a.z % b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator%(const vec3<V> &v, const S &s) {return v % vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator%(const S &s, const vec3<V> &v) {return vec3<S>(s) % v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator^(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x ^ b.x)> {return {a.x ^ b.x, a.y ^ b.y, a.z ^ b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator^(const vec3<V> &v, const S &s) {return v ^ vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator^(const S &s, const vec3<V> &v) {return vec3<S>(s) ^ v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator&(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x & b.x)> {return {a.x & b.x, a.y & b.y, a.z & b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator&(const vec3<V> &v, const S &s) {return v & vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator&(const S &s, const vec3<V> &v) {return vec3<S>(s) & v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator|(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x | b.x)> {return {a.x | b.x, a.y | b.y, a.z | b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator|(const vec3<V> &v, const S &s) {return v | vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator|(const S &s, const vec3<V> &v) {return vec3<S>(s) | v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<<(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x << b.x)> {return {a.x << b.x, a.y << b.y, a.z << b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<<(const vec3<V> &v, const S &s) {return v << vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<<(const S &s, const vec3<V> &v) {return vec3<S>(s) << v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>>(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x >> b.x)> {return {a.x >> b.x, a.y >> b.y, a.z >> b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>>(const vec3<V> &v, const S &s) {return v >> vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>>(const S &s, const vec3<V> &v) {return vec3<S>(s) >> v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x < b.x)> {return {a.x < b.x, a.y < b.y, a.z < b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<(const vec3<V> &v, const S &s) {return v < vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<(const S &s, const vec3<V> &v) {return vec3<S>(s) < v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x > b.x)> {return {a.x > b.x, a.y > b.y, a.z > b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>(const vec3<V> &v, const S &s) {return v > vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>(const S &s, const vec3<V> &v) {return vec3<S>(s) > v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<=(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x <= b.x)> {return {a.x <= b.x, a.y <= b.y, a.z <= b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<=(const vec3<V> &v, const S &s) {return v <= vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<=(const S &s, const vec3<V> &v) {return vec3<S>(s) <= v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>=(const vec3<A> &a, const vec3<B> &b) -> vec3<decltype(a.x >= b.x)> {return {a.x >= b.x, a.y >= b.y, a.z >= b.z};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>=(const vec3<V> &v, const S &s) {return v >= vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>=(const S &s, const vec3<V> &v) {return vec3<S>(s) >= v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator==(const vec3<A> &a, const vec3<B> &b) {return a.x == b.x && a.y == b.y && a.z == b.z;}
        template <typename V, scalar S> [[nodiscard]] constexpr bool operator==(const vec3<V> &v, const S &s) {return v == vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr bool operator==(const S &s, const vec3<V> &v) {return vec3<S>(s) == v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator!=(const vec3<A> &a, const vec3<B> &b) {return a.x != b.x || a.y != b.y || a.z != b.z;}
        template <typename V, scalar S> [[nodiscard]] constexpr bool operator!=(const vec3<V> &v, const S &s) {return v != vec3<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr bool operator!=(const S &s, const vec3<V> &v) {return vec3<S>(s) != v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const vec3<A> &a, const vec3<B> &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const vec3<A> &a, const B &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const A &a, const vec3<B> &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const vec3<A> &a, const vec3<B> &b) {return bool(a) || bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const vec3<A> &a, const B &b) {return bool(a) || bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const A &a, const vec3<B> &b) {return bool(a) || bool(b);}
        template <typename T> [[nodiscard]] constexpr auto operator~(const vec3<T> &v) -> vec3<decltype(~v.x)> {return {~v.x, ~v.y, ~v.z};}
        template <typename T> [[nodiscard]] constexpr auto operator+(const vec3<T> &v) -> vec3<decltype(+v.x)> {return {+v.x, +v.y, +v.z};}
        template <typename T> [[nodiscard]] constexpr auto operator-(const vec3<T> &v) -> vec3<decltype(-v.x)> {return {-v.x, -v.y, -v.z};}
        template <typename T> [[nodiscard]] constexpr bool operator!(const vec3<T> &v) {return !bool(v);}
        template <typename T> constexpr vec3<T> &operator++(vec3<T> &v) {++v.x; ++v.y; ++v.z; return v;}
        template <typename T> constexpr vec3<T> operator++(vec3<T> &v, int) {return {v.x++, v.y++, v.z++};}
        template <typename T> constexpr vec3<T> &operator--(vec3<T> &v) {--v.x; --v.y; --v.z; return v;}
        template <typename T> constexpr vec3<T> operator--(vec3<T> &v, int) {return {v.x--, v.y--, v.z--};}
        template <typename A, typename B> constexpr vec3<A> &operator+=(vec3<A> &a, const vec3<B> &b) {a.x += b.x; a.y += b.y; a.z += b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator+=(vec3<V> &v, const S &s) {return v += vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator-=(vec3<A> &a, const vec3<B> &b) {a.x -= b.x; a.y -= b.y; a.z -= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator-=(vec3<V> &v, const S &s) {return v -= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator*=(vec3<A> &a, const vec3<B> &b) {a.x *= b.x; a.y *= b.y; a.z *= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator*=(vec3<V> &v, const S &s) {return v *= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator/=(vec3<A> &a, const vec3<B> &b) {a.x /= b.x; a.y /= b.y; a.z /= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator/=(vec3<V> &v, const S &s) {return v /= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator%=(vec3<A> &a, const vec3<B> &b) {a.x %= b.x; a.y %= b.y; a.z %= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator%=(vec3<V> &v, const S &s) {return v %= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator^=(vec3<A> &a, const vec3<B> &b) {a.x ^= b.x; a.y ^= b.y; a.z ^= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator^=(vec3<V> &v, const S &s) {return v ^= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator&=(vec3<A> &a, const vec3<B> &b) {a.x &= b.x; a.y &= b.y; a.z &= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator&=(vec3<V> &v, const S &s) {return v &= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator|=(vec3<A> &a, const vec3<B> &b) {a.x |= b.x; a.y |= b.y; a.z |= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator|=(vec3<V> &v, const S &s) {return v |= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator<<=(vec3<A> &a, const vec3<B> &b) {a.x <<= b.x; a.y <<= b.y; a.z <<= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator<<=(vec3<V> &v, const S &s) {return v <<= vec3<S>(s);}
        template <typename A, typename B> constexpr vec3<A> &operator>>=(vec3<A> &a, const vec3<B> &b) {a.x >>= b.x; a.y >>= b.y; a.z >>= b.z; return a;}
        template <typename V, scalar S> constexpr vec3<V> &operator>>=(vec3<V> &v, const S &s) {return v >>= vec3<S>(s);}
        //}  vec3

        //{  vec4
        template <typename A, typename B> [[nodiscard]] constexpr auto operator+(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x + b.x)> {return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator+(const vec4<V> &v, const S &s) {return v + vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator+(const S &s, const vec4<V> &v) {return vec4<S>(s) + v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator-(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x - b.x)> {return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator-(const vec4<V> &v, const S &s) {return v - vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator-(const S &s, const vec4<V> &v) {return vec4<S>(s) - v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator*(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x * b.x)> {return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator*(const vec4<V> &v, const S &s) {return v * vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator*(const S &s, const vec4<V> &v) {return vec4<S>(s) * v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator/(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x / b.x)> {return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator/(const vec4<V> &v, const S &s) {return v / vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator/(const S &s, const vec4<V> &v) {return vec4<S>(s) / v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator%(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x % b.x)> {return {a.x % b.x, a.y % b.y, a.z % b.z, a.w % b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator%(const vec4<V> &v, const S &s) {return v % vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator%(const S &s, const vec4<V> &v) {return vec4<S>(s) % v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator^(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x ^ b.x)> {return {a.x ^ b.x, a.y ^ b.y, a.z ^ b.z, a.w ^ b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator^(const vec4<V> &v, const S &s) {return v ^ vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator^(const S &s, const vec4<V> &v) {return vec4<S>(s) ^ v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator&(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x & b.x)> {return {a.x & b.x, a.y & b.y, a.z & b.z, a.w & b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator&(const vec4<V> &v, const S &s) {return v & vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator&(const S &s, const vec4<V> &v) {return vec4<S>(s) & v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator|(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x | b.x)> {return {a.x | b.x, a.y | b.y, a.z | b.z, a.w | b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator|(const vec4<V> &v, const S &s) {return v | vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator|(const S &s, const vec4<V> &v) {return vec4<S>(s) | v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<<(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x << b.x)> {return {a.x << b.x, a.y << b.y, a.z << b.z, a.w << b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<<(const vec4<V> &v, const S &s) {return v << vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<<(const S &s, const vec4<V> &v) {return vec4<S>(s) << v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>>(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x >> b.x)> {return {a.x >> b.x, a.y >> b.y, a.z >> b.z, a.w >> b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>>(const vec4<V> &v, const S &s) {return v >> vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>>(const S &s, const vec4<V> &v) {return vec4<S>(s) >> v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x < b.x)> {return {a.x < b.x, a.y < b.y, a.z < b.z, a.w < b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<(const vec4<V> &v, const S &s) {return v < vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<(const S &s, const vec4<V> &v) {return vec4<S>(s) < v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x > b.x)> {return {a.x > b.x, a.y > b.y, a.z > b.z, a.w > b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>(const vec4<V> &v, const S &s) {return v > vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>(const S &s, const vec4<V> &v) {return vec4<S>(s) > v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator<=(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x <= b.x)> {return {a.x <= b.x, a.y <= b.y, a.z <= b.z, a.w <= b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator<=(const vec4<V> &v, const S &s) {return v <= vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator<=(const S &s, const vec4<V> &v) {return vec4<S>(s) <= v;}
        template <typename A, typename B> [[nodiscard]] constexpr auto operator>=(const vec4<A> &a, const vec4<B> &b) -> vec4<decltype(a.x >= b.x)> {return {a.x >= b.x, a.y >= b.y, a.z >= b.z, a.w >= b.w};}
        template <typename V, scalar S> [[nodiscard]] constexpr auto operator>=(const vec4<V> &v, const S &s) {return v >= vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr auto operator>=(const S &s, const vec4<V> &v) {return vec4<S>(s) >= v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator==(const vec4<A> &a, const vec4<B> &b) {return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;}
        template <typename V, scalar S> [[nodiscard]] constexpr bool operator==(const vec4<V> &v, const S &s) {return v == vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr bool operator==(const S &s, const vec4<V> &v) {return vec4<S>(s) == v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator!=(const vec4<A> &a, const vec4<B> &b) {return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;}
        template <typename V, scalar S> [[nodiscard]] constexpr bool operator!=(const vec4<V> &v, const S &s) {return v != vec4<S>(s);}
        template <scalar S, typename V> [[nodiscard]] constexpr bool operator!=(const S &s, const vec4<V> &v) {return vec4<S>(s) != v;}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const vec4<A> &a, const vec4<B> &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const vec4<A> &a, const B &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator&&(const A &a, const vec4<B> &b) {return bool(a) && bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const vec4<A> &a, const vec4<B> &b) {return bool(a) || bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const vec4<A> &a, const B &b) {return bool(a) || bool(b);}
        template <typename A, typename B> [[nodiscard]] constexpr bool operator||(const A &a, const vec4<B> &b) {return bool(a) || bool(b);}
        template <typename T> [[nodiscard]] constexpr auto operator~(const vec4<T> &v) -> vec4<decltype(~v.x)> {return {~v.x, ~v.y, ~v.z, ~v.w};}
        template <typename T> [[nodiscard]] constexpr auto operator+(const vec4<T> &v) -> vec4<decltype(+v.x)> {return {+v.x, +v.y, +v.z, +v.w};}
        template <typename T> [[nodiscard]] constexpr auto operator-(const vec4<T> &v) -> vec4<decltype(-v.x)> {return {-v.x, -v.y, -v.z, -v.w};}
        template <typename T> [[nodiscard]] constexpr bool operator!(const vec4<T> &v) {return !bool(v);}
        template <typename T> constexpr vec4<T> &operator++(vec4<T> &v) {++v.x; ++v.y; ++v.z; ++v.w; return v;}
        template <typename T> constexpr vec4<T> operator++(vec4<T> &v, int) {return {v.x++, v.y++, v.z++, v.w++};}
        template <typename T> constexpr vec4<T> &operator--(vec4<T> &v) {--v.x; --v.y; --v.z; --v.w; return v;}
        template <typename T> constexpr vec4<T> operator--(vec4<T> &v, int) {return {v.x--, v.y--, v.z--, v.w--};}
        template <typename A, typename B> constexpr vec4<A> &operator+=(vec4<A> &a, const vec4<B> &b) {a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator+=(vec4<V> &v, const S &s) {return v += vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator-=(vec4<A> &a, const vec4<B> &b) {a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator-=(vec4<V> &v, const S &s) {return v -= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator*=(vec4<A> &a, const vec4<B> &b) {a.x *= b.x; a.y *= b.y; a.z *= b.z; a.w *= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator*=(vec4<V> &v, const S &s) {return v *= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator/=(vec4<A> &a, const vec4<B> &b) {a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator/=(vec4<V> &v, const S &s) {return v /= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator%=(vec4<A> &a, const vec4<B> &b) {a.x %= b.x; a.y %= b.y; a.z %= b.z; a.w %= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator%=(vec4<V> &v, const S &s) {return v %= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator^=(vec4<A> &a, const vec4<B> &b) {a.x ^= b.x; a.y ^= b.y; a.z ^= b.z; a.w ^= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator^=(vec4<V> &v, const S &s) {return v ^= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator&=(vec4<A> &a, const vec4<B> &b) {a.x &= b.x; a.y &= b.y; a.z &= b.z; a.w &= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator&=(vec4<V> &v, const S &s) {return v &= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator|=(vec4<A> &a, const vec4<B> &b) {a.x |= b.x; a.y |= b.y; a.z |= b.z; a.w |= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator|=(vec4<V> &v, const S &s) {return v |= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator<<=(vec4<A> &a, const vec4<B> &b) {a.x <<= b.x; a.y <<= b.y; a.z <<= b.z; a.w <<= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator<<=(vec4<V> &v, const S &s) {return v <<= vec4<S>(s);}
        template <typename A, typename B> constexpr vec4<A> &operator>>=(vec4<A> &a, const vec4<B> &b) {a.x >>= b.x; a.y >>= b.y; a.z >>= b.z; a.w >>= b.w; return a;}
        template <typename V, scalar S> constexpr vec4<V> &operator>>=(vec4<V> &v, const S &s) {return v >>= vec4<S>(s);}
        //}  vec4

        //{  input/output
        template <typename A, typename B, int D, typename T> std::basic_ostream<A,B> &operator<<(std::basic_ostream<A,B> &s, const vec<D,T> &v)
        {
            s.width(0);
            s << '[';
            for (int i = 0; i < D; i++)
            {
                if (i != 0)
                    s << ',';
                s << v[i];
            }
            s << ']';
            return s;
        }
        template <typename A, typename B, int W, int H, typename T> std::basic_ostream<A,B> &operator<<(std::basic_ostream<A,B> &s, const mat<W,H,T> &v)
        {
            s.width(0);
            s << '[';
            for (int y = 0; y < H; y++)
            {
                if (y != 0)
                    s << ';';
                for (int x = 0; x < W; x++)
                {
                    if (x != 0)
                        s << ',';
                    s << v[x][y];
                }
            }
            s << ']';
            return s;
        }
        template <typename A, typename B, int D, typename T> std::basic_istream<A,B> &operator>>(std::basic_istream<A,B> &s, vec<D,T> &v)
        {
            s.width(0);
            for (int i = 0; i < D; i++)
                s >> v[i];
            return s;
        }
        template <typename A, typename B, int W, int H, typename T> std::basic_istream<A,B> &operator>>(std::basic_istream<A,B> &s, mat<W,H,T> &v)
        {
            s.width(0);
            for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                s >> v[x][y];
            return s;
        }
        //}  input/output

        //{  matrix multiplication
        template <typename A, typename B> [[nodiscard]] constexpr vec2<larger_t<A,B>> operator*(const mat2x2<A> &a, const vec2<B> &b) {return {a.x.x*b.x + a.y.x*b.y, a.x.y*b.x + a.y.y*b.y};}
        template <typename A, typename B> [[nodiscard]] constexpr vec2<larger_t<A,B>> operator*(const mat3x2<A> &a, const vec3<B> &b) {return {a.x.x*b.x + a.y.x*b.y + a.z.x*b.z, a.x.y*b.x + a.y.y*b.y + a.z.y*b.z};}
        template <typename A, typename B> [[nodiscard]] constexpr vec2<larger_t<A,B>> operator*(const mat4x2<A> &a, const vec4<B> &b) {return {a.x.x*b.x + a.y.x*b.y + a.z.x*b.z + a.w.x*b.w, a.x.y*b.x + a.y.y*b.y + a.z.y*b.z + a.w.y*b.w};}
        template <typename A, typename B> [[nodiscard]] constexpr vec3<larger_t<A,B>> operator*(const mat2x3<A> &a, const vec2<B> &b) {return {a.x.x*b.x + a.y.x*b.y, a.x.y*b.x + a.y.y*b.y, a.x.z*b.x + a.y.z*b.y};}
        template <typename A, typename B> [[nodiscard]] constexpr vec3<larger_t<A,B>> operator*(const mat3x3<A> &a, const vec3<B> &b) {return {a.x.x*b.x + a.y.x*b.y + a.z.x*b.z, a.x.y*b.x + a.y.y*b.y + a.z.y*b.z, a.x.z*b.x + a.y.z*b.y + a.z.z*b.z};}
        template <typename A, typename B> [[nodiscard]] constexpr vec3<larger_t<A,B>> operator*(const mat4x3<A> &a, const vec4<B> &b) {return {a.x.x*b.x + a.y.x*b.y + a.z.x*b.z + a.w.x*b.w, a.x.y*b.x + a.y.y*b.y + a.z.y*b.z + a.w.y*b.w, a.x.z*b.x + a.y.z*b.y + a.z.z*b.z + a.w.z*b.w};}
        template <typename A, typename B> [[nodiscard]] constexpr vec4<larger_t<A,B>> operator*(const mat2x4<A> &a, const vec2<B> &b) {return {a.x.x*b.x + a.y.x*b.y, a.x.y*b.x + a.y.y*b.y, a.x.z*b.x + a.y.z*b.y, a.x.w*b.x + a.y.w*b.y};}
        template <typename A, typename B> [[nodiscard]] constexpr vec4<larger_t<A,B>> operator*(const mat3x4<A> &a, const vec3<B> &b) {return {a.x.x*b.x + a.y.x*b.y + a.z.x*b.z, a.x.y*b.x + a.y.y*b.y + a.z.y*b.z, a.x.z*b.x + a.y.z*b.y + a.z.z*b.z, a.x.w*b.x + a.y.w*b.y + a.z.w*b.z};}
        template <typename A, typename B> [[nodiscard]] constexpr vec4<larger_t<A,B>> operator*(const mat4x4<A> &a, const vec4<B> &b) {return {a.x.x*b.x + a.y.x*b.y + a.z.x*b.z + a.w.x*b.w, a.x.y*b.x + a.y.y*b.y + a.z.y*b.z + a.w.y*b.w, a.x.z*b.x + a.y.z*b.y + a.z.z*b.z + a.w.z*b.w, a.x.w*b.x + a.y.w*b.y + a.z.w*b.z + a.w.w*b.w};}
        template <typename A, typename B> [[nodiscard]] constexpr vec2<larger_t<A,B>> operator*(const vec2<A> &a, const mat2x2<B> &b) {return {a.x*b.x.x + a.y*b.x.y, a.x*b.y.x + a.y*b.y.y};}
        template <typename A, typename B> [[nodiscard]] constexpr vec2<larger_t<A,B>> operator*(const vec3<A> &a, const mat2x3<B> &b) {return {a.x*b.x.x + a.y*b.x.y + a.z*b.x.z, a.x*b.y.x + a.y*b.y.y + a.z*b.y.z};}
        template <typename A, typename B> [[nodiscard]] constexpr vec2<larger_t<A,B>> operator*(const vec4<A> &a, const mat2x4<B> &b) {return {a.x*b.x.x + a.y*b.x.y + a.z*b.x.z + a.w*b.x.w, a.x*b.y.x + a.y*b.y.y + a.z*b.y.z + a.w*b.y.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x2<larger_t<A,B>> operator*(const mat2x2<A> &a, const mat2x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x2<larger_t<A,B>> operator*(const mat3x2<A> &a, const mat2x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x2<larger_t<A,B>> operator*(const mat4x2<A> &a, const mat2x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x3<larger_t<A,B>> operator*(const mat2x3<A> &a, const mat2x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.z*b.x.x + a.y.z*b.x.y, a.x.z*b.y.x + a.y.z*b.y.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x3<larger_t<A,B>> operator*(const mat3x3<A> &a, const mat2x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x3<larger_t<A,B>> operator*(const mat4x3<A> &a, const mat2x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z + a.w.z*b.x.w, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z + a.w.z*b.y.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x4<larger_t<A,B>> operator*(const mat2x4<A> &a, const mat2x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.z*b.x.x + a.y.z*b.x.y, a.x.z*b.y.x + a.y.z*b.y.y, a.x.w*b.x.x + a.y.w*b.x.y, a.x.w*b.y.x + a.y.w*b.y.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x4<larger_t<A,B>> operator*(const mat3x4<A> &a, const mat2x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z, a.x.w*b.x.x + a.y.w*b.x.y + a.z.w*b.x.z, a.x.w*b.y.x + a.y.w*b.y.y + a.z.w*b.y.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat2x4<larger_t<A,B>> operator*(const mat4x4<A> &a, const mat2x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z + a.w.z*b.x.w, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z + a.w.z*b.y.w, a.x.w*b.x.x + a.y.w*b.x.y + a.z.w*b.x.z + a.w.w*b.x.w, a.x.w*b.y.x + a.y.w*b.y.y + a.z.w*b.y.z + a.w.w*b.y.w};}
        template <typename A, typename B> [[nodiscard]] constexpr vec3<larger_t<A,B>> operator*(const vec2<A> &a, const mat3x2<B> &b) {return {a.x*b.x.x + a.y*b.x.y, a.x*b.y.x + a.y*b.y.y, a.x*b.z.x + a.y*b.z.y};}
        template <typename A, typename B> [[nodiscard]] constexpr vec3<larger_t<A,B>> operator*(const vec3<A> &a, const mat3x3<B> &b) {return {a.x*b.x.x + a.y*b.x.y + a.z*b.x.z, a.x*b.y.x + a.y*b.y.y + a.z*b.y.z, a.x*b.z.x + a.y*b.z.y + a.z*b.z.z};}
        template <typename A, typename B> [[nodiscard]] constexpr vec3<larger_t<A,B>> operator*(const vec4<A> &a, const mat3x4<B> &b) {return {a.x*b.x.x + a.y*b.x.y + a.z*b.x.z + a.w*b.x.w, a.x*b.y.x + a.y*b.y.y + a.z*b.y.z + a.w*b.y.w, a.x*b.z.x + a.y*b.z.y + a.z*b.z.z + a.w*b.z.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x2<larger_t<A,B>> operator*(const mat2x2<A> &a, const mat3x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.x*b.z.x + a.y.x*b.z.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.y*b.z.x + a.y.y*b.z.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x2<larger_t<A,B>> operator*(const mat3x2<A> &a, const mat3x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x2<larger_t<A,B>> operator*(const mat4x2<A> &a, const mat3x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z + a.w.x*b.z.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z + a.w.y*b.z.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x3<larger_t<A,B>> operator*(const mat2x3<A> &a, const mat3x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.x*b.z.x + a.y.x*b.z.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.y*b.z.x + a.y.y*b.z.y, a.x.z*b.x.x + a.y.z*b.x.y, a.x.z*b.y.x + a.y.z*b.y.y, a.x.z*b.z.x + a.y.z*b.z.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x3<larger_t<A,B>> operator*(const mat3x3<A> &a, const mat3x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x3<larger_t<A,B>> operator*(const mat4x3<A> &a, const mat3x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z + a.w.x*b.z.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z + a.w.y*b.z.w, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z + a.w.z*b.x.w, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z + a.w.z*b.y.w, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z + a.w.z*b.z.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x4<larger_t<A,B>> operator*(const mat2x4<A> &a, const mat3x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.x*b.z.x + a.y.x*b.z.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.y*b.z.x + a.y.y*b.z.y, a.x.z*b.x.x + a.y.z*b.x.y, a.x.z*b.y.x + a.y.z*b.y.y, a.x.z*b.z.x + a.y.z*b.z.y, a.x.w*b.x.x + a.y.w*b.x.y, a.x.w*b.y.x + a.y.w*b.y.y, a.x.w*b.z.x + a.y.w*b.z.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x4<larger_t<A,B>> operator*(const mat3x4<A> &a, const mat3x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z, a.x.w*b.x.x + a.y.w*b.x.y + a.z.w*b.x.z, a.x.w*b.y.x + a.y.w*b.y.y + a.z.w*b.y.z, a.x.w*b.z.x + a.y.w*b.z.y + a.z.w*b.z.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat3x4<larger_t<A,B>> operator*(const mat4x4<A> &a, const mat3x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z + a.w.x*b.z.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z + a.w.y*b.z.w, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z + a.w.z*b.x.w, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z + a.w.z*b.y.w, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z + a.w.z*b.z.w, a.x.w*b.x.x + a.y.w*b.x.y + a.z.w*b.x.z + a.w.w*b.x.w, a.x.w*b.y.x + a.y.w*b.y.y + a.z.w*b.y.z + a.w.w*b.y.w, a.x.w*b.z.x + a.y.w*b.z.y + a.z.w*b.z.z + a.w.w*b.z.w};}
        template <typename A, typename B> [[nodiscard]] constexpr vec4<larger_t<A,B>> operator*(const vec2<A> &a, const mat4x2<B> &b) {return {a.x*b.x.x + a.y*b.x.y, a.x*b.y.x + a.y*b.y.y, a.x*b.z.x + a.y*b.z.y, a.x*b.w.x + a.y*b.w.y};}
        template <typename A, typename B> [[nodiscard]] constexpr vec4<larger_t<A,B>> operator*(const vec3<A> &a, const mat4x3<B> &b) {return {a.x*b.x.x + a.y*b.x.y + a.z*b.x.z, a.x*b.y.x + a.y*b.y.y + a.z*b.y.z, a.x*b.z.x + a.y*b.z.y + a.z*b.z.z, a.x*b.w.x + a.y*b.w.y + a.z*b.w.z};}
        template <typename A, typename B> [[nodiscard]] constexpr vec4<larger_t<A,B>> operator*(const vec4<A> &a, const mat4x4<B> &b) {return {a.x*b.x.x + a.y*b.x.y + a.z*b.x.z + a.w*b.x.w, a.x*b.y.x + a.y*b.y.y + a.z*b.y.z + a.w*b.y.w, a.x*b.z.x + a.y*b.z.y + a.z*b.z.z + a.w*b.z.w, a.x*b.w.x + a.y*b.w.y + a.z*b.w.z + a.w*b.w.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x2<larger_t<A,B>> operator*(const mat2x2<A> &a, const mat4x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.x*b.z.x + a.y.x*b.z.y, a.x.x*b.w.x + a.y.x*b.w.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.y*b.z.x + a.y.y*b.z.y, a.x.y*b.w.x + a.y.y*b.w.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x2<larger_t<A,B>> operator*(const mat3x2<A> &a, const mat4x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z, a.x.x*b.w.x + a.y.x*b.w.y + a.z.x*b.w.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z, a.x.y*b.w.x + a.y.y*b.w.y + a.z.y*b.w.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x2<larger_t<A,B>> operator*(const mat4x2<A> &a, const mat4x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z + a.w.x*b.z.w, a.x.x*b.w.x + a.y.x*b.w.y + a.z.x*b.w.z + a.w.x*b.w.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z + a.w.y*b.z.w, a.x.y*b.w.x + a.y.y*b.w.y + a.z.y*b.w.z + a.w.y*b.w.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x3<larger_t<A,B>> operator*(const mat2x3<A> &a, const mat4x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.x*b.z.x + a.y.x*b.z.y, a.x.x*b.w.x + a.y.x*b.w.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.y*b.z.x + a.y.y*b.z.y, a.x.y*b.w.x + a.y.y*b.w.y, a.x.z*b.x.x + a.y.z*b.x.y, a.x.z*b.y.x + a.y.z*b.y.y, a.x.z*b.z.x + a.y.z*b.z.y, a.x.z*b.w.x + a.y.z*b.w.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x3<larger_t<A,B>> operator*(const mat3x3<A> &a, const mat4x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z, a.x.x*b.w.x + a.y.x*b.w.y + a.z.x*b.w.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z, a.x.y*b.w.x + a.y.y*b.w.y + a.z.y*b.w.z, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z, a.x.z*b.w.x + a.y.z*b.w.y + a.z.z*b.w.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x3<larger_t<A,B>> operator*(const mat4x3<A> &a, const mat4x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z + a.w.x*b.z.w, a.x.x*b.w.x + a.y.x*b.w.y + a.z.x*b.w.z + a.w.x*b.w.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z + a.w.y*b.z.w, a.x.y*b.w.x + a.y.y*b.w.y + a.z.y*b.w.z + a.w.y*b.w.w, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z + a.w.z*b.x.w, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z + a.w.z*b.y.w, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z + a.w.z*b.z.w, a.x.z*b.w.x + a.y.z*b.w.y + a.z.z*b.w.z + a.w.z*b.w.w};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x4<larger_t<A,B>> operator*(const mat2x4<A> &a, const mat4x2<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y, a.x.x*b.y.x + a.y.x*b.y.y, a.x.x*b.z.x + a.y.x*b.z.y, a.x.x*b.w.x + a.y.x*b.w.y, a.x.y*b.x.x + a.y.y*b.x.y, a.x.y*b.y.x + a.y.y*b.y.y, a.x.y*b.z.x + a.y.y*b.z.y, a.x.y*b.w.x + a.y.y*b.w.y, a.x.z*b.x.x + a.y.z*b.x.y, a.x.z*b.y.x + a.y.z*b.y.y, a.x.z*b.z.x + a.y.z*b.z.y, a.x.z*b.w.x + a.y.z*b.w.y, a.x.w*b.x.x + a.y.w*b.x.y, a.x.w*b.y.x + a.y.w*b.y.y, a.x.w*b.z.x + a.y.w*b.z.y, a.x.w*b.w.x + a.y.w*b.w.y};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x4<larger_t<A,B>> operator*(const mat3x4<A> &a, const mat4x3<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z, a.x.x*b.w.x + a.y.x*b.w.y + a.z.x*b.w.z, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z, a.x.y*b.w.x + a.y.y*b.w.y + a.z.y*b.w.z, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z, a.x.z*b.w.x + a.y.z*b.w.y + a.z.z*b.w.z, a.x.w*b.x.x + a.y.w*b.x.y + a.z.w*b.x.z, a.x.w*b.y.x + a.y.w*b.y.y + a.z.w*b.y.z, a.x.w*b.z.x + a.y.w*b.z.y + a.z.w*b.z.z, a.x.w*b.w.x + a.y.w*b.w.y + a.z.w*b.w.z};}
        template <typename A, typename B> [[nodiscard]] constexpr mat4x4<larger_t<A,B>> operator*(const mat4x4<A> &a, const mat4x4<B> &b) {return {a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z + a.w.x*b.x.w, a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z + a.w.x*b.y.w, a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z + a.w.x*b.z.w, a.x.x*b.w.x + a.y.x*b.w.y + a.z.x*b.w.z + a.w.x*b.w.w, a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z + a.w.y*b.x.w, a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z + a.w.y*b.y.w, a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z + a.w.y*b.z.w, a.x.y*b.w.x + a.y.y*b.w.y + a.z.y*b.w.z + a.w.y*b.w.w, a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z + a.w.z*b.x.w, a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z + a.w.z*b.y.w, a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z + a.w.z*b.z.w, a.x.z*b.w.x + a.y.z*b.w.y + a.z.z*b.w.z + a.w.z*b.w.w, a.x.w*b.x.x + a.y.w*b.x.y + a.z.w*b.x.z + a.w.w*b.x.w, a.x.w*b.y.x + a.y.w*b.y.y + a.z.w*b.y.z + a.w.w*b.y.w, a.x.w*b.z.x + a.y.w*b.z.y + a.z.w*b.z.z + a.w.w*b.z.w, a.x.w*b.w.x + a.y.w*b.w.y + a.z.w*b.w.z + a.w.w*b.w.w};}

        template <typename A, typename B, int D> constexpr vec<D,A> &operator*=(vec<D,A> &a, const mat<D,D,B> &b) {a = a * b; return a;}
        template <typename A, typename B, int W, int H> constexpr mat<W,H,A> &operator*=(mat<W,H,A> &a, const mat<W,W,B> &b) {a = a * b; return a;}
        //}  matrix multiplication
        //} Operators
    }

    inline namespace Utility // Low-level helper functions
    {
        //{ Member access
        // Returns I-th vector element. This function considers scalars to be 1-element vectors.
        // Returns a non-const reference only if the parameter is a non-const lvalue; otherwise returns a const reference.
        template <int I, typename T> constexpr auto &get_vec_element(T &&vec)
        {
            static_assert(I >= 0 && I < 4);
            constexpr bool not_const = std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>;
            if constexpr (!vector<std::remove_reference_t<T>>)
                return std::conditional_t<not_const, T &, const T &>(vec);
            else
                return std::conditional_t<not_const, vec_base_t<std::remove_reference_t<T>> &, const vec_base_t<std::remove_reference_t<T>> &>(vec.template get<I>());
        }

        // A simple constexpr `for` loop.
        template <int D, typename F> constexpr void cexpr_for(F &&func)
        {
            static_assert(D >= 1 && D <= 4);
            func(std::integral_constant<int,0>{});
            if constexpr (D > 1) func(std::integral_constant<int,1>{});
            if constexpr (D > 2) func(std::integral_constant<int,2>{});
            if constexpr (D > 3) func(std::integral_constant<int,3>{});
        }
        //} Member access

        //{ Custom operators
        struct op_type_dot {};
        struct op_type_cross {};

        template <typename A> struct op_expr_type_dot
        {
            A &&a;
            template <typename B> [[nodiscard]] constexpr decltype(auto) operator/(B &&b) {return std::forward<A>(a).dot(std::forward<B>(b));}
            template <typename B> constexpr decltype(auto) operator/=(B &&b) {a = std::forward<A>(a).dot(std::forward<B>(b)); return std::forward<A>(a);}
        };
        template <typename A> struct op_expr_type_cross
        {
            A &&a;
            template <typename B> [[nodiscard]] constexpr decltype(auto) operator/(B &&b) {return std::forward<A>(a).cross(std::forward<B>(b));}
            template <typename B> constexpr decltype(auto) operator/=(B &&b) {a = std::forward<A>(a).cross(std::forward<B>(b)); return std::forward<A>(a);}
        };

        template <typename T> inline constexpr op_expr_type_dot<T> operator/(T &&param, op_type_dot) {return {std::forward<T>(param)};}
        template <typename T> inline constexpr op_expr_type_cross<T> operator/(T &&param, op_type_cross) {return {std::forward<T>(param)};}
        //} Custom operators

        //{ Ranges
        template <typename T> class vector_range
        {
            static_assert(vector<T> && !std::is_const_v<T> && std::is_integral_v<vec_base_t<T>>, "The template parameter must be an integral vector.");

            T vec_begin = T(0);
            T vec_end = T(0);

          public:
            class iterator
            {
                friend class vector_range<T>;

                T vec_begin = T(0);
                T vec_end = T(0);
                T vec_cur = T(0);
                bool finished = 1;

                iterator(T vec_begin, T vec_end) : vec_begin(vec_begin), vec_end(vec_end), vec_cur(vec_begin), finished((vec_begin >= vec_end).any()) {}

              public:
                using difference_type   = std::ptrdiff_t;
                using value_type        = T;
                using pointer           = const T *;
                using reference         = const T &;
                using iterator_category = std::forward_iterator_tag;

                iterator() {}

                iterator &operator++()
                {
                    bool stop = 0;
                    cexpr_for<vec_size_v<T>>([&](auto index)
                    {
                        if (stop)
                            return;

                        constexpr int i = index.value;

                        auto &elem = get_vec_element<i>(vec_cur);
                        elem++;
                        if (elem >= get_vec_element<i>(vec_end))
                        {
                            elem = get_vec_element<i>(vec_begin);

                            if constexpr (i == vec_size_v<T> - 1)
                                finished = 1;
                        }
                        else
                        {
                            stop = 1;
                        }
                    });

                    return *this;
                }
                iterator operator++(int)
                {
                    iterator ret = *this;
                    ++(*this);
                    return ret;
                }

                reference operator*() const
                {
                    return vec_cur;
                }
                pointer operator->() const
                {
                    return &vec_cur;
                }

                bool operator==(const iterator &other) const
                {
                    if (finished != other.finished)
                        return 0;
                    if (finished && other.finished)
                        return 1;
                    return vec_cur == other.vec_cur;
                }
                bool operator!=(const iterator &other) const
                {
                    return !(*this == other);
                }
            };

            vector_range() {}
            vector_range(T vec_begin, T vec_end) : vec_begin(vec_begin), vec_end(vec_end) {}

            iterator begin() const
            {
                return iterator(vec_begin, vec_end);
            }

            iterator end() const
            {
                return {};
            }

            template <int A, typename B> friend vector_range operator+(const vector_range &range, vec<A,B> offset)
            {
                static_assert(std::is_same_v<T, vec<A,B>>, "The offset must have exactly the same type as the range.");
                return vector_range(range.vec_begin + offset, range.vec_end + offset);
            }
            template <int A, typename B> friend vector_range operator+(vec<A,B> offset, const vector_range &range)
            {
                return range + offset;
            }
        };

        template <typename T> class vector_range_halfbound
        {
            static_assert(vector<T> && !std::is_const_v<T> && std::is_integral_v<vec_base_t<T>>, "The template parameter must be an integral vector.");

            T vec_begin = T(0);

          public:
            vector_range_halfbound(T vec_begin) : vec_begin(vec_begin) {}

            template <int A, typename B> friend vector_range<T> operator<(const vector_range_halfbound &range, vec<A,B> point)
            {
                static_assert(std::is_same_v<T, vec<A,B>>, "The upper limit must have exactly the same type as the lower limit.");
                return vector_range<T>(range.vec_begin, point);
            }
            template <int A, typename B> friend vector_range<T> operator<=(const vector_range_halfbound &range, vec<A,B> point)
            {
                return range < point+1;
            }
        };

        struct vector_range_factory
        {
            template <int A, typename B> vector_range<vec<A,B>> operator()(vec<A,B> size) const
            {
                return vector_range<vec<A,B>>(vec<A,B>(0), size);
            }

            template <int A, typename B> friend vector_range_halfbound<vec<A,B>> operator<=(vec<A,B> point, vector_range_factory)
            {
                return {point};
            }
            template <int A, typename B> friend vector_range_halfbound<vec<A,B>> operator<(vec<A,B> point, vector_range_factory)
            {
                return point+1 <= vector_range_factory{};
            }
        };
        //} Ranges
    }

    inline namespace Common // Common functions
    {
        // Named operators.
        inline constexpr op_type_dot dot;
        inline constexpr op_type_cross cross;

        // Helper class for writing nested loops.
        // Example usage:
        //   for (auto v : vec_a <= vector_range <= vec_b) // `<` are also allowed, in one or both positions.
        //   for (auto v : vector_range(vec_a)) // Equivalent to `vec..(0) <= vector_range < vec_a`.
        inline constexpr vector_range_factory vector_range;

        // Helper for applying a function to one or several scalars or vectors.
        // Mixing scalars and vectors is allowed, but vectors must have the same size.
        // If at least one vector is passed, the result is also a vector.
        // If `D != 1`, forces the result to be the vector of this size, or causes a hard error if not possible.
        template <int D = 1, typename F, typename ...P>
        requires have_common_vec_size<D, vec_size_v<P>...>
        constexpr auto apply_elementwise(F &&func, P &&... params)
        {
            constexpr int size = common_vec_size_v<D, vec_size_v<std::remove_reference_t<P>>...>;

            using ret_type = decltype(std::declval<F>()(get_vec_element<0>(std::declval<P>())...));

            if constexpr (std::is_void_v<ret_type>)
            {
                cexpr_for<size>([&](auto index)
                {
                    func(get_vec_element<index.value>(params)...); // No forwarding to prevent moving.
                });
                return void();
            }
            else
            {
                vec_or_scalar_t<size, ret_type> ret{};
                cexpr_for<size>([&](auto index)
                {
                    get_vec_element<index.value>(ret) = func(get_vec_element<index.value>(params)...); // No forwarding to prevent moving.
                });
                return ret;
            }
        }

        // The value of pi.
        template <typename T> [[nodiscard]] constexpr T pi() {return T(3.14159265358979323846l);}
        constexpr float       f_pi  = pi<float>();
        constexpr double      d_pi  = pi<double>();
        constexpr long double ld_pi = pi<long double>();

        // Conversions between degrees and radians.
        template <typename T> [[nodiscard]] constexpr auto to_rad(T in)
        {
            using fp_t = floating_point_t<T>;
            return in * pi<fp_t>() / fp_t(180);
        }
        template <typename T> [[nodiscard]] constexpr auto to_deg(T in)
        {
            using fp_t = floating_point_t<T>;
            return in * fp_t(180) / pi<fp_t>();
        }

        // Returns the sign of the argument as `int` or `ivecN`.
        template <typename T> [[nodiscard]] constexpr change_vec_base_t<T,int> sign(T val)
        {
            // Works on scalars and vectors.
            return (val > 0) - (val < 0);
        }

        // `clamp[_var][_min|_max|_abs] (value, min, max)`.
        // Clamps scalars or vectors.
        // `_var` functions modify the first parameter instead of returning the result.
        // `_min` functions don't have a `max` parameter, and vice versa.
        // `_abs` functions don't have a `min` parameter, they use `-max` as `min`.
        // If both `min` and `max` are omitted, 0 and 1 are assumed.
        // If bounds contradict each other, only the `max` bound is used.

        template <typename A, typename B> constexpr void clamp_var_min(A &var, B min)
        {
            static_assert(vector<B> <= vector<A>, "If `min` is a vector, `var` has to be a vector as well.");
            static_assert(std::is_floating_point_v<vec_base_t<B>> <= std::is_floating_point_v<vec_base_t<A>>, "If `min` is a floating-point, `var` has to be floating-point as well.");
            static_assert(std::is_floating_point_v<vec_base_t<A>> || std::is_signed_v<vec_base_t<A>> == std::is_signed_v<vec_base_t<B>>, "If both arguments are integral, they must have the same signedness.");

            if constexpr (no_vectors_v<A,B>)
            {
                if (!(var >= min)) // The condition is written like this to catch NaNs, they always compare to false.
                    var = min;
            }
            else
            {
                apply_elementwise(clamp_var_min<vec_base_t<A>, vec_base_t<B>>, var, min);
            }
        }

        template <typename A, typename B> constexpr void clamp_var_max(A &var, B max)
        {
            static_assert(vector<B> <= vector<A>, "If `max` is a vector, `var` has to be a vector as well.");
            static_assert(std::is_floating_point_v<vec_base_t<B>> <= std::is_floating_point_v<vec_base_t<A>>, "If `max` is a floating-point, `var` has to be floating-point as well.");
            static_assert(std::is_floating_point_v<vec_base_t<A>> || std::is_signed_v<vec_base_t<A>> == std::is_signed_v<vec_base_t<B>>, "If both arguments are integral, they must have the same signedness.");

            if constexpr (no_vectors_v<A,B>)
            {
                if (!(var <= max)) // The condition is written like this to catch NaNs, they always compare to false.
                    var = max;
            }
            else
            {
                apply_elementwise(clamp_var_max<vec_base_t<A>, vec_base_t<B>>, var, max);
            }
        }

        template <typename A, typename B, typename C> constexpr void clamp_var(A &var, B min, C max)
        {
            clamp_var_min(var, min);
            clamp_var_max(var, max);
        }

        template <typename A, typename B> constexpr void clamp_var_abs(A &var, B abs_max)
        {
            static_assert(std::is_signed_v<vec_base_t<B>>, "`abs_max` must be signed."); // This allows floating-point types too.
            clamp_var(var, -abs_max, abs_max);
        }

        template <typename A, typename B> [[nodiscard]] constexpr A clamp_min(A val, B min)
        {
            clamp_var_min(val, min);
            return val;
        }

        template <typename A, typename B> [[nodiscard]] constexpr A clamp_max(A val, B max)
        {
            clamp_var_max(val, max);
            return val;
        }

        template <typename A, typename B, typename C> [[nodiscard]] constexpr A clamp(A val, B min, C max)
        {
            clamp_var(val, min, max);
            return val;
        }

        template <typename A, typename B> [[nodiscard]] constexpr A clamp_abs(A val, B abs_max)
        {
            clamp_var_abs(val, abs_max);
            return val;
        }

        template <typename A> [[nodiscard]] constexpr A clamp(A val) {return clamp(val, 0, 1);}
        template <typename A> [[nodiscard]] constexpr A clamp_min(A val) {return clamp_min(val, 0);}
        template <typename A> [[nodiscard]] constexpr A clamp_max(A val) {return clamp_max(val, 1);}
        template <typename A> [[nodiscard]] constexpr A clamp_abs(A val) {return clamp_abs(val, 1);}
        template <typename A> constexpr void clamp_var(A &var) {clamp_var(var, 0, 1);}
        template <typename A> constexpr void clamp_var_min(A &var) {clamp_var_min(var, 0);}
        template <typename A> constexpr void clamp_var_max(A &var) {clamp_var_max(var, 1);}
        template <typename A> constexpr void clamp_var_abs(A &var) {clamp_var_abs(var, 1);}

        // Rounds a floating-point scalar or vector.
        // Returns an integral type (`int` by default).
        template <typename I = int, typename F> [[nodiscard]] change_vec_base_t<F,I> iround(F x)
        {
            static_assert(std::is_floating_point_v<vec_base_t<F>>, "Argument must be floating-point.");
            static_assert(std::is_integral_v<I> && std::is_signed_v<I>, "Template argument must be integral and signed.");

            if constexpr(no_vectors_v<F>)
            {
                if constexpr (sizeof (I) <= sizeof (long))
                    return std::lround(x);
                else
                    return std::llround(x);
            }
            else
            {
                return apply_elementwise(iround<I, vec_base_t<F>>, x);
            }
        }

        // Various useful functions.
        // Some of them are imported from `std` and extended to operate on vectors. Some are custom.

        using std::abs;
        template <typename T, std::enable_if_t<!no_vectors_v<T>, std::nullptr_t> = nullptr>
        [[nodiscard]] T abs(T x)
        {
            return apply_elementwise([](auto val){return std::abs(val);}, x);
        }

        using std::round;
        template <typename T, std::enable_if_t<!no_vectors_v<T>, std::nullptr_t> = nullptr>
        [[nodiscard]] T round(T x)
        {
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Argument must be floating-point.");
            return apply_elementwise([](auto val){return std::round(val);}, x);
        }

        using std::floor;
        template <typename T, std::enable_if_t<!no_vectors_v<T>, std::nullptr_t> = nullptr>
        [[nodiscard]] T floor(T x)
        {
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Argument must be floating-point.");
            return apply_elementwise([](auto val){return std::floor(val);}, x);
        }

        using std::ceil;
        template <typename T, std::enable_if_t<!no_vectors_v<T>, std::nullptr_t> = nullptr>
        [[nodiscard]] T ceil(T x)
        {
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Argument must be floating-point.");
            return apply_elementwise([](auto val){return std::ceil(val);}, x);
        }

        using std::trunc;
        template <typename T, std::enable_if_t<!no_vectors_v<T>, std::nullptr_t> = nullptr>
        [[nodiscard]] T trunc(T x)
        {
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Argument must be floating-point.");
            return apply_elementwise([](auto val){return std::trunc(val);}, x);
        }

        template <typename T> [[nodiscard]] T frac(T x)
        {
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Argument must be floating-point.");

            if constexpr (no_vectors_v<T>)
                return std::modf(x, 0);
            else
                return apply_elementwise(frac<vec_base_t<T>>, x);
        }

        using std::nextafter;
        template <typename A, typename B, std::enable_if_t<!no_vectors_v<A, B>, std::nullptr_t> = nullptr>
        [[nodiscard]] A nextafter(A a, B b)
        {
            static_assert(vector<B> <= vector<A>, "If `b` is a vector, `a` has to be a vector as well.");
            static_assert(std::is_floating_point_v<vec_base_t<A>> && std::is_floating_point_v<vec_base_t<B>> && std::is_same_v<vec_base_t<A>, vec_base_t<B>>, "Arguments must be floating-point and have the same base type.");
            return apply_elementwise([](auto a, auto b){return std::nextafter(a, b);}, a, b);
        }

        // Integer division, slightly changed to behave nicely for negative values of the left operand:
        //           i : -4  -3  -2  -1  0  1  2  3  4
        // div_ex(i,2) : -2  -2  -1  -1  0  0  1  1  2
        template <typename A, typename B> [[nodiscard]] constexpr A div_ex(A a, B b)
        {
            static_assert(vector<B> <= vector<A>, "If `b` is a vector, `a` has to be a vector as well.");
            static_assert(std::is_integral_v<vec_base_t<A>> && std::is_integral_v<vec_base_t<B>>, "Arguments must be integral.");

            if constexpr (no_vectors_v<A,B>)
            {
                if (a >= 0)
                    return a / b;
                else
                    return (a + 1) / b - sign(b);
            }
            else
            {
                return apply_elementwise(div_ex<vec_base_t<A>, vec_base_t<B>>, a, b);
            }
        }

        // True integral modulo that remains periodic for negative values of the left operand.
        template <typename A, typename B> [[nodiscard]] constexpr A mod_ex(A a, B b)
        {
            static_assert(vector<B> <= vector<A>, "If `b` is a vector, `a` has to be a vector as well.");
            static_assert(std::is_integral_v<vec_base_t<A>> && std::is_integral_v<vec_base_t<B>>, "Arguments must be integral.");

            if constexpr (no_vectors_v<A,B>)
            {
                if (a >= 0)
                    return a % b;
                else
                    return abs(b) - 1 + (a + 1) % b;
            }
            else
            {
                return apply_elementwise(mod_ex<vec_base_t<A>, vec_base_t<B>>, a, b);
            }
        }

        // Simple implementation of `pow` for non-negative integral powers.
        template <typename A, typename B> [[nodiscard]] constexpr A ipow(A a, B b)
        {
            // `A` can be a scalar or a vector. `B` has to be scalar.
            static_assert(std::is_integral_v<B>, "Power must be integral.");
            A ret = 1;
            while (b-- > 0)
                ret *= a;
            return ret;
        }

        using std::pow;
        template <typename A, typename B, std::enable_if_t<!no_vectors_v<A, B>, std::nullptr_t> = nullptr>
        [[nodiscard]] auto pow(A a, B b)
        {
            return apply_elementwise([](auto val_a, auto val_b){return std::pow(val_a, val_b);}, a, b);
        }

        // Computes the smooth step function. Doesn't clamp `x`.
        template <typename T> [[nodiscard]] constexpr T smoothstep(T x)
        {
            // No special handling required for `T` being a vector.
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Argument must be floating-point.");
            return (3 - 2*x) * x*x;
        }

        // Performs linear interpolation. Returns `a * (1-factor) + b * factor`.
        template <typename F, typename A, typename B> [[nodiscard]] constexpr auto mix(F factor, A a, B b)
        {
            static_assert(std::is_floating_point_v<vec_base_t<F>>, "`factor` must be floating-point.");
            // No special handling required for the parameters being vectors.
            using type = larger_t<A, B>;
            return type(a) * (1-factor) + type(b) * factor;
        }

        // Returns a `min` or `max` value of the parameters.
        template <typename ...P> [[nodiscard]] constexpr larger_t<P...> min(P ... params)
        {
            if constexpr (no_vectors_v<P...>)
                return std::min({larger_t<P...>(params)...});
            else
                return apply_elementwise(min<vec_base_t<P>...>, params...);
        }
        template <typename ...P> [[nodiscard]] constexpr larger_t<P...> max(P ... params)
        {
            if constexpr (no_vectors_v<P...>)
                return std::max({larger_t<P...>(params)...});
            else
                return apply_elementwise(max<vec_base_t<P>...>, params...);
        }
    }

    inline namespace Misc // Misc functions
    {
        // A functor that performs linear mapping on scalars or vectors.
        template <typename T> struct linear_mapping
        {
            static_assert(std::is_floating_point_v<vec_base_t<T>>, "Template parameter must be floating-point.");

            T scale = T(1), offset = T(0);

            constexpr linear_mapping() {}

            constexpr linear_mapping(T src_a, T src_b, T dst_a, T dst_b)
            {
                T factor = 1 / (src_a - src_b);
                scale = (dst_a - dst_b) * factor;
                offset = (dst_b * src_a - dst_a * src_b) * factor;
            }

            constexpr T operator()(T x) const
            {
                return x * scale + offset;
            }

            using matrix_t = mat<vec_size_v<T>+1, vec_size_v<T>+1, vec_base_t<T>>;
            constexpr matrix_t matrix() const
            {
                matrix_t ret{};
                for (int i = 0; i < vec_size_v<T>; i++)
                {
                    ret[i][i] = scale[i];
                    ret[vec_size_v<T>][i] = offset[i];
                }
                return ret;
            }
        };

        // Shrinks a vector as little as possible to give it specific proportions.
        // Always returns a floating-point type.
        template <typename A, typename B> [[nodiscard]] constexpr auto shrink_to_proportions(A value, B proportions)
        {
            static_assert(vector<A> && vector<B> && vec_size_v<A> == vec_size_v<B>, "Arguments must be vectors of same size.");
            using type = larger_t<floating_point_t<A>,floating_point_t<B>>;
            return (type(value) / type(proportions)).min() * type(proportions);
        }
        // Expands a vector as little as possible to give it specific proportions.
        // Always returns a floating-point type.
        template <typename A, typename B> [[nodiscard]] constexpr auto expand_to_proportions(A value, B proportions)
        {
            static_assert(vector<A> && vector<B> && vec_size_v<A> == vec_size_v<B>, "Arguments must be vectors of same size.");
            using type = larger_t<floating_point_t<A>,floating_point_t<B>>;
            return (type(value) / type(proportions)).max() * type(proportions);
        }

        // Finds an intersection point of two lines.
        template <typename T> [[nodiscard]] constexpr vec2<T> line_intersection(vec2<T> a1, vec2<T> a2, vec2<T> b1, vec2<T> b2)
        {
            static_assert(std::is_floating_point_v<T>, "Arguments must be floating-point.");
            auto delta_a = a2 - a1;
            auto delta_b = b2 - b1;
            return ((a1.y - b1.y) * delta_b.x - (a1.x - b1.x) * delta_b.y) / (delta_a.x * delta_b.y - delta_a.y * delta_b.x) * delta_a + a1;
        }

        // Projects a point onto a line. `dir` is assumed to be normalized.
        template <int D, typename T> [[nodiscard]] constexpr vec<D,T> project_onto_line_norm(vec<D,T> point, vec<D,T> dir)
        {
            static_assert(std::is_floating_point_v<T>, "Arguments must be floating-point.");
            return dir * point.dot(dir);
        }
        // Projects a point onto a line.
        template <int D, typename T> [[nodiscard]] constexpr vec<D,T> project_onto_line(vec<D,T> point, vec<D,T> dir)
        {
            return project_onto_line_norm(point, dir.norm());
        }

        // Projects a point onto a plane. `plane_normal` is assumed to be normalized.
        template <typename T> [[nodiscard]] constexpr vec3<T> project_onto_plane_norm(vec3<T> point, vec3<T> plane_normal)
        {
            return point - project_onto_line_norm(point, plane_normal);
        }
        // Projects a point onto a plane.
        template <typename T> [[nodiscard]] constexpr vec3<T> project_onto_plane(vec3<T> point, vec3<T> plane_normal)
        {
            return project_onto_plane_norm(point, plane_normal.norm());
        }

        // Compares the angles of `a` and `b` without doing any trigonometry. Works with integers too.
        // The assumed angles are in range [0;2pi), with +X having angle 0.
        // Zero vectors are considered to be greater than everything else.
        template <typename T> [[nodiscard]] constexpr bool less_positively_rotated(vec2<T> a, vec2<T> b)
        {
            // This check makes (0,0) worse than any other vector,
            // and doesn't seem to affect the result if zero vectors are not involved.
            if (int d = (a == vec2<T>()) - (b == vec2<T>()))
            return d < 0;

            if (int d = (a.y < 0) - (b.y < 0))
            return d < 0;
            if (int d = (a.y == 0 && a.x < 0) - (b.y == 0 && b.x < 0))
            return d < 0;

            return a.x * b.y > b.x * a.y;
        }

        // Same, but angle 0 is mapped to `dir` instead of +X.
        template <typename T> [[nodiscard]] constexpr bool less_positively_rotated(vec2<T> dir, vec2<T> a, vec2<T> b)
        {
            imat2 mat = imat2(dir, dir.rot90());
            return less_positively_rotated(a * mat, b * mat);
        }
    }

    namespace Export
    {
        using Vector::vec; // Vector and matrix definitions. We use this instead of `using namespace Vector` to avoid bringing...
        using Vector::mat; // ...the overloaded operators into the global namespace, mostly for better error messages and build speed.
        using namespace Alias; // Convenient type aliases.
        using namespace Common; // Common functions.

        // Common types.
        using std::int8_t;
        using std::uint8_t;
        using std::int16_t;
        using std::uint16_t;
        using std::int32_t;
        using std::uint32_t;
        using std::int64_t;
        using std::uint64_t;
        using std::size_t;
        using std::ptrdiff_t;
        using std::intptr_t;
        using std::uintptr_t;

        // Common standard functions.
        using std::sqrt;
        using std::cos;
        using std::sin;
        using std::tan;
        using std::acos;
        using std::asin;
        using std::atan;
        using std::atan2;
    }
}

namespace std
{
    template <int D, typename T> struct less<Math::vec<D,T>>
    {
        using result_type = bool;
        using first_argument_type = Math::vec<D,T>;
        using second_argument_type = Math::vec<D,T>;
        constexpr bool operator()(const Math::vec<D,T> &a, const Math::vec<D,T> &b) const
        {
            return a.tie() < b.tie();
        }
    };

    template <int D, typename T> struct hash<Math::vec<D,T>>
    {
        using result_type = std::size_t;
        using argument_type = Math::vec<D,T>;
        std::size_t operator()(const Math::vec<D,T> &v) const
        {
            std::size_t ret = std::hash<decltype(v.x)>{}(v.x);
            for (int i = 1; i < D; i++)
                ret ^= std::hash<decltype(v.x)>{}(v[i]) + 0x9e3779b9 + (ret << 6) + (ret >> 2); // From Boost.
            return ret;
        }
    };
}

// Quaternions

namespace Math
{
    inline namespace Quat // Quaternions.
    {
        template <typename T> struct quat
        {
            static_assert(std::is_floating_point_v<T>, "The base type must be floating-point.");
            using type = T;
            using vec3_t = vec3<T>;
            using vec4_t = vec4<T>;
            using mat3_t = mat3<T>;
            type x = 0, y = 0, z = 0, w = 1; // This represents zero rotation.

            constexpr quat() {}
            constexpr quat(type x, type y, type z, type w) : x(x), y(y), z(z), w(w) {}
            explicit constexpr quat(const vec4_t &vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}

            // Normalizes the axis. If it's already normalized, use `with_normalized_axis()` instead.
            constexpr quat(vec3_t axis, type angle) {*this = with_normalized_axis(axis.norm(), angle);}
            [[nodiscard]] static constexpr quat with_normalized_axis(vec3_t axis, type angle) {angle *= type(0.5); return quat((axis * std::sin(angle)).to_vec4(std::cos(angle)));}

            [[nodiscard]] constexpr vec4_t as_vec() const {return {x, y, z, w};}
            [[nodiscard]] constexpr vec3_t xyz() const {return {x, y, z};}
            [[nodiscard]] type *as_array() {return &x;}
            [[nodiscard]] const type *as_array() const {return &x;}

            [[nodiscard]] constexpr quat norm() const {return quat(as_vec().norm());}
            [[nodiscard]] constexpr quat approx_norm() const {return quat(as_vec().approx_norm());}

            [[nodiscard]] constexpr vec3_t axis_denorm() const { return xyz(); }
            [[nodiscard]] constexpr vec3_t axis_norm() const { return xyz().norm(); }
            [[nodiscard]] constexpr float angle() const { return 2 * std::atan2(xyz().len(), w); }

            // Negates the rotation. Not strictly an inversion in the mathematical sense, since the length stays unchanged (while it's supposed to become `1 / old_length`).
            [[nodiscard]] constexpr quat inverse() const {return quat(xyz().to_vec4(-w));}
            // Negates the three imaginary parts of the quaternion, `xyz`. Effectively inverts the rotation, but works slower than `inverse()`. Useful only for low-level quaternion things.
            [[nodiscard]] constexpr quat conjugate() const {return quat((-xyz()).to_vec4(w));}

            // Uses iterative normalization to keep denormalization from accumulating due to lack of precision.
            template <typename TT> [[nodiscard]] constexpr quat<larger_t<T,TT>> operator*(const quat<TT> &other) const {return mult_without_norm(other).approx_norm();}
            constexpr quat &operator*=(const quat &other) {return *this = *this * other;}

            // Simple quaternion multiplication, without any normalization.
            template <typename TT> [[nodiscard]] constexpr quat<larger_t<T,TT>> mult_without_norm(const quat<TT> &other) const
            {
                return quat<larger_t<T,TT>>(vec4<larger_t<T,TT>>(
                    x * other.w + w * other.x - z * other.y + y * other.z,
                    y * other.w + z * other.x + w * other.y - x * other.z,
                    z * other.w - y * other.x + x * other.y + w * other.z,
                    w * other.w - x * other.x - y * other.y - z * other.z
                ));
            }

            // Transforms a vector by this quaternion. Only makes sense if the quaternion is normalized.
            template <typename TT> [[nodiscard]] constexpr vec3<larger_t<T,TT>> operator*(const vec3<TT> &other) const
            {
                // This is called the "Euler-Rodrigues formula".
                // We could also use `*this * other * this->conjugate()`, but that looks less optimized.
                vec3<larger_t<T,TT>> tmp = xyz().cross(other);
                return other + 2 * w * tmp + 2 * xyz().cross(tmp);
            }

            // Transforms a vector by this quaternion, inversed. Mimics a similar matrix operation.
            template <typename TT> [[nodiscard]] friend constexpr vec3<larger_t<T,TT>> operator*(const vec3<TT> &v, const quat &q)
            {
                return q.inverse() * v;
            }

            // Returns a rotation matrix for this quaternion. Only makes sense if the quaternion is normalized.
            [[nodiscard]] constexpr mat3_t matrix() const
            {
                return mat3_t(
                    1 - (2*y*y + 2*z*z), 2*x*y - 2*z*w, 2*x*z + 2*y*w,
                    2*x*y + 2*z*w, 1 - (2*x*x + 2*z*z), 2*y*z - 2*x*w,
                    2*x*z - 2*y*w, 2*y*z + 2*x*w, 1 - (2*x*x + 2*y*y)
                );
            }

            // Returns a rotation matrix for this quaternion. Works even if the quaternion is not normalized.
            [[nodiscard]] constexpr mat3_t matrix_from_denorm() const
            {
                type f = 1 / as_vec().len_sqr();
                mat3_t m = matrix();
                return mat3_t(m.x * f, m.y * f, m.z * f);
            }
        };

        using fquat = quat<float>;
        using dquat = quat<double>;
        using ldquat = quat<long double>;

        template <typename A, typename B, typename T> std::basic_ostream<A,B> &operator<<(std::basic_ostream<A,B> &s, const quat<T> &q)
        {
            s.width(0);
            if (q.axis_denorm() == vec3<T>(0))
                s << "[angle=0";
            else
                s << "[axis=" << q.axis_denorm()/q.axis_denorm().max() << " angle=" << to_deg(q.angle()) << "(deg)";
            return s << " len=" << q.as_vec().len() << ']';
        }

        template <typename A, typename B, typename T> std::basic_istream<A,B> &operator>>(std::basic_istream<A,B> &s, quat<T> &q)
        {
            vec4<T> vec;
            s >> vec;
            q = quat(vec);
            return s;
        }
    }

    inline namespace Utility
    {
        // Check if `T` is a quaternion type (possibly const).
        template <typename T> struct is_quat_impl : std::false_type {};
        template <typename T> struct is_quat_impl<      quat<T>> : std::true_type {};
        template <typename T> struct is_quat_impl<const quat<T>> : std::true_type {};
        template <typename T> inline constexpr bool is_quat_v = is_quat_impl<T>::value;
    }

    namespace Export
    {
        using namespace Quat;
    }
}

namespace std
{
    template <typename T> struct less<Math::quat<T>>
    {
        using result_type = bool;
        using first_argument_type = Math::quat<T>;
        using second_argument_type = Math::quat<T>;
        constexpr bool operator()(const Math::quat<T> &a, const Math::quat<T> &b) const
        {
            return a.as_vec().tie() < b.as_vec().tie();
        }
    };

    template <typename T> struct hash<Math::quat<T>>
    {
        using result_type = std::size_t;
        using argument_type = Math::quat<T>;
        std::size_t operator()(const Math::quat<T> &q) const
        {
            return std::hash<Math::vec4<T>>{}(q.as_vec());
        }
    };
}

using namespace Math::Export;
