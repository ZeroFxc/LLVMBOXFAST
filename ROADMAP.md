# LLVMBOXFAST Development Roadmap

## Vision
Produce a single binary (`llvmbox`) that contains a complete LLVM/Clang/Lld toolchain,
runnable on Linux, Android (as executable), and usable as a shared library via JNI.

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    llvmbox.so (shared library)                    │
│                                                                 │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌──────────┐ │
│  │   LLVM     │  │   Clang    │  │    LLD     │  │ LLVM-Ar  │ │
│  │  Core      │  │   Driver   │  │   Linker   │  │  etc.    │ │
│  └────────────┘  └────────────┘  └────────────┘  └──────────┘ │
│                          ▲                                      │
│                          │ C / JNI API                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  include/llvobox.h  (stable C API)                       │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
         │
         │ JNI Interop
         ▼
┌──────────────────────┐
│ com.llvmbox.LLVMBox  │
│ (Java wrapper class) │
└──────────────────────┘
```

## File Layout (Final Package)

```
llvmbox-<version>-<platform>/
├── bin/
│   └── llvmbox              <- single binary (same as .so but executable)
├── lib/
│   └── llvmbox.so           <- shared library for Android JNI integration
├── include/
│   ├── llvmbox.h            <- stable C API header
│   └── jni/
│       └── llvmbox_jni.h    <- JNI bridge header
├── jni/
│   └── java/
│       └── com/llvmbox/
│           └── LLVMBox.java <- prebuilt Java wrapper
├── lib-dev/                 <- static libraries for custom tool building
│   ├── liball_llvm_clang_lld.a
│   ├── libLLVM*.a
│   ├── libClang*.a
│   └── liblld*.a
├── sysroot/                 <- built-in headers (for standalone use)
│   ├── include/             <- musl/clang headers
│   ├── lib/                 <- musl + compiler-rt + libc++
│   └── lib-lto/             <- ThinLTO libraries
└── share/
    └── llvmbox/
        └── llvmbox.cfg      <- default config
```

## Build Pipeline (CI)

```
                          ┌─────────────────┐
   push/tag ───────────►  │  GitHub Actions  │
                          └────────┬────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │                             │
              ┌─────▼─────┐               ┌───────▼──────┐
              │ Linux x64 │               │ Android A64  │
              │ Build     │               │ Cross-build  │
              └─────┬─────┘               └───────┬──────┘
                    │                             │
          ┌─────────┴──────────┐         ┌────────┴────────┐
          │                    │         │                 │
    ┌─────▼──────┐     ┌───────▼───┐ ┌───▼────────┐ ┌─────▼──────┐
    │  llvmbox   │     │  llvmbox  │ │ llvmbox    │ │  llvmbox   │
    │  (ELF bin) │     │  (.so)    │ │ (Android   │ │  (.so)     │
    │            │     │           │ │  ELF bin)  │ │ (Android)  │
    └────────────┘     └───────────┘ └────────────┘ └────────────┘
                                            │
                                     ┌──────┴──────┐
                                     │ Final .aar  │
                                     │ (Android    │
                                     │  Archive)   │
                                     └─────────────┘
```

## Phases

### Phase 1 (v0.3) ✅ DONE — Linux x86_64 base
- [x] Download LLVM source
- [x] Apply musl triple + libcxx locale patches
- [x] Build clang + lld + runtimes
- [x] Produce single binary `llvmbox`
- [x] Upload artifact + GitHub Release

### Phase 2 (v0.4) — Android ARM64 executable
- [ ] Add NDK download + cross-compile step
- [ ] Build `llvmbox` targeting `aarch64-linux-android`
- [ ] Test in Android emulator via adb
- [ ] Publish `llvmbox-<ver>-android-arm64`

**Key technical points for Android:**
- Android uses Bionic libc (not glibc, not musl)
- Need Android NDK r27+ for cross-compilation
- API level 24+ for 64-bit, API 21+ for 32-bit
- NDK provides: sysroot (`$NDK/sysroot`), clang wrappers (`$NDK/toolchains/llvm/`), linker (`ld.lld`)

### Phase 3 (v0.5) — Android .so + JNI
- [ ] Build llvmbox as position-independent shared library (.so)
- [ ] Add `-fPIC` + `-shared` flags
- [ ] Implement JNI bridge (`Java_com_llvmbox_LLVMBox_*`)
- [ ] Package as Android Archive (`.aar`)
- [ ] Add Java wrapper class (prebuilt)

**API for JNI:**
```java
// On Android
LLVMBox compiler = new LLVMBox();
compiler.compile("-O2 -o /data/local/tmp/hello hello.c", source);
```

### Phase 4 (v0.6) — ThinLTO + Dev libraries
- [ ] Ship LTO libraries (`lib-lto/`)
- [ ] Ship `liball_llvm_clang_lld.a` for building custom tools
- [ ] Add ThinLTO cache support

## Header File Guidelines

### Placement
- `include/llvobox.h` — ship this with both binary and .so distributions
- Keep it **stable** (don't break ABI between versions)
- Users include it from:
  - Their C/C++ host programs
  - JNI code (it functions as `extern "C"` symbols)
  - Android `.so` loading code

### Design Principles
1. **C API only** — use `extern "C"` to avoid name mangling
2. **Opaque handles** — `llvobox_session*` hides internal state
3. **Stack-style allocation** — creator/destroyer pattern, no global state
4. **Null-tolerant** — all functions accept NULL gracefully

## Android Cross-Compilation (How-To)

### Using NDK directly
```bash
# Download NDK
NDK=/path/to/android-ndk-r27
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
TARGET=aarch64-linux-android24

$TOOLCHAIN/bin/clang --target=$TARGET --sysroot=$NDK/sysroot  ...
```

### Via CMake (external toolchain)
```cmake
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 24)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_NDK /path/to/ndk)
set(CMAKE_ANDROID_ST_TYPE c++_static)
```

### Static vs Dynamic for Android
| Approach | Build Type | Dependencies | APK Size |
|----------|-----------|-------------|----------|
| Static bin | Binary | None (bionic always present) | ~60 MB |
| Shared .so | .so (PIC) | libc++_shared.so | ~40 MB |
| Static .so | .so (PIC, static link libcpp) | None | ~60 MB |

Recommendation: **static-link libc++** into a single PIC .so for easiest APK integration.

## References

- [rsms/llvmbox](https://github.com/rsms/llvmbox) — musl static linking, sysroot management
- [nwdxlgzs/llvm-box](https://github.com/nwdxlgzs/llvm-box) — in-process lld patch, unified entry
- [Android NDK CMake](https://developer.android.com/ndk/guides/cmake) — cross-compile toolchain setup
- [musl.cc](https://musl.cc) — prebuilt musl cross-compilers
- [musl.libc.org](https://musl.libc.org) — musl source tarballs
