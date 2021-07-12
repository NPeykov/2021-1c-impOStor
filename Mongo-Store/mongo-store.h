#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"
#include <signal.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

char *puntoMontaje;
char *dirMetadata;
char *dirFiles;
char *dirBitacora;
char *dirBlocks;
t_config* mongoConfig;
t_log* mongoLogger;
int socket_cliente;
int blocks;
int block_size;
t_bitarray *bitmap;
t_list* archAbiertos;

void crearEstructuraFileSystem();
t_bitarray* crear_bitmap(char *ubicacion, int cant_bloques);

void liberar_bloque(t_bitarray* bitmap, int bloque);
void ocupar_bloque(t_bitarray* bitmap, int bloque);
int obtener_bloque_libre(t_bitarray* bitmap) ;

void *gestionarCliente(int cliente);
void gestionarSabotaje();
void generar_oxigeno(int);
void consumir_oxigeno(int);
void generar_comida(int);
void consumir_comida(int);
void generar_basura(int);
void descartar_basura(int);
void (*signal(int sig, void (*func)(int)))(int) ;

typedef enum{
	SUPERBLOQUE,
	FILES
}sabotaje_code;
#endif
