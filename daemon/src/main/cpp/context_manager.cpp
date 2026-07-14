#include "context_manager.hpp"
#include "jni_env_guard.hpp"
#include "logging.hpp"

ContextManager &ContextManager::GetInstance() {
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
    JNIEnv *Env = Guard.GetEnv();
    if (!Env) {
        LOG_E("resolveApplicationContext: failed to get JNIEnv");
        return nullptr;
    }

    // =========================================================
    // 方案 1：AppGlobals.getInitialApplication()
    // =========================================================
    jclass appGlobalsClass = FIND_HIDDEN_CLASS(Env, APP_GLOBALS_CLASS_NAME);
    if (appGlobalsClass) {
        jmethodID getInitialApp = Env->GetStaticMethodID(
                appGlobalsClass,
                "getInitialApplication",
                "()Landroid/app/Application;"
        );
        if (getInitialApp) {
            jobject app = Env->CallStaticObjectMethod(appGlobalsClass, getInitialApp);
            if (Env->ExceptionCheck()) {
                Env->ExceptionClear();
                LOG_E("AppGlobals.getInitialApplication() threw exception");
            } else if (app) {
                Context = Env->NewGlobalRef(app);
                LOG_D("Application context obtained via AppGlobals");
                Env->DeleteLocalRef(app);
                Env->DeleteLocalRef(appGlobalsClass);
                return Context;
            }
        }
        Env->DeleteLocalRef(appGlobalsClass);
    }

    // =========================================================
    // 方案 2：ActivityThread.currentActivityThread().getApplication()
    // =========================================================
    jclass atClass = FIND_HIDDEN_CLASS(Env, ACTIVITY_THREAD_CLASS_NAME);
    if (atClass) {
        jmethodID currentAt = Env->GetStaticMethodID(
                atClass,
                "currentActivityThread",
                "()Landroid/app/ActivityThread;"
        );
        if (currentAt) {
            jobject atObj = Env->CallStaticObjectMethod(atClass, currentAt);
            if (atObj && !Env->ExceptionCheck()) {
                jmethodID getApp = Env->GetMethodID(
                        atClass,
                        "getApplication",
                        "()Landroid/app/Application;"
                );
                if (getApp) {
                    jobject app = Env->CallObjectMethod(atObj, getApp);
                    if (Env->ExceptionCheck()) {
                        Env->ExceptionClear();
                    } else if (app) {
                        Context = Env->NewGlobalRef(app);
                        LOG_D("Application context obtained via ActivityThread");
                        Env->DeleteLocalRef(app);
                    }
                }
                Env->DeleteLocalRef(atObj);
            } else {
                Env->ExceptionClear();
            }
        }
        Env->DeleteLocalRef(atClass);
    }

    if (Context) {
        LOG_D("Application context successfully cached");
    } else {
        LOG_E("All methods failed to obtain Application context");
    }

    return Context;
}
