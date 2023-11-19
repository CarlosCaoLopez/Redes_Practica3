#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "receiver.h"
#include "loging.h"


#define MAX_BYTES_RECV 128



/**
 * @brief   Crea un receiver.
 *
 * Crea un receiver nuevo con un nuevo socket, y guarda en él la información necesaria.
 *
 * @param domain        Dominio de comunicación. 
 * @param type          Tipo de protocolo usado para el socket.
 * @param protocol      Protocolo particular a usar en el socket. Normalmente solo existe
 *                      un protocolo para la combinación dominio-tipo dada, en cuyo caso se
 *                      puede especificar con un 0.
 * @param receiver_port Número de puerto en el que escucha el receiver.
 *
 * @return  Receiver que guarda toda la información relevante sobre sí mismo con la que
 *          fue creado, y con un socket abierto en el cual está listo para recibir información.
 */
Receiver create_receiver(int domain, int type, int protocol, uint16_t receiver_port) {
    Receiver receiver;
    

    memset(&receiver, 0, sizeof(Receiver));     /* Inicializar los campos a 0 */

    receiver = (Receiver){
        .domain = domain,
        .type = type,
        .protocol = protocol,
        .receiver_port = receiver_port,
        .receiver_address.sin_family = domain,
        .receiver_address.sin_port = htons(receiver_port),
        .receiver_address.sin_addr = INADDR_ANY,
    };

    
    /*Reservamos memoria para la IP del emsisor */
    receiver.sender_ip = (char *) calloc(INET_ADDRSTRLEN, sizeof(char));

    /* Crear el socket del receiver */
    if ( (receiver.socket = socket(domain, type, protocol)) < 0) fail("No se pudo crear el socket");
    
    /* Asignar IPs a las que escuchar y número de puerto por el que atender peticiones (bind) */
    if (bind(receiver.socket, (struct sockaddr *) &receiver.receiver_address, sizeof(struct sockaddr_in)) < 0) {
        fail("No se pudo asignar dirección IP");
    }

    return receiver;
}




/**
 * @brief   Cierra el receivere.
 *
 * Cierra el socket asociado al receiver y libera toda la memoria
 * reservada para el receivere.
 *
 * @param receiver    Receiver a cerrar.
 */

void close_receiver(Receiver* receiver) {
    /* Cerrar el socket del receivere */
    if (receiver->socket != -1) {
        if (close(receiver->socket)) fail("No se pudo cerrar el socket del receivere");
    }

    //if (receiver->hostname) free(receiver->hostname);
    //if (receiver->ip) free(receiver->ip);
    if (receiver->sender_ip) free(receiver->sender_ip);

    /* Limpiar la estructura poniendo todos los campos a 0 */
    memset(receiver, 0, sizeof(Receiver));
    receiver->socket = -1;    /* Poner un socket no válido para que se sepa que no se puede usar ni volver a cerrar */

    return;
}
