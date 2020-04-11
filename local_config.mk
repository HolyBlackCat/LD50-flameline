ifeq ($(TARGET_OS),windows)
# `--target=x86_64-w64-windows-gnu` is necessary when using official Clang binaries. Otherwise it should be a no-op.
# `-femulated-tls` is necessary when using libstdc++'s atomics.
C_COMPILER   = clang --target=x86_64-w64-windows-gnu -femulated-tls
CXX_COMPILER = clang++ --target=x86_64-w64-windows-gnu -femulated-tls
C_LINKER     = clang --target=x86_64-w64-windows-gnu -femulated-tls -fuse-ld=lld
CXX_LINKER   = clang++ --target=x86_64-w64-windows-gnu -femulated-tls -fuse-ld=lld
else
C_COMPILER   = clang-9
CXX_COMPILER = clang++-9
C_LINKER     = clang-9 -fuse-ld=lld
CXX_LINKER   = clang++-9 -fuse-ld=lld
endif
