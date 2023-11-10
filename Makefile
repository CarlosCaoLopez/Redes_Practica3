###############
#- VARIABLES -#
###############

# Compilador y opciones de compilación
CC = gcc
CFLAGS = -Wall -Wpedantic -g

# Carpeta con las cabeceras
HEADERS_DIR = base

# Opción de compilación que indica dónde están los archivos .h
INCLUDES = -I$(HEADERS_DIR)

# Archivos de cabecera para generar dependencias
HEADERS = $(HEADERS_DIR)/sender.h $(HEADERS_DIR)/reciver.h $(HEADERS_DIR)/getip.h $(HEADERS_DIR)/loging.h

# Fuentes con las funcionalidades básicas de reciver y sender (implementaciones de los .h)
COMMON = $(HEADERS:.h=.c)

# Emisor y receptor básicos
BASE = basic

## Servidor básico
### Fuentes
SRC_BASE_SENDER_SPECIFIC = $(BASE)/emisor.c
SRC_BASE_SENDER = $(SRC_BASE_SENDER_SPECIFIC) $(COMMON)

### Objetos
OBJ_BASE_SENDER = $(SRC_BASE_SENDER:.c=.o)

### Ejecutable o archivo de salida
OUT_BASE_SENDER = $(BASE)/sender

## Cliente básico
### Fuentes
SRC_BASE_RECIVER_SPECIFIC = $(BASE)/receptor.c
SRC_BASE_RECIVER = $(SRC_BASE_RECIVER_SPECIFIC) $(COMMON)

### Objetos
OBJ_BASE_RECIVER = $(SRC_BASE_RECIVER:.c=.o)

### Ejecutable o archivo de salida
OUT_BASE_RECIVER = $(BASE)/reciver



# Listamos todos los archivos de salida
OUT = $(OUT_BASE_SENDER) $(OUT_BASE_RECIVER) 


# Regla por defecto: compila todos los ejecutables
all: $(OUT)

# Compila ambos servidores
sender: $(OUT_BASE_SENDER) 

# Compila ambos clientes
reciver: $(OUT_BASE_RECIVER) 

# Compila servidor y cliente básicos
basic: $(OUT_BASE_SENDER) $(OUT_BASE_RECIVER)

# Genera el ejecutable del servidor básico, dependencia de sus objetos.
$(OUT_BASE_SENDER): $(OBJ_BASE_SENDER)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BASE_SENDER)

# Genera el ejecutable del cliente básico, dependencia de sus objetos.
$(OUT_BASE_RECIVER): $(OBJ_BASE_RECIVER)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BASE_RECIVER) 

# Genera los ficheros objeto .o necesarios, dependencia de sus respectivos .c y todas las cabeceras.
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES)

# Borra todos los resultados de la compilación (prerrequisito: cleanobj)
clean: cleanobj
	rm -f $(OUT)

# Borra todos los ficheros objeto del directorio actual y todos sus subdirectorios
cleanobj:
	find . -name "*.o" -delete
