#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/manager.h"
#include "../include/packet.h"
#include "../include/client.h"
using namespace std;

void* discover(void *arg);
void* waitForCommand(void *arg);
// void* broadcast(void *arg);
// void* listenForPackets(void *arg);
int type = 0;
pthread_t discoveryThreadId, listenToDiscoveryId;
Manager manager;
Client client;
int stopDiscovery = 0;

int main(int argc, char *argv[]) { 
    int didCreateDiscoverThread, didCreateCommandThread;
    if (argc < 1 || argc > 2) {
        cout << "Usage: ./sleep_server [manager]" << endl;
    }

    if(argc == 2 && !strcmp("manager", argv[1])) {
        type = 1;
        manager = Manager();
        manager.printMachines();
        didCreateDiscoverThread = pthread_create(&manager.discoveryThreadId, NULL, &discover, NULL);
        didCreateCommandThread = pthread_create(&manager.commandThreadId, NULL, &waitForCommand, NULL);
        pthread_join(manager.discoveryThreadId, 0);
        pthread_join(manager.commandThreadId, 0);
    } else {
        client = Client();
        didCreateDiscoverThread = pthread_create(&client.discoveryThreadId, NULL, &discover, NULL);
        didCreateCommandThread = pthread_create(&client.waitForCommandThreadId, NULL, &waitForCommand, NULL);
        pthread_join(client.discoveryThreadId, 0);
        pthread_join(client.waitForCommandThreadId, 0);
    }

    return 0;
}

void errno_abort(const char* header) {
    perror(header);
    exit(EXIT_FAILURE);
}

void* waitForCommand(void *arg) {
    if(type == 1) {
        manager.waitForCommand();
    } else {
        client.waitForCommand();
    }
} 

void* discover(void *arg) {
// #define SERVERPORT 4567
    struct sockaddr_in send_addr, recv_addr;

    int trueflag = 1, count = 0;
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        errno_abort("socket");
#ifndef RECV_ONLY
    // if(type == 1){
        if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
                    &trueflag, sizeof trueflag) < 0)
            errno_abort("setsockopt");

        memset(&send_addr, 0, sizeof send_addr);
        send_addr.sin_family = AF_INET;
        send_addr.sin_port = (in_port_t) htons(DISCOVER_PORT);
        // broadcasting address for unix (?)
        // inet_aton("191.168.0.255", &send_addr.sin_addr);
        send_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    // } 
#endif // ! RECV_ONLY
    // else {
#ifndef SEND_ONLY
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                    &trueflag, sizeof trueflag) < 0)
            errno_abort("setsockopt");

        memset(&recv_addr, 0, sizeof recv_addr);
        recv_addr.sin_family = AF_INET;
        recv_addr.sin_port = (in_port_t) htons(DISCOVER_PORT);
        recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(fd, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0)
            errno_abort("bind");
    // }
#endif // ! SEND_ONLY
    Packet packetTosend = createPacket(SLEEP_SERVICE_DISCOVERY, 0, 0, "");
    // void *received = NULL;
    Packet received;
    socklen_t recv_addr_len = sizeof(recv_addr);
    while (1) {
#ifndef RECV_ONLY
        if(type == 1) {
            usleep(1000000);
            if (sendto(fd, (void *)&packetTosend, sizeof(Packet), 0, (struct sockaddr*) &send_addr, sizeof send_addr) < 0)
                errno_abort("send");
            if (recvfrom(fd, (void*)&received, sizeof(Packet), 0, (struct sockaddr *) &recv_addr, &recv_addr_len) < 0) {
                errno_abort("recv");
            } else {
                MACHINE newMachine;
                strcpy(newMachine.hostname, received.payload);
                newMachine.status = AWAKEN;
                newMachine.ip_address = inet_ntoa(recv_addr.sin_addr);
                if(manager.add(newMachine)) {
                    // cout << u8"\033[2J\033[1;1H"; 
                    manager.printMachines();
                }
            }
        }
         else {
            if (recv(fd, (void*)&received, sizeof(Packet), 0) < 0) {
                errno_abort("recv");
            } else {
                printf("received");
            }
            // } else {
            //     if (sendto(fd, (void *)&packetTosend, sizeof(Packet), 0, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0)
            //         errno_abort("respond");
            // }
        }
        
        
#endif // ! RECV_ONLY

// #ifndef SEND_ONLY
//         char rbuf[256] = {};
//         if (recv(fd, rbuf, sizeof(rbuf)-1, 0) < 0)
//             errno_abort("recv");
//         printf("recv: %s\n", rbuf);
// #endif // ! SEND_ONLY
        }
    close(fd);
}

// void* discover(void *arg) {
// 	sockaddr_in serv, client;
//     int socket_stream, bytes_sent, bytes_received;
// 	Packet *received = NULL;
//     int broadcast=1;

//     assert((socket_stream=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))!=-1);
	
//     setsockopt(socket_stream, SOL_SOCKET, SO_BROADCAST,
//                 &broadcast, sizeof broadcast);

//     memset(&serv, 0, sizeof(serv));
//     serv.sin_family = AF_INET;
//     serv.sin_port = htons(DISCOVER_PORT);
//     serv.sin_addr.s_addr = inet_addr(manager.getIp());
// 	socklen_t clilen = sizeof(struct sockaddr_in);
	
// 	if (bind(socket_stream, (struct sockaddr *) &serv, sizeof(struct sockaddr)) < 0) 
// 		printf("ERROR on binding");

// 	Packet packetTosend = createPacket(SLEEP_SERVICE_DISCOVERY, 0, 0, "");
// 	while(1) {
// 		if(type == 1) {
// 			bytes_sent = sendto(socket_stream, (void*)&packetTosend, sizeof(Packet), 0, (const struct sockaddr*)&serv, sizeof(struct sockaddr));
// 			if (bytes_sent < 0) cout << "ERROR on sendto" << endl;
// 		}
// 	}
    
	// int sockfd, n;
	// unsigned int length;
	// struct sockaddr_in serv_addr, from;
	
	// char buffer[256];
	// if (argc < 2) {
	// 	fprintf(stderr, "usage %s hostname\n", argv[0]);
	// 	exit(0);

	// }
	// if (server == NULL) {
    //     fprintf(stderr,"ERROR, no such host\n");
    //     exit(0);
    // }	
	
	// if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	// 	printf("ERROR opening socket");
	
	// serv_addr.sin_family = AF_INET;     
	// serv_addr.sin_port = htons(DISCOVER_PORT);    
	// serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	// bzero(&(serv_addr.sin_zero), 8);  

	// printf("Enter the message: ");
	// bzero(buffer, 256);
	// fgets(buffer, 256, stdin);

	// n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	// if (n < 0) 
	// 	printf("ERROR sendto");
	
	// length = sizeof(struct sockaddr_in);
	// n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length);
	// if (n < 0)
	// 	printf("ERROR recvfrom");

	// printf("Got an ack: %s\n", buffer);
		
	// close(sockfd);
	
//     cout << "teste";
//     // if(type == 1) manager.printMachines();
//     int sockfd, n;
// 	socklen_t clilen;
// 	struct sockaddr_in serv_addr, cli_addr;
// 	Packet *received = NULL;
	
//     if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
// 		printf("ERROR opening socket");
// cout << "teste";
// 	serv_addr.sin_family = AF_INET;
// 	serv_addr.sin_port = htons(DISCOVER_PORT);
// 	serv_addr.sin_addr.s_addr = INADDR_ANY;
// 	bzero(&(serv_addr.sin_zero), 8);    
// 	 cout << "teste";
// 	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
// 		printf("ERROR on binding");
// 	cout << "teste";
// 	clilen = sizeof(struct sockaddr_in);
    
// 	Packet packetToSend = createPacket(SLEEP_SERVICE_DISCOVERY, 0, 0, "");
//     while (1) {
//     // /* send to socket */
//     //     n = sendto(sockfd, (void *)&packetToSend, sizeof(Packet), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
//     //     if (n < 0) printf("ERROR on sendto");

//     // /* receive from socket */
//     //     n = recvfrom(sockfd, received, sizeof(Packet), 0, (struct sockaddr *) &cli_addr, &clilen);
//     //     if (n < 0) printf("ERROR on recvfrom");

//     //     if(type == 1) {
//     //         machine newMachine;
//     //         newMachine.ip_address = inet_ntoa(cli_addr.sin_addr);
//     //         newMachine.status = 1;
//     //         manager.add(newMachine);
//     //         system("clear");
//     //         manager.printMachines();
//     //     } else {
//     //         n = sendto(sockfd, (void *)&packetToSend, sizeof(Packet), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
//     //         if (n  < 0) 
//     //             printf("ERROR on sendto");
//     //     }
        
// 	 }
	
// 	close(sockfd);
// }
// void* broadcast(void *arg) {
//     int socketSD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if (socketSD < 0) {
//         cout << "Error: Could not open socket." << endl;
//     }

//     // set socket options enable broadcast
//     int broadcastEnable = 1;
//     int ret = setsockopt(socketSD, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
//     if (ret) {
//         cout << "Error: Could not open set socket to broadcast mode" << endl;
//         close(socketSD);
//     }
    
//     // Configure the port and ip we want to send to
//     struct sockaddr_in broadcastAddr, cli_addr;
//     memset(&broadcastAddr, 0, sizeof(broadcastAddr));
//     broadcastAddr.sin_family = AF_INET;

//     inet_pton(AF_INET, manager.getIp(), &broadcastAddr.sin_addr);
//     broadcastAddr.sin_port = htons(DISCOVER_PORT);
//     broadcastAddr.sin_addr.s_addr = INADDR_ANY;
//     socklen_t clilen = sizeof(struct sockaddr);
//     Packet packetTosend = createPacket(SLEEP_SERVICE_DISCOVERY, 0, 0, "");
//     Packet *buf = NULL;

//     // if (bind(socketSD, (struct sockaddr *) &broadcastAddr, sizeof(struct sockaddr)) < 0) 
// 	// 	printf("ERROR on binding");

// 	// while(type) {
//     //     if(type == 1) {
//     //         ret = sendto(socketSD, (void*)&packetTosend, sizeof(Packet), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
//     //         if (ret < 0) {
//     //             cout << "Error: Could not open send broadcast." << endl;
//     //         }
            

//     //         // ssize_t n = recvfrom(socketSD, buf, sizeof(Packet), 0, (struct sockaddr *) &cli_addr, &clilen);
//     //         // if (n < 0)
//     //         //     printf("%d", n); 
//     //         //     printf("ERROR on recvfrom");
//     //         // }
//     //     }
// 	// }
    
//     close(socketSD);
// }

// void* listenForPackets(void *arg) {
//     int listeningSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if (listeningSocket <= 0) {
//         cout << "Error: listenForPackets - socket() failed." << endl;
//     }
    
//     // set timeout to 2 seconds.
//     struct timeval timeV;
//     timeV.tv_sec = 2;
//     timeV.tv_usec = 0;
    
//     // if (setsockopt(listeningSocket, SOL_SOCKET, SO_RCVTIMEO, &timeV, sizeof(timeV)) == -1) {
//     //     cout << "Error: listenForPackets - setsockopt failed" << endl;
//     //     close(listeningSocket);
//     // }
    
//     // bind the port
//     struct sockaddr_in sockaddr;
//     memset(&sockaddr, 0, sizeof(sockaddr));
    
//     sockaddr.sin_len = sizeof(sockaddr);
//     sockaddr.sin_family = AF_INET;
//     sockaddr.sin_port = htons(DISCOVER_PORT);
//     sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
//     int status = bind(listeningSocket, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
//     if (status == -1) {
//         close(listeningSocket);
//         cout << "Error: listenForPackets - bind() failed." << endl;
//     }
    
//     // receive
//     struct sockaddr_in receiveSockaddr;
//     socklen_t receiveSockaddrLen = sizeof(receiveSockaddr);
//                 cout << "teste";

//     size_t bufSize = 9216;
//     Packet packetTosend = createPacket(SLEEP_SERVICE_DISCOVERY, 0, 0, "");
//     while(1) {
//         void *received = malloc(sizeof(Packet));
//         ssize_t result = recvfrom(listeningSocket, &received, sizeof(Packet), 0, (struct sockaddr *)&receiveSockaddr, &receiveSockaddrLen);
//         if (result > 0) {
            
//             char addrBuf[INET_ADDRSTRLEN];
//             if (inet_ntop(AF_INET, &receiveSockaddr.sin_addr, addrBuf, (size_t)sizeof(addrBuf)) == NULL) {
//                 addrBuf[0] = '\0';
//             }

            

//             if(type == 1) {
//                 machine newMachine;
//                 newMachine.ip_address = inet_ntoa(receiveSockaddr.sin_addr);
//                 newMachine.status = 1;
//                 manager.add(newMachine);
//                 system("clear");
//                 manager.printMachines();
//             } else {
//                 int bytes_sent = sendto(listeningSocket, (void*)&packetTosend, sizeof(Packet), 0, (struct sockaddr*)&receiveSockaddr, sizeof(receiveSockaddr));
//                 if (bytes_sent < 0) cout << "ERROR on sendto" << endl;
//             }
//         }
//     }
    
//     close(listeningSocket);
// }
    
    
    // int bytes_sent;
    // Packet packetTosend = createPacket(SLEEP_SERVICE_DISCOVERY, 0, 0, "");
    // while(1) {
    //     Packet *received = NULL;

    //     int result = recvfrom(listeningSocket, received, sizeof(*received), 0, (struct sockaddr *)&receiveSockaddr, &receiveSockaddrLen);
    //     if (result < 0) cout << "ERROR on recvfrom" << endl;
    //     else {
    //         if(type == 1) {
    //             machine newMachine;
    //             newMachine.ip_address = inet_ntoa(receiveSockaddr.sin_addr);
    //             newMachine.status = 1;
    //             manager.add(newMachine);
    //             system("clear");
    //             manager.printMachines();
    //         } else {
    //             int bytes_sent = sendto(listeningSocket, (void*)&packetTosend, sizeof(Packet), 0, (struct sockaddr*)&receiveSockaddr, sizeof(receiveSockaddr));
    //             if (bytes_sent < 0) cout << "ERROR on sendto" << endl;
    //         }
    //     }
    // } 


    // if (result > 0) {
        
    //     // if ((size_t)result != bufSize) {
    //     //     buf = realloc(buf, result);
    //     // }
        
    //     char addrBuf[INET_ADDRSTRLEN];
    //     if (inet_ntop(AF_INET, &receiveSockaddr.sin_addr, addrBuf, (size_t)sizeof(addrBuf)) == NULL) {
    //         addrBuf[0] = '\0';
    //     }
        
    //     cout << buf << endl;
        
    // } else {
    //     free(buf);
    // }
    
    // close(listeningSocket);
// }

