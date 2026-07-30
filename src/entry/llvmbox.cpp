// llvmbox.cpp — unified entry point for the single-binary llvmbox toolchain
//
// This source becomes the main() of the final binary. It dispatches between
// the clang main and lld main based on argv[0] basename. Just like a
// multi-call binary (busybox, lld itself).
//
//   ./llvmbox hello.c -o hello          -> clang driver mode
//   ./llvmbox -cc1 ...                  -> forced cc1 front-end mode
//   ./llvmbox lld ...                   -> lld-mode (universal flavor)
//   ln -s ./llvmbox ld.lld  && ./ld.lld -> lld ELF flavor
//   ln -s ./llvmbox ld64.lld            -> lld Mach-O flavor
//   ln -s ./llvmbox lld-link            -> lld COFF flavor
//   ln -s ./llvmbox wasm-ld             -> lld WebAssembly flavor

#include "llvm/Config/llvm-config.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/LLVMDriver.h"

#include <algorithm>
#include <cstring>
#include <string>

// Defined in clang/tools/driver/driver.cpp and lld/tools/lld/lld.cpp
// These are C++ symbols — let the compiler mangle them naturally.
int clang_main(int argc, char **argv, const llvm::ToolContext &);
int lld_main(int argc, char **argv, const llvm::ToolContext &);

static std::string to_lower(llvm::StringRef s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return out;
}

static bool is_lld_arg(const char *a) {
    if (!a) return false;
    if (std::strcmp(a, "lld") == 0) return true;
    if (std::strcmp(a, "ld.lld") == 0) return true;
    if (std::strcmp(a, "ld64.lld") == 0) return true;
    if (std::strcmp(a, "lld-link") == 0) return true;
    if (std::strcmp(a, "wasm-ld") == 0) return true;
    if (std::strcmp(a, "link") == 0) return true;
    return false;
}

static const char *get_basename(const char *path) {
    if (!path) return path;
    const char *p = path;
    for (const char *q = path; *q; ++q) {
        if (*q == '/' || *q == '\\') p = q + 1;
    }
    return p;
}

int main(int argc, char **argv) {
    llvm::InitLLVM _(argc, argv);

    // Determine mode based on argv[0] basename (the name we were invoked as)
    std::string exe = get_basename(argv[0]);
    llvm::StringRef exeRef(exe);

    // Strip .exe suffix on Windows (case-insensitive)
    std::string lower = to_lower(exeRef);
    if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".exe") == 0)
        exeRef = exeRef.drop_back(4);

    bool lldMode = false;
    int argOffset = 0;

    // argv[0] basename dispatch
    if (exeRef == "ld.lld" || exeRef == "ld64.lld" ||
        exeRef == "lld-link" || exeRef == "wasm-ld") {
        lldMode = true;
        argOffset = 0;
    } else if (exeRef == "lld") {
        lldMode = true;
        argOffset = 0;
    } else if (argc >= 2 && is_lld_arg(argv[1])) {
        // ./llvmbox lld ...
        lldMode = true;
        argOffset = 1; // skip the "lld" token, keep next as argv[0] for lld
    }

    if (lldMode) {
        // lld's main expects its "argv[0]" to tell it which flavor to run.
        // For the symlink flavors (ld.lld etc.), argv[0] is already correct.
        // For the bare "ll" or "lld" case, we keep argv[0] as-is. For
        // "./llvmbox lld ...", drop argv[1] (the "lld" token).
        int lldArgc = argc - argOffset;
        char **lldArgv = argv + argOffset;
        return lld_main(lldArgc, lldArgv, {exe.data(), nullptr, false});
    }

    // Default: clang driver mode
    return clang_main(argc, argv, {exe.data(), nullptr, false});
}
