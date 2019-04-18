# Sources
override SOURCE_DIRS += src lib

# Object directory
OBJECT_DIR ?= obj

# Resulting binary
OUTPUT_FILE ?= bin/imp-re

# Flags
CXXFLAGS ?= -Wall -Wextra -pedantic-errors -std=c++2a
override CXXFLAGS += -include src/utils/common.h -include src/program/parachute.h -Ilib/include -Isrc
override LDFLAGS += -Llib -lmingw32 -lSDL2main -lSDL2.dll -lfreetype -lopenal32 -lvorbisfile -lvorbisenc -lvorbis -logg -lbz2 -lz -lfmt -pthread

# Targets
$(call new_target,debug)
debug: override CXXFLAGS += -g -D_GLIBCXX_ASSERTIONS

$(call new_target,debug_hard)
debug_hard: override CXXFLAGS += -g -D_GLIBCXX_DEBUG

$(call new_target,release)
release: override CXXFLAGS += -DNDEBUG -O3
release: override LDFLAGS += -O3 -s -mwindows

# File-specific flags
obj/lib/%.cpp.o: override CXXFLAGS += -O3

# Precompiled headers
PRECOMPILED_HEADERS ?= src/game/*.cpp>src/game/master.hpp
