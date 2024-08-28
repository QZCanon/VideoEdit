#ifndef TASK_H
#define TASK_H

#include "functional"
#include <atomic>
#include <memory>

class Task;

using TaskFunc = std::function<void()>;
using InitFunc = std::function<void()>;
using TaskSPtr = std::shared_ptr<Task>;

#define CREATE_TASK_OCJECT() std::make_shared<Task>()

class Task
{
public:
    Task() = default;

    Task(const Task& other)
    {
        mTaskCallback = other.mTaskCallback;
        mInitCallback = other.mInitCallback;
        active.store(other.active.load());
    }

    Task& operator=(const Task& other)
    {
        mTaskCallback = other.mTaskCallback;
        mInitCallback = other.mInitCallback;
        active.store(other.active.load());
        return *this;
    }

    void SetTaskFunc(TaskFunc func)
    {
        mTaskCallback = std::move(func);
    }

    void SetInitfunc(InitFunc func)
    {
        mInitCallback = std::move(func);
    }

    void operator()()
    {   // 通过重载（）执行回调函数
        if (mTaskCallback) {
            mTaskCallback();
        }
    }

    void Init()
    {
        if (mInitCallback) {
            mInitCallback();
        }
    }

    void CancleTask()
    {
        active.store(false);
    }

    void ActivateTask()
    {
        active.store(true);
    }

    bool IsActive()
    {
        return active.load();
    }

private:
    TaskFunc mTaskCallback;
    InitFunc mInitCallback;
    std::atomic<bool> active{false};
};

#endif
