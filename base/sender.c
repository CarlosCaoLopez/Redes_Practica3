#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "getip.h"
#include "sender.h"
#include "loging.h"

#define BUFFER_LEN 128

/** Variables globales que exportar en el fichero de cabecera para el manejo de señales */
//uint8_t socket_io_pending;
//uint8_t terminate;


/**
 * @brief   Maneja las señales que recibe el emisor
 *
 * Maneja las señales de SIGIO, SIGINT y SIGTERM que puede recibir el emisor durante su ejecución.
 *  - SIGIO: tuvo lugar un evento de I/O en el socket del emisor.
 *  - SIGINT, SIGTERM: terminar la ejecución del programa segura.
 * Establece los valores de las variables globales socket_io_pending y terminate apropiadamente.
 *
 *  @param signum   Número de señal recibida.
 */
//static void signal_handler(int signum) {
//    switch (signum) {
//        case SIGIO:
//            socket_io_pending++;    /* Aumentar en 1 el número de eventos de entrada/salida pendientes */
//            break;
//        case SIGINT:
//        case SIGTERM:
//            terminate = 1;          /* Marca que el programa debe terminar */
//            break;
//	case SIGCHLD:
//	    while(wait(NULL) > 0);    
//        default:
//            break;
//    }
//
//}
//

/**
 * @brief   Crea un emisor.
 *
 * Crea un emisor nuevo con un nuevo socket, le asigna un puerto y 
 * lo marca como pasivo para poder escuchar conexiones.
 * Si el parámetro log no es NULL, crea también un archivo de log para guardar un registro de actividad.
 *
 * @param domain    Dominio de comunicación. 
 * @param type      Tipo de protocolo usado para el socket.
 * @param protocol  Protocolo particular a usar en el socket. Normalmente solo existe
 *                  un protocolo para la combinación dominio-tipo dada, en cuyo caso se
 *                  puede especificar con un 0.
 * @param port      Número de puerto en el que escuchar (en orden de host).
 * @param backlog   Longitud máxima de la cola de conexiones pendientes.
 * @param logfile   Nombre del archivo en el que guardar el registro de actividad.
 *
 * @return  Servidor que guarda toda la información relevante sobre sí mismo con la que
 *          fue creado, y con un socket pasivo abierto en el cual está listo para escuchar 
 *          y aceptar conexiones entrantes desde cualquier IP y del dominio y por puerto 
 *          especificados.
 */
Sender create_sender(int domain, int type, int protocol, uint16_t own_port, uint16_t remote_port, char* remote_address) {
    Sender sender;
    char buffer[BUFFER_LEN] = {0};

    memset(&sender, 0, sizeof(Sender));     /* Inicializar los campos a 0 */

    sender = (Sender) {
        .domain = domain,
        .type = type,
        .protocol = protocol,
        .own_port = own_port,
        .remote_port = remote_port,
       // .backlog = backlog,
        .own_address.sin_family = domain,
        .own_address.sin_port = htons(own_port),
        .own_address.sin_addr.s_addr = htonl(INADDR_ANY),
        .remote_address.sin_family = domain,
        .remote_address.sin_port = htons(remote_port),
        
           
 };

    inet_pton(domain, remote_address, &sender.remote_address.sin_addr); /* Inicializamos la direccion remota */ 
    sender.remote_ip = (char *) calloc(strlen(remote_address) + 1, sizeof(char)); /* Reservamos memoria para guardar en formato trxtual la ip a la que vaos a enviar el mensaje */ 
    strcpy(sender.remote_ip, remote_address);
    /* Abrimos el log para escritura.
     * Si no se puede abrir, avisamos y seguimos, ya que no es un error crítico. */
/*    if (logfile) {
        if ( (sender.log = fopen(logfile, "w")) == NULL)
            perror("No se pudo crear el log del emisor");
    }        
    log_printf("Inicializando emisor...\n");*/

    /* Guardar el nombre del equipo en el que se ejecuta el emisor.
     * No produce error crítico, por lo que no hay que salir */
    if (gethostname(buffer, BUFFER_LEN)) {
        perror("No se pudo obtener el nombre de host del emisor");
    } else {
        sender.hostname = (char *) calloc(strlen(buffer) + 1, sizeof(char));
        strcpy(sender.hostname, buffer);
    }

    /* Guardar la IP externa del emisor.
     * Tampoco supone un error crítico. */
    if (!getip(buffer, BUFFER_LEN)) {
        perror("No se pudo obtener la IP externa del emisor");
    } else {
        sender.ip = (char *) calloc(strlen(buffer) + 1, sizeof(char));
        strcpy(sender.ip, buffer);
    }

    /* Crear el socket del emisor */
    if ( (sender.socket = socket(domain, type, protocol)) < 0) {
        fail("No se pudo crear el socket");
    }

    /* Asignar IPs a las que escuchar y número de puerto por el que atender peticiones (bind) */
    if (bind(sender.socket, (struct sockaddr *) &sender.own_address, sizeof(struct sockaddr_in)) < 0) {
        fail("No se pudo asignar dirección IP");
    }

    /* Configurar el emisor para enviarse a sí mismo un SIGIO cuando se produzca actividad en el socket, para evitar bloqueos esperando por conexiones */
/*    if (fcntl(sender.socket, F_SETFL, O_ASYNC | O_NONBLOCK) < 0) {
        log_printf(ANSI_COLOR_RED "Error al configurar el envío de SIGIO en el socket.\n" ANSI_COLOR_RESET);
        fail("No se pudo configurar el envío de SIGIO en el socket");
    }
    if (fcntl(sender.socket, F_SETOWN, getpid()) < 0) {
        log_printf(ANSI_COLOR_RED "Error al configurar el autoenvío de señales en el socket.\n" ANSI_COLOR_RESET);
        fail("No se pudo configurar el autoenvío de señales en el socket");
    
   }

    if(signal(SIGCHLD, signal_handler) == SIG_ERR){
    	
        log_printf(ANSI_COLOR_RED "Error al establecer el manejo de la señal SIGCHLD.\n" ANSI_COLOR_RESET);
    }

    if (signal(SIGIO, signal_handler) == SIG_ERR) {
        log_printf(ANSI_COLOR_RED "Error al establecer el manejo de la señal SIGIO.\n" ANSI_COLOR_RESET);
        fail("No se pudo establecer el manejo de la señal SIGIO");
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        log_printf(ANSI_COLOR_RED "Error al establecer el manejo de la señal SIGINT.\n" ANSI_COLOR_RESET);
        fail("No se pudo establecer el manejo de la señal SIGINT");
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        log_printf(ANSI_COLOR_RED "Error al establecer el manejo de la señal SIGTERM.\n" ANSI_COLOR_RESET);
        fail("No se pudo establecer el manejo de la señal SIGTERM");
    }
*/

    printf("Emisor creado con éxito y listo para recibir/enviar.\n"
            "Hostname: %s; IP: %s; Puerto: %d; IP receptor: %s; Puerto receptor: %d\n\n", sender.hostname, sender.ip, sender.own_port, sender.remote_ip, sender.remote_port);

    return sender;
}


/**
 * @brief   Escucha conexiones de clientes.
 *
 * Si no hay ninguna conexión pendiente, la función retorna y en client queda guardado un cliente
 * con todos sus campos a 0, excepto el socket, que tiene a -1.
 * En caso de que sí haya una conexión pendiente, se acepta, se informa de ella y se crea una nueva
 * estructura en la que guardar la información del cliente conectado, y un nuevo socket
 * conectado al cliente para atender sus peticiones.
 * Esta función no es responsable de liberar el cliente referenciado si este ya estuviese
 * iniciado, por lo que de ser así se debe llamar a close_client antes de invocar a esta función.
 * 
 * @param sender    Servidor que poner a escuchar conexiones. Debe tener un socket
 *                  asociado marcado como pasivo.
 * @param client    Dirección en la que guardar la información del cliente conectado.
 *                  Guarda en el campo socket del cliente el nuevo socket conectado al cliente.
 */
//void listen_for_connection(Sender sender, Client* client) {
//    char ipname[INET_ADDRSTRLEN];
//    socklen_t address_length = sizeof(struct sockaddr_in);
//
//    memset(client, 0, sizeof(Client));  /* Inicializar los campos del cliente a 0. No se liberan campos */
//
//    /* Aceptar la conexión del cliente en el socket pasivo del emisor */
//    if ( (client->socket = accept(sender.socket, (struct sockaddr *) &(client->address), &address_length)) < 0) {
//        if (errno == EAGAIN || errno == EWOULDBLOCK) {  /* Hemos marcado al socket con O_NONBLOCK; no hay conexiones pendientes, así que lo registramos y salimos */
//            socket_io_pending = 0;
//            return;
//        }
//        log_printf(ANSI_COLOR_RED "Error al aceptar una conexión.\n" ANSI_COLOR_RESET);
//        fail("No se pudo aceptar la conexión");
//    }
//
//
//    /* Rellenar los campos del cliente con la información relevante para el emisor.
//     * Todos los campos que se desconozcan se dejarán con el valor por defecto que deja 
//     * la función close_client */
//    client->domain = client->address.sin_family;
//    client->type = sender.type; /* Asumimos que usan el mismo tipo de protocolo de comunicación */
//    client->protocol = sender.protocol;
//    /* Obtener el nombre de la dirección IP del cliente y guardarlo */
//    inet_ntop(client->domain, &(client->address.sin_addr), ipname, INET_ADDRSTRLEN);
//    client->ip = (char *) calloc(strlen(ipname) + 1, sizeof(char));
//    strcpy(client->ip, ipname);
//    client->sender_ip = (char *) calloc(strlen(sender.ip) + 1, sizeof(char));
//    strcpy(client->sender_ip, sender.ip);
//    client->port = ntohs(client->address.sin_port);
//    client->sender_port = sender.port;
//
//    /* Informar de la conexión */
//    printf("Cliente conectado desde %s:%u.\n", client->ip, client->port);
//    log_printf("Cliente conectado desde %s:%u.\n", client->ip, client->port);
//
//   // socket_io_pending--;    /* Una conexión ya manejada */
//
//    return;
//}
//

/**
 * @brief   Cierra el emisor.
 *
 * Cierra el socket asociado al emisor y libera toda la memoria
 * reservada para el emisor.
 *
 * @param sender    Servidor a cerrar.
 */
void close_sender(Sender* sender) {
    /* Cerrar el socket del emisor */
    if (sender->socket >= 0) {
        if (close(sender->socket)) {
            fail("No se pudo cerrar el socket del emisor");
        } 
    }

    if (sender->hostname) free(sender->hostname);
    if (sender->ip) free(sender->ip);
    if (sender->remote_ip) free(sender->remote_ip);

    /* Limpiar la estructura poniendo todos los campos a 0 */
    memset(sender, 0, sizeof(Sender));
    sender->socket = -1;    /* Poner un socket no válido para que se sepa que no se puede usar ni volver a cerrar */
    
    return;
}
