###############
#- VARIABLES -#
###############

# Compilador y opciones de compilación
CC = gcc
CFLAGS = -Wall -Wpedantic -Wno-missing-braces -g

# Carpeta con las cabeceras
HEADERS_DIR = base

# Opción de compilación que indica dónde están los archivos .h
INCLUDES = -I$(HEADERS_DIR)

# Archivos de cabecera para generar dependencias
HEADERS = $(HEADERS_DIR)/sender.h $(HEADERS_DIR)/receiver.h $(HEADERS_DIR)/getip.h $(HEADERS_DIR)/loging.h

# Fuentes con las funcionalidades básicas de cliente y servidor (implementaciones de los .h)
COMMON = $(HEADERS:.h=.c)

# Servidor y cliente básicos
BASIC = basic

## Servidor básico
### Fuentes
SRC_BASIC_SERVER_SPECIFIC = $(BASIC)/emisor.c
SRC_BASIC_SERVER = $(SRC_BASIC_SERVER_SPECIFIC) $(COMMON)

### Objetos
OBJ_BASIC_SERVER = $(SRC_BASIC_SERVER:.c=.o)

### Ejecutable o archivo de salida
OUT_BASIC_SERVER = $(BASIC)/emisor

## Cliente básico
### Fuentes
SRC_BASIC_CLIENT_SPECIFIC = $(BASIC)/emisor.c
SRC_BASIC_CLIENT = $(SRC_BASIC_CLIENT_SPECIFIC) $(COMMON)

### Objetos
OBJ_BASIC_CLIENT = $(SRC_BASIC_CLIENT:.c=.o)

### Ejecutable o archivo de salida
OUT_BASIC_CLIENT = $(BASIC)/receiver

# Servidor y cliente de mayúsculas
MAYUS = mayus

## Servidor de mayúsculas
### Fuentes
SRC_MAYUS_SERVER_SPECIFIC = $(MAYUS)/servidorUDP.c
SRC_MAYUS_SERVER = $(SRC_MAYUS_SERVER_SPECIFIC) $(COMMON)

### Objetos
OBJ_MAYUS_SERVER = $(SRC_MAYUS_SERVER:.c=.o)

### Ejecutable o archivo de salida
OUT_MAYUS_SERVER = $(MAYUS)/servidorUDP

## Cliente básico
### Fuentes
SRC_MAYUS_CLIENT_SPECIFIC = $(MAYUS)/clienteUDP.c
SRC_MAYUS_CLIENT = $(SRC_MAYUS_CLIENT_SPECIFIC) $(COMMON)

### Objetos
OBJ_MAYUS_CLIENT = $(SRC_MAYUS_CLIENT:.c=.o)

### Ejecutable o archivo de salida
OUT_MAYUS_CLIENT = $(MAYUS)/clienteUDP

# Listamos todos los archivos de salida
OUT = $(OUT_BASIC_SERVER) $(OUT_BASIC_CLIENT) $(OUT_MAYUS_SERVER) $(OUT_MAYUS_CLIENT)


############
#- REGLAS -#
############

# Regla por defecto: compila todos los ejecutables
all: $(OUT)

# Compila ambos servidores
server: $(OUT_BASIC_SERVER) $(OUT_MAYUS_SERVER)

# Compila ambos clientes
client: $(OUT_BASIC_CLIENT) $(OUT_MAYUS_CLIENT)

# Compila servidor y cliente básicos
basic: $(OUT_BASIC_SERVER) $(OUT_BASIC_CLIENT)

# Compila servidor y cliente de mayúsculas
mayus: $(OUT_MAYUS_SERVER) $(OUT_MAYUS_CLIENT)

# Genera el ejecutable del servidor básico, dependencia de sus objetos.
$(OUT_BASIC_SERVER): $(OBJ_BASIC_SERVER)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BASIC_SERVER)

# Genera el ejecutable del cliente básico, dependencia de sus objetos.
$(OUT_BASIC_CLIENT): $(OBJ_BASIC_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BASIC_CLIENT) 

# Genera el ejecutable del servidor de mayúsculas, dependencia de sus objetos.
$(OUT_MAYUS_SERVER): $(OBJ_MAYUS_SERVER)
	$(CC) $(CFLAGS) -o $@ $(OBJ_MAYUS_SERVER)

# Genera el ejecutable del cliente de mayúsculas, dependencia de sus objetos.
$(OUT_MAYUS_CLIENT): $(OBJ_MAYUS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJ_MAYUS_CLIENT)

# Genera los ficheros objeto .o necesarios, dependencia de sus respectivos .c y todas las cabeceras.
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES)

# Borra todos los resultados de la compilación (prerrequisito: cleanobj)
clean: cleanobj
	rm -f $(OUT)

# Borra todos los ficheros objeto del directorio actual y todos sus subdirectorios
cleanobj:
	find . -name "*.o" -delete


