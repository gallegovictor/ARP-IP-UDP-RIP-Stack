#include "ripv2_route_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <timerms.h>
#include <arpa/inet.h>


/* ipv4_route_t * ipv4_route_create
 * ( ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw );
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta IPv4 con los parámetros especificados:
 *   dirección de subred, máscara, nombre de interfaz y dirección de siguiente
 *   salto.
 *
 *   Esta función reserva memoria para la estructura creada. Debe utilizar la
 *   función 'ipv4_route_free()' para liberar dicha memoria.
 *
 * PARÁMETROS:
 *   'subnet': Dirección IPv4 de la subred destino de la nueva ruta.
 *     'mask': Máscara de la subred destino de la nueva ruta.
 *    'iface': Nombre del interfaz empleado para llegar a la subred destino de
 *             la nueva  ruta.
 *             Debe tener una longitud máxima de 'IFACE_NAME_LENGTH' caracteres.
 *       'gw': Dirección IPv4 del encaminador empleado para llegar a la subred
 *             destino de la nueva ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la ruta creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la ruta.
 */
entrada_rip_t *ripv2_route_create
        (ipv4_addr_t subnet, ipv4_addr_t mask, ipv4_addr_t next_hop, uint32_t metric) {

    entrada_rip_t *route = (entrada_rip_t *) malloc(sizeof(entrada_rip_t));

    if ((route != NULL) &&
        (subnet != NULL) && (mask != NULL) && (next_hop != NULL)) {
        route->family_directions = DEFAULT_FAMILY_DIRECTION;
        route->route_label = UNUSED;
        memcpy(route->subnet, subnet, IPv4_ADDR_SIZE);
        memcpy(route->mask, mask, IPv4_ADDR_SIZE);
        memcpy(route->gw, next_hop, IPv4_ADDR_SIZE);
        route->metric = metric;

    }

    return route;
}


/*Esta funcion recibe como paraemtro un unsigned char y cuenta
 *todos los bits a 1 en este. Para hacer esto el for no para hasta
 *que mask este a 0s, y con cada itineracion hace un shift a la izquierda
 *de todos los bytes. Luego sumamos si el ultimo byte es 1
 */

int ripv2_switch_lookup(unsigned char mask) {
    int c;
    for (c = 0; mask; mask >>= 1) {
        c += mask & 1;
    }
    return c;
}


/* int ipv4_route_lookup ( ipv4_route_t * route, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función indica si la dirección IPv4 especificada pertence a la
 *   subred indicada. En ese caso devuelve la longitud de la máscara de la
 *   subred.
 *
 * ************************************************************************
 * * Esta función NO está implementada, debe implementarla usted para que *
 * * funcione correctamente la función 'ipv4_route_table_lookup()'.       *
 * ************************************************************************
 *
 * PARÁMETROS:
 *   'route': Ruta a la subred que se quiere comprobar.
 *    'addr': Dirección IPv4 destino.
 *
 * VALOR DEVUELTO:
 *   Si la dirección IPv4 pertenece a la subred de la ruta especificada, debe
 *   devolver un número positivo que indica la longitud del prefijo de
 *   subred. Esto es, el número de bits a uno de la máscara de subred.
 *   La función devuelve '-1' si la dirección IPv4 no pertenece a la subred
 *   apuntada por la ruta especificada.
 */
int ripv2_route_lookup(entrada_rip_t *route, ipv4_addr_t addr) {
    int prefix_length = 0;

    //recorremos los 4 octetos
    for (int i = 0; i < 4; i++) {
        //si en algun momento la ip a buscar con la mascara aplicada no es
        //igual a la ip de la subred paramos y retornamos -1
        if ((addr[i] & route->mask[i]) != route->subnet[i]) {
            return -1;
        }
        //contamos a la vez el numero de 1s cada octeto de la mascara
        prefix_length += ripv2_switch_lookup(route->mask[i]);
    }
    return prefix_length;
}

/* void ipv4_route_print ( ipv4_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime la ruta especificada por la salida estándar.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea imprimir.
 */
void ripv2_route_print(entrada_rip_t *route) {
    if (route != NULL) {
        char subnet_str[IPv4_STR_MAX_LENGTH];
        ipv4_addr_str(route->subnet, subnet_str);
        char mask_str[IPv4_STR_MAX_LENGTH];
        ipv4_addr_str(route->mask, mask_str);
        char gw_str[IPv4_STR_MAX_LENGTH];
        ipv4_addr_str(route->gw, gw_str);
        int metric = route->metric;

        printf("%s/%s via %s %i", subnet_str, mask_str, gw_str, metric);
    }
}


/* void ipv4_route_free ( ipv4_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la ruta especificada, que
 *   ha sido creada con 'ipv4_route_create()'.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea liberar.
 */
void ripv2_route_free(entrada_rip_t *route) {
    if (route != NULL) {
        free(route);
    }
}

/* ipv4_route_t* ipv4_route_read ( char* filename, int linenum, char * line )
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta IPv4 a partir de la línea del fichero
 *   de la tabla de rutas especificada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero de la tabla de rutas
 *    'linenum': Número de línea del fichero de la tabal de rutas.
 *       'line': Línea del fichero de la tabla de rutas a procesar.
 *
 * VALOR DEVUELTO:
 *   La ruta leída, o NULL si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función imprime un mensaje de error y devuelve NULL si se ha
 *   producido algún error al leer la ruta.
 */
entrada_rip_t *ripv2_route_read(char *filename, int linenum, char *line) {
    entrada_rip_t *route = NULL;

    char subnet_str[256];
    char mask_str[256];
    char gw_str[256];
    char str_metric[4];

    /* Parse line: Format "<subnet> <mask> <iface> <gw>\n" */
    int params = sscanf(line, "%s %s %s %s\n",
                        subnet_str, mask_str, gw_str, str_metric);
    if (params != 4) {
        fprintf(stderr, "%s:%d: Invalid IPv4 Route format: '%s' (%d params)\n",
                filename, linenum, line, params);
        fprintf(stderr,
                "%s:%d: Format must be: <subnet> <mask> <gw> <metric>\n",
                filename, linenum);
        return NULL;
    }

    /* Parse IPv4 route subnet address */
    ipv4_addr_t subnet;
    int err = ipv4_str_addr(subnet_str, subnet);
    if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <subnet> value: '%s'\n",
                filename, linenum, subnet_str);
        return NULL;
    }

    /* Parse IPv4 route subnet mask */
    ipv4_addr_t mask;
    err = ipv4_str_addr(mask_str, mask);
    if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <mask> value: '%s'\n",
                filename, linenum, mask_str);
        return NULL;
    }

    /* Parse IPv4 route gateway */
    ipv4_addr_t gateway;
    err = ipv4_str_addr(gw_str, gateway);
    if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <gw> value: '%s'\n",
                filename, linenum, gw_str);
        return NULL;
    }

    int metric;
    metric = atoi(str_metric);
    if (metric == 0) {
        fprintf(stderr, "Metric incorrecta, 0 no es valido\n");
        return NULL;
    }

    /* Create new route with parsed parameters */
    route = ripv2_route_create(subnet, mask, gateway, metric);
    if (route == NULL) {
        fprintf(stderr, "%s:%d: Error creating the new route\n",
                filename, linenum);
    }

    return route;
}


/* void ipv4_route_output ( ipv4_route_t * route, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la ruta IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *      'route': Ruta a imprimir.
 *     'header': '0' para imprimir una línea con la cabecera de la ruta.
 *        'out': Salida por la que imprimir la ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si la ruta se ha impreso correctamente.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 */
int ripv2_route_output(entrada_rip_t *route, int header, FILE *out) {
    int err;

    if (header == 0) {
        err = fprintf(out, "# SubnetAddr  \tSubnetMask  \tGateway  \tMetric\n");
        if (err < 0) {
            return -1;
        }
    }

    char subnet_str[IPv4_STR_MAX_LENGTH];
    char mask_str[IPv4_STR_MAX_LENGTH];
    char gw_str[IPv4_STR_MAX_LENGTH];
    char metric_str[4];

    if (route != NULL) {
        ipv4_addr_str(route->subnet, subnet_str);
        ipv4_addr_str(route->mask, mask_str);
        ipv4_addr_str(route->gw, gw_str);
        sprintf(metric_str, "%d", route->metric);

        err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\n",
                      subnet_str, mask_str, gw_str, metric_str);
        if (err < 0) {
            return -1;
        }
    }

    return 0;
}

/* ipv4_route_table_t * ipv4_route_table_create();
 *
 * DESCRIPCIÓN:
 *   Esta función crea una tabla de rutas IPv4 vacía.
 *
 *   Esta función reserva memoria para la tabla de rutas creada, para
 *   liberarla es necesario llamar a la función 'ipv4_route_table_free()'.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la tabla de rutas creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la tabla de rutas.
 */
rip_route_table_t *ripv2_route_table_create() {
    rip_route_table_t *table;

    table = (rip_route_table_t *) malloc(sizeof(struct rip_route_table));
    if (table != NULL) {
        int i;
        for (i = 0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            table->routes[i] = NULL;
        }
    }

    return table;
}


/* int ipv4_route_table_add ( ipv4_route_table_t * table,
 *                            ipv4_route_t * route );
 * DESCRIPCIÓN:
 *   Esta función añade la ruta especificada en la primera posición libre de
 *   la tabla de rutas.
 *
 * PARÁMETROS:
 *   'table': Tabla donde añadir la ruta especificada.
 *   'route': Ruta a añadir en la tabla de rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el indice de la posición [0,IPv4_ROUTE_TABLE_SIZE-1]
 *   donde se ha añadido la ruta especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no ha sido posible añadir la ruta
 *   especificada.
 */



int ripv2_route_table_add(rip_route_table_t *table, entrada_rip_t *route) {
    int route_index = -1;

    if (table != NULL) {
        /* Find an empty place in the route table */
        int i;
        for (i = 0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            if (table->routes[i] == NULL) {
                table->routes[i] = route;
                route_index = i;
                break;
            }
        }
    }

    return route_index;
}


entrada_rip_t *ripv2_route_table_remove(rip_route_table_t *table, int index) {
    entrada_rip_t *removed_rip_entry = NULL;
    if ((table != NULL) && (index >= 0) && (index < RIP_ROUTE_TABLE_SIZE)) {
        removed_rip_entry = table->routes[index];
        table->routes[index] = NULL;
    }
    return removed_rip_entry;
}

//probablemente ni se utilice
entrada_rip_t *ripv2_route_table_lookup(rip_route_table_t *table, entrada_rip_t *entrada) {

    entrada_rip_t *best_route = NULL;
    int best_route_prefix = -1;

    if (table != NULL) {
        for (int i = 0; i<RIP_ROUTE_TABLE_SIZE; i++) {
            entrada_rip_t *rip_entry = table->routes[i];
            if (rip_entry != NULL) {
                int route_i_lookup = ripv2_route_lookup(rip_entry, entrada->subnet);
                if (route_i_lookup > best_route_prefix) {
                    best_route_prefix = route_i_lookup;
                    best_route = rip_entry;
                }
            }
        }
    }
    return best_route;

}

entrada_rip_t *ripv2_route_table_get(rip_route_table_t *table, int index) {

    entrada_rip_t *entry = NULL;
    if ((table != NULL) && (index >= 0) && (index < RIP_ROUTE_TABLE_SIZE)) {
        entry = table->routes[index];

    }
    return entry;
}

int ripv2_route_table_find(rip_route_table_t *table, entrada_rip_t *entry_to_find) {

    entrada_rip_t *entry = NULL;
    int route_index = -2;
    if ((table != NULL)) {
        route_index = -1;
        for (int i = 0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            entry = table->routes[i];
            if (entry != NULL && memcmp(entry_to_find->subnet, entry->subnet, sizeof(ipv4_addr_t)) == 0 &&
                    memcmp(entry_to_find->mask, entry->mask, sizeof(ipv4_addr_t)) == 0) {
                route_index = i;
                break;
            }
        }
    }
    return route_index;

}

void ripv2_route_table_free(rip_route_table_t *table) {

    if (table != NULL) {
        for (int i = 0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            entrada_rip_t *entry = table->routes[i];
            if (entry != NULL) {
                table->routes[i] = NULL;
                ripv2_route_free(entry);
            }
        }
        free(table);
    }
}

int ripv2_route_table_read(char *filename, rip_route_table_t *table) {
    int read_routes = 0;

    FILE *route_file = fopen(filename, "r");
    if (route_file == NULL) {
        fprintf(stderr, "Error opening input rip Routes file \"%s\": %s.\n",
                filename, strerror(errno));
        return -1;
    }

    int linenum = 0;
    char line_buf[1024];
    int err = 0;

    while ((!feof(route_file)) && (err == 0)) {

        linenum++;


        /* Read next line of file */
        char *line = fgets(line_buf, 1024, route_file);
        if (line == NULL) {
            break;
        }
;
        /* If this line is empty or a comment, just ignore it */
        if ((line_buf[0] == '\n') || (line_buf[0] == '#')) {
            err = 0;
            continue;
        }

        /* Parse route from line */
        entrada_rip_t *new_entry = ripv2_route_read(filename, linenum, line);
        if (new_entry == NULL) {
            err = -1;
            break;
        }

        /* Add new route to Route Table */
        if (table != NULL) {
            err = ripv2_route_table_add(table, new_entry);
            if (err >= 0) {
                err = 0;
                read_routes++;
            }
        }
    } /* while() */

    if (err == -1) {
        read_routes = -1;
    }

    /* Close IP Route Table file */
    fclose(route_file);

    return read_routes;


}

int ripv2_route_table_output(rip_route_table_t *table, FILE *out) {
    int err;

    if (table != NULL) {
        for (int i = 0; i<RIP_ROUTE_TABLE_SIZE; i++) {
            entrada_rip_t *entry = table->routes[i];
            if (entry != NULL) {
                err = ripv2_route_output(entry, i, out);
                if (err == -1) {
                    return -1;
                }
            }
        }
    }
    return 0;
}

int ripv2_route_output_with_timers(entrada_rip_t *route, int header, FILE *out, timerms_t timer) {
    int err;

    if (header == 0) {
        err = fprintf(out, "# SubnetAddr  \tSubnetMask  \tGateway  \tMetric  \tTimer\n");
        if (err < 0) {
            return -1;
        }
    }

    char subnet_str[IPv4_STR_MAX_LENGTH];
    char mask_str[IPv4_STR_MAX_LENGTH];
    char gw_str[IPv4_STR_MAX_LENGTH];
    char metric_str[4];

    if (route != NULL) {
        ipv4_addr_str(route->subnet, subnet_str);
        ipv4_addr_str(route->mask, mask_str);
        ipv4_addr_str(route->gw, gw_str);
        sprintf(metric_str, "%d", route->metric);

        err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\t%li\n",
                      subnet_str, mask_str, gw_str, metric_str, timerms_left(&timer));
        if (err < 0) {
            return -1;
        }
    }

    return 0;
}

int ripv2_route_table_output_with_timers(rip_route_table_t *table, timers_t *timers) {
    int err;

    if (table != NULL) {
        for (int i = 0; i<RIP_ROUTE_TABLE_SIZE; i++) {
            entrada_rip_t *entry = table->routes[i];
            if (entry != NULL) {
                err = ripv2_route_output_with_timers(entry, i, stdout, timers->list_timers[i]);
                if (err == -1) {
                    return -1;
                }
            }
        }
    }
    return 0;
}




int ripv2_route_table_write(rip_route_table_t *table, char *filename) {
    int num_entradas = 0;
    FILE *entrada_file = fopen(filename, "w");
    if (entrada_file == NULL) {
        fprintf(stderr, "Error opening input file \"%s\": %s.\n",
                filename, strerror(errno));
        return -1;
    }
    fprintf(entrada_file, "# %s\n", filename);
    fprintf(entrada_file, "#\n");
    if (table != NULL) {
        num_entradas = ripv2_route_table_output(table, entrada_file);
        if (num_entradas == -1) {
            fprintf(stderr, "Error writing file \"%s\": %s.\n",
                    filename, strerror(errno));
            return -1;
        }
    }
    fclose(entrada_file);
    return num_entradas;
}

void ripv2_route_table_print(rip_route_table_t *table) {
    if (table != NULL) {
        ripv2_route_table_output(table, stdout);
    }
}



int ripv2_route_table_remove_expired(rip_route_table_t *table, timers_t *table_timers) {

    int removed = 0;

    if (table != NULL) {
        for (int i = 0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            if (table->routes[i] != NULL) {
                if (timerms_left( &(table_timers->list_timers[i])) == 0 ||
                        ripv2_is_infinite(table->routes[i]->metric)) {
                    ripv2_route_table_remove(table, i);
                    removed++;
                }
            }
        }
    }
    return removed;
}

int ripv2_timeleft(rip_route_table_t *table, timers_t table_timers){

    long int min_time = -1;
    for(int i = 0; i<RIP_ROUTE_TABLE_SIZE; i++){
        if(table->routes[i] != NULL){
            if(min_time == -1 || timerms_left( &(table_timers.list_timers[i]) ) < min_time ) {
                min_time = timerms_left( &(table_timers.list_timers[i]) );
            }
        }
    }
    return min_time;
}


void ripv2_inicialize_timers(int last_index, timers_t *table_timers) {

    if(last_index > 0 && last_index <= RIP_ROUTE_TABLE_SIZE) {
        for (int i = 0; i < RIP_ROUTE_TABLE_SIZE; i++) {
            if (i < last_index) {
                timerms_reset( &(table_timers->list_timers[i]), RIP_ROUTE_DEFAULT_TIME);
            }
        }
    }

}

void ripv2_print_timers(rip_route_table_t* table, timers_t *timers) {

    for(int i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
        entrada_rip_t *entry = table->routes[i];
        if(entry != NULL) {
            printf("%li\n", timerms_left( &(timers->list_timers[i]) ));
        }
    }

}


int ripv2_is_infinite(uint32_t metric) {
    return (metric < 0 || metric > 15);
}
