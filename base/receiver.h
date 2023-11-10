#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/**
 * Estructura que contiene toda la información relevante del
 * receivere, el socket por el que se comunica con el servidor
 * y la dirección del servidor con el que se está comunicando.
 */
typedef struct {
    int socket;         /* Socket asociado al receivere por el que conectarse a un servidor y comunicarse con él */
    int domain;         /* Dominio de comunicación. Especifica la familia de protocolos que se usan para la comunicación */
    int type;           /* Tipo de protocolo usado para el socket */
    int protocol;       /* Protocolo particular usado en el socket */
    //char* hostname;     /* Nombre del equipo en el que está ejecutándose el receivere */
    //char* ip;           /* IP externa del receivere (en formato textual) */
    char* sender_ip;    /* IP del servidor al que conectarse (en formato textual) */
    uint16_t port;      /* Puerto por el que envía conexiones el receivere (en orden de host, pensado para uso del servidor) */
    uint16_t sender_port;   /* Puerto en el que el servidor escucha peticiones (en orden de host) */
    struct sockaddr_in address;       /* Estructura con el dominio de comunicación e IP y puerto por los que se comunica el receivere (pensada para uso del servidor) */
    struct sockaddr_in sender_address;  /* Estructura con el dominio de comunicación e IP y puerto del servidor al que conectarse */
} Receiver;


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

Receiver create_receiver(int domain, int type, int protocol, char* sender_ip, uint16_t sender_port);


/**
 * @brief   Conecta el receivere con el servidor.
 *
 * Crea una conexión con el servidor especificado durante la creación del receivere
 * a través de su socket.
 *
 * @param receiver    Receivere a conectar.
 */
//void connect_to_sender(Receiver receiver);

/**
 * @brief   Conecta el receivere con el servidor.
 *
 * Envia mensaje especificado a traves del socket
 *
 * @param receiver    Receivere a conectar.
 * @param message   Mensaje a enviar
 */

void send_message(Receiver receiver, char *message);


/**
 * @brief   Conecta el receivere con el servidor.
 *
 * Recibe mensaje  
 *
 * @param receiver    Receivere a conectar.
 * @param message   Mensaje a enviar
 */

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
