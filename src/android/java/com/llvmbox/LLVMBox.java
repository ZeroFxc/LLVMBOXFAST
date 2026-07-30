package com.llvmbox;

/**
 * Java wrapper for the llvmbox.so native library.
 *
 * Usage:
 *   LLVMBox compiler = new LLVMBox();
 *   int result = compiler.compile("-O2 -o hello hello.c", sourceCode);
 *   compiler.destroy();
 */
public class LLVMBox {

    static {
        System.loadLibrary("llvmbox");
    }

    private long nativeHandle;

    public LLVMBox() {
        nativeHandle = nativeCreate();
    }

    public void destroy() {
        if (nativeHandle != 0) {
            nativeDestroy(nativeHandle);
            nativeHandle = 0;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            destroy();
        } finally {
            super.finalize();
        }
    }

    /**
     * Compile source code.
     * @param args compiler flags (e.g. "-O2 -static -o /data/local/tmp/out hello.c")
     * @param code source code text
     * @return 0 on success
     */
    public int compile(String args, String code) {
        return nativeCompile(nativeHandle, args, code);
    }

    // Native methods
    private static native long nativeCreate();
    private static native void nativeDestroy(long handle);
    private static native int nativeCompile(long handle, String args, String code);
    private static native byte[] nativeCompileToBinary(long handle,
                                                       String source,
                                                       String options,
                                                       String triple);
    private static native String nativeGetVersion();
}
