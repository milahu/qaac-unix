#ifndef PIPED_READER_H
#define PIPED_READER_H

#include "FilterBase.h"
#include "win32util.h"
#include <unistd.h>
#include <pthread.h> // pthread_create

class PipedReader: public FilterBase {
    std::shared_ptr<FILE> m_readPipe;
    std::shared_ptr<void> m_writePipe, m_thread;
    int64_t m_position;
public:
    PipedReader(std::shared_ptr<ISource> &src);
    ~PipedReader();
    size_t readSamples(void *buffer, size_t nsamples);
    void start()
    {
        pthread_t thread;
        int ret = pthread_create(&thread, nullptr, staticInputThreadProc, this);
        if (ret != 0) {
            throw std::runtime_error(std::strerror(ret));
        }

        // custom deleter function for pthread_join
        auto joinThread = [](void* ptr) { pthread_join(*static_cast<pthread_t*>(ptr), nullptr); };

        m_thread.reset(reinterpret_cast<void*>(thread), joinThread);

    }
    int64_t getPosition() { return m_position; }
private:
    void inputThreadProc();
    static void* staticInputThreadProc(void *arg)
    {
        PipedReader *self = static_cast<PipedReader*>(arg);
        self->inputThreadProc();
        return nullptr;
    }
};

#endif
