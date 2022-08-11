#ifndef PACKET_H
#define PACKET_H

#include "stdint.h"
#include "constants.h"

typedef struct __packet {
    uint16_t type;          // Type of packet 
    uint16_t sqn;           // Sequence number
    uint16_t len;       	// Payload length 
    uint16_t timestamp;     // Timestamp, in a "hhmm" format. E.g. 22h04min -> 2204, 2h15min -> 215
    uint16_t userid;        //  Used only for communication between RMs, to indicate what user will do what operation
    char payload[HOSTNAME_SIZE];		    // Message data
    int status;             // Status of machine
} Packet;

Packet createPacket(uint16_t type, uint16_t sqn, time_t timestamp, std::string payload);

Packet createPacket(uint16_t type, uint16_t sqn, time_t timestamp, std::string payload)
{
    Packet packet;
    packet.type = type;
    packet.sqn = sqn;
    packet.timestamp = timestamp;

    // memset(packet.payload, 0, MAX_MESSAGE_SIZE);
    // strcpy(packet.payload, payload.c_str());
    packet.len = sizeof(packet.payload);

    return packet;
}

#endif