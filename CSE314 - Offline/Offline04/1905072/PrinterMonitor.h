#ifndef __PRINTER_MONITOR_H
#define __PRINTER_MONITOR_H
#include <iostream>
#include <vector>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

// Two improvements over Dining Philosopher problem
class PrinterMonitor
{
private:
    enum State
    {
        NONE,
        WAITING,
        PRINTING
    };
    int n_printers;
    vector<sem_t> mutex; // i-th mutex locks all elements with index j where j%4=i
    vector<State> state;
    vector<sem_t> student_mtx;
    int n_students, students_per_group;

public:
    PrinterMonitor(int n) : mutex(n)
    {
        n_printers = n;
        for (int i = 0; i < n; i++)
        {
            sem_init(&mutex[i], 0, 1);
        }
    }
    void init(int n, int m)
    {
        n_students = n;
        students_per_group = m;
        state.resize(n_students);
        student_mtx.resize(n_students);
        for (int i = 0; i < n_students; i++)
        {
            sem_init(&student_mtx[i], 0, 0);
            state[i] = NONE;
        }
    }

    void requestPrinter(int sid)
    {
        // What if student 1 and 2 requests simultaneously?
        // One of them will be blocked if we had used one mutex for whole array
        // But they doesn't have any dependencies. Two students have dependency only if their (id MOD 4) is same.
        // So, since id MOD 4 has only 4 unique value(0,1,2,3) we will use 4 mutex
        // mutex[i] for students with id % 4=i
        sem_wait(&mutex[sid % n_printers]); // lock state array
        state[sid] = WAITING;
        test(sid);
        sem_post(&mutex[sid % n_printers]); // unlock state array
        sem_wait(&student_mtx[sid]);        // sleep if didn't get printer
    }
    void releasePrinter(int sid)
    {
        state[sid] = NONE;
        sem_wait(&mutex[sid % n_printers]); // lock state array
        int group_id = sid / students_per_group;
        for (int i = group_id * students_per_group; i < (group_id + 1) * students_per_group; i++) // Same group
        {
            if (i % n_printers == sid % n_printers && i != sid)
            {
                if (test(i))
                    goto X; // Gave printer to i. No need to test further.
            }
        }
        for (int i = sid % n_printers; i < n_students; i += n_printers) // All other
        {
            if (test(i))
                goto X; // Gave printer to i. No need to test further.
        }
    X:
        sem_post(&mutex[sid % n_printers]); // unlock state array
    }
    bool test(int sid)
    {
        if (state[sid] == WAITING)
        {
            // Checking if anyone else is printing on my station
            for (int i = sid % n_printers; i < n_students; i += n_printers)
            {
                if (state[i] == PRINTING)
                {
                    return false;
                }
            }
            // Start printing
            state[sid] = PRINTING;
            sem_post(&student_mtx[sid]);
            return true;
        }
        return false;
    }
};
#endif