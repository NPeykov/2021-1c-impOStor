#ifndef MiRAM_H_
#define MiRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"

int servidor;

void *gestionarCliente(int cliente);

int socket_mi_ram;
t_config *config;
char* puerto;
int socket_cliente;

// Todos los int de 32bits hacen referencia a una direccion en la memoria

typedef struct{
	uint32_t pid; //Identificador de la Patota
	uint32_t tareas; //Lista de tareas de la patota
} pcb; //Patota Control Block

typedef struct{
	uint32_t tid;//Identificador del Tripulante
	char status;//Estado del tripulante (Ejecucion,Bloqueado,Nuevo o Listo)
	uint32_t posX;//Posicion en el eje x
	uint32_t posY;//Posicion en el eje Y
	uint32_t proxIns; //Proxima Instruccion
	uint32_t pcb; //Direccion de la PCB asociada al tripulante
} tcb; //Tripulante Control Block


#endif
