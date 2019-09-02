ifeq ($(TARGET_OS),windows)
C_COMPILER = clang
CXX_COMPILER = clang++
C_LINKER = clang -fuse-ld=lld
CXX_LINKER = clang++ -fuse-ld=lld
else
C_COMPILER = clang-8
CXX_COMPILER = clang++-8
C_LINKER = clang-8 -fuse-ld=lld
CXX_LINKER = clang++-8 -fuse-ld=lld
endif

ifeq ($(TARGET_OS),windows)
WINDRES = windres
POST_BUILD_COMMANDS = @$(call rmfile,$(OUTPUT_FILE).lib)
endif
