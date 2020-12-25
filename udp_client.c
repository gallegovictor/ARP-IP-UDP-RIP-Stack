
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <timerms.h>

#include "ipv4.h"
#include "udp.h"


int main(int argc, char *argv[]) {
    //Necesitamos como parametros el nombre de la interfaz
    //el nombre del archivo de text de la config y routas
    //y la ip al que se le debe enviar el mensaje

    if ((argc <= 6) || (argc > 7)) {
        printf("       <string.txt>: Nombre del archivo config.txt\n");
        printf("       <string.txt>: Nombre del archivo route_table.txt\n");
        printf("        <ip>: ip del pc del cual necesitas su MAC\n");
        printf("        <puerto_in>: nuestro puerto\n");
        printf("        <puerto_out>: puerto al que se manda\n");
        printf("       <int>: numero bytes a mandar\n");
        exit(-1);
    }

    //procesamos los argumentos

    char *config_name = argv[1];
    char *route_table_name = argv[2];
    char *ip_str = argv[3];
    uint16_t port_in = atoi(argv[4]);
    uint16_t port_out = atoi(argv[5]);

    char *payload_len_str = argv[6];
    int payload_len_input = atoi(payload_len_str);

    if (payload_len_input > UDP_PACKET_LEN) {
        printf("Longitud del argumento payload demasiado larga\n");
        exit(-1);
    }

    //comprobamos que la IP es valida
    ipv4_addr_t ip_addr;
    if (ipv4_str_addr(ip_str, ip_addr) != 0) {
        printf("Ip no valida\n");
        exit(-1);
    }

    udp_layer_t *udp_layer = udp_open(port_in, config_name, route_table_name);
    if (udp_layer == NULL) {
        printf("Fallo al abrir la interfaz");
        exit(-1);
    }

    unsigned char payload[payload_len_input];
    int i;
    for (i = 0; i < payload_len_input; i++) {
        payload[i] = (unsigned char) i;
    };

    printf("Enviando paquete\n");
    if (udp_send(udp_layer, ip_addr, port_out, payload, payload_len_input) == -1) {
        printf("No se pudo enviar\n");
        exit(-1);
    }
    printf("Escuchando a trama de vuelta\n");

    int timeout = -1;
    ipv4_addr_t sender;
    uint16_t *port = malloc(sizeof(uint16_t));

    int payload_len = udp_recv(udp_layer, timeout, sender, port, payload, payload_len_input);

    if (payload_len == -1) {
        printf("Error al recibir la trama\n");
        exit(-1);
    } else if (payload_len > 0) {
        printf("Recibidos %d bytes\n", payload_len);
    }

    free(port);

    udp_close(udp_layer);

}
