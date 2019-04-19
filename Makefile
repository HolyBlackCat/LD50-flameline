# This file is an universal makefile for simple C/C++ executable projects.
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
#    set_current       - Selects a target based on the value of `TARGET`. The setting is saved in `.current_mode.mk`.
#    set_current_clean - Same as `set_current`, but also cleans the repo.
#
#
#
# It requires a GCC-like compiler with support for `-MMD -MP` flags.
# You need to have a separate file called `project_config.mk` in the same directory, which has to contain some high-level build options.
# An example of such file is provided below:
#
#    override SOURCE_DIRS += src lib   <- All *.c/*.cpp/*.rc files from these directories will be compiled and linked.
#    override SOURCES += icon.rc   <- As well as these files.
#
#    OBJECT_DIR = obj   <- Objects will be placed here.
#
#    OUTPUT_FILE = bin/ball-game   <- Name for the resulting binary. On Windows ".exe" is appended automatically.
#
#       <- Compiler flags
#    CXXFLAGS = -Wall -Wextra -pedantic-errors -std=c++2a   <- Those can be easily overriden by user.
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
#       <- File-specific flags. See definition of `FILE_SPECIFIC_FLAGS` below for explanation.
#    FILE_SPECIFIC_FLAGS = lib/*.cpp > -03
#
#       <- Precompiled heders. See definition of `PRECOMPILED_HEADERS` below for explanation.
#    PRECOMPILED_HEADERS = src/game/*.cpp>src/game/master.hpp
#
# Other undocumented variables might exist.
#
# Machine-local config (such as compiler paths) is stored in `.local_config.mk`.
# You will be asked to create this file and add specific variables to it if they're missing.
#
# You might want to add `.*.mk` to your `.gitignore`.


# --- DEFAULT TARGET ---
build:


# --- DEFINITIONS ---

# Some constants.
override space := $(strip) $(strip)
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
override touch = type nul >>$1 2>NUL || (exit 0)
override echo = echo $(subst <,^<,$(subst >,^>,$1))
override native_path = $(subst /,\,$1)
override cur_dir := $(subst \,/,$(shell echo %CD%))
else
override silence = >/dev/null 2>/dev/null || true
override rmfile = rm -f $1 $(silence)
override rmdir = rm -rf $1 $(silence)
override mkdir = mkdir -p $1 $(silence)
override touch = touch $1 $(silence)
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

# Default settings with higher priority than env variables
POST_BUILD_COMMANDS =

# Include the config
-include .local_config.mk

# Default settings with lower priority than env variables
WINDRES ?= windres

ifeq (,$(or $(C_COMPILER), $(CXX_COMPILER)))
$(error No compiler specified.\
	$(lf)Define `C_COMPILER` and/or `CXX_COMPILER` in `.local_config.mk` or directly when invoking `make`)
endif
ifeq (,$(or $(C_LINKER), $(CXX_LINKER)))
$(error No linker specified.\
	$(lf)Define `C_LINKER` and/or `CXX_LINKER` in `.local_config.mk` or directly when invoking `make`.\
	$(lf)Normally they should be equal to `C_COMPILER` and `CXX_COMPILER`.\
	$(lf)\
	$(lf)If you're using Clang, consider using LLD linker to improve linking times. See comments in the makefile for details)
# To use LLD linker, append `-fuse-ld=lld` to `*_LINKER` variables.
# If you're using LLD on Windows, note following:
# * MSYS2's LLD appears to be broken as of now (often hangs when run), so you need to install the official binaries. Specify
#     path to the official clang/clang++ binary in `C/CXX_LINKER`, along with `-target ...` flag with a target matching whatever target MSYS2's `clang --version` outputs.
# * LLD will generate `<exec_name>.lib` file alongside the resulting binary. Add following to `.local_config.mk` to automatically delete it:
#
#     ifeq ($(TARGET_OS),windows)
#     POST_BUILD_COMMANDS = @$(call rmfile,$(OUTPUT_FILE).lib)
#     endif
#
endif


# --- CONFIG FUNCTIONS ---
override mode_list :=
override last_mode :=

# Function: make new mode.
# Example: `$(call new_mode,debug)`.
override define new_mode =
$(eval
override mode_list += $1
override last_mode := $1
.PHONY: __mode_$(strip $1)
__mode_$(strip $1): __generic_build
)
endef

# Function: set mode-specific flags. It only affects the last defined mode.
# Example: `$(mode_flags) CXXFLAGS += -g`.
override mode_flags = __mode_$(last_mode): override$(space)

# Internal function: check if mode exists.
# Note that we shouldn't use `findstring` here, since it can match a part of mode name.
override mode_exists = $(filter $1,$(mode_list))


# --- DEFAULT VARIABLE VALUES ---
SOURCES :=
SOURCE_DIRS := .
OUTPUT_FILE := program
OUTPUT_FILE_EXT = $(OUTPUT_FILE)$(extension_exe) # Output filename with extension. Normally you don't need to touch this, override `OUTPUT_FILE` instead. Note that we can't use `:=` here.
OBJECT_DIR := obj
CFLAGS := -std=c11 -Wall -Wextra -pedantic-errors -g
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic-errors -g
LDFLAGS :=
LINKER_MODE := CXX # C or CXX
ALLOW_PCH := 1 # 0 or 1

PRECOMPILED_HEADERS :=
# Controls recompiled headers. Must be a space-separated list.
# Each entry is written as `patterns>header`, where `header` is a header file name (without `.gch`)
# and `patterns` is a `|`-separated list of file names or name patterns (using `*` as a wildcard character).
# Files that match a pattern will use this precompiled header.
# Example: `PRECOMPILED_HEADERS = src/game/*.cpp|src/states/*.cpp>src/pch.hpp lib/*.c>lib/common.h`.
# If `ALLOW_PCH` == 0, then headers are not precompiled, but are still included using compiler flags.

FILE_SPECIFIC_FLAGS :=
# Applies additional flags to specific files. Must be a `|`-separated list.
# Each entry is written as `patterns>flags`, where `patterns` is a space-separated list of file names or
# name patterns (using `*` as a wildcard character) and `flags` is a space-separated list of flags.
# Example: `FILE_SPECIFIC_FLAGS = lib/*.cpp > -03 -ffast-math | src/game/main.cpp > -O0`.


# --- INCLUDE PROJECT CONFIG ---
-include project_config.mk


# --- SELECT BUILD MODE ---

# Prevent env variables from overriding `target`.
mode :=

# Try loading saved mode name
override current_mode :=
-include .current_mode.mk
$(if $(current_mode),$(if $(call mode_exists,$(current_mode)),,$(error Invalid mode name specified in `.current_mode.mk`. Delete that file and try again)))

# Target: Make sure a valid mode is selected. Save current mode to the config file if necessary.
.PHONY: __check_mode
__check_mode:
ifeq ($(strip $(mode)),)
ifeq ($(strip $(current_mode)),)
__check_mode:
	$(error No build mode selected.\
		$(lf)Add `mode=...` to the flags. Selected mode will be remembered and used by default until changed.\
		$(lf)You can also do `make use mode=...` to change mode without doing anything else)
endif
else
ifneq ($(strip $(mode)),$(strip $(current_mode)))
# Check if specified target is valid
__check_mode:
	$(if $(call mode_exists,$(mode)),,$(error Invalid build mode specified.\
		$(lf)Expected one of: $(mode_list)))
	@$(call echo,override current_mode := $(mode)) >.current_mode.mk
	$(info [Info] Changed build mode to: $(mode))
override current_mode := $(mode)
endif
endif

# Target: Make sure no mode is specified in the flags. (Use this for targets that don't need to know build mode.)
.PHONY:
__no_mode_needed:
	$(if $(mode),$(error This operation doesn't require a build mode))


# --- COMBINE FLAGS ---
override common_object_dir := $(OBJECT_DIR)
override OBJECT_DIR := $(OBJECT_DIR)/$(current_mode)
override CFLAGS += $(CFLAGS_EXTRA)
override CXXFLAGS += $(CXXFLAGS_EXTRA)
override FILE_SPECIFIC_FLAGS += $(FILE_SPECIFIC_FLAGS_EXTRA)


# --- LOCATE FILES ---

# Source files.
override SOURCES += $(strip $(foreach dir,$(SOURCE_DIRS),$(call rwildcard, $(dir), *.c *.cpp *.rc))) # Note the `+=`.

# Object files.
override objects := $(SOURCES:%=$(OBJECT_DIR)/%.o)

# Dependency lists
override dep_files := $(patsubst %.o,%.d,$(filter %.c.o %.cpp.o,$(objects)))


# --- HANDLE PRECOMPILED HEADERS ---
# List of all precompiled headers.
override pch_headers := $(foreach x,$(PRECOMPILED_HEADERS),$(word 2,$(subst >, ,$x)))
# Add precompiled headers as dependencies for corresponding source files. The rest is handled automatically.
$(foreach x,$(PRECOMPILED_HEADERS),$(foreach y,$(filter $(subst *,%,$(subst |, ,$(word 1,$(subst >, ,$x)))),$(SOURCES)),$(eval $(OBJECT_DIR)/$y.o: $(OBJECT_DIR)/$(word 2,$(subst >, ,$x)).gch)))
ifeq ($(strip $(ALLOW_PCH)),1)
# Register dependency files for precompiled headers.
override dep_files += $(foreach x,$(PRECOMPILED_HEADERS),$(OBJECT_DIR)/$(word 2,$(subst >, ,$x)).d)
# This function processes compiler flags for files. If this file uses a PCH, all `-include` flags are removed and the PCH is included instead.
override add_pch_to_flags = $(if $2,$(filter-out -include|%,$(subst -include ,-include|,$(strip $1))) -include $(patsubst %.gch,%,$2),$1)
else
# This function processes compiler flags for files when PCHs are disabled. If this file uses a PCH, non-precompiled version of that header is included.
override add_pch_to_flags = $1 $(if $2,-include $(patsubst $(OBJECT_DIR)/%.gch,%,$2))
endif


# --- HANDLE FILE-SPECIFIC FLAGS ---
override file_local_flags :=
$(foreach x,$(subst |, ,$(subst $(space),<,$(FILE_SPECIFIC_FLAGS))),$(foreach y,$(filter $(subst *,%,$(subst <, ,$(word 1,$(subst >, ,$x)))),$(SOURCES)),\
																	  $(eval $(OBJECT_DIR)/$y.o: override file_local_flags += $(strip $(subst <, ,$(word 2,$(subst >, ,$x)))))))


# --- RULES FOR CREATING FOLDERS ---
override files_requiring_folders = $(OUTPUT_FILE_EXT) $(objects) $(pch_headers)
$(foreach x,$(files_requiring_folders),$(eval $x: | $(dir $x)))
$(foreach x,$(sort $(dir $(files_requiring_folders))),$(eval $x: ; @$(call mkdir, $x)))


# --- TARGETS ---

# Public: set mode without doing anything else.
.PHONY: use
use: __check_mode
	$(if $(mode),,$(error No build mode specified.\
		$(lf)Add `mode=...` to the flags to change mode))

# Public: build stuff.
.PHONY: build
build: __check_mode __mode_$(current_mode)

# Public: clean everything.
.PHONY: clean
clean: __no_mode_needed
	$(info [Cleaning] <everything>)
	@$(call rmdir,$(common_object_dir))
	@$(call rmfile,$(OUTPUT_FILE_EXT))
	$(info [Done])

# Public: clean files for the current build mode.
.PHONY: clean_mode
clean_mode: __check_mode
	$(info [Cleaning] $(current_mode))
	@$(call rmdir,$(OBJECT_DIR))
	@$(call rmfile,$(OUTPUT_FILE_EXT))
	$(info [Done])

# Internal: generic build. This is used by `__mode_*` targets.
.PHONY: __generic_build
__generic_build: $(OUTPUT_FILE_EXT)

# Internal: actually build the executable.
# Note that object files come before linker flags.
$(OUTPUT_FILE_EXT): $(objects)
	$(info [Linking] $(OUTPUT_FILE_EXT))
	@$($(LINKER_MODE)_LINKER) $(objects) $(LDFLAGS) -o $@
	$(if $(POST_BUILD_COMMANDS),$(info [Finishing]))
	$(POST_BUILD_COMMANDS)
	$(info [Done])

# Internal targets that build the source files.
# Note that flags come before the files. Note that file-specific flags aren't passed to `add_pch_to_flags`, to prevent removal of `-include` from them.
# * C sources
$(OBJECT_DIR)/%.c.o: %.c
	$(info [C] $<)
	@$(strip $(C_COMPILER) -MMD -MP $(call add_pch_to_flags,$(CFLAGS),$(filter %.gch,$^)) $(file_local_flags) $< -c -o $@)
# * C++ sources
$(OBJECT_DIR)/%.cpp.o: %.cpp
	$(info [C++] $<)
	@$(strip $(CXX_COMPILER) -MMD -MP $(call add_pch_to_flags,$(CXXFLAGS),$(filter %.gch,$^)) $(file_local_flags) $< -c -o $@)
ifeq ($(strip $(ALLOW_PCH)),1)
# * C precompiled headers
$(OBJECT_DIR)/%.h.gch: %.h
	$(info [C header] $<)
	@$(strip $(C_COMPILER) -MMD -MP $(CFLAGS) $(file_local_flags) $< -c -o $@)
# * C++ precompiled headers
$(OBJECT_DIR)/%.hpp.gch: %.hpp
	$(info [C++ header] $<)
	@$(strip $(CXX_COMPILER) -MMD -MP $(CXXFLAGS) $(file_local_flags) $< -c -o $@)
else
# * C precompiled headers (skip)
$(OBJECT_DIR)/%.h.gch: %.h
	@$(call touch,$@)
# * C++ precompiled headers (skip)
$(OBJECT_DIR)/%.hpp.gch: %.hpp
	@$(call touch,$@)
endif
# * Windows resources
$(OBJECT_DIR)/%.rc.o: %.rc
	$(info [Resource] $<)
	@$(WINDRES) $(WINDRES_FLAGS) -i $< -o $@

# Helpers for generating compile_commands.json
# Note that we avoid using `:=` here because those aren't used often.
EXCLUDE_FILES =
EXCLUDE_DIRS =
override EXCLUDE_FILES += $(foreach dir,$(EXCLUDE_DIRS), $(call rwildcard,$(dir),*.c *.cpp *.h *.hpp))
override include_files = $(filter-out $(EXCLUDE_FILES), $(SOURCES))
override get_file_headers = $(foreach x,$(PRECOMPILED_HEADERS),$(if $(filter $(subst *,%,$(subst |, ,$(word 1,$(subst >, ,$x)))),$1),-include $(word 2,$(subst >, ,$x))))
override get_file_local_flags = $(foreach x,$(subst |, ,$(subst $(space),<,$(FILE_SPECIFIC_FLAGS))),$(if $(filter $(subst *,%,$(subst <, ,$(word 1,$(subst >, ,$x)))),$1),$(subst <, ,$(word 2,$(subst >, ,$x)))))
override file_command = && $(call echo,{"directory": "."$(comma) "file": "$(cur_dir)/$3"$(comma) "command": "$(strip $1 $2 $(call get_file_headers,$3) $(call get_file_local_flags,$3) $3)"}$(comma)) >>compile_commands.json
override all_commands = $(foreach f,$(filter %.c,$(include_files)),$(call file_command,$(C_COMPILER),$(CFLAGS),$f)) \
						$(foreach f,$(filter %.cpp,$(include_files)),$(call file_command,$(CXX_COMPILER),$(CXXFLAGS),$f))
override all_stub_commands = $(foreach file,$(EXCLUDE_FILES),$(call file_command,,,$(file)))

# Public: generate `compile_commands.json`.
#   Target-specific
.PHONY: commands
commands: __no_mode_needed
	$(info [Generating] compile_commands.json)
	@$(call echo,[) >compile_commands.json $(all_commands) && $(call echo,]) >>compile_commands.json
	$(info [Done])

# Public: same as `commands`, but `compile_commands.json` will contain invalid commands for `EXCLUDE_FILES` and all files from `EXCLUDE_DIRS`.
#   This helps to get rid on unwanted warnings in files not under your control when using clangd, by disabling clangd on those excluded files.
.PHONY: commands_fixed
commands_fixed: __no_mode_needed
	$(info [Generating] compile_commands.json (fixed))
	@$(call echo,[) >compile_commands.json $(all_commands) $(all_stub_commands) && $(call echo,]) >>compile_commands.json
	$(info [Done])

# Public: `remove compile_commands.json`.
.PHONY: clean_commands
clean_commands: __no_mode_needed
	@$(call rmfile,compile_commands.json)


# --- INCLUDE DEPENDENCIES ---
-include $(dep_files)
