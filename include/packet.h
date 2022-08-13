#ifndef PACKET_H
#define PACKET_H

#include "stdint.h"
#include <string.h>
#include "constants.h"

typedef struct __packet {
    uint16_t type;          // Type of packet 
    uint16_t sqn;           // Sequence number
    uint16_t len;       	// Payload length 
    uint16_t timestamp;     // Timestamp, in a "hhmm" format. E.g. 22h04min -> 2204, 2h15min -> 215
    uint16_t userid;        //  Used only for communication between RMs, to indicate what user will do what operation
    char payload[HOSTNAME_SIZE];		    // Message data
    int status;             // Status of machine
    unsigned char mac_address[6];
} Packet;

Packet createPacket(uint16_t type, uint16_t sqn, time_t timestamp, char* payload, unsigned char* mac_address);

Packet createPacket(uint16_t type, uint16_t sqn, time_t timestamp, char* payload, unsigned char *mac_addres)
{
    Packet packet;
    packet.type = type;
    packet.sqn = sqn;
    packet.timestamp = timestamp;

    memset(packet.payload, 0, MAX_MESSAGE_SIZE);
    strcpy(packet.payload, payload);
    memcpy(packet.mac_address, mac_addres, sizeof(packet.mac_address));
    packet.len = sizeof(packet.payload);

    return packet;
}

#endif