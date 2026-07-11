cmake_minimum_required(VERSION 3.22.1)

project("unerixd")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${CMAKE_PROJECT_NAME} SHARED
        daemon.cpp
        entry.cpp
        jni_env_guard.cpp
        jvm_manager.cpp
        context_manager.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log
)