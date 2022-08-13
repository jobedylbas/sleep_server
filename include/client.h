#ifndef CLIENT_H
#define CLIENT_H

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

class Client {
    private:
        int sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen);
        int getHostname(char *buf, size_t buflen);
        unsigned char* getMacAddress();
    public:
        MACHINE me, manager;
        Client();
        void leave();
        void waitForCommand();
        void printManager();
        pthread_t discoveryThreadId, monitoringId, waitForCommandThreadId;
        int discoverySD;
};

#endif