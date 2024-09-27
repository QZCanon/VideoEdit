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

const int THREAD_POOL_MAX_SIZE = 10;

class TaskRunner
{
public:
    using TaskRSPtr = TaskRunner*;
    static TaskRSPtr GetInstance() {
        static TaskRunner runner;
        return &runner;
    }
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
            runners[i]->start();
        }
    }

private:
    class BaseRunner : public Thread
    {
    public:
        explicit BaseRunner(TaskRunner* run) : m_runner(run) {}

    protected:
        void run() override
        {
            bool endCurrentTask = true;
            TaskSPtr t;
            while (loop()) {
                if (endCurrentTask) {
                    endCurrentTask = false;
                    m_runner->taskQueue.Take(t);
                    std::call_once(m_flag, [&] {
                        if (t) {
                            t->Init();
                        }
                    });
                }
                if (t && t->IsActive()) {
                    (*t)();
                    Delay_us(t->Sleep());
                } else {
                    t = nullptr;
                    endCurrentTask = true;
                }
            }
        }

    private:

        void Delay_us(int timeout)
        {
            auto start = std::chrono::system_clock::now();
            while (true)
            {
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count();
                if (duration > timeout) {
                    // LOG_DEBUG() << "timeout occurred,timeout " << timeout;
                    break;
                }
            }
        }

    private:
        TaskRunner* m_runner{nullptr};
        std::once_flag m_flag;
    };

private:
    SyncQueue<TaskSPtr, THREAD_POOL_MAX_SIZE> taskQueue;
    std::vector<BaseRunner*> runners;
};

#endif // TASK_RUNNER_HPP
