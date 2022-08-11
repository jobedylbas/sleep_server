#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <list>
#include "string.h"
#include "machine.h"
#include <mutex>
#include <sys/syscall.h>    // SYS_sysctl
#include <sys/sysctl.h>     // CTL_KERN, KERN_HOSTNAME
#include <unistd.h>         // syscall
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

class Manager {
    private:
        MACHINE me;
        mutex* mtx;
        list<MACHINE> machines;
        int sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen);
        int getHostname(char *buf, size_t buflen);
    public:
        Manager();
        int add(MACHINE machine); // Returns a bool for operation
        int remove(MACHINE machine);
        int change(MACHINE machine, int status);
        void printMachines();
        char* getIp();
        pthread_t discoveryThreadId, monitoringThreadId, commandThreadId;
        void errno_abort(const char* header);
        void* discover(void*);
        int discoverySD;
        void waitForCommand();
        int createCommunicationSocket();      
};

#endif