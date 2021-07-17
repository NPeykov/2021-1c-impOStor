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

//Esenciales del Servidor
t_log *logs_ram;
int socket_principal_ram;
t_config *config;
char* puerto;
char* tipoMemoria;
int tamaniomemoria;

//Malloc principal de memoria
void* memoria;

//Lista de procesos activos
t_list* patotas;

//Contador para asignar y saber cantidad de patotas que ingresaron al sistema
int numero_patota = 1;


//Estructuras multi-esquema
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

#endif
