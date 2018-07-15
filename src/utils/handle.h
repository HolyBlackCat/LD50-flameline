#ifndef HANDLE_H_INCLUDED
#define HANDLE_H_INCLUDED

template <typename T, T Default = T()> class Handle
{
    T value = Default;

  public:
    using type = T;
    inline constexpr static type null = Default;

    constexpr Handle() noexcept {}
    constexpr Handle(T value) noexcept : value(value) {}
    constexpr Handle(Handle &&other) noexcept : value(other.value) {other.value = null;}
    constexpr Handle &operator=(Handle &&other) noexcept {value = other.value; other.value = null; return *this;}

    constexpr explicit operator bool() const {return value != null;}
    constexpr       type &operator*()       {return value;}
    constexpr const type &operator*() const {return value;}
    constexpr       type *operator->()       {return &value;}
    constexpr const type *operator->() const {return &value;}
};

#endif
