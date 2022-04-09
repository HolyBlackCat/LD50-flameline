MODE := release

CC := emcc
CXX := em++
# LINKER :=
# Flags taken from `emsdk/upstream/emscripten/emcmake.py`.
CMAKE := cmake -DCMAKE_TOOLCHAIN_FILE=$$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_CROSSCOMPILING_EMULATOR="$$EMSDK_NODE;--experimental-wasm-threads"
EXT_exe := .html
