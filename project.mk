# --- Build modes ---

_proj_win_subsystem :=
_proj_commonflags :=
_proj_cxxflags :=
_proj_ldflags :=

$(call NewMode,debug)
$(Mode)COMMON_FLAGS := -g
$(Mode)CXXFLAGS := -D_GLIBCXX_DEBUG

$(call NewMode,debug_soft)
$(Mode)COMMON_FLAGS := -g
$(Mode)CXXFLAGS := -D_GLIBCXX_ASSERTIONS

$(call NewMode,release)
$(Mode)COMMON_FLAGS := -O3
$(Mode)_proj_commonflags := -flto
$(Mode)CXXFLAGS := -DNDEBUG
$(Mode)LDFLAGS := -s
$(Mode)_proj_win_subsystem := -mwindows

$(call NewMode,profile)
$(Mode)COMMON_FLAGS := -O3 -pg
$(Mode)CXXFLAGS := -DNDEBUG
$(Mode)_proj_win_subsystem := -mwindows

$(call NewMode,sanitize_address)
$(Mode)COMMON_FLAGS := -g -fsanitize=address
$(Mode)CXXFLAGS := -D_GLIBCXX_DEBUG

$(call NewMode,sanitize_ub)
$(Mode)COMMON_FLAGS := -g -fsanitize=undefined
$(Mode)CXXFLAGS := -D_GLIBCXX_DEBUG

DIST_NAME := *_$(TARGET_OS)_v1.^
ifneq ($(MODE),release)
DIST_NAME := $(DIST_NAME)_$(MODE)
endif

# --- Project config ---

# ASSETS_IGNORED_PATTERNS += atlas.*

_proj_cxxflags += -std=c++2b -pedantic-errors -Wall -Wextra -Wdeprecated -Wextra-semi -Wno-gnu-zero-variadic-macro-arguments
_proj_cxxflags += -include src/program/common_macros.h -include src/program/parachute.h
_proj_cxxflags += -Isrc -Ilib/include
_proj_cxxflags += -Ilib/include/cglfl_gl3.2_core # OpenGL version

ifeq ($(TARGET_OS),windows)
_proj_ldflags += $(_proj_win_subsystem)
endif

_file_cxxflags = $(if $(filter lib/implementation.cpp lib/cglfl.cpp,$1),-g0 -O3)

$(call Project,exe,flameline)
$(call ProjectSetting,source_dirs,src lib)
$(call ProjectSetting,common_flags,$(_proj_commonflags))
$(call ProjectSetting,cxxflags,$(_proj_cxxflags))
$(call ProjectSetting,ldflags,$(_proj_ldflags))
$(call ProjectSetting,flags_func,_file_cxxflags)
$(call ProjectSetting,pch,src/game/*->src/game/master.hpp)
$(call ProjectSetting,libs,*)
$(call ProjectSetting,bad_lib_flags,-Dmain=%>>>-DIMP_ENTRY_POINT_OVERRIDE=%)
ifeq ($(TARGET_OS),windows)
$(call ProjectSetting,sources,src/icon.ico)
endif

src/icon.ico: $(wildcard src/icon_*.png)
	$(info [Png to icon] $@)
	@convert $^ $@


# --- Dependencies ---

# Don't need anything on Windows.
# On Linux, install following for SDL2 (from docs/README-linux.md)
# sudo apt-get install build-essential git make cmake autoconf automake \
    libtool pkg-config libasound2-dev libpulse-dev libaudio-dev \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev \
    libxinerama-dev libxxf86vm-dev libxss-dev libgl1-mesa-dev libdbus-1-dev \
    libudev-dev libgles2-mesa-dev libegl1-mesa-dev libibus-1.0-dev \
    fcitx-libs-dev libsamplerate0-dev libsndio-dev libwayland-dev \
    libxkbcommon-dev libdrm-dev libgbm-dev
# This list was last updated for SDL 2.0.20.
# `libjack-dev` was removed from the list, because it caused weird package conflicts on Ubuntu 21.10.


# --- Libraries ---

DIST_DEPS_ARCHIVE := https://github.com/HolyBlackCat/LD50-flameline/releases/download/v1.0/deps_v1.zip

_win_is_x32 :=
_win_sdl2_arch := $(if $(_win_is_x32),i686-w64-mingw32,x86_64-w64-mingw32)

# Disable unnecessary stuff.
# Note that cmake logs say that "BUILD_CLSOCKET BUILD_CPU_DEMOS BUILD_ENET" variables we set aren't "used".
# But even if we don't set them, they still appear in the cmake cache, so we set them just to be sure.
_bullet_flags := -DBUILD_BULLET2_DEMOS:BOOL=OFF -DBUILD_EXTRAS:BOOL=OFF -DBUILD_OPENGL3_DEMOS:BOOL=OFF \
	-DBUILD_UNIT_TESTS:BOOL=OFF -DBUILD_CLSOCKET:BOOL=OFF -DBUILD_CPU_DEMOS:BOOL=OFF -DBUILD_ENET:BOOL=OFF
# Use doubles instead of floats.
_bullet_flags += -DUSE_DOUBLE_PRECISION:BOOL=ON
# Disable shared libraries. This should be the default behavior (with the flags above), but we also set it for a good measure.
_bullet_flags += -DBUILD_SHARED_LIBS:BOOL=OFF
# This defaults to off if the makefile flavor is not exactly "Unix Makefiles", which is silly.
# That used to cause 'make install' to not install anything useful.
_bullet_flags += -DINSTALL_LIBS:BOOL=ON

_openal_flags := -DALSOFT_EXAMPLES=FALSE
# Enable SDL2 backend.
_openal_flags += -DALSOFT_REQUIRE_SDL2=TRUE -DALSOFT_BACKEND_SDL2=TRUE
ifneq ($(TARGET_OS),windows)
# On Linux, disable all the extra backends to make sure we only depend on SDL2.
# The list of backends was obtained by stopping the build after configuration, and looking at the CMake variables.
# We don't disable `ALSOFT_BACKEND_SDL2`, and also `ALSOFT_BACKEND_WAVE` (which is a backend that writes to a file, so it's harmless).
# The list of backends was last updated at OpenAL-soft 1.21.1.
_openal_flags += -DALSOFT_BACKEND_ALSA=FALSE -DALSOFT_BACKEND_OSS=FALSE -DALSOFT_BACKEND_PULSEAUDIO=FALSE -DALSOFT_BACKEND_SNDIO=FALSE
endif

# $(call Library,box2d-2.4.1.tar.gz)
#   $(call LibrarySetting,cmake_flags,-DBOX2D_BUILD_UNIT_TESTS:BOOL=OFF -DBOX2D_BUILD_TESTBED:BOOL=OFF)


# $(call Library,bullet3-3.22b_no-examples.tar.gz)
#   # The `_no-examples` suffix on the archive indicates that following directories were removed from it: `./data`, and everything in `./examples` except `CommonInterfaces`.
#   # This decreases the archive size from 170+ mb to 10+ mb.
#   $(call LibrarySetting,cmake_flags,$(_bullet_flags))
$(call Library,double-conversion-3.2.0.tar.gz)
$(call Library,fmt-8.1.1.zip)
  $(call LibrarySetting,cmake_flags,-DFMT_TEST=OFF)
$(call Library,freetype-2.12.0.tar.gz)
  $(call LibrarySetting,deps,zlib-1.2.12)
  # When using CMake, freetype generates a broken `.pc` file. First seen on Freetype 2.12.0. Last tested on 2.12.0.
  $(call LibrarySetting,build_system,configure_make)
  # Freetype manages to find system zlib and bzip2, despite the automatic prefix configuration.
  # Specifying a custom zlib as dependency overrides it, but we don't ship bzip2, so we need to explicitly disable it.
  # It seems their tasks for other libraries respect our prefix, so we don't disable them explicitly.
  $(call LibrarySetting,configure_flags,--without-bzip2)
$(call Library,libogg-1.3.5.tar.gz)
  # When built with CMake on MinGW, ogg/vorbis can't decide whether to prefix the libraries with `lib` or not.
  # The resulting executable doesn't find libraries because of this inconsistency.
  $(call LibrarySetting,build_system,configure_make)
$(call Library,libvorbis-1.3.7.tar.gz)
  $(call LibrarySetting,deps,ogg-1.3.5)
  # See vorbis for why we use configure+make.
  $(call LibrarySetting,build_system,configure_make)
$(call Library,openal-soft-1.21.1.tar.bz2)
ifeq ($(TARGET_OS),windows)
  $(call LibrarySetting,deps,SDL2-devel-2.0.20-mingw)
else
  $(call LibrarySetting,deps,SDL2-2.0.20)
endif
  $(call LibrarySetting,cmake_flags,$(_openal_flags))
ifeq ($(TARGET_OS),windows)
$(call Library,SDL2-devel-2.0.20-mingw.tar.gz)
  $(call LibrarySetting,build_system,copy_files)
  $(call LibrarySetting,copy_files,$(_win_sdl2_arch)->.)
# $(call Library,SDL2_net-devel-2.0.1-mingw.tar.gz)
#   $(call LibrarySetting,build_system,copy_files)
#   $(call LibrarySetting,copy_files,$(_win_sdl2_arch)->.)
else
$(call Library,SDL2-2.0.20.tar.gz)
  # Allow SDL to see system packages. If we were using `configure+make`, we'd need `configure_vars = env -uPKG_CONFIG_PATH -uPKG_CONFIG_LIBDIR` instead.
  $(call LibrarySetting,cmake_flags,-DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=ON)
# $(call Library,SDL2_net-2.0.1.tar.gz)
#   $(call LibrarySetting,deps,SDL2-2.0.20)
endif
$(call Library,zlib-1.2.12.tar.gz)
  # CMake support in ZLib is jank. On MinGW it builds `libzlib.dll`, but pkg-config says `-lz`. Last checked on 1.2.12.
  $(call LibrarySetting,build_system,configure_make)
  # Need to set `cc`, otherwise their makefile uses the executable named `cc` to link, which doesn't support `-fuse-ld=lld-N`, it seems. Last tested on 1.2.12.
  $(call LibrarySetting,configure_vars,cc="$$CC")
