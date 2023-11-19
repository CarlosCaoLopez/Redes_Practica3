#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/**
 * Estructura que contiene toda la información relevante del
 * receiver, el socket por el que se comunica con el emisor
 * y la dirección del emisor con el que se está comunicando.
 */
typedef struct {
    int socket;         /* Socket asociado al receiver por el que recibir información de un tercero */
    int domain;         /* Dominio de comunicación. Especifica la familia de protocolos que se usan para la comunicación */
    int type;           /* Tipo de protocolo usado para el socket */
    int protocol;       /* Protocolo particular usado en el socket */
    char* sender_ip;    /* IP del emisor que envía la información (en formato textual) */
    uint16_t receiver_port;      /* Puerto por el que recibe información el receptor(en orden de host) */
    uint16_t sender_port;   /* Puerto usado por el emisor para enviar datos (en orden de host) */
    struct sockaddr_in receiver_address;       /* Estructura con el dominio de comunicación e IP y puerto por los que se comunica el receiver */
    struct sockaddr_in sender_address;  /* Estructura con el dominio de comunicación e IP y puerto del emisor que envió la información */
} Receiver;


/**
 * @brief   Crea un receiver.
 *
 * Crea un receiver nuevo con un nuevo socket, y guarda en él la información necesaria
 * sobre el servidor para posteriormente poder conectarse con él.
 *
 * @param domain        Dominio de comunicación. 
 * @param type          Tipo de protocolo usado para el socket.
 * @param protocol      Protocolo particular a usar en el socket. Normalmente solo existe
 *                      un protocolo para la combinación dominio-tipo dada, en cuyo caso se
 *                      puede especificar con un 0.
 * @param receiver_port   Número de puerto en el que escucha el receiver (en orden de host).
 *
 * @return  Receivere que guarda toda la información relevante sobre sí mismo con la que
 *          fue creado, y con un socket abierto
 */

Receiver create_receiver(int domain, int type, int protocol, uint16_t receiver_port);


/**
 * @brief   Cierra el receivere.
 *
 * Cierra el socket asociado al receivere y libera toda la memoria
 * reservada para el receivere.
 *
 * @param receiver    Receivere a cerrar.
 */

void close_receiver(Receiver* receiver); 


#endif  /* CLIENT_H */
