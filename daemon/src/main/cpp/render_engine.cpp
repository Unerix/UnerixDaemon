#include "render_engine.hpp"
#include "logging.hpp"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <GLES3/gl3.h>
#include <cfloat>
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
    bIsRunning = true;
    RenderThread = thread(&RenderEngine::ImLoopTask, this);
}

void RenderEngine::OnTouch(bool Touch, int X, int Y) {
    // TextureView 下触摸坐标与渲染分辨率一致，直接透传
    bIsTouch = Touch;
    TouchX = X;
    TouchY = Y;
}

void RenderEngine::StopRender() {
    bIsRunning = false;
    if (RenderThread.joinable()) {
        RenderThread.join();
    }
}

bool RenderEngine::InitEgl() {
    Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (Display == EGL_NO_DISPLAY) {
        LOG_E("eglGetDisplay failed");
        return false;
    }
    if (!eglInitialize(Display, nullptr, nullptr)) {
        LOG_E("eglInitialize failed");
        Display = EGL_NO_DISPLAY;
        return false;
    }

    // 官方示例使用的属性集（ES3 通过下方 ContextAttribs 指定）
    const EGLint ConfigAttribs[] = {
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE,
    };
    EGLConfig Config = nullptr;
    EGLint NumConfigs = 0;
    if (!eglChooseConfig(Display, ConfigAttribs, &Config, 1, &NumConfigs) || NumConfigs == 0) {
        LOG_E("eglChooseConfig failed");
        DeinitEgl();
        return false;
    }

    // 窗口格式与 EGL 配置对齐（官方示例做法）；TextureView 的 buffer 尺寸由 SurfaceTexture
    // 管理，故尺寸传 0,0 表示不修改，仅设置格式
    EGLint Format = 0;
    eglGetConfigAttrib(Display, Config, EGL_NATIVE_VISUAL_ID, &Format);
    if (ANativeWindow_setBuffersGeometry(RenderWindow, 0, 0, Format) != 0) {
        LOG_E("Failed to set NativeWindow format");
        DeinitEgl();
        return false;
    }

    const EGLint ContextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE,
    };
    Context = eglCreateContext(Display, Config, EGL_NO_CONTEXT, ContextAttribs);
    if (Context == EGL_NO_CONTEXT) {
        LOG_E("eglCreateContext failed: 0x%x", eglGetError());
        DeinitEgl();
        return false;
    }

    EglSurface = eglCreateWindowSurface(Display, Config, RenderWindow, nullptr);
    if (EglSurface == EGL_NO_SURFACE) {
        LOG_E("eglCreateWindowSurface failed: 0x%x", eglGetError());
        DeinitEgl();
        return false;
    }

    if (!eglMakeCurrent(Display, EglSurface, EglSurface, Context)) {
        LOG_E("eglMakeCurrent failed: 0x%x", eglGetError());
        DeinitEgl();
        return false;
    }

    // 从 EGL Surface 查询真实渲染尺寸（TextureView 的 buffer 尺寸由 EGL 与 SurfaceFlinger 协商）
    EGLint W = 0, H = 0;
    if (!eglQuerySurface(Display, EglSurface, EGL_WIDTH, &W) || W <= 0) {
        W = ANativeWindow_getWidth(RenderWindow);
    }
    if (!eglQuerySurface(Display, EglSurface, EGL_HEIGHT, &H) || H <= 0) {
        H = ANativeWindow_getHeight(RenderWindow);
    }
    if (W <= 0 || H <= 0) {
        LOG_E("Failed to query render size");
        DeinitEgl();
        return false;
    }
    LogicWidth = W;
    LogicHeight = H;
    LOG_D("EGL Surface [%d x %d]", LogicWidth, LogicHeight);
    return true;
}

void RenderEngine::DeinitEgl() {
    if (Display != EGL_NO_DISPLAY) {
        eglMakeCurrent(Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (Context != EGL_NO_CONTEXT) {
            eglDestroyContext(Display, Context);
        }
        if (EglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(Display, EglSurface);
        }
        eglTerminate(Display);
    }
    Display = EGL_NO_DISPLAY;
    EglSurface = EGL_NO_SURFACE;
    Context = EGL_NO_CONTEXT;
}

void RenderEngine::ImLoopTask() {
    LOG_D("ImGui Task Start!!");
    if (!InitEgl()) {
        ANativeWindow_release(RenderWindow);
        RenderWindow = nullptr;
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &IO = ImGui::GetIO();
    IO.IniFilename = nullptr; // 不读写 imgui.ini
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    IO.DisplaySize = ImVec2((float) LogicWidth, (float) LogicHeight);
    IO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // 深色风格
    ImGui::StyleColorsDark();

//    // 以 480 逻辑宽为基准按比例放大 UI（官方示例为固定 3.5f）
//    float Scale = (float) LogicWidth / 480.0f;
//    if (Scale < 1.0f) Scale = 1.0f;
//    if (Scale > 4.0f) Scale = 4.0f;
//    ImGui::GetStyle().ScaleAllSizes(Scale);
//    LOG_D("ImGui UI scale: %.2f", Scale);

    // Setup scaling
    float main_scale = 3.5f;
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale.

    style.WindowRounding = 8.0f;

    if (!ImGui_ImplOpenGL3_Init("#version 300 es")) {
        LOG_E("ImGui_ImplOpenGL3_Init failed");
        ImGui::DestroyContext();
        DeinitEgl();
        ANativeWindow_release(RenderWindow);
        RenderWindow = nullptr;
        return;
    }
    if (!ImGui_ImplAndroid_Init(RenderWindow)) {
        LOG_E("ImGui_ImplAndroid_Init failed");
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        DeinitEgl();
        ANativeWindow_release(RenderWindow);
        RenderWindow = nullptr;
        return;
    }

    // 首帧前提交一次空帧，初始化窗口缓冲（使用与官方一致的浅色背景）
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(Display, EglSurface);

    // 主循环：约 30fps
    while (bIsRunning) {
        ImGui_ImplAndroid_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();

        // 转发 JNI 触摸事件到 ImGui
        if (bIsTouch) {
            IO.AddMousePosEvent((float) TouchX, (float) TouchY);
            IO.AddMouseButtonEvent(0, true);
        } else {
            IO.AddMouseButtonEvent(0, false);
            IO.AddMousePosEvent(-FLT_MAX, -FLT_MAX); // 松开后清除悬停状态
        }

        ImGui::NewFrame();
        ImAppEntry();
        ImGui::Render();

        glViewport(0, 0, LogicWidth, LogicHeight);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(Display, EglSurface);

        usleep(33 * 1000);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    DeinitEgl();
    ANativeWindow_release(RenderWindow);
    RenderWindow = nullptr;
    LOG_D("ImGui App Stopped!!");
}

static void ShowExampleAppMainMenuBar();

static void ShowExampleMenuFile();

static void ShowExampleAppFullscreen(bool *p_open);

void RenderEngine::ImAppEntry() {
    ShowExampleAppMainMenuBar();


    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    if (ImGui::Begin("Unerix Daemon", nullptr, flags)) {
        ImGui::Text("ImGui %s", IMGUI_VERSION);
        ImGui::Text("Render: %d x %d", LogicWidth, LogicHeight);
        ImGui::Text("Touch: %s (%d, %d)", bIsTouch ? "down" : "up", (int) TouchX, (int) TouchY);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
    }

}

static void ShowExampleAppMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {} // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void ShowExampleMenuFile() {
    ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent")) {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More..")) {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse..")) {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options")) {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 5.0f), ImGuiChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors")) {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            const char *name = ImGui::GetStyleColorName((ImGuiCol) i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol) i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}