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
    me.ip_address = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
}

void Client::printManager() {
    cout << "|\t\thostname\t\t|\tMAC Address\t|\tIP Address\t|" << endl;
    std::cout << "|\t" << manager.hostname << "\t\t|\t\t\t|\t" << manager.ip_address << "\t|\t";
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