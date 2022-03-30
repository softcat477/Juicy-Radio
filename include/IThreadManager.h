#ifndef ITHREADMANAGER_H
#define ITHREADMANAGER_H

#include <atomic>

class IThreadManager{
public:
    IThreadManager();
    virtual ~IThreadManager();
    virtual void start() = 0;

    void setStop();
    void setStart();
    bool isStopped();
protected:
    std::atomic<bool> _isStopped;
};

#endif