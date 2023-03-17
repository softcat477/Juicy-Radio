#ifndef ITHREAD_H
#define ITHREAD_H

#include <atomic>

class IThread{
public:
    IThread();
    virtual ~IThread();
    virtual void start() = 0;

    void setStop();
    void setStart();
    bool isStopped();

    void setUpdateCycle(double ms);
protected:
    std::atomic<bool> _isStopped;
    double _update_ms;
};

#endif