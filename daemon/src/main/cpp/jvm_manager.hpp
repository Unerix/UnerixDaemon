#pragma once

#include <jni.h>

class JvmManager final {

public:
    JvmManager() = default;

    JvmManager(const JvmManager &) = delete;

    JvmManager &operator=(const JvmManager &) = delete;

    static JvmManager &getInstance();

    void init(JavaVM *vm);

    JavaVM *getJavaVM();

private:
    JavaVM *GJavaVM = nullptr;
};