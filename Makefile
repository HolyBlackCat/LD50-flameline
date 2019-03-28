# This file is an universal makefile for C/C++ executables.
#
# It requires a GCC-like compiler with support for `-MMD -MP` flags.
# You need to have a separate file called `build_options.mk` in the same directory, which has to contain some high-level build options.
# An example of such file is provided below:
#
#    # Source directories
#    SOURCE_DIRS ?= src lib   <- All files from these directories will be compiled.
#
#    # Object directory
#    OBJECT_DIR ?= obj   <- All object files will be placed into this directory.
#
#    # Resulting binary
#    OUTPUT_FILE ?= bin/imp-re   <- The resulting executable. An extension is appended to it automatically if needed.
#
#    # Flags
#    CXXFLAGS ?= -Wall -Wextra -pedantic-errors -std=c++2a
#    override CXXFLAGS += -Isrc
#    override LDFLAGS += -Llib -lwhatever
#
#    # Targets
#    .PHONY: debug   <- Those are the targets. The first one is used by default. Additionally a `build` target is made, without any additional flags.
#    debug: override CXXFLAGS += -g -D_GLIBCXX_DEBUG
#    debug: build
#
#    .PHONY: release   <- Yet another target.
#    release: override CXXFLAGS += -DNDEBUG -O3
#    release: override LDFLAGS += -O3 -s
#    release: build
#
#    # File-specific flags   <- Note that target names start with `OBJECT_DIR`, followed by one of `SOURCE_DIRS`.
#    obj/lib/glfl.cpp.o: override CXXFLAGS += -O3
#    obj/lib/implementation.cpp.o: override CXXFLAGS += -O3


# --- DEFINITIONS ---

# Some constants.
override comma := ,
override dollar := $

# A recursive wildcard function.
# Source: https://stackoverflow.com/a/18258352/2752075
# Recursively searches a directory for all files matching a pattern.
# The first parameter is a directory, the second is a pattern.
# THE PARAMETER MUST END WITH /
# Example usage: SOURCES = $(call rwildcard, src/, *.cpp)
override rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))


# --- DETECT ENVIRONMENT ---

# Host OS.
ifeq ($(OS),Windows_NT)
HOST_OS ?= windows
override host_win_unix = $1
else
HOST_OS ?= unix
override host_win_unix = $2
endif

# Target OS.
TARGET_OS ?= $(HOST_OS)

ifeq ($(TARGET_OS),windows)
override extension_exe = .exe
override target_win_unix = $1
else
override extension_exe =
override target_win_unix = $2
endif

# Host shell.
ifeq ($(shell echo "foo"),"foo")
HOST_SHELL ?= windows
override shell_win_unix = $1
else
HOST_SHELL ?= unix
override shell_win_unix = $2
endif

# Shell-specific functions.
# Example usage: $(call rmfile, bin/out.exe)
ifeq ($(HOST_SHELL),windows)
override SILENCE = >NUL 2>NUL || (exit 0)
override rmfile = @del /F /Q $(subst /,\,$1) $(SILENCE)
override rmdir = @rd /S /Q $(subst /,\,$1) $(SILENCE)
override mkdir = @mkdir >NUL 2>NUL $(subst /,\,$1) $(SILENCE)
override echo = echo $1
override native_path = $(subst /,\,$1)
override cur_dir := $(subst \,/,$(shell echo %CD%))
else
override SILENCE = >/dev/null 2>/dev/null || true
override rmfile = @rm -f $1 $(SILENCE)
override rmdir = @rm -rf $1 $(SILENCE)
override mkdir = @mkdir -p $1 $(SILENCE)
override echo = echo "$(subst ",\",$(subst \,\\,$1))"
override native_path = $1
ifeq ($(HOST_OS),windows)
# We're probably on MSYS2 or Cygwin or something similar.
override cur_dir := $(shell cygpath -m $(dollar)PWD)
else
override cur_dir := $(PWD)
endif
endif


# --- INCLUDE USER CONFIG ---
include build_options.mk


# --- LOCATE FILES ---

# Source files.
override SOURCES_C = $(foreach dir,$(SOURCE_DIRS),$(call rwildcard, $(dir)/, *.c))
override SOURCES_CPP = $(foreach dir,$(SOURCE_DIRS),$(call rwildcard, $(dir)/, *.cpp))
override SOURCES = $(SOURCES_C) $(SOURCES_CPP)

# Object files.
override OBJECTS = $(patsubst %,$(OBJECT_DIR)/%,$(patsubst %.c,%.c.o,$(patsubst %.cpp,%.cpp.o,$(SOURCES))))

# Dependency lists
override DEP_FILES = $(patsubst %.o,%.d,$(OBJECTS))

# Add a proper extension to the output file.
OUTPUT_FILE_EXT = $(OUTPUT_FILE)$(extension_exe)


# --- COMBINE FLAGS ---
override CFLAGS += $(CFLAGS_EXTRA)
override CXXFLAGS += $(CXXFLAGS_EXTRA)


# --- TARGETS ---

# Target: generic build
.PHONY: build
build: $(OUTPUT_FILE_EXT)

# An internal target that actually builds the executable.
# Note that object files come before linker flags.
$(OUTPUT_FILE_EXT): $(OBJECTS)
	@$(call echo,[Linking] $(OUTPUT_FILE_EXT))
	$(call mkdir,$(dir $@))
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	@$(call echo,[Done])

# Internal targets that build the source files.
# Note that flags come before the source file.
$(OBJECT_DIR)/%.c.o: %.c
	@$(call echo,[C] $<)
	$(call mkdir,$(dir $@))
	@$(CC) -MMD -MP $(CFLAGS) $< -c -o $@
$(OBJECT_DIR)/%.cpp.o: %.cpp
	@$(call echo,[C++] $<)
	$(call mkdir,$(dir $@))
	@$(CXX) -MMD -MP $(CXXFLAGS) $< -c -o $@

# Target: clean the build
.PHONY: clean
clean:
	$(call rmdir,$(OBJECT_DIR))
	$(call rmfile,$(OUTPUT_FILE_EXT))

-include $(DEP_FILES)

# Target: generate compile_commands.json
override file_command= && $(call echo,{"directory": "."$(comma) "file": "$(cur_dir)/$2"$(comma) "command": "$1 $2"}$(comma)) >>compile_commands.json

.PHONY: commands
commands:
	@$(call echo,[Generating] compile_commands.json)
	@$(call echo,[) >compile_commands.json $(foreach file,$(SOURCES_C),$(call file_command,$(CC) $(CFLAGS),$(file))) $(foreach file,$(SOURCES_CPP),$(call file_command,$(CXX) $(CXXFLAGS),$(file))) && $(call echo,]) >>compile_commands.json
	@$(call echo,[Done])
	
# Target: clean compile_commands.json
.PHONY: clean_commands
clean_commands:
	$(call rmfile,compile_commands.json)
	