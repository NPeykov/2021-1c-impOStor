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
t_list* patotas; //Lista de tablas de segmentos
int numero_patota = 1;

// Todos los int de 32bits hacen referencia a una direccion en la memoria
typedef enum tipo_segmento {
	PCB, TCB, TAREAS
} tipo_segmento;


typedef struct{
	int idSegmento;
	tipo_segmento tipo;
	uint32_t base;
	int tamanio;
	void *dato;
} Segmento; //Patota

typedef struct{
	uint32_t pid; //Identificador de la Patota
	uint32_t tareas; //Direccion Lista de tareas de la patota
} PatotaCB; //Patota Control Block

typedef struct{
	uint32_t tid;//Identificador del Tripulante
	char status;//Estado del tripulante (Ejecucion,Bloqueado,Nuevo o Listo)
	uint32_t posX;//Posicion en el eje x
	uint32_t posY;//Posicion en el eje Y
	uint32_t proxIns; //Proxima Instruccion
	uint32_t pcb; //Direccion de la PCB asociada al tripulante
} TripuCB; //Tripulante Control Block

typedef enum{
	PAGINACION, SEGMENTACION
}esquemaMemoria;

void *gestionarCliente(int cliente);


#endif
