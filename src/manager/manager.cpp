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

    getHostname(me.hostname, HOSTNAME_SIZE);
    host_entry = gethostbyname(me.hostname);
    me.status = AWAKEN;
    me.ip_address = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[1]));
    add(me);
}

void Manager::printMachines() {
    std::list<MACHINE>::iterator it;
    cout << "|\t\thostname\t\t|\tMAC Address\t|\tIP Address\t|\tStatus\t|" << endl;
    mtx->lock();
    for (it = machines.begin(); it != machines.end(); ++it){
        std::cout << "|\t" << it->hostname << "\t\t|\t\t\t|\t" << it->ip_address << "\t|\t";
        if(it->status == 0) {
            cout << "ASLEEP" << "\t|" << endl;
        } else {
            cout << "awaken" << "\t|" << endl;
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
    struct sockaddr_in address;
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(COMMUNICATION_PORT);
  
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    while(1) {
        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);
        send(new_socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");
    }
    
  // closing the connected socket
    close(new_socket);
  // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}