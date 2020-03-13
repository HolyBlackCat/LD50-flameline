ifeq ($(TARGET_OS),windows)
# `-femulated-tls` is necessary when using libstdc++'s atomics.
C_COMPILER   = clang -femulated-tls
CXX_COMPILER = clang++ -femulated-tls
C_LINKER     = clang -femulated-tls -fuse-ld=lld
CXX_LINKER   = clang++ -femulated-tls -fuse-ld=lld
else
C_COMPILER   = clang-9
CXX_COMPILER = clang++-9
C_LINKER     = clang-9 -fuse-ld=lld
CXX_LINKER   = clang++-9 -fuse-ld=lld
endif

ifeq ($(TARGET_OS),windows)
# LLD likes to create this file for no reason.
POST_BUILD_COMMANDS = @$(call rmfile,$(OUTPUT_FILE).lib)
endif
