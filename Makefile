# This file is an universal makefile for C/C++ executables.
#
# It requires a GCC-like compiler with support for `-MMD -MP` flags.
# You need to have a separate file called `build_options.mk` in the same directory, which has to contain some high-level build options.
# An example of such file is provided below:
#
#    # Sources
#    override SOURCE_DIRS += src lib   <- All *.c/*.cpp/*.rc files from these directories will be compiled and linked.
#    override SOURCES += icon.rc   <- As well as these files.
#
#    # Object directory
#    OBJECT_DIR ?= obj   <- Objects go here.
#
#    # Resulting binary
#    OUTPUT_FILE ?= bin/ball-game   <- Name for the resulting binary. On Windows ".exe" is appended automatically.
#
#    # Flags
#    CXXFLAGS ?= -Wall -Wextra -pedantic-errors -std=c++2a
#    override CXXFLAGS += -include src/utils/common.h -include src/program/parachute.h -Ilib/include -Isrc
#    override LDFLAGS += -Llib -lmingw32 -lSDL2main -lSDL2.dll -lfreetype -lopenal32 -lvorbisfile -lvorbisenc -lvorbis -logg -lbz2 -lz
#
#    # Targets
#    .PHONY: debug
#    debug: override CXXFLAGS += -g -D_GLIBCXX_ASSERTIONS
#    debug: build
#
#    .PHONY: debug_hard
#    debug_hard: override CXXFLAGS += -g -D_GLIBCXX_DEBUG
#    debug_hard: build
#
#    .PHONY: release
#    release: override CXXFLAGS += -DNDEBUG -O3
#    release: override LDFLAGS += -O3 -s -mwindows
#    release: build
#
#    # File-specific flags
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
# Example usage: SOURCES = $(call rwildcard, src, *.c *.cpp)
override rwildcard=$(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))


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
override silence = >NUL 2>NUL || (exit 0)
override rmfile = @del /F /Q $(subst /,\,$1) $(silence)
override rmdir = @rd /S /Q $(subst /,\,$1) $(silence)
override mkdir = @mkdir >NUL 2>NUL $(subst /,\,$1) $(silence)
override echo = echo $1
override native_path = $(subst /,\,$1)
override cur_dir := $(subst \,/,$(shell echo %CD%))
else
override silence = >/dev/null 2>/dev/null || true
override rmfile = @rm -f $1 $(silence)
override rmdir = @rm -rf $1 $(silence)
override mkdir = @mkdir -p $1 $(silence)
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


# --- DEFAULT VARIABLE VALUES ---
SOURCE_DIRS ?= .
OUTPUT_FILE ?= program
OBJECT_DIR ?= obj
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic-errors -g
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic-errors -g
WINDRES ?= windres
WINDRES_FLAGS ?= -J rc -O coff


# --- LOCATE FILES ---

# Source files.
override SOURCES += $(strip $(foreach dir,$(SOURCE_DIRS),$(call rwildcard, $(dir), *.c *.cpp *.rc))) # Note the `+=`.

# Object files.
override objects = $(patsubst %,$(OBJECT_DIR)/%,$(patsubst %.c,%.c.o,$(patsubst %.cpp,%.cpp.o,$(patsubst %.rc,%.res,$(SOURCES)))))

# Dependency lists
override dep_files = $(patsubst %.o,%.d,$(filter %.o,$(objects)))

# Add a proper extension to the output file.
OUTPUT_FILE_EXT ?= $(OUTPUT_FILE)$(extension_exe)


# --- COMBINE FLAGS ---
override CFLAGS += $(CFLAGS_EXTRA)
override CXXFLAGS += $(CXXFLAGS_EXTRA)


# --- TARGETS ---

# Target: generic build
.PHONY: build
build: $(OUTPUT_FILE_EXT)

# An internal target that actually builds the executable.
# Note that object files come before linker flags.
$(OUTPUT_FILE_EXT): $(objects)
	@$(call echo,[Linking] $(OUTPUT_FILE_EXT))
	$(call mkdir,$(dir $@))
	@$(CXX) $(objects) $(LDFLAGS) -o $@
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
$(OBJECT_DIR)/%.res: %.rc
	@$(call echo,[Resource] $<)
	$(call mkdir,$(dir $@))
	@$(WINDRES) $(WINDRES_FLAGS) -i $< -o $@

# Target: clean the build
.PHONY: clean
clean:
	@$(call echo,[Cleaning])
	$(call rmdir,$(OBJECT_DIR))
	$(call rmfile,$(OUTPUT_FILE_EXT))
	@$(call echo,[Done])

# Helpers for generating compile_commands.json
override EXCLUDE_FILES += $(foreach dir,$(EXCLUDE_DIRS), $(call rwildcard,$(dir),*.c *.cpp *.h *.hpp)) # Note the `+=`.
override include_files = $(filter-out $(EXCLUDE_FILES), $(SOURCES))
override file_command = && $(call echo,{"directory": "."$(comma) "file": "$(cur_dir)/$2"$(comma) "command": "$1 $2"}$(comma)) >>compile_commands.json
override all_commands = $(foreach file,$(filter %.c,$(include_files)),$(call file_command,$(CC) $(CFLAGS),$(file))) $(foreach file,$(filter %.cpp,$(include_files)),$(call file_command,$(CXX) $(CXXFLAGS),$(file)))
override all_stub_commands = $(foreach file,$(EXCLUDE_FILES),$(call file_command,,$(file)))

# Target: generate compile_commands.json
.PHONY: commands
commands:
	@$(call echo,[Generating] compile_commands.json)
	@$(call echo,[) >compile_commands.json $(all_commands) && $(call echo,]) >>compile_commands.json
	@$(call echo,[Done])

# Target: generate compile_commands.json with invalid commands for EXCLUDE_FILES and all files from EXCLUDE_DIRS.
.PHONY: commands_fixed
commands_fixed:
	@$(call echo,[Generating] compile_commands.json (fixed))
	@$(call echo,[) >compile_commands.json $(all_commands) $(all_stub_commands) && $(call echo,]) >>compile_commands.json
	@$(call echo,[Done])

# Target: clean compile_commands.json
.PHONY: clean_commands
clean_commands:
	$(call rmfile,compile_commands.json)

# Import saved target
override saved_target =
-include .config.mk

# Target: set target for `make current`
.PHONY: set_current
.PHONY: set_current_clean
ifeq ($(TARGET),)
set_current_clean:
set_current:
	@$(call echo,Set TARGET variable to a desired target name.)
	@exit 1
else
ifeq ($(TARGET),$(strip $(saved_target)))
set_current_clean:
set_current:
else
set_current_clean: set_current clean
set_current:
	@$(call echo,override saved_target := $(TARGET)) >.config.mk
	@$(call echo,[Info] New target is: $(TARGET))
endif
endif

# Target: build current target
.PHONY: current
ifeq ($(saved_target),)
print_current:
current:
	@$(call echo,No target selected.)
	@exit 1
else
print_current:
	@$(call echo,[Target] $(saved_target))
current: print_current $(saved_target)
endif


# --- INCLUDE DEPENDENCIES ---
-include $(dep_files)
