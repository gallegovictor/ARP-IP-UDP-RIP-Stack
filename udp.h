//
// Created by serbe on 13/10/2020.
//

#ifndef RYSCA_UDP_H
#define RYSCA_UDP_H

#include "ipv4.h"

//Variables para paquete UDP
#define UDP_PROTOCOL 0x11
#define UDP_PACKET_LEN 1472
#define UDP_HEADER_LEN 8
#define ERROR 001


typedef struct udp_layer {
    uint16_t source_port;
    ipv4_layer_t *ipv4_layer;

} udp_layer_t;


typedef struct udp_packet {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t len;
    uint16_t checksum;
    unsigned char payload[UDP_PACKET_LEN];
} udp_packet_t;


udp_layer_t *udp_open(uint16_t src_port, char *ip_config, char *route_config);

int udp_send(udp_layer_t *layer, ipv4_addr_t dst, uint16_t port_out, unsigned char data[], int payload_len);

int udp_recv(udp_layer_t *layer, long int timeout, ipv4_addr_t sender, uint16_t *port, unsigned char * payload, int payload_len);

void udp_close(udp_layer_t *my_layer);

#endif //RYSCA_UDP_H


