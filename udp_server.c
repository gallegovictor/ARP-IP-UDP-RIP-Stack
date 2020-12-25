#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <timerms.h>
#include <arpa/inet.h>

#include "ipv4.h"
#include "udp.h"


int main(int argc, char *argv[]) {
    //Necesitamos como parametros el nombre de la interfaz
    //el nombre del archivo de text de la config y routas
    //y la ip al que se le debe enviar el mensaje

    if ((argc <= 3) || (argc > 4)) {
        printf("       <string.txt>: Nombre del archivo config.txt\n");
        printf("       <string.txt>: Nombre del archivo route_table.txt\n");
        printf("       <int>: Puerto a escuchar\n");
        exit(-1);
    }

    //procesamos los argumentos

    char *config_name = argv[1];
    char *route_table_name = argv[2];
    uint16_t port_in = atoi(argv[3]);

    //Abrimos la interfaz y comprobamos que se leyo el archivo;
    udp_layer_t *udp_layer = udp_open(port_in ,config_name, route_table_name);

    if (udp_layer == NULL) {
        printf("No se pudo leer correctamente el fichero config.txt\n");
        exit(-1);
    }

    unsigned char buffer[UDP_PACKET_LEN];
    uint16_t port;
    ipv4_addr_t sender;
	int payload_len;
    //ipv4_addr_t src_addr;

    while (1) {

        /* Recibir trama Ethernet del Cliente */

        long int timeout = -1;

        printf("Escuchando a tramas udp\n");

        payload_len = udp_recv(udp_layer, timeout, sender, &port, buffer, UDP_PACKET_LEN);

        if (payload_len == -1) {
            printf("Error al recibir la trama\n");
            exit(-1);
        }

        printf("Recibidos %d bytes\n", payload_len);
        break;

    }

    printf("Enviando paquetes de vuelta\n");

	printf("%i\n", port);

    udp_send(udp_layer, sender, port, buffer, payload_len);

    udp_close(udp_layer);
}
