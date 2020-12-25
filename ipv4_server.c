#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <timerms.h>
#include <arpa/inet.h>

#include "ipv4.h"


int main(int argc, char *argv[]) {
    //Necesitamos como parametros el nombre de la interfaz
    //el nombre del archivo de text de la config y routas
    //y la ip al que se le debe enviar el mensaje

    if ((argc <= 3) || (argc > 4)) {
        printf("       <string.txt>: Nombre del archivo config.txt\n");
        printf("       <string.txt>: Nombre del archivo route_table.txt\n");
        printf("        <protocol>: tipo del mensaje a enviar\n");
        exit(-1);
    }

    //procesamos los argumentos

    char *config_name = argv[1];
    char *route_table_name = argv[2];

    //Procesamos el argumento protocol y comprobamos que esta bien
    char *ipv4_type_str = argv[3];
    char *endptr;
    /* El Tipo puede indicarse en hexadecimal (0x0800) o en decimal (2048) */
    int ipv4_type_int = (int) strtol(ipv4_type_str, &endptr, 0);
    if ((*endptr != '\0') || (ipv4_type_int < 0) || (ipv4_type_int > 0x0000FFFF)) {
        printf("Error en el argumento protocol\n");
        exit(-1);
    }
    uint16_t ipv4_protocol = (uint16_t) ipv4_type_int;

    //Abrimos la interfaz y comprobamos que se leyo el archivo;
    ipv4_layer_t *ip_layer = ipv4_open(config_name, route_table_name);

    if (ip_layer == NULL) {
        printf("No se pudo leer correctamente el fichero config.txt\n");
        exit(-1);
    }

    unsigned char buffer[IPV4_FRAME_LEN];
    ipv4_addr_t src_addr;
    int payload_len;

    while (1) {

        /* Recibir trama Ethernet del Cliente */

        long int timeout = -1;

        printf("Escuchando a tramas ipv4\n");
        payload_len = ipv4_recv(ip_layer, ipv4_protocol, buffer, src_addr, MRU, timeout);

        if (payload_len == -1) {
            printf("Error al recibir la trama\n");
            exit(-1);
        }

        printf("Recibido el paquete\n");
        break;

    }

    printf("Enviando datos de vuelta\n");

    ipv4_send(ip_layer, src_addr, ipv4_protocol, buffer, payload_len);

    ipv4_close(ip_layer);
}
