#ifndef PAGINACION_H_
#define PAGINACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <math.h>
#include "mi-ram.h"

int TAM_PAG;
int TAM_MEM;
int cantidadDeFrames;
bool esLRU;
char* dirSwap;
t_list *paginas_lru;

//Posiciones de los datos respecto del inicio de tripulante
int desplazAEstado = 4;//Tamanio 1 por ser char
int desplazAPosX = 5;//Tamanio 4 uint32_t
int desplazAPosY = 9;//Tamanio 4 uint32_t
int desplazAProxIns = 13; //Tamanio 4 uint32_t

typedef enum {
	LIBRE,OCUPADO
} estado_frame;

typedef enum {
	MEM_PPAL, MEM_VIRT
} tipo_memoria;

typedef struct {
	int nro_pagina;
	int pid;
	int nro_frame_mpal;
	int nro_frame_swap;
	bool bit_uso;
	bool bit_presencia;
	t_list* estructuras_alojadas;
	int tam_disponible;
} t_pagina;

typedef struct {
	int nro_estructura;
	int base;
	int tamanio;
	tipo_estructura tipo;
	int flagid;
	int caracterRep;
} t_alojado;

t_bitarray* frames_ocupados_ppal;

pthread_mutex_t charRepresentativo;
pthread_mutex_t mutexEscribiendoMemoria;
pthread_mutex_t mutexBitArray;
pthread_mutex_t mutexTablaPaginas;
pthread_mutex_t mutexAlojados;
pthread_mutex_t mutexTablaPatotas;
pthread_mutex_t mutexNumeroPatotas;
pthread_mutex_t mutexEscribiendoSwap;
pthread_mutex_t mutex_swap_file;
pthread_mutex_t mutex_clock;
pthread_mutex_t listaLRU;

sem_t tripulantesDisponiblesPag;

int caracterRepresentativo;


/* Verifica que un frame existe en memoria y es válido */

bool traer_marco_valido(int, int);

/* Setea el frame como bit en uso */

void asignar_marco_en_uso(int, int);

/* Libera el marco que recibe por parametro */

void liberar_marco(int, int);

/* Verifica si el marco se encuentra utilizado por alguna pagina */

bool marco_vacio(int, int );

/* Busca en la memoria un marco disponible */

uint32_t buscar_marco_disponible(int );

/* Devuelve si el frame indicado esta completamente libre */

int frame_libre(int );

/* Crea la estructura administrativa para manejar los tipos en la pagina
 * y la agrega a la pagina correspondiente */

void agregar_estructura_a_pagina(t_pagina*, int ,int , int , int);

/* Lee la pagina que esta en el frame indicado. */

void* leer_memoria_pag(int, int);

/* Inserta en el malloc de memoria la pagina. */

int insertar_en_memoria_pag(t_pagina*, void*, int*, int , int*, int);

/* Busca la pagina pasada por parametro en el malloc de memoria */

void* buscar_pagina(t_pagina* );

/* Crea la pagina en la estructura administrativa de la patota pasada por parametro */

t_pagina* crear_pagina_en_tabla(t_proceso* ,int);

/* Asigna la pagina en la tabla de la patota y la inserta en el malloc de memoria */

int insertar_en_paginas(void* , t_proceso* , int, int);

/* Verifica que la patota/proceso exista */

void existencia_patota(t_proceso*);

/* Busca la patota por el pid que se pasa por parametro */

t_proceso* buscar_patota(int );

/* Busca la ultima pagina del proceso enviado por parametro */

t_pagina* buscar_ultima_pagina_disponible(t_proceso* );

/* Guarda el TCB en el malloc de memoria */

int guardar_TCB_pag(void*);

/* Guarda la patota(PCB) y las tareas en el malloc de memoria */

int guardar_PCB_pag(void*);

/* Busca la siguiente tarea en el malloc de memoria */

char* obtener_siguiente_tarea_pag(t_proceso* , uint32_t*, int );

/* Busca la direccion logica donde empiezan las paginas de tareas de un proceso/patota */

uint32_t buscar_inicio_tareas(t_proceso* );

/*  Esta funcion hay que revisarla */

uint32_t calcuar_DL_tareas_pag();

/* Verifica si una pagina tiene la estructura alojada que se pasa por parametro */

bool tiene_pagina_estructura_alojadas(t_list* , int);

/* Verifica si una pagina tiene un tripulante como estructura alojada */

bool tiene_pagina_tripu_alojado(t_list* , int);

/* Retorna la estructura administrativa de la pagina de un tripulante */

t_alojado* obtener_tripulante_pagina(t_list* , int);

/* Trae todas la paginas asociadas a un tripulante */

t_list* lista_paginas_tripulantes(t_list*, uint32_t );

/* Actualiza en memoria al tripulante */

void actualizar_tripulante_pag(t_tripulante_iniciado*);

/* Asigna la proxima tarea al tripulante de la patota pasado por parametro  */

void asignar_prox_tarea_pag(void*);

/* Recibe patota id y tripu id, elimina tanto estructuras administrativas como en memoria */

void expulsar_tripulante_pag(void*);

/* Verifica que un proceso le queden tripulantes */

bool proceso_tiene_tripulantes_pag(t_proceso*);

/* Retorna una patota si hay en el frame pasado por parametro */

t_proceso* frame_con_patota(int);

/* Retorna una pagina si el proceso enviado por parametro se encuentra en el frame enviado por parametro*/

t_pagina* pagina_que_tiene_el_frame(int ,t_proceso*);

/* Realiza el dump de memoria para paginacion */

void dump_pag();

/*Crea un buffer y da los valores a flag y el tamaño que se buscara guardar*/

void* meterEnBuffer(void* , int , int* , int* );

/*Busca el numero del primer frame vacio */

int buscar_frame_disponible(int);

/* Crea una tarea */

char* armarTarea(char*);

/* Actualiza un tripulante en memoria */

void sobreescribir_memoria(int, void*, int, int, int);

void cargarDLTripulante(void*, TripuCB*);

TripuCB* transformarEnTripulante(void*);

void swap_pages(t_pagina* , t_pagina*);

/* Asigna un marco libre en swap a una pagina */

void asignar_marco_en_swap(t_pagina* pag);

void escribir_en_archivo_swap(void *file, t_list *tabla_de_paginas, size_t tam_a_mappear,size_t tam_arch);

/* Trae la pagina a reemplazar por algoritmo de clock */

t_pagina* algoritmo_clock();

void reemplazarSegunAlgoritmo(t_pagina*);

void traer_pagina(t_pagina*);

void *obtener_dato_tripulante(t_list*, int , int, int);

void escribir_dato_tripulante(t_list*, int ,int,int,void*);

void cerrarMemoriaPag();

void limpiarProceso(t_proceso*);

bool proceso_tiene_tripulantes(t_proceso*);

#endif
