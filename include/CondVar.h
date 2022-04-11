#ifndef CONDVAR_H
#define CONDVAR_H

#include <condition_variable>
#include <mutex>

class CondVar{
public:
    CondVar();
    ~CondVar();

    CondVar(CondVar& other) = delete;
    CondVar& operator=(CondVar& other) = delete;
    CondVar(CondVar&& other) = delete;
    CondVar& operator=(CondVar&& other) = delete;

    void wait();
    void signal();
    std::condition_variable condvar;
    std::mutex m;
};

#endif