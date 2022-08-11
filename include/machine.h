#ifndef MACHINE_H
#define MACHINE_H

#include "constants.h"

typedef struct _machine {
    char hostname[HOSTNAME_SIZE];
    int status;
    char* mac_address;
    char* ip_address;
} MACHINE;

#endif