package io.unerix.daemon

class NativeLib {

    companion object {
        init {
            System.loadLibrary("unerixd")
        }
    }
}