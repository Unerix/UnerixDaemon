#include "context_manager.hpp"
#include "jni_env_guard.hpp"
#include "logging.hpp"

ContextManager &ContextManager::getInstance() {
    static ContextManager instance;
    return instance;
}

jobject ContextManager::GetContext() {
    // 已经拿到过，直接返回
    if (Context != nullptr) {
        return Context;
    }
    // 已经尝试但失败了，不再重试
    if (Attempted) {
        return nullptr;
    }
    return ResolveApplicationContext();
}

jobject ContextManager::ResolveApplicationContext() {
    Attempted = true;

    JniEnvGuard Guard;
    JNIEnv *env = Guard.GetEnv();
    if (!env) {
        LOG_E("resolveApplicationContext: failed to get JNIEnv");
        return nullptr;
    }

    // =========================================================
    // 方案 1：AppGlobals.getInitialApplication()
    // =========================================================
    jclass appGlobalsClass = FIND_HIDDEN_CLASS(env, "android/app/AppGlobals");
    if (appGlobalsClass) {
        jmethodID getInitialApp = env->GetStaticMethodID(
                appGlobalsClass,
                "getInitialApplication",
                "()Landroid/app/Application;"
        );
        if (getInitialApp) {
            jobject app = env->CallStaticObjectMethod(appGlobalsClass, getInitialApp);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                LOG_E("AppGlobals.getInitialApplication() threw exception");
            } else if (app) {
                Context = env->NewGlobalRef(app);
                LOG_D("Application context obtained via AppGlobals");
                env->DeleteLocalRef(app);
                env->DeleteLocalRef(appGlobalsClass);
                return Context;
            }
        }
        env->DeleteLocalRef(appGlobalsClass);
    }

    // =========================================================
    // 方案 2：ActivityThread.currentActivityThread().getApplication()
    // =========================================================
    jclass atClass = FIND_HIDDEN_CLASS(env, "android/app/ActivityThread");
    if (atClass) {
        jmethodID currentAt = env->GetStaticMethodID(
                atClass,
                "currentActivityThread",
                "()Landroid/app/ActivityThread;"
        );
        if (currentAt) {
            jobject atObj = env->CallStaticObjectMethod(atClass, currentAt);
            if (atObj && !env->ExceptionCheck()) {
                jmethodID getApp = env->GetMethodID(
                        atClass,
                        "getApplication",
                        "()Landroid/app/Application;"
                );
                if (getApp) {
                    jobject app = env->CallObjectMethod(atObj, getApp);
                    if (env->ExceptionCheck()) {
                        env->ExceptionClear();
                    } else if (app) {
                        Context = env->NewGlobalRef(app);
                        LOG_D("Application context obtained via ActivityThread");
                        env->DeleteLocalRef(app);
                    }
                }
                env->DeleteLocalRef(atObj);
            } else {
                env->ExceptionClear();
            }
        }
        env->DeleteLocalRef(atClass);
    }

    if (Context) {
        LOG_D("Application context successfully cached");
    } else {
        LOG_E("All methods failed to obtain Application context");
    }
    return Context;
}
