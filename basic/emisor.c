#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "sender.h"
#include "loging.h"

#define MESSAGE_SIZE 256 
#define DEFAULT_PORT 8000
#define DEFAULT_LOG "log"

struct arguments {
    int argc;
    char** argv;
    uint16_t* own_port;
    uint16_t* remote_port;
    char* remote_address;
};


static void process_args(struct arguments args);


static void print_help(char* exe_name);

void handle_data(Sender sender);

int main(int argc, char** argv) {
    Sender sender;
    uint16_t own_port;
    uint16_t remote_port;
    char remote_address[INET_ADDRSTRLEN];


    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .own_port = &own_port,
        .remote_port = &remote_port,
        .remote_address = remote_address,
    };

    set_colors();

    process_args(args);

   
    printf("Ejecutando emisor con parámetro: PORT=%u.\n\n", own_port);
    sender = create_sender(AF_INET, SOCK_DGRAM, 0, own_port, remote_port, remote_address); /*Pasamos los argumentos a la funcion de crear el sender*/


    handle_data(sender);

    printf("\nCerrando el emisor y saliendo...\n");
    close_sender(&sender);
    exit(EXIT_SUCCESS);
}


void handle_data(Sender sender) {
    char message[MESSAGE_SIZE] = {0};
    ssize_t sent_bytes;
    socklen_t length = sizeof(struct sockaddr_in);
    
    printf("\nEnviando mensaje al receptor %s:%u...\n", sender.remote_ip, sender.remote_port);

    snprintf(message, MESSAGE_SIZE, "Mensaje enviado desde %s en %s:%u. Hola Mundo! Este mensaje tiene mas de 128 bytes. Este mensaje tiene mas de 128 bytes. Este mensaje tiene mas de 128 bytes\n", sender.hostname, sender.ip, sender.own_port);
    printf("Tamaño del mensaje: %ld\n", strlen(message)+1);

    if ( (sent_bytes = sendto(sender.socket, message, strlen(message) + 1, 0, (struct sockaddr *) &sender.remote_address, length)) < 0) fail("No se pudo enviar el mensaje");
}


static void print_help(char* exe_name){
    /** Cabecera y modo de ejecución **/
    printf("Uso: %s [[-p] <port>] [-b <backlog>] [-l <log> | --no-log] [-h]\n\n", exe_name);

    /** Lista de opciones de uso **/
    printf(" Opción\t\tOpción larga\t\tSignificado\n");
    printf(" -p <port>\t--port <port>\t\tPuerto en el que escuchará el servidor.\n");
    printf(" -b <backlog>\t--backlog <backlog>\tTamaño máximo de la cola de conexiones pendientes.\n");
    printf(" -l <log>\t--log <log>\t\tNombre del archivo en el que guardar el registro de actividad del servidor.\n");
    printf(" -n\t\t--no-log\t\tNo crear archivo de registro de actividad.\n");
    printf(" -h\t\t--help\t\t\tMostrar este texto de ayuda y salir.\n");

    /** Consideraciones adicionales **/
    printf("\nPuede especificarse el parámetro <port> para el puerto en el que escucha el servidor sin escribir la opción '-p', siempre y cuando este sea el primer parámetro que se pasa a la función.\n");
 /*   printf("\nSi no se especifica alguno de los argumentos, el servidor se ejecutará con su valor por defecto, a saber: DEFAULT_PORT=%u; DEFAULT_BACKLOG=%d, DEFAULT_LOG=%s\n", DEFAULT_PORT, DEFAULT_BACKLOG, DEFAULT_LOG);*/
    printf("\nSi se especifica varias veces un argumento, o se especifican las opciones \"--log\" y \"--no-log\" a la vez, el comportamiento está indefinido.\n");
}


static void process_args(struct arguments args) {
    int i;
    char* current_arg;

    /* Inicializar los valores de puerto y backlog a sus valores por defecto */
    *args.own_port = DEFAULT_PORT;

    for (i = 1; i < args.argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = args.argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            /* Manejar las opciones largas */
            if (current_arg[1] == '-') { /* Opción larga */
                if (!strcmp(current_arg, "--own_port")) current_arg = "-p";
                else if( (!strcmp(current_arg, "--remote_port"))) current_arg = "-r";               
                else if (!strcmp(current_arg, "--address")) current_arg = "-a";             
                else if (!strcmp(current_arg, "--help")) current_arg = "-h";
            } 
            switch(current_arg[1]) {
                case 'p':   /* Puerto del sender*/
                    if (++i < args.argc) {
                        *args.own_port = atoi(args.argv[i]);
                        if (*args.own_port < 0) {
                            fprintf(stderr, "El valor de puerto especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p1'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'r':   /* Puerto remoto */
                    if (++i < args.argc) {
                        *args.remote_port = atoi(args.argv[i]);
                        if (*args.remote_port < 0) {
                            fprintf(stderr, "El valor de puerto remoto especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p2'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'a':   /* Dirección remota */
                    if (++i < args.argc) {
                        strcpy(args.remote_address, args.argv[i]); /* Copia la ip*/
                    } else {
                        fprintf(stderr, "Dirección remota no especificada tras la opción '-a'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'h':   /* Ayuda */
                    print_help(args.argv[0]);
                    exit(EXIT_SUCCESS);
                default:
                    fprintf(stderr, "Opción '%s' desconocida\n\n", current_arg);
                    print_help(args.argv[0]);
                    exit(EXIT_FAILURE);
            }
        } else if (i == 1) {    /* Se especificó el puerto como primer argumento */
            *args.own_port = atoi(args.argv[i]);
            if (*args.own_port < 0) {
                fprintf(stderr, "El valor de puerto especificado como primer argumento (%s) no es válido.\n\n", args.argv[i]);
                print_help(args.argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
}
