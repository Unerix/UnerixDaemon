#include <jni.h>
#include "entry.hpp"
#include "logging.hpp"

static Entry *EntryPtr = nullptr;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    EntryPtr = new Entry();
    EntryPtr->OnLoad(vm, reserved);
    LOG_D("JNI_OnLoad");
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    EntryPtr->OnUnload(vm, reserved);
    delete EntryPtr;
    EntryPtr = nullptr;
    LOG_D("JNI_OnUnload");
}