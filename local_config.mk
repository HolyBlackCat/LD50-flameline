ifeq ($(TARGET_OS),windows)
ifeq ($(HOST_OS),windows)
# Windows:
# `--target=x86_64-w64-windows-gnu` is necessary when using official Clang binaries. Otherwise it should be a no-op.
# `-femulated-tls` is necessary when using libstdc++'s atomics.
C_COMPILER   = clang --target=x86_64-w64-windows-gnu -femulated-tls
CXX_COMPILER = clang++ --target=x86_64-w64-windows-gnu -femulated-tls
C_LINKER     = clang --target=x86_64-w64-windows-gnu -femulated-tls -fuse-ld=lld
CXX_LINKER   = clang++ --target=x86_64-w64-windows-gnu -femulated-tls -fuse-ld=lld
else
# Cross-compiling from Linux to Windows: (assuming quasi-msys2)
C_COMPILER   = win-clang
CXX_COMPILER = win-clang++
C_LINKER     = win-clang -fuse-ld=lld-11
CXX_LINKER   = win-clang++ -fuse-ld=lld-11
# Need to specify a custom LDD wrapper, the auto-detected LDD wouldn't work.
LDD          = win-ldd -R
endif
else
# Linux:
C_COMPILER   = clang-11
CXX_COMPILER = clang++-11
C_LINKER     = clang-11 -fuse-ld=lld-11
CXX_LINKER   = clang++-11 -fuse-ld=lld-11
endif
