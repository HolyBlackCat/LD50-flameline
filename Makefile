# Source directories
SOURCE_DIRS = src lib

# Object directory
OBJECT_DIR = obj

# Resulting binary
OUTPUT_FILE = bin/out.exe

# Targets and flags
CXXFLAGS = -Wall -Wextra -pedantic-errors -std=c++2a 
LDFLAGS = -Llib -lmingw32 -lSDL2main -lSDL2.dll -lz -lfreetype

CXXFLAGS += -include src/utils/_common.h -Isrc -isystem lib/include

.PHONY: debug
debug: CXXFLAGS += -D_GLIBCXX_DEBUG -g
debug: build

.PHONY: release
release: CXXFLAGS += -DNDEBUG -O3 -s -flto
release: LDFLAGS += -flto
release: build

include Makefile.impl