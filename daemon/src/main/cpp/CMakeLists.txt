cmake_minimum_required(VERSION 3.22.1)

project("unerixd")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Building Release...")
    add_compile_definitions(APP_DEBUG=0)
else ()
    message("Building Debug...")
    add_compile_definitions(APP_DEBUG=1)
endif ()

add_library(
        ${CMAKE_PROJECT_NAME} SHARED
        main.cpp
        daemon.cpp
        entry.cpp
        jni_env_guard.cpp
        jvm_manager.cpp
        context_manager.cpp
        method_register.cpp
        render_engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_android.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_opengl3.cpp
)

find_package(boringssl REQUIRED CONFIG)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        IMGUI_IMPL_OPENGL_ES3
)

target_include_directories(
        ${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        android
        log
        EGL
        GLESv3
        boringssl::crypto_static
)