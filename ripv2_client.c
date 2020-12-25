#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <string.h>

#include "ripv2_route_table.h"
#include "arp.h"

int main(int argc, char *argv[]) {


    if ((argc <= 3) || (argc > 5)) {
        printf("       <string.txt>: Nombre del archivo config.txt\n");
        printf("       <string.txt>: Nombre del archivo route_table.txt\n");
        printf("       <ipv4>: ip del servidor a enviar request\n");
        printf("       <string.txt>: Nombre del archivo ripv2_route_table(OPCIONAL)\n");
        exit(-1);
    }

    char *config_name = argv[1];
    char *route_table_name = argv[2];
    char *ip_str = argv[3];

    //comprobamos que la IP es valida
    ipv4_addr_t ip_addr;
    if (ipv4_str_addr(ip_str, ip_addr) != 0) {
        printf("Ip no valida\n");
        exit(-1);
    }

    udp_layer_t *udp_layer = udp_open(RIP_PORT, config_name, route_table_name);
    if (udp_layer == NULL) {
        printf("Fallo al abrir la interfaz");
        exit(-1);
    }

    ripv2_msg_t msg;
    msg.type = RIPv2_REQUEST;
    msg.version = RIPv2_TYPE_VERSION;
    msg.routing_domain = UNUSED;

    int n_routes = 0;

    if (argc == 5) { //Si especifiamos tabla_routas_txt mandamos solo las que estan ahi

        char *rip_route_table_name = argv[4];
        rip_route_table_t *rip_table = ripv2_route_table_create();
        ripv2_route_table_read(rip_route_table_name, rip_table);

        ripv2_route_table_print(rip_table);

        for (int i=0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            entrada_rip_t *entry = rip_table->routes[i];
            if (entry == NULL) break;
            entry->family_directions = htons(entry->family_directions);
            entry->metric = htonl(entry->metric);
            msg.entrada[i] = *(entry);
            n_routes++;

        }

        udp_send(udp_layer, ip_addr, RIP_PORT, (unsigned char *) &msg,  sizeof(entrada_rip_t) * n_routes + RIP_HEADER_SIZE);

    }


    else { //si no especificamos nada perdimos por toda la tabla

        entrada_rip_t request_all;
        request_all.family_directions = UNUSED;
        request_all.route_label = UNUSED;
        memcpy(request_all.gw, IPv4_ZERO_ADDR, sizeof(ipv4_addr_t));
        memcpy(request_all.mask, IPv4_ZERO_ADDR, sizeof(ipv4_addr_t));
        memcpy(request_all.subnet, IPv4_ZERO_ADDR, sizeof(ipv4_addr_t));
        request_all.metric = htonl(16);

        msg.entrada[0] = request_all;
        n_routes++;

        ipv4_addr_t RIPv2_MULTICAST_ADDR = {224, 0, 0, 9};

        udp_send(udp_layer, RIPv2_MULTICAST_ADDR, RIP_PORT, (unsigned char *) &msg,  sizeof(entrada_rip_t) * n_routes + RIP_HEADER_SIZE);
    }




    uint16_t port;
    ripv2_msg_t msg_recv;

    int bytes = udp_recv(udp_layer, -1, ip_addr, &port, (unsigned char *) &msg_recv, sizeof(msg_recv));

	if ( port == RIP_PORT && msg_recv.type != RIPv2_RESPONSE) {
        printf("baya\n");
        exit(-1);
    }

    for (int i= 0; i < (bytes - RIP_HEADER_SIZE) / sizeof(entrada_rip_t); i++) {
        char subnet_str[IPv4_STR_MAX_LENGTH];
        char mask_str[IPv4_STR_MAX_LENGTH];
        char gw_str[IPv4_STR_MAX_LENGTH];

        entrada_rip_t entry = msg_recv.entrada[i];

        ipv4_addr_str(entry.subnet, subnet_str);
        ipv4_addr_str(entry.mask, mask_str);
        ipv4_addr_str(entry.gw, gw_str);


        printf("subnet: %s, mask: %s, gw: %s, metric: %i\n", subnet_str, mask_str, gw_str, ntohl(entry.metric));
    }


}
