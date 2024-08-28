#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H

enum InstalceType {
    UNKNOWN = 0,
    TASK_RUNNER
};

template<class T>
class TaskRunnerInstance {
public:
    static T* GetInstance() {
        static T t;
        return &t;
    }
};

#endif // INSTANCEMANAGER_H
