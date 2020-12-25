
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <timerms.h>

#include "ipv4_route_table.h"
#include "ipv4_config.h"
#include "udp.h"


udp_layer_t *udp_open(uint16_t src_port, char *ip_config, char *route_config) {
    /*Comprobamos que los archivos estén correctos
    if (file_config == NULL) {
        fprintf(stderr, "Error al abrir el archivo. Error %s\n", strerror(ERROR));
    }
    if (ip_config == NULL) {
        fprintf(stderr, "Error al abrir el archivo. Error %s\n", strerror(ERROR));
    }
    if (route_config == NULL) {
        fprintf(stderr, "Error al abrir el archivo. Error %s\n", strerror(ERROR));
    }
    Reservamos memoria para el layer de UDP que retornaremos al final*/
    udp_layer_t *udp_layer = malloc(sizeof(udp_layer_t));
    //Una vez rellenado el penúltimo campo, pasamos a rellenar la ip_layer_t con el método ipv4_open*/
    udp_layer->ipv4_layer = ipv4_open(ip_config, route_config);
    /*Y devolvemos nuesta udp_layer_t*/
    udp_layer->source_port = src_port;
    return udp_layer;
}


int udp_send(udp_layer_t *layer, ipv4_addr_t dst, uint16_t port_out, unsigned char payload[], int payload_len) {

    if (layer == NULL) {
        printf("Hubo un fallo al inicializar el UDP layer\n");
        return -1;
    }

    if (payload_len <= 0) {
        printf("Payload de UDP no valido\n");
        return -1;
    }

    //Rellenamos paquete
    udp_packet_t udp_frame;
    udp_frame.src_port = htons(layer->source_port);
    udp_frame.dst_port = htons(port_out);
    udp_frame.checksum = 0x000;
    int udp_frame_len = UDP_HEADER_LEN + payload_len;
    udp_frame.len = htons(udp_frame_len);
    memcpy(udp_frame.payload, (unsigned char *) payload, payload_len);


    //bonito asi parece ser
    int bytes_send = ipv4_send(layer->ipv4_layer, dst, UDP_PROTOCOL, (unsigned char *) &udp_frame, udp_frame_len);

    if (bytes_send == -1) {
        //ya manda el warning el ipv4_send, no hace falta hacer printf
        return -1;
    }
    return (bytes_send - UDP_HEADER_LEN);
}

int
udp_recv(udp_layer_t *layer, long int timeout, ipv4_addr_t sender, uint16_t *port, unsigned char *buffer,
         int buffer_len) {

    //check_parametros_correctos()
    if (layer == NULL) {
        printf("Error al inicializar UDP layer. \n");
        return -1;
    }

    if (buffer_len <= 0) {
        printf("Payload de UDP erróneo. \n");
        return -1;
    }

    //crear_variable_timer()
    timerms_t timer_udp;
    timerms_reset(&timer_udp, timeout);

    udp_packet_t *udp_frame = NULL;
    int frame_len;
    int udp_buffer_len = buffer_len + UDP_HEADER_LEN;
    unsigned char udp_buffer[udp_buffer_len];

    while (1) {
        //escuchar_puerto()
        long int time_left = timerms_left(&timer_udp);

        frame_len = ipv4_recv(layer->ipv4_layer, UDP_PROTOCOL, udp_buffer, sender, udp_buffer_len, time_left);

        if (frame_len == -1) {
            printf("Error al recibir el datagrama.\n");
            return -1;
        }

        udp_frame = (udp_packet_t *) udp_buffer;

        if (ntohs(udp_frame->dst_port) == layer->source_port) {
            break;
        }

    }

    uint16_t recv_src_port = ntohs(udp_frame->src_port); //paso necesario, no deja hacerlo directamente en el memcpy
    memcpy(port, &recv_src_port, sizeof(uint16_t));
    /*Si el payload recibido es menor que el tamaño del buffer,
    solo copiamos los datos necesarios al buffer. Por otro lado
    si nuestro buffer no es suficientemente grande para guardar
    todo el payload, se perderian datos, pero de comprobar eso se
    encargan las capas superiores, aqui solo que no de segmentatioFault
    */
    int payload_len = frame_len - UDP_HEADER_LEN;
    if (buffer_len > payload_len) {
        buffer_len = payload_len;
    }

    memcpy(buffer, udp_frame->payload, buffer_len);

    return payload_len;

}

void udp_close(udp_layer_t *my_layer) {

    ipv4_close(my_layer->ipv4_layer);
    free(my_layer);

}
