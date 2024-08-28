#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H

#include "task_runner/task_runner.hpp"

enum InstalceType {
    UNKNOWN = 0,
    TASK_RUNNER
};

class TaskRunnerInstance {
public:
    static TaskRunner* GetInstance() {
        static TaskRunner t;
        return &t;
    }
};

#endif // INSTANCEMANAGER_H
