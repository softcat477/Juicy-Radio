#ifndef CONDVAR_H
#define CONDVAR_H

#include <condition_variable>
#include <mutex>

class CondVar{
public:
    CondVar();
    ~CondVar();
    void wait();
    void signal();
    std::condition_variable condvar;
    std::mutex m;
};

#endif