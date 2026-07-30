// llvobox_jni.cpp - JNI bridge for llvbox shared library

#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/raw_ostream.h"

#include <cstring>
#include <string>
#include <vector>
#include <memory>

using namespace clang;
using namespace clang::driver;

// ──────────────────────────────────────────────────────────────────────
// Global init
// ──────────────────────────────────────────────────────────────────────
static llvm::InitLLVM* gInitLLVM = nullptr;

static void ensureInit() {
    if (!gInitLLVM) {
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllAsmParsers();
    }
}

// ──────────────────────────────────────────────────────────────────────
// Session
// ──────────────────────────────────────────────────────────────────────
struct llvbox_session {
    // Could hold per-session state (e.g., source manager,compiler instance)
};

extern "C" {

llvbox_session* llvbox_create() {
    ensureInit();
    return new llvbox_session();
}

void llvbox_destroy(llvbox_session* s) {
    delete s;
}

// Parse args string into argv array, then call clang main
int llvbox_compile(llvbox_session* /*session*/, const char* args) {
    // Split args by whitespace
    std::vector<std::string> tokens;
    std::string cur;
    for (const char* p = args; *p; ++p) {
        if (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') {
            if (!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
        } else {
            cur += *p;
        }
    }
    if (!cur.empty()) tokens.push_back(cur);

    std::vector<const char*> argv;
    for (auto& t : tokens) argv.push_back(t.data());

    // Forward to clang_main
    extern "C" int clang_main(int argc, char** argv, const void* code);
    return clang_main(argv.size(), const_cast<char**>(argv.data()), nullptr);
}

const char* llvbox_version() {
    static char ver[128];
    snprintf(ver, sizeof(ver), "llvbox-22.1.0");
    return ver;
}

} // extern "C"

// ──────────────────────────────────────────────────────────────────────
// JNI implementations
// ──────────────────────────────────────────────────────────────────────
extern "C" JNIEXPORT jlong JNICALL
Java_com_llvmbox_LLVMBox_nativeCreate(JNIEnv*, jobject) {
    return reinterpret_cast<jlong>(llvbox_create());
}

extern "C" JNIEXPORT void JNICALL
Java_com_llvmbox_LLVMBox_nativeDestroy(JNIEnv*, jobject, jlong handle) {
    llvbox_destroy(reinterpret_cast<llvbox_session*>(handle));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_llvmbox_LLVMBox_nativeCompile(JNIEnv* env, jobject,
                                        jlong handle, jstring jargs, jstring jcode) {
    const char* args = env->GetStringUTFChars(jargs, nullptr);
    const char* code = env->GetStringUTFChars(jcode, nullptr);

    // Write code to temp FILE, then compile
    int rc = llvbox_compile(reinterpret_cast<llvbox_session*>(handle), args);

    env->ReleaseStringUTFChars(jargs, args);
    env->ReleaseStringUTFChars(jcode, code);
    return (jint)rc;
}
