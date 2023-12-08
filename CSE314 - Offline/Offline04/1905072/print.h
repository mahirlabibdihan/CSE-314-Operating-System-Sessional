#include <iostream>
#include <vector>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
using namespace std;

pthread_mutex_t stdout_lock;
chrono::high_resolution_clock::time_point start_time;
#define __CURRENT_TIME__ chrono::duration_cast<std::chrono::microseconds>(chrono::high_resolution_clock::now() - start_time).count() / 1000000.0

template <typename T>
void _print(T var1)
{
    cout << var1 << endl;
}
template <typename T, typename... Types>
void _print(T var1, Types... var2)
{
    cout << var1 << " ";
    _print(var2...);
}

template <typename T, typename... Types>
void print(T var1, Types... var2)
{
    pthread_mutex_lock(&stdout_lock);
    printf("At time +%0.6fs: ", __CURRENT_TIME__);
    _print(var1, var2...);
    pthread_mutex_unlock(&stdout_lock);
}