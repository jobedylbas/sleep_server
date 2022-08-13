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
#include <sys/syscall.h>    // SYS_sysctl
#include <sys/sysctl.h>     // CTL_KERN, KERN_HOSTNAME
#include <unistd.h>         // syscall
#include <sstream>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <errno.h>
#include "../../include/manager.h"
#include "../../include/machine.h"
#include "../../include/constants.h"

int Manager::add(MACHINE machine) {
    std::list<MACHINE>::iterator it;
    int didFind = 0;
    mtx->lock();
    for (it = machines.begin(); it != machines.end(); ++it){
        didFind = machine.ip_address == it->ip_address;
    }
    if(!didFind) {
        machines.push_back(machine);
    }
    mtx->unlock();

    return !didFind;
}

int Manager::remove(MACHINE machine) {
    std::list<MACHINE>::iterator it = machines.begin();
    int didFind = 0;
    MACHINE _temp;
    mtx->lock();
    while (it != machines.end()) {
        if (!strcmp(it->ip_address, machine.ip_address)) {
            didFind = 1;
            machines.erase(it++);
            break;
        }
        else {
            ++it;
        }
    }
    mtx->unlock();

    return didFind;
}
int Manager::change(MACHINE machine, int status) {
    std::list<MACHINE>::iterator it = machines.begin();
    int didFind = 0;
    MACHINE _temp;
    mtx->lock();
    while (it != machines.end()) {
        if (!strcmp(it->ip_address, machine.ip_address)) {
            didFind = 1;
            it->status = status;
            break;
        }
        else {
            ++it;
        }
    }
    mtx->unlock();
    return didFind;
}

int Manager::sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen)
{
    return syscall(SYS_sysctl, name, namelen, oldp, oldlenp, newp, newlen);
}

int Manager::getHostname(char *buf, size_t buflen)
{
    int name[] = { CTL_KERN, KERN_HOSTNAME };
    size_t namelen = 2;

    return sysctl(name, namelen, buf, &buflen, NULL, 0);
}

Manager::Manager() {
    mtx = new std::mutex();
    struct hostent *host_entry;
    machines.resize(0);
    getHostname(me.hostname, HOSTNAME_SIZE);
    host_entry = gethostbyname(me.hostname);
    me.status = AWAKEN;
    me.ip_address = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[1]));
    memcpy(me.mac_address, getMacAddress(), sizeof(me.mac_address));
    // add(me);
}

void Manager::printMachines() {
    std::list<MACHINE>::iterator it;
    
    mtx->lock();
    if(!machines.empty()) {
        cout << "|\t\thostname\t\t|\tMAC Address\t\t|\tIP Address\t|\tStatus\t|" << endl;
        for (it = machines.begin(); it != machines.end(); ++it){
            std::cout << "|\t" << it->hostname; 
            printf("\t|\t%02X:%02X:%02X:%02X:%02X:%02X\t|\t", it->mac_address[0], it->mac_address[1], 
            it->mac_address[2], it->mac_address[3], it->mac_address[4], it->mac_address[5]);
            cout << it->ip_address << "\t|\t";
            if(it->status == 0) {
                cout << "ASLEEP" << "\t|" << endl;
            } else {
                cout << "awaken" << "\t|" << endl;
            }
        }
    }
    
    mtx->unlock();
}

char* Manager::getIp() {
    return me.ip_address;
}

void Manager::waitForCommand() {
    std::string command, hostname;
    std::stringstream _command;
    // std::istringstream f;
    while(1) {
        command.clear();
        std::getline(cin, command);
        
        if(command.find("WAKEUP") != std::string::npos) {
            _command << command;
            std::getline(_command, hostname, ' ');
            std::getline(_command, hostname, ' ');
            cout << hostname;
        } else {
            cout << "Command not found." << endl;
        }
    }
}

int Manager::createCommunicationSocket() {
//     struct sockaddr_in address;
//     int server_fd, new_socket, valread;
//     struct sockaddr_in address;
//     int opt = 1;
//     int addrlen = sizeof(address);
//     char buffer[1024] = { 0 };
//     char* hello = "Hello from server";

//     // Creating socket file descriptor
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//         perror("socket failed");
//         exit(EXIT_FAILURE);
//     }

//     // Forcefully attaching socket to the port 8080
//     if (setsockopt(server_fd, SOL_SOCKET,
//                    SO_REUSEADDR | SO_REUSEPORT, &opt,
//                    sizeof(opt))) {
//         perror("setsockopt");
//         exit(EXIT_FAILURE);
//     }

//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(COMMUNICATION_PORT);
  
//     // Forcefully attaching socket to the port 8080
//     if (bind(server_fd, (struct sockaddr*)&address,
//              sizeof(address))
//         < 0) {
//         perror("bind failed");
//         exit(EXIT_FAILURE);
//     }
//     if (listen(server_fd, 3) < 0) {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
//     if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
//                   (socklen_t*)&addrlen))
//         < 0) {
//         perror("accept");
//         exit(EXIT_FAILURE);
//     }
//     while(1) {
//         valread = read(new_socket, buffer, 1024);
//         printf("%s\n", buffer);
//         send(new_socket, hello, strlen(hello), 0);
//         printf("Hello message sent\n");
//     }
    
//   // closing the connected socket
//     close(new_socket);
//   // closing the listening socket
//     shutdown(server_fd, SHUT_RDWR);
//     return 0;
}

unsigned char* Manager::createMagicPacket(unsigned char* macAddress) {
    int i;
    unsigned char toSend[102];

    for (i=0; i<6; i++)
        toSend[i] = 0xFF;
 
    for (i=1; i<=16; i++)
        memcpy(&toSend[i*6], &macAddress, 6*sizeof(unsigned char));
    
    return toSend;
}

unsigned char* Manager::getMacAddress() {
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

    // struct ifreq ifr;
    // struct ifconf ifc;
    // char buf[1024];
    // int success = 0;

    // int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    // if (sock == -1) { return NULL; };

    // ifc.ifc_len = sizeof(buf);
    // ifc.ifc_buf = buf;
    // if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    //     close(sock);
    //     return NULL;
    //  }

    // struct ifreq* it = ifc.ifc_req;
    // const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    // for (; it != end; ++it) {
    //     strcpy(ifr.ifr_name, it->ifr_name);
    //     if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
    //         if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
    //             if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
    //                 success = 1;
    //                 break;
    //             }
    //         }
    //     }
    //     else { 
    //         close(sock);
    //         return NULL;
    //      }
    // }

    // unsigned char mac_address[6];

    // if (success) memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);

    // return mac_address;
}