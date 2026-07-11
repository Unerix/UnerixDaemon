#pragma once

#include <jni.h>

class JniEnvGuard final {

public:
    // 构造函数
    JniEnvGuard();

    // 析构函数
    ~JniEnvGuard();

    // 防止拷贝
    JniEnvGuard(const JniEnvGuard &) = delete;

    // 防止拷贝
    JniEnvGuard &operator=(const JniEnvGuard &) = delete;

    // 获取当前线程的 JNIEnv
    JNIEnv *getEnv();

private:
    // 当前线程的 JNIEnv
    JNIEnv *GJniEnv = nullptr;

    // 是否需要分离线程
    bool NeedDetach = false;
};