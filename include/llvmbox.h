// llvmbox.h - Public C API for llvmbox shared library
// This is the main header users include to interact with the LLVM/Clang toolchain
//
// File layout when packaged as .so for Android:
//   llvmbox.so          <- the shared library (all of LLVM + clang + lld linked in)
//   include/llvmbox.h   <- this file (the public C API)
//
// Usage from Java/JNI:
//   System.loadLibrary("llvmbox");
//   native int llvmbox_compile(String args);
//   native int llvmbox_run(String code);

#ifndef LLVM_BOX_H
#define LLVM_BOX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ──────────────────────────────────────────────────────────────────────
// Opaque handle to an llvmbox compiler session
// ──────────────────────────────────────────────────────────────────────
typedef struct llvmbox_session llvmbox_session;

// ──────────────────────────────────────────────────────────────────────
// Create / destroy a compiler session
// ──────────────────────────────────────────────────────────────────────

// Create a new llvmbox session.
// Returns NULL on failure.
llvmbox_session* llvmbox_create(void);

// Destroy a session and free all resources.
void llvmbox_destroy(llvmbox_session* session);

// ──────────────────────────────────────────────────────────────────────
// Compile C/C++ source code to native code
// ──────────────────────────────────────────────────────────────────────

// Compile a C source string to an object file or executable.
// args: clang-style argument string, e.g. "-O2 -static -o /tmp/out hello.c\n"
// Returns 0 on success, non-zero on failure.
int llvmbox_compile(llvmbox_session* session, const char* args);

// Compile a single C/C++ source buffer to machine code.
// Returns a newly allocated buffer with the output.
// Caller must free the returned buffer with llvmbox_free().
typedef struct {
    uint8_t* data;
    size_t   size;
} llvmbox_buf;

typedef struct {
    const char* source_code;
    size_t      source_len;
    const char* filename;       // e.g. "input.c" or "input.cpp"
    const char* options;        // e.g. "-O2"
    const char* target_triple;  // e.g. "aarch64-linux-android24"
    int         output_type;    // 0=obj, 1=asm, 2=exe
} llvmbox_compile_request;

int llvmbox_compile_buf(llvmbox_session* session,
                        const llvmbox_compile_request* req,
                        llvmbox_buf* out,
                        char** err_out);       // malloc'd error string, caller frees

// Free a buffer returned by llvmbox_compile_buf.
void llvmbox_free(void* ptr);

// ──────────────────────────────────────────────────────────────────────
// Linker (lld) interface
// ──────────────────────────────────────────────────────────────────────

// Link object files into an executable/shared library.
// args: lld-style argument string, e.g. "-o out obj1.o obj2.o -lc\n"
int llvmbox_link(llvmbox_session* session, const char* args);

// ──────────────────────────────────────────────────────────────────────
// Archiver (llvm-ar) interface
// ──────────────────────────────────────────────────────────────────────

// Manipulate static libraries (.a files).
// args: llvm-ar-style argument string, e.g. "rcs lib.a obj1.o obj2.o\n"
int llvmbox_ar(llvmbox_session* session, const char* args);

// ──────────────────────────────────────────────────────────────────────
// Get version / info
// ──────────────────────────────────────────────────────────────────────

// Returns a static string like "llvmbox-22.1.0-linux-x86_64".
const char* llvmbox_version(void);

// Returns 1 if running on Android.
int llvmbox_is_android(void);

// ──────────────────────────────────────────────────────────────────────
// JNI bindings - callable from Java via JNI
// ──────────────────────────────────────────────────────────────────────

#include <jni.h>

#ifdef LLVM_BOX_ENABLE_JNI

JNIEXPORT jlong JNICALL
Java_com_llvmbox_Compiler_nativeCreate(JNIEnv*, jobject);

JNIEXPORT void JNICALL
Java_com_llvmbox_Compiler_nativeDestroy(JNIEnv*, jobject, jlong);

JNIEXPORT jint JNICALL
Java_com_llvmbox_Compiler_nativeCompile(JNIEnv*, jobject, jlong, jstring, jstring);

JNIEXPORT jbyteArray JNICALL
Java_com_llvmbox_Compiler_nativeCompileToBinary(JNIEnv*, jobject, jlong,
                                                 jstring, jstring, jstring);

JNIEXPORT jstring JNICALL
Java_com_llvmbox_Compiler_nativeGetVersion(JNIEnv*, jobject);

#endif // LLVM_BOX_ENABLE_JNI

#ifdef __cplusplus
}
#endif

#endif // LLVM_BOX_H
