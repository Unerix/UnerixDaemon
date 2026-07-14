#pragma once

#include <jni.h>

class JvmManager final {

public:
    JvmManager() = default;

    JvmManager(const JvmManager &) = delete;

    JvmManager &operator=(const JvmManager &) = delete;

    static JvmManager &GetInstance();

    void Init(JavaVM *vm);

    JavaVM *GetJavaVM();

private:
    JavaVM *GJavaVM = nullptr;
};