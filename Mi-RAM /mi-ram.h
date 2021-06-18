#ifndef MiRAM_H_
#define MiRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"

int servidor;

int socket_mi_ram;
t_config *config;
char* puerto;
int socket_cliente;
char *tareas[];
t_list* patotas; //Lista de tablas de segmentos

// Todos los int de 32bits hacen referencia a una direccion en la memoria

typedef struct{
	int idSegmento;
	uint32_t inicio;
	int tamanio;
} TablaSegmentos; //Patota

typedef struct{
	uint32_t pid; //Identificador de la Patota
	uint32_t tareas; //Direccion Lista de tareas de la patota
} pcb; //Patota Control Block

typedef struct{
	uint32_t tid;//Identificador del Tripulante
	char status;//Estado del tripulante (Ejecucion,Bloqueado,Nuevo o Listo)
	uint32_t posX;//Posicion en el eje x
	uint32_t posY;//Posicion en el eje Y
	uint32_t proxIns; //Proxima Instruccion
	uint32_t pcb; //Direccion de la PCB asociada al tripulante
} tcb; //Tripulante Control Block


// La memoria se estructurara a modo de listas enlazadas
// Cada nodo de la lista es una posicion de memoria que almacena un dato
// Este dato es un dato de 32bits
// Y esta posicion de memoria conoce a la siguiente posicion

typedef struct{
	uint32_t dato;
	struct nodoMemoria *siguiente;
}nodoMemoria;

typedef enum{
	PAGINACION, SEGMENTACION
}esquemaMemoria;

struct nodoMemoria *memoria = NULL;
TablaSegmentos* nuevaPatota;

void crearRAM(t_config* config, struct nodoMemoria *memoria);

void *gestionarCliente(int cliente);


#endif
