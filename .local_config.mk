C_COMPILER = clang
CXX_COMPILER = clang++

C_LINKER = clang -fuse-ld=lld
CXX_LINKER = clang++ -fuse-ld=lld

WINDRES = windres

ifeq ($(TARGET_OS),windows)
POST_BUILD_COMMANDS = @$(call rmfile,$(OUTPUT_FILE).lib)
endif
