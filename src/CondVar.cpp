#include "../include/CondVar.h"
#include <mutex>
#include <condition_variable>


CondVar::CondVar(){

}
CondVar::~CondVar(){

}
void CondVar::wait(){
    std::unique_lock<std::mutex> lp{m};
    condvar.wait(lp);
}
void CondVar::signal(){
    std::scoped_lock lp{m};
    condvar.notify_all();
}