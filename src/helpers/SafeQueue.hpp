//
// Created by Til Blechschmidt on 22.10.17.
//

#ifndef HOMESH_SAFEQUEUE_HPP
#define HOMESH_SAFEQUEUE_HPP


#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

template<class T>
class SafeQueue {
public:
    SafeQueue()
            : q(), m(), c() {}

    ~SafeQueue() = default;

    void add(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    int peek(T *val, std::chrono::duration<int, std::milli> timeout) {
        std::unique_lock<std::mutex> lock(m);
        while (q.empty()) {
            if (c.wait_for(lock, timeout) == std::cv_status::timeout) return 1;
        }
        *val = q.front();
        return 0;
    }

    int take(T *val, std::chrono::duration<int, std::milli> timeout) {
        std::unique_lock<std::mutex> lock(m);
        while (q.empty()) {
            if (c.wait_for(lock, timeout) == std::cv_status::timeout) return 1;
        }
        *val = q.front();
        q.pop();
        return 0;
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};


#endif //HOMESH_SAFEQUEUE_HPP
