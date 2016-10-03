#ifndef THREAD_PRIORITY_QUEUE_H_
#define THREAD_PRIORITY_QUEUE_H_
#include "stdio.h"
#include "nsThreadUtils.h"
# include "nsIThreadManager.h"
#include <queue>

//SECLAB 18:42 09/30/2016

class SecThread{

  public:
    SecThread(nsIThread* thread, uint64_t endTime):aThread(thread),expectedEndTime(endTime){}

    SecThread & operator = (const SecThread & thread){
        this->aThread=thread.aThread;
        this->expectedEndTime=thread.expectedEndTime;
        return *this;
    }

    friend bool operator < (const SecThread & thread1, const SecThread & thread2);

  public:
    nsIThread* aThread;
    uint64_t expectedEndTime;

};

extern std::priority_queue<SecThread> secThreadQueue;

void * pushSecThread(nsIThread * aThread,uint64_t expectedEndTime);
const SecThread & getTopSecThread();
void * popSecThread();
int sizeSecThread();
/*
std::priority_queue<SecThread> secThreadQueue;

void * pushSecThread(nsIThread * aThread,uint64_t expectedEndTime){
    const SecThread secThread(aThread,expectedEndTime);
    secThreadQueue.push(secThread);
    return NULL;
}

const SecThread & getTopSecThread(){
    const SecThread & secThread=secThreadQueue.top();
    return secThread;
}

void * popSecThread(){
    secThreadQueue.pop();
    return NULL;
}

int sizeSecThread(){
    return secThreadQueue.size();
}*/



#endif
