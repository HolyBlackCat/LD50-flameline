# Sources
SOURCE_DIRS := src lib

# Object directory
OBJECT_DIR := obj

# Resulting binary
OUTPUT_FILE := bin/imp-re
LINKER_MODE := CXX

# Dependency set name
LIBRARY_PACK_NAME := imp-re_deps_21-12
USED_PACKAGES := sdl2 openal freetype2 ogg vorbis vorbisfile zlib fmt double-conversion # bullet


# Flags
# `-Wno-gnu-zero-variadic-macro-arguments` - Allow omitting the last `,` when passing zero args to a variadic macro parameter.
#     The standard allows this since C++20, but Clang's `-pedantic-errors` incorrectly rejects it otherwise.
CXXFLAGS := -std=c++20 -pedantic-errors -Wall -Wextra -Wdeprecated -Wextra-semi -Wno-gnu-zero-variadic-macro-arguments
LDFLAGS :=
# Important flags
override CXXFLAGS += -include src/program/common_macros.h -include src/program/parachute.h
override CXXFLAGS += -Isrc -Ilib/include $(subst -Dmain,-DIMP_ENTRY_POINT_OVERRIDE,$(sort $(deps_compiler_flags)))
override CXXFLAGS += -Ilib/include/cglfl_gl3.2_core # OpenGL version
override LDFLAGS += $(filter-out -mwindows,$(deps_linker_flags))

# Build modes
$(call new_mode,debug)
$(mode_flags) CXXFLAGS += -g -D_GLIBCXX_DEBUG

$(call new_mode,debug_soft)
$(mode_flags) CXXFLAGS += -g -D_GLIBCXX_ASSERTIONS

$(call new_mode,release)
$(mode_flags) CXXFLAGS += -DNDEBUG -O3
$(mode_flags) LDFLAGS += -O3 -s
ifeq ($(TARGET_OS),windows)
$(mode_flags) LDFLAGS += -mwindows
endif

$(call new_mode,profile)
$(mode_flags) CXXFLAGS += -DNDEBUG -O3 -pg
$(mode_flags) LDFLAGS += -O3 -pg
ifeq ($(TARGET_OS),windows)
$(mode_flags) LDFLAGS += -mwindows
endif

$(call new_mode,sanitize_address)
$(mode_flags) CXXFLAGS += -g -D_GLIBCXX_DEBUG -fsanitize=address
$(mode_flags) LDFLAGS += -fsanitize=address

$(call new_mode,sanitize_ub)
$(mode_flags) CXXFLAGS += -g -D_GLIBCXX_DEBUG -fsanitize=undefined
$(mode_flags) LDFLAGS += -fsanitize=undefined

# File-specific flags
FILE_SPECIFIC_FLAGS := lib/implementation.cpp lib/cglfl.cpp > -g0 -O3

# Precompiled headers
PRECOMPILED_HEADERS := src/game/*.cpp src/game/*.h > src/game/master.hpp

# Code generation
GEN_CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic-errors
override generators_dir := gen
override generated_headers := math:src/utils/mat.h macros:src/macros/generated.h
override generate_file = $(call host_native_path,$2) : $(generators_dir)/make_$1.cpp ; \
	@+$(MAKE) -f gen/Makefile _gen_dir=$(generators_dir) _gen_source_file=make_$1 _gen_target_file=$2 --no-print-directory
$(foreach f,$(generated_headers),$(eval $(call generate_file,$(word 1,$(subst :, ,$f)),$(word 2,$(subst :, ,$f)))))
