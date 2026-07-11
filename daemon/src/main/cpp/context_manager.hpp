#pragma once

#include <jni.h>

#define FIND_HIDDEN_CLASS(env, className) (env)->FindClass(className)

class ContextManager final {

public:
    ContextManager() = default;

    ContextManager(const ContextManager &) = delete;

    ContextManager &operator=(const ContextManager &) = delete;

    static ContextManager &getInstance();

    jobject GetContext();

private:
    jobject ResolveApplicationContext();

    jobject Context = nullptr;
    bool Attempted = false;
};