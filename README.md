# LLVMBOXFAST

Portable, statically-linked LLVM toolkit built via GitHub Actions with ccache acceleration.

Based on the principles of [rsms/llvmbox](https://github.com/rsms/llvmbox) and drawing on techniques from [nwdxlgzs/llvm-box](https://github.com/nwdxlgzs/llvm-box).

## Features

- **Zero runtime dependencies** — binaries statically linked against musl libc
- **Built-in sysroot** — uses musl headers/libs, no system headers needed
- **ccache acceleration** — first build ~4-6h, subsequent builds <30 min
- **Multi-arch backends** — X86, AArch64, ARM, RISCV, WebAssembly

## Quick start

```sh
tar xf llvmbox-*.tar.xz
./llvmbox-*/bin/clang --version
./llvmbox-*/bin/clang -static -o hello hello.c
```

## Patches applied

Located in `patches/`:

| Patch | Source | Description |
|-------|--------|-------------|
| `clang-22.1.0-add-musl-triples.patch` | rsms/llvmbox | Adds musl triple support in GNU toolchain |
| `libcxx-22.1.0-musl-locale.patch` | rsms/llvmbox | Fix `strtoll_l`/`strtoull_l` on musl (not available) |
| `clang-22.1.0-gnu-as-needed.patch` | new | Default `--as-needed` for musl static linking |

## CI triggers

- Push to `main`/`master` → build + artifact upload
- Tag `v*` → build + draft Release creation
- `workflow_dispatch` → manual build with custom LLVM version

## How it works

```
┌──────────────────────────────────────────────────────────────────────────┐
│ 1. Download musl prebuilt toolchain (musl.cc)                            │
│ 2. Download LLVM 22.1.0 source tarball                                   │
│ 3. Apply patches (musl triples, locale fix, as-needed)                   │
│ 4. Configure CMake with musl sysroot + static linking                   │
│ 5. ninja build (clang + lld + compiler-rt + libc++ + libcxxabi + libunwind) │
│ 6. Strip + package as tar.xz                                             │
└──────────────────────────────────────────────────────────────────────────┘
```

## Build configuration

- `CMAKE_BUILD_TYPE=MinSizeRel`
- `LLVM_TARGETS_TO_BUILD=X86;AArch64;ARM;RISCV;WebAssembly`
- `LLVM_ENABLE_PROJECTS=clang;lld;clang-tools-extra`
- `LLVM_ENABLE_RUNTIMES=compiler-rt;libcxx;libcxxabi;libunwind`
- `CLANG_DEFAULT_CXX_STDLIB=libc++`
- `CLANG_DEFAULT_RTLIB=compiler-rt`
- `CLANG_DEFAULT_LINKER=lld`
- `CLANG_DEFAULT_TARGET_TRIPLE=x86_64-linux-musl`
- `CLANG_VENDOR=llvmbox`

## Sources & Inspiration

- [rsms/llvmbox](https://github.com/rsms/llvmbox) — musl-based portable LLVM
- [nwdxlgzs/llvm-box](https://github.com/nwdxlgzs/llvm-box) — in-process linker patches
- [musl.cc](https://musl.cc) — prebuilt musl cross-compiler toolchains

## License

MIT
