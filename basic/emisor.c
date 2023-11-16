#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "sender.h"
#include "loging.h"

#define MESSAGE_SIZE 128
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 16
#define DEFAULT_LOG "log"

/**
 * Estructura de datos para pasar a la función process_args.
 * Debe contener siempre los campos int argc, char** argv, provenientes de main,
 * y luego una cantidad variable de punteros a las variables que se quieran inicializar
 * a partir de la entrada del programa.
 */
struct arguments {
    int argc;
    char** argv;
    uint16_t* own_port;
    uint16_t* remote_port;
    int* backlog;
    char* remote_address;
};

/**
 * @brief   Procesa los argumentos del main
 *
 * Procesa los argumentos proporcionados al programa por línea de comandos,
 * e inicializa las variables del programa necesarias acorde a estos.
 *
 * @param args  Estructura con los argumentos del programa y punteros a las
 *              variables que necesitan inicialización.
 */
static void process_args(struct arguments args);

/**
 * @brief   Imprime la ayuda del programa
 *
 * @param exe_name  Nombre del ejecutable (argv[0])
 */
static void print_help(char* exe_name);

/**
 * @brief   Maneja la conexión desde el lado del servidor.
 *
 * Envía un mensaje al receivere diciéndole que se aceptó su conexión
 * y con la información del servidor.
 *
 * @param sender    Servidor que maneja la conexión.
 * @param receiver    Receivere conectado que solicita el servicio.
 */
void handle_data(Sender sender);



int main(int argc, char** argv) {
    Sender sender;
   // Receiver receiver;
    uint16_t own_port;
    uint16_t remote_port;
    int backlog;
    char remote_address[INET_ADDRSTRLEN];


    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .own_port = &own_port,
        .remote_port = &remote_port,
        .backlog = &backlog,
        .remote_address = remote_address,
    };

    set_colors();

    process_args(args);

    printf("Puerto remoto y direc antes crear sender: %u; %s\n", remote_port, remote_address);
    printf("Ejecutando emisor con parámetros: PORT=%u, BACKLOG=%d.\n\n", own_port, backlog);
    sender = create_sender(AF_INET, SOCK_DGRAM, 0, own_port, remote_port, remote_address, backlog); /*Pasamos los argumentos a la funcion de crear el sender*/


    handle_data(sender);

//    while (!terminate) {
//        if (!socket_io_pending) pause();    /* Pausamos la ejecución hasta que se reciba una señal de I/O o de terminación */
//       // listen_for_connection(sender, &receiver);
//        if (receiver.socket == -1) continue;  /* Falsa alarma, no había conexiones pendientes o se recibió una señal de terminación */
//
//        handle_connection(sender, receiver);
//
//        printf("\nCerrando la conexión del receivere %s:%u.\n\n", receiver.ip, receiver.port);
//        close_receiver(&receiver);  /* Ya hemos gestionado al receivere, podemos olvidarnos de él */
//    }
//
    printf("\nCerrando el emisor y saliendo...\n");
    close_sender(&sender);
    exit(EXIT_SUCCESS);
}


void handle_data(Sender sender) {
    char message[MESSAGE_SIZE] = {0};
    ssize_t sent_bytes;
    socklen_t length = sizeof(struct sockaddr_in);
    
    printf("\nEnviando mensaje al receptor %s:%u...\n", sender.remote_ip, sender.remote_port);

    snprintf(message, MESSAGE_SIZE, "Mensaje enviado desde %s en %s:%u. Hola Mundo!\n", sender.hostname, sender.ip, sender.own_port);

    /* Enviar el mensaje al receiver */
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
    printf("\nSi no se especifica alguno de los argumentos, el servidor se ejecutará con su valor por defecto, a saber: DEFAULT_PORT=%u; DEFAULT_BACKLOG=%d, DEFAULT_LOG=%s\n", DEFAULT_PORT, DEFAULT_BACKLOG, DEFAULT_LOG);
    printf("\nSi se especifica varias veces un argumento, o se especifican las opciones \"--log\" y \"--no-log\" a la vez, el comportamiento está indefinido.\n");
}


static void process_args(struct arguments args) {
    int i;
    char* current_arg;

    /* Inicializar los valores de puerto y backlog a sus valores por defecto */
    *args.own_port = DEFAULT_PORT;
    *args.backlog = DEFAULT_BACKLOG;
   // *args.logfile = DEFAULT_LOG;

    for (i = 1; i < args.argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = args.argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            /* Manejar las opciones largas */
            if (current_arg[1] == '-') { /* Opción larga */
                if (!strcmp(current_arg, "--own_port")) current_arg = "-p";
                else if( (!strcmp(current_arg, "--remote_port"))) current_arg = "-r";
                else if (!strcmp(current_arg, "--backlog")) current_arg = "-b";
                else if (!strcmp(current_arg, "--address")) current_arg = "-a";
               // else if (!strcmp(current_arg, "--no-log")) current_arg = "-n";
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
                case 'b':   /* Backlog */
                    if (++i < args.argc) {
                        *args.backlog = atoi(args.argv[i]);
                        if (*args.backlog < 0) {
                            fprintf(stderr, "El valor de backlog especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stderr, "Tamaño del backlog no especificado tras la opción '-b'.\n\n");
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
               // case 'n':   /* No-log */
               //     *args.logfile = NULL;
               //     break;
               // case 'h':   /* Ayuda */
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
