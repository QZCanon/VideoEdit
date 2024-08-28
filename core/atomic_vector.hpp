#ifndef ATOMIC_VECTOR_HPP
#define ATOMIC_VECTOR_HPP

#include <vector>
#include <mutex>

template <class T>
class AtomicVector {
public:
    void push_back(const T& t) {
        std::lock_guard<std::mutex> lck(m_mutex);
        m_list.push_back(t);
    }

    void remove(const T& t) {
        std::lock_guard<std::mutex> lck(m_mutex);
        m_list.erase(std::remove(m_list.begin(), m_list.end(), t));
    }

    T& operator[](size_t n) {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_list[n];
    }

    size_t size() {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_list.size();
    }


private:
    std::vector<T> m_list;
    mutable std::mutex m_mutex;
};

#endif // ATOMIC_VECTOR_HPP
