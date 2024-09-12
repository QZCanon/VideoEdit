#ifndef ATOMIC_VECTOR_HPP
#define ATOMIC_VECTOR_HPP

#include <vector>
#include <mutex>
#include  <algorithm>

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

    auto find(const T& t) {
        std::lock_guard<std::mutex> lck(m_mutex);
        auto ret = std::find(m_list.begin(), m_list.end(), t);
        if (ret != m_list.end()) {
            return ret;
        }
        return m_list.end();
    }

    auto begin() {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_list.begin();
    }

    auto end() {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_list.end();
    }


private:
    std::vector<T> m_list;
    mutable std::mutex m_mutex;
};

#endif // ATOMIC_VECTOR_HPP
