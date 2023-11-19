#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "sender.h"
#include "loging.h"

//#define MESSAGE_SIZE 128
#define DEFAULT_PORT 8000
#define DEFAULT_LOG "log"
#define FILENAME_LEN 128
#define MAX_BYTES_RECV 2056

struct arguments {
    int argc;
    char** argv;
    char* remote_address;
    uint16_t* own_port;
    uint16_t* remote_port;
    char* input_file_name;
};


static void process_args(struct arguments args);


static void print_help(char* exe_name);

void handle_data(Sender sender, char* input_file_name);


int main(int argc, char** argv) {
    Sender sender;
    uint16_t own_port;
    uint16_t remote_port;
    char input_file_name[FILENAME_LEN];
    char remote_address[INET_ADDRSTRLEN];


    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .own_port = &own_port,
        .remote_port = &remote_port,
        .remote_address = remote_address,
        .input_file_name = input_file_name
    };

    set_colors();

    process_args(args);

   
    printf("Ejecutando emisor con parámetro: PORT=%u.\n\n", own_port);
    sender = create_sender(AF_INET, SOCK_DGRAM, 0, own_port, remote_port, remote_address); /*Pasamos los argumentos a la funcion de crear el sender*/


    handle_data(sender, input_file_name);

    printf("\nCerrando el emisor y saliendo...\n");
    close_sender(&sender);
    exit(EXIT_SUCCESS);
}


void handle_data(Sender sender, char* input_file_name){
    ssize_t sent_bytes = 0, recv_bytes = 0;
    FILE *fp_input, *fp_output;
    char recv_buffer[MAX_BYTES_RECV];
    size_t buffer_size; /* Necesitamos una variable con el tamaño del buffer para getline */
    char* send_buffer;  /* Buffer para guardar las líneas del archivo a enviar. Como se usa getline, tiene que asignarse dinamicamente */
    socklen_t address_size = sizeof(struct sockaddr_in);

    /* Apertura de los archivos */
    if ( !(fp_input = fopen(input_file_name, "r")) ) fail("Error en la apertura del archivo de lectura");
	
    /* Enviamos el nombre del archivo */
    printf("Se procede a enviar el archivo: %s\n", input_file_name);

    if ( (sent_bytes = sendto(sender.socket, input_file_name, strlen(input_file_name) + 1, 0, (struct sockaddr *) &sender.remote_address, address_size)) < 0) fail("No se pudo enviar el mensaje");

    /* Esperamos a recibir la linea */
    if( (recv_bytes = recvfrom(sender.socket, recv_buffer, MAX_BYTES_RECV, 0, (struct sockaddr *) &(sender.remote_address), &address_size)) < 0) fail("No se pudo recibir el mensaje");

    /* Recibido el nombre del archivo en mayúsculas */
    /* Abrimos en modo escritura el archivo */    
    if ( !(fp_output = fopen(recv_buffer, "w")) ) fail("Error en la apertura del archivo de escritura");

    /* Procesamiento y envio del archivo */
    /* Inicializamos el buffer de envío, en el que leeremos del archivo con getline */
    buffer_size = MAX_BYTES_RECV;
    send_buffer = (char *) calloc(buffer_size, sizeof(char));
    while (!feof(fp_input)) {
        /* Leemos hasta que lo que devuelve getline es EOF, cerramos la conexión en ese caso */
        if(getline(&send_buffer, &buffer_size, fp_input) == EOF){ /* Escaneamos la linea hasta el final del archivo */
            //shutdown(client.socket, SHUT_RD);   /* Le decimos al servidor que pare de recibir */
            continue;
        }
        if ( (sent_bytes = sendto(sender.socket, send_buffer, strlen(send_buffer) + 1, 0, (struct sockaddr *) &sender.remote_address, address_size)) < 0) fail("No se pudo enviar el mensaje");

        /*Esperamos a recibir la linea*/
        if( (recv_bytes = recvfrom(sender.socket, recv_buffer, MAX_BYTES_RECV, 0, (struct sockaddr *) &(sender.remote_address), &address_size)) < 0) fail("No se pudo recibir el mensaje");

        fprintf(fp_output, "%s", recv_buffer);
    }
    
    /* Cerramos los archivos al salir */
    if (fclose(fp_input)) fail("No se pudo cerrar el archivo de lectura");
    if (fclose(fp_output)) fail("No se pudo cerrar el archivo de escritura");

    if (send_buffer) free(send_buffer);

    return;
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
    uint8_t set_file = 0, set_ip = 0, set_port = 0;   /* Flags para saber si se setearon el fichero a convertir, la IP y puerto */
    /* Inicializar los valores de puerto a sus valores por defecto */
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
                case 'p':   /* Puerto del propio*/
                    if (++i < args.argc) {
                        *args.own_port = atoi(args.argv[i]);
                        if (*args.own_port < 0) {
                            fprintf(stderr, "El valor de puerto especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'r':   /* Puerto remoto */
                    if (++i < args.argc) {
                        *args.remote_port = atoi(args.argv[i]);
                        set_port = 1;
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
                        set_ip = 1;
                    } else {
                        fprintf(stderr, "Dirección remota no especificada tras la opción '-a'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                    case 'f':   /* Fichero */
                    if (++i < args.argc) {
                        strncpy(args.input_file_name, args.argv[i], FILENAME_LEN);
                        set_file = 1;
                    } else {
                        fprintf(stderr, "Fichero no especificado tras la opción '-f'\n\n");
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
        if (!set_file || !set_ip || !set_port) { 
        fprintf(stderr, "%s%s%s\n", (set_file ? "" : "No se especificó fichero para convertir a mayúsculas.\n"),
                                    (set_ip ? "" : "No se especificó la IP del servidor al que conectarse.\n"), 
                                    (set_port ? "" : "No se especificó el puerto del servidor al que conectarse.\n"));
        print_help(args.argv[0]);
        exit(EXIT_FAILURE);
    }
}
