#ifndef DL_H
#define DL_H

#include <string>
#include <memory>
#include <dlfcn.h> // dlopen

class AutoCast {
    void* m_pointer;
public:
    AutoCast(void* p): m_pointer(p) {}
    template <typename U>
    operator U*() { return reinterpret_cast<U*>(m_pointer); }
};

class DL {
    std::shared_ptr<void> m_handle;
public:
    DL() {}
    DL(void* handle, bool own=true)
    {
        if (own)
            m_handle.reset(handle, dlclose);
        else
            m_handle.reset(handle, [](void*){});
    }
    bool load(const std::string &path)
    {
        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (handle) m_handle.reset(handle, dlclose);
        return loaded();
    }
    bool loaded() const { return m_handle.get() != nullptr; }
    void reset() { m_handle.reset(); }
    AutoCast fetch(const char *name)
    {
        return AutoCast(dlsym(m_handle.get(), name));
    }
};

#endif
