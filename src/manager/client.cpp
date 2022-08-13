#include <iostream>
#include <list>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <sstream>
#include "../../include/client.h"
#include "../../include/machine.h"
#include "../../include/constants.h"
#include <sys/syscall.h>    // SYS_sysctl
#include <sys/sysctl.h>     // CTL_KERN, KERN_HOSTNAME
#include <unistd.h>         // syscall
#include <sys/ioctl.h>
#include <net/if.h> 
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <errno.h>

int Client::sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen)
{
    return syscall(SYS_sysctl, name, namelen, oldp, oldlenp, newp, newlen);
}

int Client::getHostname(char *buf, size_t buflen)
{
    int name[] = { CTL_KERN, KERN_HOSTNAME };
    size_t namelen = 2;

    return sysctl(name, namelen, buf, &buflen, NULL, 0);
}

Client::Client() {
    struct hostent *host_entry;

    getHostname(me.hostname, HOSTNAME_SIZE);
    host_entry = gethostbyname(me.hostname);
    me.status = AWAKEN;
    memcpy(me.mac_address, getMacAddress(), sizeof(me.mac_address));
    me.ip_address = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    manager.ip_address = "0";
    // cout << "|\t\thostname\t\t|\tMAC Address\t|\tIP Address\t|" << endl;
}

void Client::printManager() {
    if (strcmp(manager.ip_address, "0")) {
        std::cout << "|\t" << manager.hostname;
        printf("\t|\t%02X:%02X:%02X:%02X:%02X:%02X\t|\t", manager.mac_address[0], manager.mac_address[1], 
            manager.mac_address[2], manager.mac_address[3], manager.mac_address[4], manager.mac_address[5]);
    
        std::cout << "\t\t|\t" << manager.ip_address << "\t|\t";
    }
   
}

void Client::leave() {
    
}

void Client::waitForCommand() {
    std::string command, hostname;
    std::stringstream _command;
    // std::istringstream f;
    while(1) {
        command.clear();
        std::getline(cin, command);
        
        if(command.find("LEAVE") != std::string::npos) {
            leave();
        } else {
            cout << "Command not found." << endl;
        }
    }
}

unsigned char* Client::getMacAddress() {
    // unsigned char mac_address[6];
    int         mib[6];
    size_t len;
    char            *buf;
    unsigned char       *ptr;
    struct if_msghdr    *ifm;
    struct sockaddr_dl  *sdl;

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    // mib[5] = 1;
    if ((mib[5] = if_nametoindex("en0")) == 0) {
        perror("if_nametoindex error");
        return NULL;
    }

    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        perror("sysctl 1 error");
        return NULL;
    }

    if ((buf = (char*)malloc(len)) == NULL) {
        perror("malloc error");
        return NULL;
    }

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        perror("sysctl 2 error");
        return NULL;
    }

    ifm = (struct if_msghdr *)buf;
    sdl = (struct sockaddr_dl *)(ifm + 1);
    ptr = (unsigned char *)LLADDR(sdl);
    // printf("%02x:%02x:%02x:%02x:%02x:%02x\n", *ptr, *(ptr+1), *(ptr+2),
	// 		*(ptr+3), *(ptr+4), *(ptr+5));
    return ptr;
}