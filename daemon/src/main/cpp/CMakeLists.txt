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
)

find_package(lvgl REQUIRED CONFIG)
find_package(boringssl REQUIRED CONFIG)

target_include_directories(
        ${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        android
        log
        lvgl::lvgl
        lvgl::lvgl_demos
        boringssl::crypto_static
)