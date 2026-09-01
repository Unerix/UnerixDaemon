#pragma once

#include <atomic>
#include <thread>
#include <android/native_window.h>
#include <EGL/egl.h>

class RenderEngine {
public:
    ~RenderEngine();

    // 绑定 NativeWindow（宽高由 Java 侧传入，不固定）并启动渲染线程
    void StartRender(ANativeWindow *Window, int Width, int Height);

    // 接收触摸（Touch=true 按下），将屏幕坐标映射为逻辑坐标
    void OnTouch(bool Touch, int X, int Y);

    // 停止渲染线程并等待退出
    void StopRender();

private:
    ANativeWindow *RenderWindow = nullptr;    // 原生窗口
    int LogicWidth = 0;                       // 渲染/触摸基准宽（Java 传入，TextureView 实际尺寸）
    int LogicHeight = 0;                      // 渲染/触摸基准高（Java 传入，TextureView 实际尺寸）
    std::atomic<bool> bIsTouch = false;       // 触摸状态
    std::atomic<int> TouchX = 0;              // 触摸 X（渲染坐标）
    std::atomic<int> TouchY = 0;              // 触摸 Y（渲染坐标）
    std::atomic<bool> bIsRunning = false;     // 渲染线程运行标志
    std::thread RenderThread;                 // 渲染线程

    EGLDisplay Display = EGL_NO_DISPLAY;      // EGL 显示
    EGLSurface EglSurface = EGL_NO_SURFACE;   // EGL 窗口表面
    EGLContext Context = EGL_NO_CONTEXT;      // EGL 渲染上下文

    void ImAppEntry();
    void ImLoopTask();

    bool InitEgl();
    void DeinitEgl();
};
