#pragma once

#include <jni.h>

#define FIND_HIDDEN_CLASS(env, className) (env)->FindClass(className)

#define APP_GLOBALS_CLASS_NAME "android/app/AppGlobals"
#define ACTIVITY_THREAD_CLASS_NAME "android/app/ActivityThread"

class ContextManager final {

public:
    ContextManager() = default;

    ContextManager(const ContextManager &) = delete;

    ContextManager &operator=(const ContextManager &) = delete;

    static ContextManager &GetInstance();

    jobject GetContext();

private:
    jobject ResolveApplicationContext();

    jobject Context = nullptr;
    bool Attempted = false;
};