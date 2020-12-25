#ifndef _ARP_H
#define _ARP_H

#include "eth.h"
#include "ipv4.h"
#include <stdint.h>


extern mac_addr_t MAC_BCAST_ADDR;

struct arp_message;

int arp_resolve(eth_iface_t *iface, ipv4_addr_t src, ipv4_addr_t destino, mac_addr_t mac);


#endif /* _ARP_H */
