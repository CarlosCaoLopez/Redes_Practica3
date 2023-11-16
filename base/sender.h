#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "receiver.h"

/**
 * Estructura que contiene toda la información relevante 
 * del emisor y por el que envia mensajes.
 */
typedef struct {
    int socket;     /* Socket asociado al servidor en el que escuchar conexiones o atender peticiones */
    int domain;     /* Dominio de comunicación. Especifica la familia de protocolos que se usan para la comunicación */
    int type;       /* Tipo de protocolo usado para el socket */
    int protocol;   /* Protocolo particular usado en el socket */
    uint16_t own_port;  /* Puerto en el que el emisor envia mensajes (en orden de host) */
    uint16_t remote_port;  /* Puerto en el que el receptor recibirá datos (en orden de host) */
    char* hostname; /* Nombre del equipo en el que está ejecutándose el emisor (vestigios del anterior proyecto) */
    char* ip;       /* IP externa del emisor (en formato textual) */
    char* remote_ip; /* IP del receptor en formato textual */
    struct sockaddr_in own_address;  /* Estructura con el dominio de comunicación, IPs a las que atender (dirección propia)*/
    struct sockaddr_in remote_address;  /* Estructura con el dominio de comunicación, IPs a las que atender (dirección del emisor)*/

} Sender;


Sender create_sender(int domain, int type, int protocol, uint16_t own_port, uint16_t remote_port,char* remote_address);

void close_sender(Sender* sender); 


#endif  /* SERVER_H */
