#include "arp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <rawnet.h>
#include <timerms.h>
#include <arpa/inet.h>

#define IP_PROTOCOL 0x0800 //especificamos protocolo ip
#define HARDW_TYPE 0x0001 //especificamos que el hardware es eth

#define ARP_TYPE 0x0806 //especificamos que el mensaje es de tipo ARP
#define ARP_REQUEST 0x0001 //simbolo para ARP request
#define ARP_REPLY 0x0002 //simbolo para ARP reply

//mac de broadcast
mac_addr_t UNKNOW_MAC = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //cuando no sabemos la mac a utilizar


timerms_t timer;
long int timeout = 2000; //2 segundos
long int ecotimeout = 3000; //3 segundos
//definimos la cabecera

typedef struct arp_message {
    uint16_t hard_addr; //mac
    uint16_t protocol_type;
    uint8_t hard_size;
    uint8_t protocol_length;
    uint16_t opcode;
    mac_addr_t mac_sender;
    ipv4_addr_t ip_sender;
    mac_addr_t mac_target;
    ipv4_addr_t ip_target;

} arp_message_t;


int arp_resolve(eth_iface_t *iface, ipv4_addr_t src, ipv4_addr_t destino, mac_addr_t mac) {

    //Creamos y rellenamos la estructura de tipo arp_message que se utilizara como payload
    arp_message_t arp_payload;
    arp_payload.hard_addr = htons(HARDW_TYPE);// correspondiente a eth
    arp_payload.protocol_type = htons(IP_PROTOCOL); //correspondiente a ip
    arp_payload.hard_size = 6;//pq eth tiene 6 octetos
    arp_payload.protocol_length = 4;
    arp_payload.opcode = htons(ARP_REQUEST); //1 request; 2 reply
    eth_getaddr(iface, arp_payload.mac_sender); //guardamos en mac_send la mac de la interfaz abierta
    memcpy(arp_payload.ip_sender, src,
           IPv4_ADDR_SIZE); //hastq que no implementemos la capa ip dejamos esto a 0
    memcpy(arp_payload.mac_target, UNKNOW_MAC, MAC_ADDR_SIZE); //En c la mejor forma de copiar arrays por ser
    memcpy(arp_payload.ip_target, destino, IPv4_ADDR_SIZE); //punteros es con memcpy
    //enviamos en broadcast un arp request

    if (eth_send(iface, MAC_BCAST_ADDR, ARP_TYPE, (unsigned char *) &arp_payload, sizeof(arp_payload)) ==
        -1) {
        return -2; //si no se ha podido enviar retornamos -2
    }
    printf("Enviado arp request\n");

    unsigned char buffer[sizeof(arp_message_t)];
    arp_message_t *arp_message = NULL;
    int ecoARP = 0;

    timerms_reset(&timer,
                  timeout); //arrancamos el timer//arrancamos el timer para enviar un arp a los 2 segundos sin respuesta

    //escuchamos a la respuesta mientras que el timer siga vivo
    while (1) {


        //si han pasado 2 segundos y no hemos recibido respuesta mandamos otra vez
        if (timerms_left(&timer) == 0 && ecoARP == 0) {
            eth_send(iface, MAC_BCAST_ADDR, ARP_TYPE, (unsigned char *) &arp_payload, sizeof(arp_message_t));
            ecoARP = 1;
            printf("Enviado eco arp request\n");
            timerms_reset(&timer, ecotimeout);
        }

        //solo recibimos si el mensaje es del tipo arp
        int buffer_len = eth_recv(iface, mac, ARP_TYPE, buffer, sizeof(arp_message_t),
                                  timerms_left(&timer));

        if (buffer_len == -1) {  
            printf("Se Produjo un fallo al enviar el ARP request\n");           
            return buffer_len;
        } else if ((buffer_len == 0 && ecoARP == 1)) {

            printf("Time out del ARP request\n");           
            return buffer_len;
          
}
        

        if (buffer_len < sizeof(arp_message_t)) {
            continue;
        }
        //eth_recv nos devuelve del tipo undefined char, asi que convertimos antes de copiar
        arp_message = (arp_message_t *) buffer;

        //comprobamos que proviene de la ip que buscamos y ademas es arp reply
        if (ntohs(arp_message->opcode) == ARP_REPLY && memcmp(arp_message->ip_sender, destino, IPv4_ADDR_SIZE) == 0) {

            memcpy(mac, arp_message->mac_sender, MAC_ADDR_SIZE);
            printf("ARP reply recibido\n");
            return 1;
        }

    }

}
