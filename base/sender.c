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
        .own_address.sin_family = domain,
        .own_address.sin_port = htons(own_port),
        .own_address.sin_addr.s_addr = htonl(INADDR_ANY),
        .remote_address.sin_family = domain,
        .remote_address.sin_port = htons(remote_port),
        
           
 };

    inet_pton(domain, remote_address, &sender.remote_address.sin_addr); /* Inicializamos la direccion remota */ 
    sender.remote_ip = (char *) calloc(strlen(remote_address) + 1, sizeof(char)); /* Reservamos memoria para guardar en formato trxtual la ip a la que vaos a enviar el mensaje */ 
    strcpy(sender.remote_ip, remote_address);

    if (gethostname(buffer, BUFFER_LEN)) {
        perror("No se pudo obtener el nombre de host del emisor");
    } else {
        sender.hostname = (char *) calloc(strlen(buffer) + 1, sizeof(char));
        strcpy(sender.hostname, buffer);
    }

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

    printf("Emisor creado con éxito y listo para recibir/enviar.\n"
            "Hostname: %s; IP: %s; Puerto: %d; IP receptor: %s; Puerto receptor: %d\n\n", sender.hostname, sender.ip, sender.own_port, sender.remote_ip, sender.remote_port);

    return sender;
}

/**
 * @brief   Cierra el emisor.
 *
 * Cierra el socket asociado al emisor y libera toda la memoria
 * reservada para el emisor.
 *
 * @param sender    Sender a cerrar.
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
