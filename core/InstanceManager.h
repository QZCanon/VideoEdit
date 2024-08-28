#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H

template<class T>
class Instance {
public:
    static T* GetInstance() {
        static T t;
        return &t;
    }
};

#endif // INSTANCEMANAGER_H
