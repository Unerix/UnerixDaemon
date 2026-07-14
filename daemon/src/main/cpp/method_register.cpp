#include "method_register.hpp"
#include <jni.h>
#include "jni_env_guard.hpp"
#include "logging.hpp"
#include "render_engine.hpp"
#include <android/native_window_jni.h>

static RenderEngine *GlobalApp = nullptr;

// 创建单例
static void NativeCreate(JNIEnv *Env, jobject) {
    if (GlobalApp == nullptr) {
        GlobalApp = new RenderEngine();
    }
}

// Surface → ANativeWindow → 启动渲染
static void NativeStart(JNIEnv *Env, jobject, jobject Surface) {
    if (GlobalApp == nullptr || Surface == nullptr) {
        return;
    }
    ANativeWindow *NativeWindow = ANativeWindow_fromSurface(Env, Surface);
    if (NativeWindow == nullptr) {
        return;
    }
    GlobalApp->StartRender(NativeWindow);
}

// 转发触摸
static void NativeOnTouch(JNIEnv *Env, jobject, jboolean Touch, jint X, jint Y) {
    if (GlobalApp == nullptr) {
        return;
    }
    GlobalApp->OnTouch(Touch, X, Y);
}

// 暂停渲染
static void NativeStop(JNIEnv *Env, jobject) {
    if (GlobalApp == nullptr) {
        return;
    }
    GlobalApp->StopRender();
}

// 销毁单例
static void NativeDestroy(JNIEnv *Env, jobject) {
    if (GlobalApp != nullptr) {
        delete GlobalApp;
        GlobalApp = nullptr;
    }
}

static const JNINativeMethod NativeMethods[] = {
        {"lvglCreate", "()V", (void *) NativeCreate},
        {"lvglStart", "(Landroid/view/Surface;)V", (void *) NativeStart},
        {"lvglOnTouch", "(ZII)V", (void *) NativeOnTouch},
        {"lvglStop", "()V", (void *) NativeStop},
        {"lvglDestroy", "()V", (void *) NativeDestroy},
};

void MethodRegister() {
    JniEnvGuard Guard;
    JNIEnv *Env = Guard.GetEnv();
    jclass Clazz = Env->FindClass("io/unerix/daemon/UnerixView");
    if (Clazz == nullptr) {
        LOG_E("JNI_OnLoad: cannot find class.");
        return;
    }
    jint Count = sizeof(NativeMethods) / sizeof(NativeMethods[0]);
    if (Env->RegisterNatives(Clazz, NativeMethods, Count) != JNI_OK) {
        LOG_E("JNI_OnLoad: RegisterNatives failed");
        return;
    }
}