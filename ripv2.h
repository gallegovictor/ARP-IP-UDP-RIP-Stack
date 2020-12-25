//
// Created by serbe on 01/12/2020.
//

#ifndef RYSCA_RIPV2_H
#define RYSCA_RIPV2_H

#include "udp.h"
#include <timerms.h>
#include "ripv2_route_table.h"

#define UNUSED 0x0000
#define DEFAULT_FAMILY_DIRECTION 2
#define RIPv2_TYPE_VERSION 2
#define RIPv2_RESPONSE 2
#define RIPv2_REQUEST 1
#define RIP_ROUTE_TABLE_SIZE 25
#define RIP_PORT 520
#define RIP_HEADER_SIZE 4

typedef struct entrada_rip{

    uint16_t family_directions;
    uint16_t route_label; //unused
    ipv4_addr_t subnet;
    ipv4_addr_t mask;
    ipv4_addr_t gw;
    uint32_t metric;

} entrada_rip_t;


typedef struct rip_route_table {
    entrada_rip_t *routes[RIP_ROUTE_TABLE_SIZE];
} rip_route_table_t;


typedef struct ripv2_msg{

        uint8_t type;
        uint8_t version;
        uint16_t routing_domain; //unused
        entrada_rip_t entrada[RIP_ROUTE_TABLE_SIZE];

} ripv2_msg_t;

typedef struct timers{
    timerms_t list_timers[RIP_ROUTE_TABLE_SIZE];
}timers_t;



#endif //RYSCA_RIPV2_H
