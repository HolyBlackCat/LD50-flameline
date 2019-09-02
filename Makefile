# This file is an universal makefile for simple C/C++ executable projects.
#
#
# ---- COMPILING
#
# -- CONFIGURATION
#
# This makefile doesn't contain any project-specific details.
# Project configuration (i.e. sources and compiler settings) is stored in `project_config.mk`.
#
# Machine-specific configuration (i.e. what compiler to use) is stored in `local_config.mk`.
# If `local_config.mk` is absent, you will be asked to create it.
# Make sure it contains valid compiler paths before building.
#
# -- WINDOWS SUPPORT
#
# Windows builds are supported, but not with MSVC compiler.
# You need to use GCC or Clang. (You can get them from MSYS2.)
# On windows you need MSYS2 shell (or something similar).
# Native Windows shell with (`mingw32-make` instead of `make`) may or may not work, since `mingw32-make` is buggy.
#
# -- USAGE TL;DR
#
# Inspect `local_config.mk` and adjust any compiler and tool paths if necessary.
#
# Then run `make mode=release -j4`. If this project doesn't support 'release'
# build mode, you'll be given a list of the available modes to pick from.
#
# Some projects might expect you to provide a set of prebuilt dependencies, consult a project documentation.
#
# -- USAGE
#
# This makefile uses a concept of 'build modes'.
# A build mode is essentially a named set of compiler settings, such as 'debug', 'release', and so on.
# See `project_config.mk` for the list of available modes for this project (look
# for `$(call new_mode, ...)` mode declarations).
#
# Object files and precompiled headers for each mode are stored separately,  which allows switching
# between modes quickly. But this doesn't apply to the resulting executable, so it will always be
# relinked when you build after changing the mode.
#
# A list of public `make` targets is provided below.
#
# Targets marked with `[M]` are affected by the selected build mode. Those targets can be
# invoked with `mode=...` (`...` = mode name) to selected a specific mode.
# Last used mode is remembered (saved to `.current_mode.mk`) and is used by default until changed again.
# If you haven't selected a mode yet, failure to specify `mode=...` is a error.
# Targets NOT marked with `[M]` are not affected by current build mode and don't accept the `mode=...` flag.
##
# [M] * `make`                - Same as `make build`.
# [M] * `make build`          - Builds the project using current build mode.
# [M] * `make use`            - Switches to a different build mode without doing anything else. `mode=...` has to be specified.
# [M] * `make clean_mode`     - Deletes object files and precompiled headers for the current build mode. The executable is not deleted.
# [M] * `make clean_pch`      - Deletes precompiled headers for the current build mode. The executable is not deleted.
#     * `make clean`          - Deletes all object files, precompiled headers, and the executable. If we're on Linux and dependency management
#                               is enabled, also clean the startup script (see below).
#     * `make commands`       - Generate `compile_commands.json`. Mode-specific flags are ignored.
#     * `make commands_fixed` - Generate `compile_commands.json` with deliberately broken commands for some files.
#                               When using VS Code with Clangd, this helps to disable Clangd for specific files to avoid unwanted warnings.
#                               See commends on the declaration of `commands_fixed` for usage details.
#     * `make clean_commands` - Delete `compile_commands.json`.
#
# Next, there is a 'dependency management' feature that projects can enable.
# If a project uses dependency management, it relies on a packa of prebuilt libraries that have to be downloaded separately.
# When you build the project for the first time, at the very beginning of the build sequence, an expected location of those dependencies
# will be checked, and some information about them will be collected.
# Then, at the end of the build sequence, dynamic libraries used by the application (the standard ones,and the ones coming from the library
# pack, as determined by `LDD`) will be copied to the build directory.
# Additionally, if you're on Linux, a startup script is generated in the build directory. It's the intended way to start the resulting application.
# It adds the current directory (containing dynamic libraries) to LD_LIBRARY_PATH, and then runs the executable.
#
# If you enable dependency management, several additional targets are enabled:
#
#	  * `make find_deps`      - Checks if the prebuilt dependencies are in the correct location. Runs `pkg-config` on them and saves the results.
#                               This runs automatically at the beginning of the build sequence, but you can use this target to check dependencies
#								without building anything.
#	  * `make clean_dynamic_libs` - Erases dynamic libraries that were previously copied from a pack of prebuilt dependencies into the build
#									directory, and the ones that constitute the standard library and were copied from system directories.
#									If they're missing, they will be copied back in place at the end of the build sequence.
#	  * `make clean_deps`     - Same as `clean_dynamic_libs`, but also erases the files generated by `find_deps`.
#
#
# The project can be cleaned completely by running `make clean clean_deps clean_commands`.
#
#
# End of documentation.


# --- DEFAULT TARGET ---
build:


# --- DEFINITIONS ---

# Some constants.
override space := $(strip) $(strip)
override comma := ,
override dollar := $
override open_par := (
override close_par := )
override define lf :=
$(strip)
$(strip)
endef

# If a target depends on `__force_this_target`, it will be always considered outdated.
.PHONY: __force_this_target
__force_this_target:

# A recursive wildcard function.
# Source: https://stackoverflow.com/a/18258352/2752075
# Recursively searches a directory for all files matching a pattern.
# The first parameter is a directory, the second is a pattern.
# Example usage: SOURCES = $(call rwildcard, src, *.c *.cpp)
# This implementation differs from the original. It was changed to correctly handle directory names without trailing `/`.
override rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Same as `$(shell ...)`, but triggers a error on failure.
ifeq ($(filter --trace,$(MAKEFLAGS)),)
override safe_shell = $(shell $1)$(if $(filter-out 0,$(.SHELLSTATUS)),$(error Unable to execute `$1`, status $(.SHELLSTATUS)))
else
override safe_shell = $(info Shell command: $1)$(shell $1)$(if $(filter-out 0,$(.SHELLSTATUS)),$(error Unable to execute `$1`, status $(.SHELLSTATUS)))
endif

# Same as `safe_shell`, but discards the output and expands to a single space.
override safe_shell_exec = $(call space,$(call safe_shell,$1))

# Trim space on the left of $1.
override trim_left = $(eval override __trim_left_var := $1)$(__trim_left_var)$(eval override undefine __trim_left_var)

# Checks if $2 contains any word from $1 as a substring.
override contains_any_of = $(word 1,$(foreach x,$1,$(findstring $x,$2)))


# --- DETECT ENVIRONMENT ---

# Host OS.
ifeq ($(OS),Windows_NT)
HOST_OS ?= windows
else
HOST_OS ?= linux
endif

ifeq ($(HOST_OS),windows)
override host_win_linux = $1
override host_extension_exe := .exe
override host_native_path = $(subst /,\,$1)
else
override host_win_linux = $2
override host_extension_exe :=
override host_native_path = $1
endif

# Target OS.
TARGET_OS ?= $(HOST_OS)

ifeq ($(TARGET_OS),windows)
override target_win_linux = $1
override extension_exe := .exe
override extension_dll := .dll
override directory_dll := bin
else
override target_win_linux = $2
override extension_exe :=
override extension_dll := .so
override directory_dll := lib
endif

# Host shell.
ifeq ($(shell echo "foo"),"foo")
HOST_SHELL ?= windows
else
HOST_SHELL ?= linux
endif

ifeq ($(HOST_SHELL),windows)
override shell_win_linux = $1
else
override shell_win_linux = $2
endif

# Shell-specific functions.
# Example usage: $(call rmfile,bin/out.exe)
override silence = >$(dev_null) 2>$(dev_null) || $(success)
ifeq ($(HOST_SHELL),windows)
# - Utilities
override dev_null := NUL
override success := (exit 0)
# - Shell commands
override rmfile = del /F /Q "$1" $(silence)
override rmdir = rd /S /Q "$1" $(silence)
override mkdir = mkdir "$1" $(silence)
override move = move /Y "$1" "$2"
override copy = copy /Y "$1" "$2"
override touch = type nul >>"$1"
override echo = echo $(subst <,^<,$(subst >,^>,$1))
override echo_lf := echo.
override pause := pause
override set_env = set "$1=$2"
# - Functions
override native_path = $(subst /,\,$1)
override dir_target_name = $(patsubst %\,%,$(subst /,\,$1))
else
# - Utilities
override dev_null := /dev/null
override success := true
# - Shell commands
override rmfile = rm -f '$1'
override rmdir = rm -rf '$1'
override mkdir = mkdir -p '$1'
override move = mv -f '$1' '$2'
override copy = cp -f '$1' '$2'
override touch = touch '$1'
override echo = echo '$(subst ','"'"',$1)'
override echo_lf := echo
override pause := read -s -n 1 -p "Press any key to continue . . ." && echo
override set_env = export "$1=$2"
# - Functions
override native_path = $1
override dir_target_name = $1
endif


# --- IMPORT LOCAL CONFIG ---

# You can override default values specified below in `local_config.mk`.

POST_BUILD_COMMANDS :=

# Note that error messages below are triggered lazily, only when the corresponding variables are used.
C_COMPILER = $(error No C compiler specified.\
	$(lf)Define `C_COMPILER` in `local_config.mk` or directly when invoking `make`)

CXX_COMPILER = $(error No C++ compiler specified.\
	$(lf)Define `CXX_COMPILER` in `local_config.mk` or directly when invoking `make`)

C_LINKER = $(error No C linker specified.\
    $(lf)Define `C_LINKER` in `local_config.mk` or directly when invoking `make`.\
    $(lf)Normally it should be equal to `C_COMPILER`.\
    $(lf)\
    $(lf)If you're using Clang, consider using LLD linker to improve linking times. See comments in the makefile for details)

CXX_LINKER = $(error No C++ linker specified.\
    $(lf)Define `CXX_LINKER` in `local_config.mk` or directly when invoking `make`.\
    $(lf)Normally it should be equal to `CXX_COMPILER`.\
    $(lf)\
    $(lf)If you're using Clang, consider using LLD linker to improve linking times. See comments in the makefile for details)

# Using LLD linker with Clang
#
# To use LLD linker, append `-fuse-ld=lld` to `*_LINKER` variables.
# If you're using LLD on Windows, LLD will generate `<exec_name>.lib` file alongside the resulting binary. Add following to `local_config.mk` to automatically delete it:
#
#     ifeq ($(TARGET_OS),windows)
#     POST_BUILD_COMMANDS = @$(call rmfile,$(OUTPUT_FILE).lib)
#     endif
#

PKGCONFIG := pkg-config

ifeq ($(TARGET_OS),windows)
WINDRES := windres
else
WINDRES = $(error Attempt to use `windres`, but we're not on Windows)
endif

ifeq ($(TARGET_OS),windows)
LDD := ntldd -R
else
LDD := ldd
endif

# Following variables have no effect if dependency management is disabled:
# [
LIBRARY_PACK_DIR := ../_dependencies
# Will look for dependencies in this directory.
STD_LIB_NAME_PATTERNS := libgcc libstdc++ libc++ pthread librt
# If one of those strings is present in a dynamic library name, that library is considered to be a part of the standard library.
BANNED_LIBRARY_PATH_PATTERNS :=
ifeq ($(TARGET_OS),windows)
BANNED_LIBRARY_PATH_PATTERNS += :/Windows
endif
# If one of those strings is present in a path to a library that matches `STD_LIB_NAME_PATTERNS`, a error will be emitted.
# ]


# Include the config
-include local_config.mk


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
OUTPUT_FILE_EXT = $(OUTPUT_FILE)$(extension_exe)# Output filename with extension. Normally you don't need to touch this, override `OUTPUT_FILE` instead. Note that we can't use `:=` here.
OBJECT_DIR := obj
CFLAGS := -std=c11 -Wall -Wextra -pedantic-errors -g
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic-errors -g
WINDRES_FLAGS := -O res
LDFLAGS :=
LINKER_MODE := CXX# C or CXX
ALLOW_PCH := 1# 0 or 1

PRECOMPILED_HEADERS :=
# Controls precompiled headers. Must be a `|`-separated list.
# Each entry is written as `patterns>header`, where `header` is a header file name (probably ending in `.h` or `.hpp`)
# and `patterns` is a space-separated list of file names or name patterns (using `*` as a wildcard character).
# Files that match a pattern will use this precompiled header.
# Example: `PRECOMPILED_HEADERS = src/game/*.cpp src/states/*.cpp > src/pch.hpp | lib/*.c > lib/common.h`.
# If `ALLOW_PCH` == 0, then headers are not precompiled, but are still included using compiler flags.

FILE_SPECIFIC_FLAGS :=
# Applies additional flags to specific files. Must be a `|`-separated list.
# Each entry is written as `patterns>flags`, where `patterns` is a space-separated list of file names or
# name patterns (using `*` as a wildcard character) and `flags` is a space-separated list of flags.
# Example: `FILE_SPECIFIC_FLAGS = lib/*.cpp > -03 -ffast-math | src/game/main.cpp > -O0`.

LIBRARY_PACK_NAME :=
# If non-empty, enables automatic dependency (library) management.
# It should be set to a name of a library pack.

# Following variables have no effect if dependency management is disabled:
# [
STARTUP_SCRIPT := _start
# This is used only on linux.
# A shell script with this name will be created in the same directory as $(OUTPUT_FILE).
# Running this script will start the executable with a proper LD_LIBRARY_PATH, which allows
# us to use shared libraries from the same directory as the executable itself.
STARTUP_SCRIPT_PATH = $(dir $(OUTPUT_FILE_EXT))/$(STARTUP_SCRIPT)
# Normally you don't need to adjust this manually. Note that we can't use `:=` here.
USED_PACKAGES :=
# Used pkg-config packages from the library pack.
# ]


# --- ADDITIONAL MAKEFILE LOCATIONS ---
override current_mode_file := $(OBJECT_DIR)/current_mode.mk
override lib_pack_info_file := $(OBJECT_DIR)/library_pack_info.mk


# --- INCLUDE LIBRARY PACK INFO FILE ---
override deps_library_pack_name :=
override deps_packages :=
override deps_compiler_flags :=
override deps_linker_flags :=
-include $(lib_pack_info_file)


# --- INCLUDE PROJECT CONFIG ---
-include project_config.mk
override dependency_management_enabled := $(if $(strip $(LIBRARY_PACK_NAME)),yes)


# --- SELECT BUILD MODE ---

# Prevent env variables from overriding `target`.
mode :=
override mode := $(strip $(mode))
override mode_list := $(strip $(mode_list))

# Try loading saved mode name
override current_mode :=
-include $(current_mode_file)
override current_mode := $(strip $(current_mode))
$(if $(current_mode),$(if $(call mode_exists,$(current_mode)),,$(error Invalid mode name specified in `$(current_mode_file)`. Delete that file (or do `make clean`) and try again)))

# Internal: Make sure a valid mode is selected. Save current mode to the config file if necessary.
# Note that mingw32-make doesn't seem to have `else if[n]eq` conditionals, so we have to use the `__else` hack.
.PHONY: __check_mode
# If no mode is selected.
ifeq ($(or $(mode),$(current_mode)),)
__check_mode:
	$(error No build mode selected.\
		$(lf)Add `mode=...` to the flags. Selected mode will be remembered and used by default until changed.\
		$(lf)You can also do `make use mode=...` to change mode without doing anything else.\
		$(lf)Supported modes are: $(mode_list))
override __else := 0
else
override __else := 1
endif

# If using a saved mode.
ifeq ($(__else),1)
ifeq ($(mode),)
__check_mode:
	$(info [Mode] $(current_mode))
override __else := 0
else
override __else := 1
endif
endif

# If mode was specified with a flag.
ifeq ($(__else),1)
__check_mode:
	$(if $(call mode_exists,$(mode)),,$(error Invalid build mode specified.\
		$(lf)Expected one of: $(mode_list)))
	$(info [Mode] $(strip $(old_mode) -> $(mode)))
ifneq ($(mode),$(current_mode)) # Specified mode differs from the saved one, file update is necessary.
	@$(call echo,override current_mode := $(mode))>$(current_mode_file)
endif

ifneq ($(mode),$(current_mode)) # Specified mode differs from the saved one.
override mode_changed := 1
endif

override old_mode := $(current_mode)
override current_mode := $(mode)
endif

# Internal: Make sure no mode is specified in the flags. (Use this for targets that don't need to know build mode.)
.PHONY:
__no_mode_needed:
	$(if $(mode),$(error This operation doesn't require a build mode))


# --- COMBINE FLAGS ---
override common_object_dir := $(OBJECT_DIR)
override OBJECT_DIR := $(OBJECT_DIR)/$(current_mode)

CFLAGS_EXTRA :=
CXXFLAGS_EXTRA :=
LDFLAGS_EXTRA :=
FILE_SPECIFIC_FLAGS_EXTRA :=
override CFLAGS += $(CFLAGS_EXTRA)
override CXXFLAGS += $(CXXFLAGS_EXTRA)
override LDFLAGS += $(LDFLAGS_EXTRA)
override FILE_SPECIFIC_FLAGS += $(FILE_SPECIFIC_FLAGS_EXTRA)


# --- LOCATE FILES ---

# Source files.
override source_file_extensions := *.c *.cpp
ifeq ($(TARGET_OS),windows)
override source_file_extensions += *.rc
endif

override SOURCES += $(strip $(foreach dir,$(SOURCE_DIRS),$(call rwildcard,$(dir),$(source_file_extensions))))# Note the `+=`.

# Object files.
override objects := $(SOURCES:%=$(OBJECT_DIR)/%.o)

# Dependency lists
override dep_files := $(patsubst %.o,%.d,$(filter %.c.o %.cpp.o,$(objects)))


# --- HANDLE PRECOMPILED HEADERS ---
# A raw list of all precompiled header entries. You can iterate over it, then use following functions to extract data from each element.
override pch_all_entries = $(subst |, ,$(subst $(space),<,$(PRECOMPILED_HEADERS)))
# Extract all file patterns from a precompiled header entry.
override pch_entry_file_patterns = $(subst *,%,$(subst <, ,$(word 1,$(subst >, ,$1))))
# Extract header name from a precompiled header entry.
override pch_entry_header = $(strip $(subst <, ,$(word 2,$(subst >, ,$1))))
# List of all precompiled header files.
override compiled_headers := $(foreach x,$(pch_all_entries),$(OBJECT_DIR)/$(call pch_entry_header,$x).gch)
# Add precompiled headers as dependencies for corresponding source files. The rest is handled automatically.
$(foreach x,$(pch_all_entries),$(foreach y,$(filter $(call pch_entry_file_patterns,$x),$(SOURCES)),$(eval $(OBJECT_DIR)/$y.o: $(OBJECT_DIR)/$(call pch_entry_header,$x).gch)))
ifeq ($(strip $(ALLOW_PCH)),1)
# Register dependency files for precompiled headers.
override dep_files += $(foreach x,$(pch_all_entries),$(OBJECT_DIR)/$(call pch_entry_header,$x).d)
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
override files_requiring_folders := $(OUTPUT_FILE_EXT) $(objects) $(compiled_headers) $(current_mode_file) $(lib_pack_info_file)
$(foreach x,$(files_requiring_folders),$(eval $x: | $(call dir_target_name,$(dir $x))))
$(foreach x,$(sort $(dir $(files_requiring_folders))),$(eval $(call dir_target_name,$x): ; @$(call mkdir,$x)))


# --- TARGETS ---

# Public: set mode without doing anything else.
.PHONY: use
use: __check_mode
	$(if $(mode),,$(error No build mode specified.\
		$(lf)Add `mode=...` to the flags to change mode))

# Public: build stuff.
.PHONY: build
build: __check_mode __mode_$(current_mode)

# Public: clean most files.
.PHONY: clean
clean: __no_mode_needed
	@$(call echo,[Cleaning] Everything)
	@$(call rmdir,$(common_object_dir))
	@$(call rmfile,$(OUTPUT_FILE_EXT))
ifneq ($(strip $(LIBRARY_PACK_NAME)),)
	@$(call rmfile,$(STARTUP_SCRIPT_PATH))
endif
	@$(call echo,[Done])

# Public: clean files for the current build mode, including precompiled headers but not including the executable.
.PHONY: clean_mode
clean_mode: __check_mode
	@$(call echo,[Cleaning] Mode '$(mode)')
	@$(call rmdir,$(OBJECT_DIR))
	@$(call echo,[Done])

# Public: clean precompiled headers for the current build mode.
.PHONY: clean_pch
clean_pch: __check_mode
	@$(call echo,[Cleaning] PCH for mode '$(mode)') $(foreach x,$(compiled_headers),&& ($(call rmfile,$x)))
	@$(call echo,[Done])

# Public: clean absolutely everything.
.PHONY: clean_everything
clean_everything: clean clean_commands
ifneq ($(dependency_management_enabled),)
clean_everything: clean_deps
endif

# Internal: Generic build. This is used by `__mode_*` targets.
.PHONY: __generic_build
__generic_build: $(OUTPUT_FILE_EXT)

# Internal: Actually build the executable.
# Note that object files come before linker flags.
$(OUTPUT_FILE_EXT): $(objects)
	@$(call echo,[Linking] $(OUTPUT_FILE_EXT))
	@$($(LINKER_MODE)_LINKER) $(objects) $(LDFLAGS) -o $@
	@$(if $(POST_BUILD_COMMANDS),$(call echo,[Finishing]))
	$(POST_BUILD_COMMANDS)
	@$(call echo,[Done])

# Make sure the executable is rebuilt correctly if build mode changes.
ifneq ($(strip $(mode_changed)),)
$(OUTPUT_FILE_EXT): __force_this_target
else
$(OUTPUT_FILE_EXT): $(current_mode_file)
endif

# Internal targets that build the source files.
# Note that flags come before the files. Note that file-specific flags aren't passed to `add_pch_to_flags`, to prevent removal of `-include` from them.
# * C sources
$(OBJECT_DIR)/%.c.o: %.c
	@$(call echo,[C] $<)
	@$(strip $(C_COMPILER) -MMD -MP $(call add_pch_to_flags,$(CFLAGS),$(filter %.gch,$^)) $(file_local_flags) $< -c -o $@)
# * C++ sources
$(OBJECT_DIR)/%.cpp.o: %.cpp
	@$(call echo,[C++] $<)
	@$(strip $(CXX_COMPILER) -MMD -MP $(call add_pch_to_flags,$(CXXFLAGS),$(filter %.gch,$^)) $(file_local_flags) $< -c -o $@)
ifeq ($(strip $(ALLOW_PCH)),1)
# * C precompiled headers
$(OBJECT_DIR)/%.h.gch: %.h
	@$(call echo,[C header] $<)
	@$(strip $(C_COMPILER) -MMD -MP $(CFLAGS) $(file_local_flags) $< -c -o $@)
# * C++ precompiled headers
$(OBJECT_DIR)/%.hpp.gch: %.hpp
	@$(call echo,[C++ header] $<)
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
	@$(call echo,[Resource] $<)
	@$(WINDRES) $(WINDRES_FLAGS) -i $< -o $@

# Helpers for generating compile_commands.json
# Note that we avoid using `:=` on some variables here because they aren't used often.
EXCLUDE_FILES :=
EXCLUDE_DIRS :=
ifneq ($(and $(filter windows,$(HOST_OS)),$(filter linux,$(HOST_SHELL))),)
# If we're on windows and use linux-style shell, we need to fix the path. `/c/foo/bar` -> `c:/foo/bar`
override current_dir = $(subst <, ,$(subst $(space),/,$(strip $(join $(subst /, ,$(subst $(space),<,$(CURDIR))),:))))
else
override current_dir = $(CURDIR)
endif
override EXCLUDE_FILES += $(foreach d,$(EXCLUDE_DIRS), $(call rwildcard,$d,*.c *.cpp *.h *.hpp))
override include_files = $(filter-out $(EXCLUDE_FILES), $(SOURCES))
override get_file_headers = $(foreach x,$(pch_all_entries),$(if $(filter $(call pch_entry_file_patterns,$x),$1),-include $(call pch_entry_header,$x)))
override get_file_local_flags = $(foreach x,$(subst |, ,$(subst $(space),<,$(FILE_SPECIFIC_FLAGS))),$(if $(filter $(subst *,%,$(subst <, ,$(word 1,$(subst >, ,$x)))),$1),$(subst <, ,$(word 2,$(subst >, ,$x)))))
override file_command = && $(call echo,{"directory": "$(current_dir)"$(comma) "file": "$(current_dir)/$3"$(comma) "command": "$(strip $1 $2 $(call get_file_headers,$3) $3)"}$(comma)) >>compile_commands.json
override all_commands = $(foreach f,$(filter %.c,$(include_files)),$(call file_command,$(C_COMPILER),$(CFLAGS) $(call get_file_local_flags,$3),$f)) \
						$(foreach f,$(filter %.cpp,$(include_files)),$(call file_command,$(CXX_COMPILER),$(CXXFLAGS) $(call get_file_local_flags,$3),$f))
override all_stub_commands = $(foreach file,$(EXCLUDE_FILES),$(call file_command,,,$(file)))

# Public: generate `compile_commands.json`.
# Any target-specific flags are ignored.
.PHONY: commands
commands: __no_mode_needed
	@$(call echo,[Generating] compile_commands.json)
	@$(call echo,[) >compile_commands.json $(all_commands) && $(call echo,]) >>compile_commands.json
	@$(call echo,[Done])

# Public: same as `commands`, but `compile_commands.json` will contain invalid commands for `EXCLUDE_FILES` and all files from `EXCLUDE_DIRS`.
# This helps to get rid on unwanted warnings in files not under your control when using clangd, by disabling clangd on those excluded files.
.PHONY: commands_fixed
commands_fixed: __no_mode_needed
	@$(call echo,[Generating] compile_commands.json (fixed))
	@$(call echo,[) >compile_commands.json $(all_commands) $(all_stub_commands) && $(call echo,]) >>compile_commands.json
	@$(call echo,[Done])

# Public: remove compile_commands.json.
.PHONY: clean_commands
clean_commands: __no_mode_needed
	@$(call echo,[Cleaning] Commands)
	@$(call rmfile,compile_commands.json)
	@$(call echo,[Done])


# --- OPTIONAL LIBRARY MANAGEMENT ---

# A list of dynamic libraries currently sitting in the build directory.
override dynamic_libraries := $(wildcard $(dir $(OUTPUT_FILE_EXT))/*$(extension_dll))
override erase_dynamic_libraries = $(if $(dynamic_libraries),@$(success) $(foreach x,$(dynamic_libraries),&& $(call rmfile,$x)))

# Public: erase dynamic libraries that were copied from the prebuilt dependencies.
.PHONY: clean_dynamic_libs
clean_dynamic_libs: __no_mode_needed
	@$(call echo,[Cleaning] Dynamic libraries)
	$(erase_dynamic_libraries)
	@$(call echo,[Done])

# Public: same as `clean_dynamic_libs`, but also erase cached library information.
.PHONY: clean_deps
clean_deps: __no_mode_needed
	@$(call echo,[Cleaning] Dependencies)
	$(erase_dynamic_libraries)
	@$(call rmfile,$(lib_pack_info_file))
	@$(call echo,[Done])

# Public: update dependency information.
# This calls `pkg-config` on a directory that should contain prebuilt dependencies, and saves the flags that it reports.
# Yes, this is effectively an empty target. As long as it's mentioned in `targets_requiring_deps_info`, it does the job.
.PHONY: find_deps
find_deps: __no_mode_needed
	$(if $(MAKE_RESTARTS),@$(call echo,[Done]))


override targets_requiring_deps_info := build commands commands_fixed find_deps


ifneq ($(dependency_management_enabled),)
ifneq ($(strip $(if $(MAKECMDGOALS),$(filter $(targets_requiring_deps_info),$(MAKECMDGOALS)),yes)),)# Don't update dependencies unless necessary.


# Generate a startup script (right after building the executable)
ifeq ($(strip $(TARGET_OS)),linux)
__generic_build: | $(STARTUP_SCRIPT_PATH)
$(STARTUP_SCRIPT_PATH): | $(OUTPUT_FILE_EXT)
	@$(call echo,[Deps] Generating startup script)
	@$(call echo,#!/bin/sh) >$(STARTUP_SCRIPT_PATH)
	@$(call echo,export "LD_LIBRARY_PATH=$$(pwd)/$$(dirname $$0):$$LD_LIBRARY_PATH" && cd $$(pwd)/$$(dirname $$0) && ./$(notdir $(OUTPUT_FILE_EXT))) >>$(STARTUP_SCRIPT_PATH)
	@chmod +x $(STARTUP_SCRIPT_PATH)
endif


# A path to the library pack.
override library_pack_path := $(LIBRARY_PACK_DIR)/$(LIBRARY_PACK_NAME)

# Same as $(PKGCONFIG), but with some commands to set proper library paths.
# Using `=` here instead of `:=`, because this variable isn't required very often.
override pkgconfig_with_path = $(call set_env,PKG_CONFIG_PATH,) && $(call set_env,PKG_CONFIG_LIBDIR,$(library_pack_path)/lib/pkgconfig) && $(PKGCONFIG)

# Runs `pkgconfig_with_path` with $1 (--cflags or --libs) as flags, and sanitizes the results.
# Any `-rpath` flags are removed.
override run_pkgconfig = $(strip $(foreach x,$(call safe_shell,$(pkgconfig_with_path) --define-prefix $1 $(USED_PACKAGES)),$(if $(findstring -rpath,$x),,$x)))


# Check if we should update the dependencies.
ifneq ($(strip $(LIBRARY_PACK_NAME)),$(strip $(deps_library_pack_name)))
$(lib_pack_info_file): __force_this_target
endif
ifneq ($(strip $(USED_PACKAGES)),$(strip $(deps_packages)))
$(lib_pack_info_file): __force_this_target
endif


# Generate the library pack info.
# Note that `deps_packages` is the last variable. If `pkg-config` fails, the file won't contain this variable, which will cause it to be regenerated.
$(lib_pack_info_file):
	$(info [Deps] Updating dependency information...)
	$(if $(wildcard $(library_pack_path)),,$(error Prebuilt dependencies not found in `$(library_pack_path)`))
	$(erase_dynamic_libraries)
	$(call safe_shell_exec,$(call echo,override deps_library_pack_name := $(LIBRARY_PACK_NAME)) >$@)
	$(call safe_shell_exec,$(call echo,override deps_compiler_flags := $(call run_pkgconfig,--cflags)) >>$@)
	$(call safe_shell_exec,$(call echo,override deps_linker_flags := $(call run_pkgconfig,--libs)) >>$@)
	$(call safe_shell_exec,$(call echo,override deps_packages := $(USED_PACKAGES)) >>$@)
	$(eval include $(lib_pack_info_file))
	$(info [Deps] Packages used:)
	$(info [Deps]   $(deps_packages))
	$(info [Deps] Compiler flags:)
	$(info [Deps]   $(deps_compiler_flags))
	$(info [Deps] Linker flags:)
	$(info [Deps]   $(deps_linker_flags))


# Copy dynamic libraries (right after building the executable)
ifeq ($(dynamic_libraries),)

override deps_entry_dir = $(subst >, ,$(word 1,$(subst <, ,$1)))
override deps_entry_name_long = $(subst >, ,$(word 2,$(subst <, ,$1)))
override deps_entry_name = $(call deps_entry_name_long,$(word 1,$(subst .so,.so ,$1)))
override deps_entry_summary = $(call deps_entry_dir,$1) >> $(call deps_entry_name,$1)

# A list of dynamic libraries found in the library pack.
override available_libs := $(notdir $(wildcard $(library_pack_path)/$(directory_dll)/*$(extension_dll)))

ifeq ($(TARGET_OS),linux)
override ldd_with_path := $(call set_env,LD_LIBRARY_PATH,$(library_pack_path)/$(directory_dll):$$LD_LIBRARY_PATH) && $(LDD)
else
ifeq ($(HOST_SHELL),windows)
override ldd_with_path := $(call set_env,PATH,$(call native_path,$(library_pack_path)/$(directory_dll));%PATH%) && $(LDD)
else
override ldd_with_path := $(call set_env,PATH,$(call native_path,$(library_pack_path)/$(directory_dll));$$PATH) && $(LDD)
endif
endif

.PHONY: __dynamic_libs
__generic_build: | __dynamic_libs
__dynamic_libs: | $(OUTPUT_FILE_EXT)
	$(info [Deps] Determining dependencies for `$(OUTPUT_FILE_EXT)`)
	$(erase_dynamic_libraries)
	$(eval override _local_dep_list := $(call safe_shell,$(ldd_with_path) -- $(OUTPUT_FILE_EXT)))
	$(if $(strip $(_local_dep_list)),,$(error Got empty output from LDD))
	$(eval override _local_dep_list := $(subst $(space)=>$(space),<,$(subst $(space)$(open_par),<,$(subst $(close_par) ,|,$(_local_dep_list)))))
	$(eval override _local_dep_list := $(subst |, ,$(subst $(space),>,$(subst | ,|,$(foreach x,$(_local_dep_list),$(call trim_left,$x))))))
	$(eval override _local_dep_list := $(sort $(foreach x,$(_local_dep_list),$(if $(filter 2,$(words $(subst <, ,$x))),./,$(subst \,/,$(dir $(word 2,$(subst <, ,$x)))))<$(word 1,$(subst <, ,$x)))))
	$(foreach x,$(_local_dep_list),$(info [Deps] - $(call deps_entry_summary,$x)))
	$(eval override _local_std_libs := $(foreach x,$(_local_dep_list),$(if $(call contains_any_of,$(STD_LIB_NAME_PATTERNS),$(call deps_entry_name,$x)),$x)))
	$(eval override _local_other_libs := $(foreach x,$(_local_dep_list),$(if $(filter $(available_libs),$(call deps_entry_name,$x)),$x)))
	$(foreach x,$(_local_std_libs) $(_local_other_libs),$(if $(call contains_any_of,$(BANNED_LIBRARY_PATH_PATTERNS),$(call deps_entry_dir,$x)),\
		$(error Didn't expect to find `$(call deps_entry_name,$x)` in `$(call deps_entry_dir,$x)`)))
	$(info [Deps] Following libraries will be copied to `$(dir $(OUTPUT_FILE_EXT))`:)
	$(foreach x,$(_local_std_libs),$(info [Deps] - [standard] $(call deps_entry_summary,$x)))
	$(foreach x,$(_local_other_libs),$(info [Deps] - [external] $(call deps_entry_summary,$x)))
	@$(foreach x,$(_local_std_libs) $(_local_other_libs),$(call copy,$(call deps_entry_dir,$x)$(call deps_entry_name_long,$x),$(dir $(OUTPUT_FILE_EXT))) &&) $(call echo,[Deps] Copying completed)

endif

endif
endif


# --- INCLUDE DEPENDENCIES ---
-include $(dep_files)
