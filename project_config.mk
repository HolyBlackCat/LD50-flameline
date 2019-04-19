# Sources
SOURCE_DIRS = src lib

# Object directory
OBJECT_DIR = obj

# Resulting binary
OUTPUT_FILE = bin/imp-re
LINKER_MODE = CXX

# Flags
CXXFLAGS = -Wall -Wextra -pedantic-errors -std=c++2a
LDFLAGS =
# Important flags
override CXXFLAGS += -include src/utils/common.h -include src/program/parachute.h -Ilib/include -Isrc
override LDFLAGS += -Llib -lmingw32 -lSDL2main -lSDL2.dll -lfreetype -lopenal32 -lvorbisfile -lvorbisenc -lvorbis -logg -lbz2 -lz -lfmt -pthread

# Build modes
$(call new_mode,debug)
$(mode_flags) CXXFLAGS += -g -D_GLIBCXX_ASSERTIONS

$(call new_mode,debug_hard)
$(mode_flags) CXXFLAGS += -g -D_GLIBCXX_DEBUG

$(call new_mode,release)
$(mode_flags) CXXFLAGS += -DNDEBUG -O3
$(mode_flags) LDFLAGS += -O3 -s -mwindows

# File-specific flags
FILE_SPECIFIC_FLAGS = lib/*.cpp > -O3

# Precompiled headers
PRECOMPILED_HEADERS = src/game/*.cpp>src/game/master.hpp
