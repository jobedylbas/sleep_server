#ifndef MACHINE_H
#define MACHINE_H

#include "constants.h"

typedef struct _machine {
    char hostname[HOSTNAME_SIZE];
    int status;
    unsigned char mac_address[6];
    char* ip_address;
} MACHINE;

#endif