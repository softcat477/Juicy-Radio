#include "../include/IThread.h"
IThread::IThread():_isStopped(false){

}
IThread::~IThread(){

}
void IThread::setStop(){
    _isStopped.store(true);
}
void IThread::setStart(){
    _isStopped.store(false);
}
bool IThread::isStopped(){
    return _isStopped.load();
}
void IThread::setUpdateCycle(double ms) {
    _update_ms = ms;
}