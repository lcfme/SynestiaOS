//
// Created by XingfengYang on 2020/7/17.
//

#include <mutex.h>
#include <stdbool.h>
#include <thread.h>

extern Thread *currentThread;
extern KernelStatus schd_switch_next();
extern KernelStatus schd_add_to_schduler(Thread *thread);

void mutex_create(Mutex *mutex, Atomic *atomic) {
  mutex->val = atomic;
  mutex->waitList = nullptr;
  atomic_create(atomic);
}

bool mutex_acquire(Mutex *mutex) {
  if (atomic_get(mutex->val) == 0) {
    atomic_set(mutex->val, 1);
    return 1;
  } else {
    // can not get the lock, just add to lock wait list
    KernelStatus enQueueStatus = kqueue_enqueue(mutex->waitList, &currentThread->threadReadyQueue);
    if (enQueueStatus != OK) {
      printf("[Mutex]: thread add to wait list failed. \n");
    }
    // 2. switch to the next thread in scheduler
    KernelStatus thradSwitchNextStatus = schd_switch_next();
    if (thradSwitchNextStatus != OK) {
      printf("[Mutex]: thread switch to next failed. \n");
    }
    return false;
  }
}

void mutex_release(Mutex *mutex) {
  atomic_set(mutex->val, 0);
  KQueue *queueNode = kqueue_dequeue(mutex->waitList);
  Thread *releasedThread = getNode(queueNode, Thread, threadReadyQueue);
  KernelStatus addToSchduler = schd_add_to_schduler(releasedThread);
  if (addToSchduler != OK) {
    printf("[Mutex]: thread add to schduler failed. \n");
  }
}
