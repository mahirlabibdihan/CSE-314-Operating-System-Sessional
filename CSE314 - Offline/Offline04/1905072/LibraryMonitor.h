#ifndef __LIBRARY_MONITOR_H
#define __LIBRARY_MONITOR_H
#include <semaphore.h>
#include <pthread.h>
#include <vector>
using namespace std;

class LibraryMonitor
{
    sem_t mutex, db;
    int rc;
    int n_records;

public:
    LibraryMonitor()
    {
        sem_init(&mutex, 0, 1);
        sem_init(&db, 0, 1);
        rc = 0;
    }
    int getSubmissions()
    {
        return n_records;
    }
    void requestRead(void)
    {
        sem_wait(&mutex);
        rc++;
        if (rc == 1)
        {
            sem_wait(&db);
        }
        sem_post(&mutex);
    }
    void releaseRead(void)
    {
        sem_wait(&mutex);
        rc--;
        if (rc == 0)
        {
            sem_post(&db);
        }
        sem_post(&mutex);
    }
    void requestWrite(void)
    {
        sem_wait(&db);
    }
    void releaseWrite(void)
    {
        n_records++;
        sem_post(&db);
    }
};
#endif