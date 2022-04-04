### Flameline

A small metroidvania with time travel, made for Ludum Dare \#50 in 48 hours.

Written in C++ using a [custom framework](https://github.com/HolyBlackCat/imp-re). Cross-compiled from Linux to Windows using [quasi-msys2](https://github.com/HolyBlackCat/quasi-msys2).

Grab the latest binaries at [itch.io](https://holyblackcat.itch.io/flameline) or at the [releases](https://github.com/HolyBlackCat/LD50-flameline/releases) page.

![screenshot](media/screenshot_1.png)

---

**Building**

MSVC is not supported. On Windows you need [MSYS2](https://stackoverflow.com/q/30069830). To cross-compile from Linux prefer [Quasi-MSYS2](https://github.com/HolyBlackCat/quasi-msys2).

You need at least following: latest `clang` (tested on Clang 14; on Windows prefer the version from MSYS2 repos), GCC 11 (for libstdc++), `make`, `cmake`, `zip`, `unzip`, `tar`, `zstd`, `rsync`, `patchelf` (on linux only), `ldd`**/**`ntldd` (for linux and windows respectively).

On Linux you also need to install the dependencies of SDL2, listed somewhere in [`project.mk`](/project.mk) (or in its readme). On Windows we use prebuild SDL2, so no dependencies are needed.

If you get weird makefile errors, you might need to update `make`. Get source [here](http://ftp.gnu.org/gnu/make/), compile using `./configure && make && sudo make install`.

To build and run, simply do `make`. No `-j...` necessary.

---

Licensed under [ZLIB LICENSE](LICENSE.md).
