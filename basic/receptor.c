#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "receiver.h"
#include "loging.h"

#define MAX_BYTES_RECV 128

/**
 * Estructura de datos para pasar a la función process_args.
 * Debe contener siempre los campos int argc, char** argv, provenientes de main,
 * y luego una cantidad variable de punteros a las variables que se quieran inicializar
 * a partir de la entrada del programa.
 */
struct arguments {
    int argc;
    char** argv;
    char* sender_ip;
    uint16_t* sender_port;
};

/**
 * @brief   Procesa los argumentos del main.
 *
 * Procesa los argumentos proporcionados al programa por línea de comandos,
 * e inicializa las variables del programa necesarias acorde a estos.
 *
 * @param args  Estructura con los argumentos del programa y punteros a las
 *              variables que necesitan inicialización.
 */
static void process_args(struct arguments args);

/**
 * @brief Imprime la ayuda del programa.
 *
 * @param exe_name  Nombre del ejecutable (argv[0]).
 */
static void print_help(char* exe_name);


/**
 * @brief   Maneja los datos que envía el emisor.
 *
 * Recibe mensajes del emisor hasta que este corta la conexión.
 *
 * @param receiver    Receivere que recibe los datos, previamente conectado al emisor.
 */
void handle_data(Receiver receiver);



int main(int argc, char** argv){
	Receiver receiver;
	char sender_ip[INET_ADDRSTRLEN];
    uint16_t sender_port;
    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .sender_ip = sender_ip,
        .sender_port = &sender_port
    };

    set_colors();
	
    process_args(args);

	receiver = create_receiver(AF_INET, SOCK_STREAM, 0, sender_ip, sender_port);

    //connect_to_sender(receiver); 
    
	handle_data(receiver);
	
	close_receiver(&receiver);
    printf("Saliendo\n");
    exit(EXIT_SUCCESS);
}


void handle_data(Receiver receiver){
    ssize_t recv_bytes;
    socklen_t address_size = sizeof(struct sockaddr_in); 
    char message[MAX_BYTES_RECV];

    if ((recv_bytes = recvfrom(receiver.socket, message, MAX_BYTES_RECV, 0, (struct sockaddr *) &(receiver.sender_address), &address_size)) < 0) fail("No se pudo enviar el mensaje");
    printf("Mensaje recibido de %ld bytes con éxito al emisor %s por el puerto %d\n", recv_bytes,receiver.sender_ip, receiver.sender_port);
    return;
}

static void print_help(char* exe_name){
    /** Cabecera y modo de ejecución **/
    printf("Uso: %s [-i] <IP> [-p] <port> [-h]\n\n", exe_name);

    /** Lista de opciones de uso **/
    printf(" Opción\t\tOpción larga\t\tSignificado\n");
    printf(" -i/-I <IP>\t--ip/--IP <IP>\t\tIP del emisor al que conectarse, o \"localhost\" si el emisor se ejecuta en el mismo host que el receivere.\n");
    printf(" -p <port>\t--port <port>\t\tPuerto en el que escucha el emisor al que conectarse.\n");
    printf(" -h\t\t--help\t\t\tMostrar este texto de ayuda y salir.\n");

    /** Consideraciones adicionales **/
    printf("\nPueden especificarse los parámetros <IP> y <port> para la IP y puerto en los que escucha el emisor sin escribir las opciones '-I' ni '-p', siempre y cuando estos sean el primer y segundo parámetros que se pasan a la función, respectivamente.\n");
    printf("\nSi se especifica varias veces un argumento, el comportamiento está indefinido.\n");
}

static void process_args(struct arguments args) {
    int i;
    char* current_arg;
    uint8_t set_ip = 0, set_port = 0;   /* Flags para saber si se setearon la IP y puerto */

    for (i = 1; i < args.argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = args.argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            /* Manejar las opciones largas */
            if (current_arg[1] == '-') { /* Opción larga */
               // if (!strcmp(current_arg, "--IP") || !strcmp(current_arg, "--ip")) current_arg = "-i";
                else if (!strcmp(current_arg, "--port")) current_arg = "-p";
                else if (!strcmp(current_arg, "--help")) current_arg = "-h";
            } 
            switch(current_arg[1]) {
              //  case 'I':   /* IP */
              //  case 'i':
              //      if (++i < args.argc) {
              //          if (!strcmp(args.argv[i], "localhost")) args.argv[i] = "127.0.0.1"; /* Permitir al receivere indicar localhost como IP */
              //          strncpy(args.sender_ip, args.argv[i], INET_ADDRSTRLEN);
              //          set_ip = 1;
              //      } else {
              //          fprintf(stderr, "IP no especificada tras la opción '-i'\n\n");
              //          print_help(args.argv[0]);
              //          exit(EXIT_FAILURE);
              //      }
              //      break;
                  case 'p':   /* Puerto */
                    if (++i < args.argc) {
                        *args.sender_port = atoi(args.argv[i]); /* Aqui necesito el puerto del programa que recibe!*/
                        if (*args.sender_port < 0) {
                            fprintf(stderr, "El valor de puerto especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                        set_port = 1;
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'.\n\n");
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
        } else if (i == 1) {    /* Se especificó la IP como primer argumento */
            if (!strcmp(args.argv[i], "localhost")) args.argv[i] = "127.0.0.1"; /* Permitir al receivere indicar localhost como IP */
            strncpy(args.sender_ip, args.argv[i], INET_ADDRSTRLEN);
            set_ip = 1;
        } else if (i == 2) {    /* Se especificó el puerto como segundo argumento */
            *args.sender_port = atoi(args.argv[i]);
            if (*args.sender_port < 0) {
                fprintf(stderr, "El valor de puerto especificado (%s) no es válido.\n\n", args.argv[i]);
                print_help(args.argv[0]);
                exit(EXIT_FAILURE);
            }
            set_port = 1;
        }
    }

    if (!set_ip || !set_port) {
        fprintf(stderr, "%s%s\n",   (set_ip ? "" : "No se especificó la IP del emisor al que conectarse.\n"), 
                                    (set_port ? "" : "No se especificó el puerto del emisor al que conectarse.\n"));
        print_help(args.argv[0]);
        exit(EXIT_FAILURE);
    }
}
