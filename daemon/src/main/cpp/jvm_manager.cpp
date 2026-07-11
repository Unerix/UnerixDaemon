#include "jvm_manager.hpp"

JvmManager &JvmManager::getInstance() {
    static JvmManager instance;
    return instance;
}

void JvmManager::init(JavaVM *vm) {
    GJavaVM = vm;
}

JavaVM *JvmManager::getJavaVM() {
    return GJavaVM;
}
