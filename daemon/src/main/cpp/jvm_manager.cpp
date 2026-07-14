#include "jvm_manager.hpp"

JvmManager &JvmManager::GetInstance() {
    static JvmManager instance;
    return instance;
}

void JvmManager::Init(JavaVM *vm) {
    GJavaVM = vm;
}

JavaVM *JvmManager::GetJavaVM() {
    return GJavaVM;
}
