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
#include <math.h>
#include "sabotaje_mongo.h"


//Semaforos
sem_t contador_sabotaje; //iniciar en 1
sem_t dar_orden_sabotaje;
//sem_t semaforo_bitmap;
sem_t semaforo_bitacora;
sem_t semaforo_para_file_oxigeno;
sem_t semaforo_para_file_comida;
sem_t semaforo_para_file_basura;

//MUTEXS
pthread_mutex_t mutex_disco_logico;
pthread_mutex_t mutex_bitmap;

typedef struct {
	void *tripulante;
	op_code accion;
} tripulante_con_su_accion;

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
char *dirSuperbloque;
t_disco_logico *disco_logico;
t_config* mongoConfig;
t_log* mongoLogger;
int socket_cliente;
//int blocks;
//int block_size;
int numero_sabotaje;
m_movimiento_tripulante * tripulanteEnMovimiento;
pthread_t hilo_sabotaje;
t_list* archAbiertos;
int socket_mongo_store, socket_cliente;
char* puerto;
char *block_mmap;
size_t block_mmap_size;


//relacionadas a escribir las acciones del tripulante
char *generarTextoAEscribir(tripulante_con_su_accion *);
void escribir_en_su_bitacora_la_accion(tripulante_con_su_accion*);

char *rutaBitacoraDelTripulante(tripulante_con_su_accion*);


//bajadas a disco
void mostrar_estado_bitarray(void);
void gestionar_bajadas_a_disco(void);
void bajar_datos_superbloque(void);

void iniciar_recursos_mongo(void);
void crearEstructuraFileSystem();
void crearblocks(char*);
void crear_estructura_filesystem();
void crearEstructurasBloques();
void crearEstructuraDiscoLogico();
void crearBitMapLogico();
void inicializar_archivo(char* , int , char);
t_bloque* buscar_ultimo_bloque_del_tripulante(char*);
int cantidad_bloques_a_ocupar(char* texto);
void copiar_datos_de_bloques(t_list*);
int ultima_posicion_escrita(int,int);


void *gestionarCliente(int cliente);
void gestionarSabotaje();
void generar_oxigeno(int);
void consumir_oxigeno(int);
void generar_comida(int);
void consumir_comida(int);
void generar_basura(int);
void descartar_basura(int);
void (*signal(int sig, void (*func)(int)))(int) ;



#endif
