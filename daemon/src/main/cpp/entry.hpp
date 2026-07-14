#pragma once

class Entry final {

public:
    Entry();

    ~Entry();

    void OnLoad(JavaVM *vm, void *reserved);

    void OnUnload(JavaVM *vm, void *reserved);
};
