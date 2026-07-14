#pragma once

#include <atomic>
#include <thread>
#include <android/native_window.h>
#include <lvgl.h>

class RenderEngine {
public:
    ~RenderEngine();

    // 绑定 NativeWindow 并启动渲染线程
    void StartRender(ANativeWindow *Window);

    // 接收触摸（Touch=true 按下），将屏幕坐标映射为逻辑坐标
    void OnTouch(bool Touch, int X, int Y);

    // 停止渲染线程并等待退出
    void StopRender();

private:
    ANativeWindow *RenderWindow = nullptr;          // 原生窗口
    ANativeWindow_Buffer WindowBuffer;        // 帧缓冲信息
    const int AppWidth = 480;                 // LVGL 逻辑宽
    const int AppHeight = 320;                // LVGL 逻辑高
    int ScreenWidth = 0;                      // 物理屏宽（坐标映射）
    int ScreenHeight = 0;                     // 物理屏高（坐标映射）
    uint32_t *SurfaceBuffer = nullptr;        // 全帧累积缓冲（解决多缓冲闪烁）
    size_t SurfaceSize = 0;                   // 累积缓冲大小
    std::atomic<bool> bIsTouch = false;       // 触摸状态
    std::atomic<int> TouchX = 0;              // 触摸 X（逻辑坐标）
    std::atomic<int> TouchY = 0;              // 触摸 Y（逻辑坐标）
    std::atomic<bool> bIsRunning = false;     // 渲染线程运行标志
    std::thread RenderThread;                 // 渲染线程

    void LvAppEntry();
    void LvLoopTask();

    void LvFlushCallback(lv_display_t *Display, const lv_area_t *Area, uint8_t *PxMap);
    void LvTouchCallback(lv_indev_t *IndevDriver, lv_indev_data_t *Data);
    static uint32_t LvTickGet();
    static void LvLogPrint(lv_log_level_t Level, const char *Buf);
    static void LvFlushCbStatic(lv_display_t *Display, const lv_area_t *Area, uint8_t *PxMap);
    static void LvTouchCbStatic(lv_indev_t *IndevDriver, lv_indev_data_t *Data);
};