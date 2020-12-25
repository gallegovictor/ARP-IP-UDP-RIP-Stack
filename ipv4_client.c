#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <timerms.h>

#include "ipv4.h"


int main(int argc, char *argv[]) {
    //Necesitamos como parametros el nombre de la interfaz
    //el nombre del archivo de text de la config y routas
    //y la ip al que se le debe enviar el mensaje

    if ((argc <= 5) || (argc > 6)) {
        printf("       <string.txt>: Nombre del archivo config.txt\n");
        printf("       <string.txt>: Nombre del archivo route_table.txt\n");
        printf("        <protocol>: tipo del mensaje a enviar\n");
        printf("        <ip>: ip del pc del cual necesitas su MAC\n");
        printf("        <int>: bytes a mandar\n");
        exit(-1);
    }

    //procesamos los argumentos


    char *config_name = argv[1];
    char *route_table_name = argv[2];
    char *ipv4_type_str = argv[3];
    char *endptr;
    /* El Tipo puede indicarse en hexadecimal (0x0800) o en decimal (2048) */
    int ipv4_type_int = (int) strtol(ipv4_type_str, &endptr, 0);
    if ((*endptr != '\0') || (ipv4_type_int < 0) || (ipv4_type_int > 0x0000FFFF)) {
        printf("Error en el argumento protocol\n");
        exit(-1);
    }
    uint16_t ipv4_protocol = (uint16_t) ipv4_type_int;


    char *ip_str = argv[4];

    //comprobamos que la IP es valida
    ipv4_addr_t ip_addr;
    if (ipv4_str_addr(ip_str, ip_addr) != 0) {
        printf("Ip no valida\n");
        exit(-1);
    }
    
    char *payload_len_str = argv[5];
    int payload_len_input = atoi(payload_len_str);
    
    if (payload_len_input > MRU){
        printf("Longitud del argumento payload demasiado larga\n");
        exit(-1);
    }

    //Abrimos la interfaz y comprobamos que se leyo el archivo;
    ipv4_layer_t *ip_layer = ipv4_open(config_name, route_table_name);

    if (ip_layer == NULL) {
        printf("No se pudo leer correctamente el fichero config.txt\n");
        exit(-1);
    }


    //rellenamos el payload de numeros aleatorios

    unsigned char payload[payload_len_input];
    int i;
    for (i = 0; i < payload_len_input; i++) {
        payload[i] = (unsigned char) i;
    };

    //mandamos el mensaje, Que protocolo?

    if (ipv4_send(ip_layer, ip_addr, ipv4_protocol, payload, payload_len_input) == -1) {
        printf("No se pudo enviar el paquete\n");
        exit(-1);
    }
    printf("enviado el paquete\n");

    unsigned char buffer[MRU];
    ipv4_addr_t src;

    printf("Escuchando a trama de vuelta\n");
    int payload_len = ipv4_recv(ip_layer, 0X45, buffer, src, MRU, 2000);

    if (payload_len == -1) {
        printf("Error al recibir la trama\n");
        exit(-1);
    } else if (payload_len > 0) {
        printf("Recibido el paquete\n");

    }

    ipv4_close(ip_layer);

}
