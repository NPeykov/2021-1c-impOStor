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

//char *puntoMontaje;
char *dirMetadata;
char *dirFiles;
char *dirBitacora;
//char *dirBlocks;
t_log* mongoLogger;
int socket_cliente;
//int blocks;
//int block_size;
pthread_t hilo_sabotaje;
t_list* archAbiertos;
int socket_mongo_store, socket_cliente;
char* puerto;

char *superbloque;
void *bitarrayComoVoid;
t_bitarray *bitmap;
uint32_t block_size;
uint32_t blocks;

int tamanio_de_bloque;
int cantidad_de_bloques;

t_bitarray* crear_bitmap(char *ubicacion, int cant_bloques);

void liberar_bloque(t_bitarray* bitmap, int bloque);
void ocupar_bloque(t_bitarray* bitmap, int bloque);
int obtener_bloque_libre(t_bitarray* bitmap) ;
void actualizar_posicion(m_movimiento_tripulante *tripulante);


#endif /* BITMAP_H_ */
