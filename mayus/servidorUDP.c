#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <wctype.h>


#include "receiver.h"
#include "loging.h"

#define MAX_BYTES_RECV 2056
#define DEFAULT_PORT 8500

/**
 * Estructura de datos para pasar a la función process_args.
 * Debe contener siempre los campos int argc, char** argv, provenientes de main,
 * y luego una cantidad variable de punteros a las variables que se quieran inicializar
 * a partir de la entrada del programa, a saber el puerto del receptor.
 */
struct arguments {
    int argc;
    char** argv;
    uint16_t* receiver_port;
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
 * @param receiver    Receiver que recibe los datos, previamente conectado al emisor.
 */
void handle_data(Receiver receiver);

/**
 * @brief   Transforma una string a mayúsculas
 *
 * Transforma la string source a mayúsculas, utilizando para ello wstrings para poder transformar
 * caracteres especiales que ocupen más de un byte. Por tanto, permite pasar a mayúsculas strings en
 * el idioma definido en locale.
 *
 * @param source    String fuente a transformar en mayúsculas.
 *
 * @return  String dinámicamente alojada (por tanto, debe liberarse con un free) que contiene los mismos
 *          caractereres que source pero en mayúsculas.
 */
static char* toupper_string(const char* source);



int main(int argc, char** argv){
	Receiver receiver;
    uint16_t receiver_port;
    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .receiver_port = &receiver_port
    };

    set_colors();
	
    process_args(args);

	receiver = create_receiver(AF_INET, SOCK_DGRAM, 0, receiver_port);
    
	handle_data(receiver);
	
	close_receiver(&receiver);
    printf("Saliendo\n");
    exit(EXIT_SUCCESS);
}


void handle_data(Receiver receiver){
    char* output;
    char input[MAX_BYTES_RECV];
    ssize_t recv_bytes, sent_bytes;
    int flag=0;
    socklen_t address_size = sizeof(struct sockaddr_in);

    
    

    while (1) {
        if ( (recv_bytes = recvfrom(receiver.socket, input, MAX_BYTES_RECV, 0, (struct sockaddr *) &(receiver.sender_address), &address_size)) < 0) fail("Error al recibir la línea de texto");
        if (!recv_bytes) return;    /* Se recibió una orden de cerrar la conexión */
        printf("Linea recibida:\t%s\n", input);
        /* Guardamos la ip del clienteUDP en formato textual*/
        inet_ntop(receiver.domain, &receiver.sender_address.sin_addr, receiver.sender_ip, INET_ADDRSTRLEN);
        
        if(flag == 0){
            printf("\nManejando al cliente %s:%u...\n", receiver.sender_ip, ntohs(receiver.sender_address.sin_port));
            flag=1;
        }

        output = toupper_string(input); 
        printf("Linea a ser enviada:\t %s \n", output);
        if ( (sent_bytes = sendto(receiver.socket, output, strlen(output) + 1, 0, (struct sockaddr *) &receiver.sender_address, address_size)) < 0) {
            
            fail("Error al enviar la línea de texto al cliente");

        }

        if (output) free(output);
    }
}

/**
 * @brief   Transforma una string a mayúsculas
 *
 * Transforma la string source a mayúsculas, utilizando para ello wstrings para poder transformar
 * caracteres especiales que ocupen más de un byte. Por tanto, permite pasar a mayúsculas strings en
 * el idioma definido en locale.
 *
 * @param source    String fuente a transformar en mayúsculas.
 *
 * @return  String dinámicamente alojada (por tanto, debe liberarse con un free) que contiene los mismos
 *          caractereres que source pero en mayúsculas.
 */
static char* toupper_string(const char* source) {
    wchar_t* wide_source;
    wchar_t* wide_destiny;
    ssize_t wide_size, size;
    char* destiny;
    int i;

    wide_size = mbstowcs(NULL, source, 0); /* Calcular el número de wchar_t que ocupa el string source */
    /* Alojar espacio para wide_source y wide_destiny */
    wide_source = (wchar_t *) calloc(wide_size + 1, sizeof(wchar_t));
    wide_destiny = (wchar_t *) calloc(wide_size + 1, sizeof(wchar_t));

    /* Transformar la fuente en un wstring */
    mbstowcs(wide_source, source, wide_size + 1);
    /* Transformar wide_source a mayúsculas y guardarlo en wide_destiny */
    for (i = 0; wide_source[i]; i++) {
        wide_destiny[i] = towupper(wide_source[i]);
    }

    /* Transoformar de vuelta a un string normal */
    size = wcstombs(NULL, wide_destiny, 0); /* Calcular el número de char que ocupa el wstring wide_destiny */
    destiny = (char *) calloc(size + 1, sizeof(char));
    wcstombs(destiny, wide_destiny, size + 1);

    if (wide_source) free(wide_source);
    if (wide_destiny) free(wide_destiny);

    return destiny;
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


    /* Inicializar los valores de puerto y backlog a sus valores por defecto */
    *args.receiver_port = DEFAULT_PORT;
 
    for (i = 1; i < args.argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = args.argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            /* Manejar las opciones largas */
            if (current_arg[1] == '-') { /* Opción larga */
                if (!strcmp(current_arg, "--port")) current_arg = "-p";
                else if (!strcmp(current_arg, "--help")) current_arg = "-h";
            } 
            switch(current_arg[1]) {
                  case 'p':   /* Puerto */
                    if (++i < args.argc) {
                        *args.receiver_port = atoi(args.argv[i]);
                        if (*args.receiver_port < 0) {
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
                case 'h':   /* Ayuda */
                    print_help(args.argv[0]);
                    exit(EXIT_SUCCESS);
                default:
                    fprintf(stderr, "Opción '%s' desconocida\n\n", current_arg);
                    print_help(args.argv[0]);
                    exit(EXIT_FAILURE);
            }
        } else if (i == 1) {    /* Se especificó el puerto como primer argumento */
            *args.receiver_port = atoi(args.argv[i]);
            if (*args.receiver_port < 0) {
                fprintf(stderr, "El valor de puerto especificado (%s) no es válido.\n\n", args.argv[i]);
                print_help(args.argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }


}
