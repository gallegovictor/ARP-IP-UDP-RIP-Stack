#include "arp.h"
#include "ipv4.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <timerms.h>


//generar fichero con el cache de ARP
//Inicializarlo a 0??


int main(int argc, char *argv[]) {
    /* Mostrar mensaje de ayuda si el número de argumentos es incorrecto */
    char *myself = basename(argv[0]);
    if ((argc <= 2) || (argc > 3)) {
        printf("Uso: %s <iface> <ip> [<long>]\n", myself);
        printf("       <iface>: Nombre de la interfaz ARP\n");
        printf("        <ip>: ip del pc del cual necesitas su MAC\n");
        exit(-1);
    }

    //procesamos los argumentos
    char *iface_name = argv[1];
    ipv4_addr_t ipv4_addr_dest;

    if (ipv4_str_addr(argv[2], ipv4_addr_dest)) {
        printf("Direccion ip erronea\n");
        exit(-1);
    }


    //abrimos el puerto
    eth_iface_t *iface = eth_open(iface_name);
    if (iface == NULL) {
        printf("No se pudo abrir la interfaz\n");
        exit(-1);
    }
    mac_addr_t mac;

    int resolve = arp_resolve(iface, IPv4_ZERO_ADDR, ipv4_addr_dest, mac);

    if (resolve == -2) {
        printf("No se pudo enviar el mensaje arp request\n");
        exit(-1);
    } else if (resolve == -1 || resolve == 0) {
        printf("No se recibio ningun ARP reply\n");
        exit(-1);
    }
    char mac_str[MAC_ADDR_SIZE];
    mac_addr_str(mac, mac_str);
    printf("ip destino= %s -> Mac destino= %s\n", argv[2], mac_str);
    eth_close(iface);


}
