//
// Created by qinzhou on 23-5-8.
//

#ifndef TASK_RUNNER_HPP
#define TASK_RUNNER_HPP

#include <vector>
#include "core/sync_queue.h"
#include "thread.hpp"
#include "task.hpp"
#include "Logger/logger.h"
#include <chrono>

const int THREAD_POOL_MAX_SIZE = 10;

class TaskRunner
{
public:
    explicit TaskRunner()
    {
        runners.reserve(THREAD_POOL_MAX_SIZE);
        for (int i = 0; i < THREAD_POOL_MAX_SIZE; i++) {
            runners.push_back(new BaseRunner(this));
        }
        Start();
    }

    ~TaskRunner()
    {
        taskQueue.Stop();
        for (int i = 0; i < THREAD_POOL_MAX_SIZE; i++) {
            runners[i]->stop();
            runners[i]->join();
            delete runners[i];
            runners[i] = nullptr;
        }
    }

    void AddTask(const TaskSPtr& task)
    {
        if (taskQueue.Size() >= THREAD_POOL_MAX_SIZE) {
            LOG_DEBUG() << "task queue's size =" << taskQueue.Size();
        }
        task->ActivateTask();
        taskQueue.Put(task);
    }

private:
    void Start()
    {
        for (int i = 0; i < THREAD_POOL_MAX_SIZE; i++) {
            runners[i]->start(); // use base class
        }
    }

private:
    class BaseRunner : public Thread
    {
    public:
        explicit BaseRunner(TaskRunner* run) : runner(run) {}

    protected:
        void run() override
        {
            bool exitCurrentTask = true;
            TaskSPtr t;
            while (loop()) {
                if (exitCurrentTask == true) {
                    exitCurrentTask = false;
                    runner->taskQueue.Take(t);
                    std::call_once(m_flag, [&] { t->Init(); });
                }
                if (t->IsActive()) {
                    (*t)();
                } else {
                    t = nullptr;
                    exitCurrentTask = true;
                }
                // std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }

    private:
        TaskRunner* runner{nullptr};
        std::once_flag m_flag;
    };

private:
    SyncQueue<TaskSPtr, THREAD_POOL_MAX_SIZE> taskQueue;
    std::vector<BaseRunner*> runners;
};

#endif // TASK_RUNNER_HPP
