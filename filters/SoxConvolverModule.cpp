#include "SoxConvolverModule.h"

#define CHECK(expr) do { if (!(expr)) throw std::runtime_error("!!!"); } \
    while (0)

bool SoXConvolverModule::load(const std::string &path)
{
    if (!m_dl.load(path)) return false;
    try {
        CHECK(version = m_dl.fetch("lsx_convolver_version_string"));
        CHECK(create = m_dl.fetch("lsx_convolver_create"));
        CHECK(close = m_dl.fetch("lsx_convolver_close"));
        CHECK(process = m_dl.fetch("lsx_convolver_process"));
        CHECK(process_ni = m_dl.fetch("lsx_convolver_process_ni"));
        CHECK(design_lpf = m_dl.fetch("lsx_design_lpf"));
        CHECK(free = m_dl.fetch("lsx_free"));
        return true;
    } catch (...) {
        m_dl.reset();
        return false;
    }
}

