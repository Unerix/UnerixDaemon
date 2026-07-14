#include "render_engine.hpp"
#include "logging.hpp"
#include <lvgl.h>
#include <demos/lv_demos.h>
#include <unistd.h>

using namespace std;

RenderEngine::~RenderEngine() {
    StopRender();
}

void RenderEngine::StartRender(ANativeWindow *Window) {
    StopRender();
    if (Window == nullptr) {
        return;
    }
    RenderWindow = Window;
    ScreenWidth = ANativeWindow_getWidth(RenderWindow);
    ScreenHeight = ANativeWindow_getHeight(RenderWindow);
    if (ANativeWindow_setBuffersGeometry(RenderWindow, AppWidth, AppHeight, WINDOW_FORMAT_RGBA_8888) != 0) {
        LOG_E("Failed to set NativeWindow geometry [%d x %d]", AppWidth, AppHeight);
        ANativeWindow_release(RenderWindow);
        Window = nullptr;
        return;
    }
    LOG_D("LV Screen [%d x %d]", AppWidth, AppHeight);
    bIsRunning = true;
    RenderThread = thread(&RenderEngine::LvLoopTask, this);
}

void RenderEngine::OnTouch(bool Touch, int X, int Y) {
    if (ScreenWidth <= 0 || ScreenHeight <= 0) {
        return;
    }
    bIsTouch = Touch;
    TouchX = X * AppWidth / ScreenWidth;
    TouchY = Y * AppHeight / ScreenHeight;
}

void RenderEngine::StopRender() {
    bIsRunning = false;
    if (RenderThread.joinable()) {
        RenderThread.join();
    }
}

void RenderEngine::LvLoopTask() {
    LOG_D("LV Task Start!!");
    lv_init();
    lv_tick_set_cb(LvTickGet);
    lv_log_register_print_cb(LvLogPrint);

    auto *Disp = lv_display_create(AppWidth, AppHeight);
    lv_display_set_user_data(Disp, this);
    lv_display_set_flush_cb(Disp, RenderEngine::LvFlushCbStatic);

    size_t BufSize = (size_t) AppWidth * AppHeight * 2;
    auto *Buf = malloc(BufSize);
    lv_display_set_buffers(Disp, Buf, nullptr, BufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    auto *Indev = lv_indev_create();
    lv_indev_set_user_data(Indev, this);
    lv_indev_set_type(Indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(Indev, RenderEngine::LvTouchCbStatic);

    LvAppEntry();

    // 首帧前提交一次空帧，初始化窗口缓冲
    if (ANativeWindow_lock(RenderWindow, &WindowBuffer, nullptr) == 0) {
        ANativeWindow_unlockAndPost(RenderWindow);
    }

    // 主循环：按 LVGL 返回间隔休眠（1~33ms，约 30fps）
    while (bIsRunning) {
        uint32_t TimeTillNext = lv_timer_handler();
        if (TimeTillNext < 1) TimeTillNext = 1;
        if (TimeTillNext > 33) TimeTillNext = 33;
        usleep(TimeTillNext * 1000);
    }

    ANativeWindow_release(RenderWindow);
    lv_deinit();
    free(Buf);
    if (SurfaceBuffer != nullptr) {
        free(SurfaceBuffer);
        SurfaceBuffer = nullptr;
    }
    RenderWindow = nullptr;
    LOG_D("LV App Stopped!!");
}

void RenderEngine::LvAppEntry() {
    lv_demo_widgets();
}

uint32_t RenderEngine::LvTickGet() {
    static struct timeval TimeVal;
    gettimeofday(&TimeVal, nullptr);
    return (TimeVal.tv_sec * 1000) + (TimeVal.tv_usec / 1000);
}

void RenderEngine::LvLogPrint(lv_log_level_t Level, const char *Buf) {
    switch (Level) {
        case LV_LOG_LEVEL_INFO:
            LOG_I("%s", Buf);
            break;
        case LV_LOG_LEVEL_WARN:
            LOG_W("%s", Buf);
            break;
        case LV_LOG_LEVEL_ERROR:
            LOG_E("%s", Buf);
            break;
        case LV_LOG_LEVEL_TRACE:
            LOG_D("%s", Buf);
            break;
        default:
            LOG_I("%s", Buf);
    }
}

void RenderEngine::LvTouchCallback(lv_indev_t *IndevDriver, lv_indev_data_t *Data) {
    if (bIsTouch) {
        Data->point.x = (short) TouchX;
        Data->point.y = (short) TouchY;
        Data->state = LV_INDEV_STATE_PR;
    } else {
        Data->state = LV_INDEV_STATE_REL;
    }
}

void RenderEngine::LvFlushCallback(lv_display_t *Display, const lv_area_t *Area, uint8_t *PxMap) {
    if (bIsRunning && RenderWindow != nullptr) {
        if (SurfaceBuffer == nullptr) {
            SurfaceSize = sizeof(uint32_t) * WindowBuffer.stride * WindowBuffer.height;
            SurfaceBuffer = (uint32_t *) malloc(SurfaceSize);
        }
        int Width = Area->x2 - Area->x1 + 1;
        int Height = Area->y2 - Area->y1 + 1;
        auto *Src = (uint16_t *) PxMap;
        auto Stride = WindowBuffer.stride;
        // 逐像素 RGB565 → RGBA_8888
        for (int I = 0; I < Height; I++) {
            auto *Dst = &SurfaceBuffer[(Area->y1 + I) * Stride + Area->x1];
            auto *SrcLine = &Src[I * Width];
            for (int J = 0; J < Width; J++) {
                uint16_t Color = SrcLine[J];
                auto Red = (uint8_t) (((Color >> 11) & 0x1F) * 255 / 31);
                auto Green = (uint8_t) (((Color >> 5) & 0x3F) * 255 / 63);
                auto Blue = (uint8_t) ((Color & 0x1F) * 255 / 31);
                Dst[J] = (0xFFu << 24) | ((uint32_t) Blue << 16) | ((uint32_t) Green << 8) | Red;
            }
        }
        if (ANativeWindow_lock(RenderWindow, &WindowBuffer, nullptr) == 0) {
            memcpy(WindowBuffer.bits, SurfaceBuffer, SurfaceSize);
            ANativeWindow_unlockAndPost(RenderWindow);
        }
    }
    lv_disp_flush_ready(Display);
}

void RenderEngine::LvTouchCbStatic(lv_indev_t *IndevDriver, lv_indev_data_t *Data) {
    auto *Engine = (RenderEngine *) lv_indev_get_user_data(IndevDriver);
    Engine->LvTouchCallback(IndevDriver, Data);
}

void RenderEngine::LvFlushCbStatic(lv_display_t *Display, const lv_area_t *Area, uint8_t *PxMap) {
    auto *Engine = (RenderEngine *) lv_display_get_user_data(Display);
    Engine->LvFlushCallback(Display, Area, PxMap);
}
