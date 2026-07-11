#include <jni.h>
#include "entry.hpp"
#include "jvm_manager.hpp"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    JvmManager::getInstance().init(vm);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {

}