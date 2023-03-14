#ifndef CONDVAR_H
#define CONDVAR_H

#include <condition_variable>
#include <mutex>

class CondVar{
public:
    CondVar();
    ~CondVar();

    // CondVar(CondVar& other) = delete;
    // CondVar& operator=(CondVar& other) = delete;
    // CondVar(CondVar&& other) = delete;
    // CondVar& operator=(CondVar&& other) = delete;


    CondVar(CondVar& other) :
        condvar{},
        m{}
    {

    }
    CondVar& operator=(CondVar& other) {
        return *this;
    }
    CondVar(CondVar&& other) :
        condvar{},
        m{}
    {

    }
    CondVar& operator=(CondVar&& other) {
        return *this;
    }

    void wait();
    void signal();
    std::condition_variable condvar;
    std::mutex m;
};

#endif