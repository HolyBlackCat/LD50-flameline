# This file is an universal makefile for C/C++ executables.
#
# Notable make targets include:
#    Whatever build mode targets you define (see below).
#    generic_build     - The base for your custom targets. Has no special flags.
#    clean             - Speaks for itself.
#    commands          - Generate `compile_commands.json` from all source files, excluding files listed in `EXCLUDE_FILES` and all files from directories listed in `EXCLUDE_DIRS`.
#    commands_fixed    - Same as `commands`, but also purposefully generates broken rules for excluded files. Using this rule while excluding all sources
#                          not under your control (such as library headers) might improve experience when using clangd (e.g. as a vscode plugin), since
#                          it prevents any clang-tidy warnings from appearing in excluded files. Sadly it also prevents clangd from working with excluded files at all.
#    current           - Builds currently selected target. This is useful for IDE integration.
#    set_current       - Selects a target based on the value of `TARGET`. The setting is saved in `.current_target.mk`.
#    set_current_clean - Same as `set_current`, but also cleans the repo.
#
#
#
# It requires a GCC-like compiler with support for `-MMD -MP` flags.
# You need to have a separate file called `build_options.mk` in the same directory, which has to contain some high-level build options.
# An example of such file is provided below:
#
#    override SOURCE_DIRS += src lib   <- All *.c/*.cpp/*.rc files from these directories will be compiled and linked.
#    override SOURCES += icon.rc   <- As well as these files.
#
#    OBJECT_DIR ?= obj   <- Objects will be placed here.
#
#    OUTPUT_FILE ?= bin/ball-game   <- Name for the resulting binary. On Windows ".exe" is appended automatically.
#
#       <- Compiler flags
#    CXXFLAGS ?= -Wall -Wextra -pedantic-errors -std=c++2a   <- Those can be easily overriden by user.
#    LDFLAGS =
#       <- Important compiler flags (those are not affected by setting `*FLAGS` variables from outside)
#    override CXXFLAGS += -include src/utils/common.h -include src/program/parachute.h -Ilib/include -Isrc   <- Those are harder to override, define important flags this way.
#    override LDFLAGS += -Llib -lmingw32 -lSDL2main -lSDL2.dll -lfreetype -lopenal32 -lvorbisfile -lvorbisenc -lvorbis -logg -lbz2 -lz
#
#       <- Build targets. The first target becomes the default `make` target.
#    $(call new_target,debug)
#    debug: override CXXFLAGS += -g -D_GLIBCXX_ASSERTIONS   <- Target-specific flags.
#    $(call new_target,release)
#    release: override CXXFLAGS += -DNDEBUG -O3
#    release: override LDFLAGS += -O3 -s -mwindows
#
#       <- File-specific flags. You can apply flags to several flags by using `%` as a wildcard symbol.
#    obj/lib/glfl.cpp.o: override CXXFLAGS += -O3
#    obj/lib/implementation.cpp.o: override CXXFLAGS += -O3
#
#       <- Precompiled heders. See definition of `PRECOMPILED_HEADERS` below for explanation.
#    PRECOMPILED_HEADERS ?= src/game/*.cpp>src/game/master.hpp
#
# Other undocumented variables might exist.
#
# Machine-local config (such as compiler paths) is stored in `.local_config.mk`.
# You will be asked to create this file and add specific variables to it if they're missing.
#
# You might want to add `.*.mk` to your `.gitignore`.


# --- DEFINITIONS ---

# Some constants.
override comma := ,
override dollar := $
override define lf :=
$(strip)
$(strip)
endef

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
override rmfile = del /F /Q $(subst /,\,$1) $(silence)
override rmdir = rd /S /Q $(subst /,\,$1) $(silence)
override mkdir = mkdir >NUL 2>NUL $(subst /,\,$1) $(silence)
override echo = echo $(subst <,^<,$(subst >,^>,$1))
override native_path = $(subst /,\,$1)
override cur_dir := $(subst \,/,$(shell echo %CD%))
else
override silence = >/dev/null 2>/dev/null || true
override rmfile = rm -f $1 $(silence)
override rmdir = rm -rf $1 $(silence)
override mkdir = mkdir -p $1 $(silence)
override echo = echo "$(subst ",\",$(subst \,\\,$1))"
override native_path = $1
ifeq ($(HOST_OS),windows)
# We're probably on MSYS2 or Cygwin or something similar.
override cur_dir := $(shell cygpath -m $(dollar)PWD)
else
override cur_dir := $(PWD)
endif
endif


# --- IMPORT LOCAL CONFIG

# Default value with higher priority than env variables
POST_BUILD_COMMANDS =

# Include the config
-include .local_config.mk

# Default value with lower priority than env variables
WINDRES ?= windres

ifeq (,$(or $(C_COMPILER), $(CXX_COMPILER)))
$(error No compiler specified.\
   $(lf)Define `C_COMPILER` and/or `CXX_COMPILER` in `.local_config.mk` or somewhere else.\
   $(lf)..)
endif
ifeq (,$(or $(C_LINKER), $(CXX_LINKER)))
$(error No linker specified.\
   $(lf)Define `C_LINKER` and/or `CXX_LINKER` in `.local_config.mk` or somewhere else.\
   $(lf)Normally they should be equal to `C_COMPILER` and `CXX_COMPILER`.\
   $(lf)\
   $(lf)If you're using Clang, you can also add `-fuse-ld=lld` to those variables to greatly improve linking times. See comments in the makefile for details.\
   $(lf)..)
# Using `-fuse-ld=lld` enables Clang's experimental LLD linker.
# If you're using LLD on Windows, note following:
# * MSYS2's LLD appears to be broken as of now (often hangs when run), so if you want LLD, you need to install the official binaries, then specify
#     path to the official clang/clang++ binary in `C/CXX_LINKER`, along with `-target ...` flag with a target matching whatever target MSYS2's `clang --version` outputs.
# * LLD will generate `<exec_name>.lib` file alongside the resulting binary. Add following to `.local_config.mk` to automatically delete it:
#
#     ifeq ($(TARGET_OS),windows)
#     POST_BUILD_COMMANDS = @$(call rmfile, $(OUTPUT_FILE).lib)
#     endif
#
endif


# --- CONFIG FUNCTIONS ---
override define new_target =
$(eval
.PHONY: $(strip $1)
$(strip $1): generic_build
)
endef


# --- DEFAULT VARIABLE VALUES ---
SOURCES =
SOURCE_DIRS = .
OUTPUT_FILE = program
OBJECT_DIR = obj
CFLAGS = -std=c11 -Wall -Wextra -pedantic-errors -g
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic-errors -g
LDFLAGS =
LINKER_MODE = CXX # C or CXX

PRECOMPILED_HEADERS =
# `PRECOMPILED_HEADERS` must be a space-separated list of precompiled header entries.
# Each entry is written as `patterns>header`, where `header` is a header file name (without `.gch`)
# and `patterns` is a `|`-separated list of file patterns (using `*` as the wildcard character).
# Files that match a pattern will use this precompiled header.
# Example: `PRECOMPILED_HEADERS = src/game/*.cpp|src/states/*.cpp>src/pch.hpp lib/*.c>lib/common.h`.


# --- INCLUDE PROJECT CONFIG ---
-include build_options.mk


# --- LOCATE FILES ---

# Source files.
override SOURCES += $(strip $(foreach dir,$(SOURCE_DIRS),$(call rwildcard, $(dir), *.c *.cpp *.rc))) # Note the `+=`.

# Object files.
override objects = $(SOURCES:%=$(OBJECT_DIR)/%.o)

# Dependency lists
override dep_files = $(patsubst %.o,%.d,$(filter %.c.o %.cpp.o,$(objects)))

# Add a proper extension to the output file.
OUTPUT_FILE_EXT = $(OUTPUT_FILE)$(extension_exe)


# --- HANDLE PRECOMPILED HEADERS ---
# Here we add precompiled headers as dependencies for corresponding source files. The rest is handled automatically
$(foreach x,$(PRECOMPILED_HEADERS),$(foreach y,$(filter $(subst *,%,$(subst |, ,$(word 1,$(subst >, ,$x)))),$(SOURCES)),$(eval $(OBJECT_DIR)/$y.o: $(OBJECT_DIR)/$(word 2,$(subst >, ,$x)).gch)))
override dep_files += $(foreach x,$(PRECOMPILED_HEADERS),$(OBJECT_DIR)/$(word 2,$(subst >, ,$x)).d)


# --- COMBINE FLAGS ---
override CFLAGS += $(CFLAGS_EXTRA)
override CXXFLAGS += $(CXXFLAGS_EXTRA)


# --- TARGETS ---

# Target: generic build
.PHONY: generic_build
generic_build: $(OUTPUT_FILE_EXT)

# An internal target that actually builds the executable.
# Note that object files come before linker flags.
$(OUTPUT_FILE_EXT): $(objects)
	@$(call echo,[Linking] $(OUTPUT_FILE_EXT))
	@$(call mkdir,$(dir $@))
	@$($(LINKER_MODE)_LINKER) $(objects) $(LDFLAGS) -o $@
	$(if $(POST_BUILD_COMMANDS),@$(call echo,[Finishing]))
	$(POST_BUILD_COMMANDS)
	@$(call echo,[Done])

# Internal targets that build the source files.
# Note that flags come before the files.
# * C sources
$(OBJECT_DIR)/%.c.o: %.c
	@$(call echo,[C] $<)
	@$(call mkdir,$(dir $@))
	@$(C_COMPILER) -MMD -MP $(foreach f,$(filter %.gch,$^),-include-pch $f) $(CFLAGS) $< -c -o $@
# * C++ sources
$(OBJECT_DIR)/%.cpp.o: %.cpp
	@$(call echo,[C++] $<)
	@$(call mkdir,$(dir $@))
	@$(CXX_COMPILER) -MMD -MP $(foreach f,$(filter %.gch,$^),-include-pch $f) $(CXXFLAGS) $< -c -o $@
# * C precompiled headers
$(OBJECT_DIR)/%.h.gch: %.h
	@$(call echo,[C header] $<)
	@$(call mkdir,$(dir $@))
	@$(C_COMPILER) -MMD -MP $(CFLAGS) $< -c -o $@
# * C++ precompiled headers
$(OBJECT_DIR)/%.hpp.gch: %.hpp
	@$(call echo,[C++ header] $<)
	@$(call mkdir,$(dir $@))
	@$(CXX_COMPILER) -MMD -MP $(CXXFLAGS) $< -c -o $@
# * Windows resources
$(OBJECT_DIR)/%.rc.o: %.rc
	@$(call echo,[Resource] $<)
	@$(call mkdir,$(dir $@))
	@$(WINDRES) $(WINDRES_FLAGS) -i $< -o $@

# Target: clean the build
.PHONY: clean
clean:
	@$(call echo,[Cleaning])
	@$(call rmdir,$(OBJECT_DIR))
	@$(call rmfile,$(OUTPUT_FILE_EXT))
	@$(call echo,[Done])

# Helpers for generating compile_commands.json
EXCLUDE_FILES =
EXCLUDE_DIRS =
override EXCLUDE_FILES += $(foreach dir,$(EXCLUDE_DIRS), $(call rwildcard,$(dir),*.c *.cpp *.h *.hpp))
override include_files = $(filter-out $(EXCLUDE_FILES), $(SOURCES))
override file_command = && $(call echo,{"directory": "."$(comma) "file": "$(cur_dir)/$2"$(comma) "command": "$1 $2"}$(comma)) >>compile_commands.json
override all_commands = $(foreach file,$(filter %.c,$(include_files)),$(call file_command,$(C_COMPILER) $(CFLAGS),$(file))) $(foreach file,$(filter %.cpp,$(include_files)),$(call file_command,$(CXX_COMPILER) $(CXXFLAGS),$(file)))
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
	@$(call rmfile,compile_commands.json)

# Import saved target
override saved_target =
-include .current_target.mk

# Target: set target for `make current`
TARGET =
.PHONY: set_current
.PHONY: set_current_clean
ifeq ($(TARGET),)
set_current_clean:
set_current:
	@$(call echo,Set TARGET variable to the desired target name.)
	@exit 1
else
ifeq ($(TARGET),$(strip $(saved_target)))
set_current_clean:
set_current:
else
set_current_clean: set_current clean
set_current:
	@$(call echo,override saved_target := $(TARGET)) >.current_target.mk
	@$(call echo,[Target] -> $(TARGET))
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
