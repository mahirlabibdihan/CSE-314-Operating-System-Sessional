#include <iostream>
#include <vector>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "print.h"
#include "PrinterMonitor.h"
#include "BinderMonitor.h"
#include "LibraryMonitor.h"
#include <random>
#include <chrono>
using namespace std;

#define N_PRINTERS 4
#define N_BINDERS 2
#define N_BOOKS 1
#define N_STAFFS 2

// std::chrono::high_resolution_clock::time_point start_time;

int n_students, n_groups, students_per_group;
int printer_delay, binder_delay, report_delay; // printer_delay: Printing time, binder_delay: Binding time, report_delay: Entry book access

const double lambda = 5.0; // Inter-arrival rate (events per second)
unsigned seed = chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(1905072);
std::poisson_distribution<int> distribution(lambda);

PrinterMonitor printer_monitor(N_PRINTERS);
BinderMonitor binder_monitor(N_BINDERS);
LibraryMonitor library_monitor;

// #define __CURRENT_TIME__ chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count() / 1000.0
void *studentPrint(void *arg)
{
    sleep(distribution(generator)); // random wait
    int sid = *(int *)arg;
    print("Student", sid + 1, "has arrived at the printing station", (sid + 1) % N_PRINTERS + 1);
    printer_monitor.requestPrinter(sid);
    print("Student", sid + 1, "has started printing");
    sleep(printer_delay); // Simulate
    print("Student", sid + 1, "has finished printing");
    printer_monitor.releasePrinter(sid);
    delete (int *)arg;
    pthread_exit(NULL);
}

void groupPrint(int group_id)
{
    vector<pthread_t> student_threads(n_groups);
    for (int i = group_id * n_groups; i < group_id * n_groups + n_groups; i++)
    {
        pthread_create(&student_threads[i % n_groups], NULL, studentPrint, new int(i));
    }
    for (int i = group_id * n_groups; i < group_id * n_groups + n_groups; i++)
    {
        pthread_join(student_threads[i % n_groups], NULL);
    }
    print("Group", group_id + 1, "has finished printing");
}

void groupBind(int group_id)
{
    print("Group", group_id + 1, "as arrived at the binding station");
    binder_monitor.requestBinder();
    print("Group", group_id + 1, "has started binding");
    sleep(binder_delay); // Simulate
    print("Group", group_id + 1, "has finished binding");
    binder_monitor.releaseBinder();
}

void groupSubmit(int group_id)
{
    print("Group", group_id + 1, "as arrived at the library");
    library_monitor.requestWrite();
    print("Group", group_id + 1, "has started submitting");
    sleep(report_delay);
    print("Group", group_id + 1, "has submitted the report");
    library_monitor.releaseWrite();
}
void *groupFunction(void *arg)
{
    int group_id = *((int *)arg);
    sleep(distribution(generator)); // random wait
    groupPrint(group_id);
    sleep(distribution(generator)); // random wait
    groupBind(group_id);
    sleep(distribution(generator)); // random wait
    groupSubmit(group_id);
    delete (int *)arg;
    pthread_exit(NULL);
}

void *staffFunction(void *arg)
{
    int staff_id = *((int *)arg);
    do
    {
        sleep(distribution(generator)); // random wait
        library_monitor.requestRead();
        print("Staff", staff_id + 1, "has started reading the entry book.", "No of submissions =", library_monitor.getSubmissions());
        sleep(report_delay);
        print("Staff", staff_id + 1, "has finished reading the entry book.");
        library_monitor.releaseRead();
    } while (library_monitor.getSubmissions() < n_groups);
    delete (int *)arg; // Deallocating memory
    pthread_exit(NULL);
}
int main()
{
    start_time = std::chrono::high_resolution_clock::now(); // Get the current time

    pthread_mutex_init(&stdout_lock, 0);
    cin >> n_students >> students_per_group;
    cin >> printer_delay >> binder_delay >> report_delay;
    n_groups = n_students / students_per_group;

    // Threads for n_groups groups
    vector<pthread_t> group_threads(n_groups);
    printer_monitor.init(n_students, students_per_group);
    for (int i = 0; i < n_groups; i++)
    {
        pthread_create(&group_threads[i], NULL, groupFunction, new int(i));
    }

    // Threads for 2 staffs
    vector<pthread_t> staff_threads(N_STAFFS);
    for (int i = 0; i < N_STAFFS; i++)
    {
        pthread_create(&staff_threads[i], NULL, staffFunction, new int(i));
    }

    pthread_exit(NULL);
    return EXIT_SUCCESS;
}