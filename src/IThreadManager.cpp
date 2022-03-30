#include "../include/IThreadManager.h"
IThreadManager::IThreadManager():_isStopped(false){

}
IThreadManager::~IThreadManager(){

}
void IThreadManager::setStop(){
    _isStopped.store(true);
}
void IThreadManager::setStart(){
    _isStopped.store(false);
}
bool IThreadManager::isStopped(){
    return _isStopped.load();
}