C_COMPILER = clang
CXX_COMPILER = clang++
LINKER = Z:/Lander/LLVM/bin/clang++ -target w64-x86_64-windows-gnu -fuse-ld=lld

ifeq ($(TARGET_OS),windows)
POST_BUILD_COMMANDS = @$(call rmfile, $(OUTPUT_FILE).lib)
endif