#ifndef __BINDER_MONITOR_H
#define __BINDER_MONITOR_H
#include <semaphore.h>
#include <pthread.h>
#include <vector>
using namespace std;

class BinderMonitor
{
    int n_binders;
    sem_t available;

public:
    BinderMonitor(int n)
    {
        n_binders = n;
        sem_init(&available, 0, n);
    }
    void requestBinder()
    {
        sem_wait(&available);
    }
    void releaseBinder()
    {
        sem_post(&available);
    }
};
#endif