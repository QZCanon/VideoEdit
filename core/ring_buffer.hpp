#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <cstddef>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "Logger/logger.h"

template <typename T, size_t bufferSize>
class RingBuffer
{
public:
    using Deleter = std::function<void(T &)>;
    using Loc = std::unique_lock<std::mutex>;

    RingBuffer(Deleter deleter)
        : m_deleter(deleter), m_writePosition(0), m_readPosition(0), m_isStopped(false)
    {
    }
    RingBuffer() :m_deleter(nullptr), m_writePosition(0), m_readPosition(0), m_isStopped(false) {}

    ~RingBuffer() { Clear(); }

    void Wait()
    {
        if (!Empty()) {
            return;
        }
        Loc guard{m_mutex};
        while (Empty() && !m_isStopped.load()) {
            m_emtpy.wait(guard);
        }
    }

    template <class TimeOut>
    bool Wait(TimeOut timeout)
    {
        if (!Empty()) {
            return true;
        }
        auto pred = [&] { return (!Empty() || m_isStopped.load()); };
        Loc guard{m_mutex};
        return m_emtpy.wait_until(guard, timeout, pred);
    }

    bool Full() {
        return (Next(m_writePosition.load())  == m_readPosition);
    }

    bool Empty() {
        return m_writePosition == m_readPosition;
    }

    void Clear()
    {
        Loc guard{m_mutex};
        if (m_deleter) {
            auto rp = m_readPosition.load();
            auto wp = m_writePosition.load();
            while (rp != wp) {
                m_deleter(m_buffer[rp]);
                rp = Next(rp);
            }
        }
        m_writePosition = 0;
        m_readPosition = 0;
    }

    /**
     * pop front if the buffer is bull
     */
    void PushBack(T &&x)
    {
        Loc guard{m_mutex};
        // LOG_DEBUG() << "m_isStopped: " << m_isStopped;
        while (Full() && !m_isStopped.load()) {
            // LOG_DEBUG() << "wait...";
            m_full.wait(guard);
        }
        if (m_isStopped.load()) {
            return;
        }
        // LOG_DEBUG() << "push...";
        auto wp =  m_writePosition.load();
        m_buffer[wp] = std::move(x);
        m_writePosition = Next(wp);
        if (m_readPosition == wp) {
            m_emtpy.notify_all();
        }
    }

    T FrontAndPop()
    {
        Loc guard{m_mutex};
        auto rp = m_readPosition.load();
        T t = m_buffer[rp];
        m_readPosition = Next(rp);
        if (rp == Next(m_writePosition.load())) {
            m_full.notify_all();
        }
        return t;
    }

    static size_t Next(size_t pos) { return (pos + 1) % bufferSize; }

private:
    mutable std::mutex m_mutex;          //
    std::condition_variable m_emtpy;     //
    std::condition_variable m_full;      //
    std::atomic<size_t> m_writePosition; //
    std::atomic<size_t> m_readPosition;  //
    std::atomic_bool m_isStopped;        //
    std::array<T, bufferSize> m_buffer;  //
    Deleter m_deleter;                   //
};

#endif
