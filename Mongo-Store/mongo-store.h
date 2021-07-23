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
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "bitmap.h"
//Semaforos
sem_t contador_sabotaje; //iniciar en 1
sem_t dar_orden_sabotaje;
sem_t semaforo_bitmap;
sem_t semaforo_bitacora;

typedef struct {
	t_list *bloques;
} t_disco_logico;

typedef struct {
	int inicio;
	int fin;
	int id_bloque;
	int espacio;
	int posicion_para_escribir;
} t_bloque;

char *puntoMontaje;
char *dirMetadata;
char *dirFiles;
char *dirBitacora;
char *dirBlocks;
t_disco_logico *disco_logico;
t_config* mongoConfig;
t_log* mongoLogger;
int socket_cliente;
int blocks;
int block_size;
int numero_sabotaje=0;
m_movimiento_tripulante * tripulanteEnMovimiento;
pthread_t hilo_sabotaje;
t_list* archAbiertos;
int socket_mongo_store, socket_cliente;
char* puerto;
char *block_mmap;



void crearEstructuraFileSystem();
void crearEstructurasBloques();
t_disco_logico* crearEstructuraDiscoLogico();
void enviar_aviso_sabotaje_a_discordiador();
t_bloque* buscar_ultimo_bloque_del_tripulante(char*);
int cantidad_bloques_a_ocupar(char* texto);
void actualizar_posicion(m_movimiento_tripulante *tripulante);
void rutina(int n);
void *gestionarCliente(int cliente);
void gestionarSabotaje();
void generar_oxigeno(int);
void consumir_oxigeno(int);
void generar_comida(int);
void consumir_comida(int);
void generar_basura(int);
void descartar_basura(int);
void (*signal(int sig, void (*func)(int)))(int) ;
void enviar_aviso_sabotaje_a_discordiador(void *data);
char* siguiente_posicion_sabotaje();
typedef enum{
	SUPERBLOQUE,
	FILES
}sabotaje_code;
#endif
