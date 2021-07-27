/*
 * bitmap.h
 *
 *  Created on: 19 jul. 2021
 *      Author: utnso
 */

#ifndef BITMAP_H_
#define BITMAP_H_
/*
 * bitmap1.h
 *
 *  Created on: 19 jul. 2021
 *      Author: utnso
 */

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
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "mongo-store.h"


//Semaforos
sem_t contador_sabotaje; //iniciar en 1


t_log* mongoLogger;
int socket_cliente;
int blocks;
int block_size;
pthread_t hilo_sabotaje;
t_list* archAbiertos;
int socket_mongo_store, socket_cliente;
char* puerto;

void *superbloque;
int g_tamanio_superbloque;
char *bitarrayEnChar;
t_bitarray *bitmap;
uint32_t *g_block_size;
uint32_t *g_blocks;

uint32_t g_nuevo_blocks;
uint32_t g_nuevo_block_size;

int tamanio_de_bloque;
int cantidad_de_bloques;

t_bitarray* crear_bitmap(char *, int );
void liberar_bloque(int );
void ocupar_bloque(int );
int obtener_bloque_libre(void) ;
void actualizar_posicion(m_movimiento_tripulante *);


#endif /* BITMAP_H_ */
