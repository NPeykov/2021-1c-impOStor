#ifndef MiRAM_H_
#define MiRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"
#include <commons/string.h>
#include "segmentacion.h"
//#include "paginacion.h"
#include <commons/temporal.h>
#include <commons/txt.h>
#include <signal.h>

int servidor;
int socket_mi_ram;
t_config *config;
char* puerto;
int socket_cliente;
int numero_patota = 1;
char* tipoMemoria;
void* memoria;
int tamaniomemoria;
t_list* patotas;
t_log *logs_ram;

// Todos los int de 32bits hacen referencia a una direccion en la memoria
typedef enum {
	PCB, TCB, TAREAS
} tipo_estructura;

typedef struct{
	int pid;
	t_list *tabla;
} t_proceso;

typedef struct {
	int cantidad_tripulantes;
	char *contenido_tareas;
	int socket;
} t_datos_inicio_patota;

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

#endif
