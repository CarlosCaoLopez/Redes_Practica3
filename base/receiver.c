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

#define BUFFER_LEN 64


/**
 * @brief   Crea un receivere.
 *
 * Crea un receivere nuevo con un nuevo socket, y guarda en él la información necesaria
 * sobre el servidor para posteriormente poder conectarse con él.
 *
 * @param domain        Dominio de comunicación. 
 * @param type          Tipo de protocolo usado para el socket.
 * @param protocol      Protocolo particular a usar en el socket. Normalmente solo existe
 *                      un protocolo para la combinación dominio-tipo dada, en cuyo caso se
 *                      puede especificar con un 0.
 * @param sender_ip     Dirección IP del servidor al que conectarse (en formato textual).
 * @param sender_port   Número de puerto en el que escucha el servidor (en orden de host).
 *
 * @return  Receivere que guarda toda la información relevante sobre sí mismo con la que
 *          fue creado, y con un socket abierto en el cual está listo para conectarse 
 *          al servidor con la IP y puerto especificados.
 */
Receiver create_receiver(int domain, int type, int protocol, char* sender_ip, uint16_t sender_port) {
    Receiver receiver;
    char buffer[BUFFER_LEN] = {0};

    memset(&receiver, 0, sizeof(Receiver));     /* Inicializar los campos a 0 */

    receiver = (Receiver) {
        .domain = domain,
        .type = type,
        .protocol = protocol,
        .sender_port = sender_port,
        .sender_address.sin_family = domain,
        .sender_address.sin_port = htons(sender_port),
    };
    if (inet_pton(receiver.domain, sender_ip, &(receiver.sender_address.sin_addr)) != 1) {  /* La string no se pudo traducir a una IP válida */
        fprintf(stderr, "La IP especificada no es válida\n\n");
        exit(EXIT_FAILURE);
    }

    /* Guardar la IP del servidor en formato textual */
    receiver.sender_ip = (char *) calloc(strlen(sender_ip) + 1, sizeof(char));
    strcpy(receiver.sender_ip, sender_ip);

    /* Crear el socket del receivere */
    if ( (receiver.socket = socket(domain, type, protocol)) < 0) fail("No se pudo crear el socket");

    return receiver;
}



/**
 * @brief   Conecta el receivere con el servidor.
 *
 * Crea una conexión con el servidor especificado durante la creación del receivere
 * a través de su socket.
 *
 * @param receiver    Receivere a conectar.
 */

/*void connect_to_sender(Receiver receiver) {
    if (connect(receiver.socket, (struct sockaddr *) &(receiver.sender_address), sizeof(struct sockaddr_in)) < 0) fail("No se pudo conetar con el servidor");

    printf("Conectado con éxito al servidor %s por el puerto %d\n", receiver.sender_ip, receiver.sender_port);

    return;
}*/

/**
 * @brief   Envia mensaje al servidor
 * 
 */ 



/**
 * @brief   Cierra el receivere.
 *
 * Cierra el socket asociado al receivere y libera toda la memoria
 * reservada para el receivere.
 *
 * @param receiver    Receivere a cerrar.
 */

void close_receiver(Receiver* receiver) {
    /* Cerrar el socket del receivere */
    if (receiver->socket != -1) {
        if (close(receiver->socket)) fail("No se pudo cerrar el socket del receivere");
    }

    //if (receiver->hostname) free(receiver->hostname);
    if (receiver->ip) free(receiver->ip);
    if (receiver->sender_ip) free(receiver->sender_ip);

    /* Limpiar la estructura poniendo todos los campos a 0 */
    memset(receiver, 0, sizeof(Receiver));
    receiver->socket = -1;    /* Poner un socket no válido para que se sepa que no se puede usar ni volver a cerrar */

    return;
}
