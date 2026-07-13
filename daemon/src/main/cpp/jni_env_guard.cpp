#include "jni_env_guard.hpp"
#include "jvm_manager.hpp"

JniEnvGuard::JniEnvGuard() {
    // 获取JVM指针
    JavaVM *jvm = JvmManager::getInstance().getJavaVM();
    if (!jvm) return;

    // 检查当前线程是否已经附加到 JVM
    jint status = jvm->GetEnv(
            reinterpret_cast<void **>(&GJniEnv),
            JNI_VERSION_1_6
    );

    if (status == JNI_EDETACHED) {
        // 当前线程未附加，手动附加
        if (jvm->AttachCurrentThread(&GJniEnv, nullptr) == JNI_OK) {
            NeedDetach = true;
        } else {
            GJniEnv = nullptr;
            NeedDetach = false;
        }
    } else if (status != JNI_OK) {
        // 其他错误情况
        GJniEnv = nullptr;
        NeedDetach = false;
    }
}

JniEnvGuard::~JniEnvGuard() {
    if (NeedDetach) {
        JavaVM *jvm = JvmManager::getInstance().getJavaVM();
        if (jvm) {
            jvm->DetachCurrentThread();
        }
    }
}

JNIEnv *JniEnvGuard::GetEnv() {
    return GJniEnv;
}
