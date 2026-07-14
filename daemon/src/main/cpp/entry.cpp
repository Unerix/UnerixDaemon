#include <jni.h>
#include "entry.hpp"
#include "jvm_manager.hpp"
#include "method_register.hpp"

Entry::Entry() {

}

Entry::~Entry() {

}

void Entry::OnLoad(JavaVM *vm, void *reserved) {
    JvmManager::GetInstance().Init(vm);
    MethodRegister();
}

void Entry::OnUnload(JavaVM *vm, void *reserved) {

}