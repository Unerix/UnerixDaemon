#include "jni_env_guard.hpp"
#include "jvm_manager.hpp"

JniEnvGuard::JniEnvGuard() {
    // 获取JVM指针
    JavaVM *Jvm = JvmManager::GetInstance().GetJavaVM();
    if (!Jvm) return;

    // 检查当前线程是否已经附加到 JVM
    jint status = Jvm->GetEnv(
            reinterpret_cast<void **>(&GJniEnv),
            JNI_VERSION_1_6
    );

    if (status == JNI_EDETACHED) {
        // 当前线程未附加，手动附加
        if (Jvm->AttachCurrentThread(&GJniEnv, nullptr) == JNI_OK) {
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
        JavaVM *Jvm = JvmManager::GetInstance().GetJavaVM();
        if (Jvm) {
            Jvm->DetachCurrentThread();
        }
    }
}

JNIEnv *JniEnvGuard::GetEnv() {
    return GJniEnv;
}
