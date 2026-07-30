# LLVMBOXFAST

Portable, self-contained LLVM toolchain distribution built via GitHub Actions with `ccache` acceleration.

Based on [rsms/llvmbox](https://github.com/rsms/llvmbox) but with simplified CI build and LLVM 22.1.0 support.

## What is this?

A minimized LLVM build that ships clang, lld, compiler-rt, libc++, libc++abi, libunwind — with no external dependencies.

## Quick start

```sh
tar xf llvmbox-*.tar.xz
./llvmbox-*/bin/clang --version
```

## Build

### Manual (via GitHub Actions)

1. Push a tag `v*` to trigger the release workflow
2. Wait for the build to complete (~4-6 hours for first build, ~15-30 min for cached)
3. Download the release artifact from the Releases page

### Trigger manually

Go to **Actions** → **Build LLVMBOX** → **Run workflow** and specify the LLVM version.

## CI Features

- **ccache**: Incremental builds reduce time from hours to minutes
- **Artifact upload**: Every build produces a downloadable tarball
- **Automatic releases**: Push `v*` tag to create a draft Release
- **LLVM version input**: Build any LLVM version via workflow_dispatch

## Configuration

| Setting | Value |
|---------|-------|
| LLVM Version | 22.1.0 |
| Target Arch | X86, AArch64, ARM, RISCV, WebAssembly |
| Build CPUs | 2 (safe for 7GB RAM runners) |
| ccache size | 4 GB |
| CMake generator | Ninja |

## Source Inspired By

- [rsms/llvmbox](https://github.com/rsms/llvmbox) — The original llvmbox project
- [llvm/llvm-project](https://github.com/llvm/llvm-project) — LLVM source

## License

MIT
