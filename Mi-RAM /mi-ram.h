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
t_list* patotas; //Lista de tablas de segmentos (TABLA DE PROCESOS)
int numero_patota = 1;
char* tipoMemoria;
void *memoria;
int tamaniomemoria;

// Todos los int de 32bits hacen referencia a una direccion en la memoria
typedef enum tipo_segmento {
	PCB, TCB, TAREAS
} tipo_segmento;


typedef struct{
	int idSegmento;
	tipo_segmento tipo;//PCB TCB o Tareas
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

typedef struct{
	int id;
	t_list *tabla_de_segmentos;
	int memoriaPedida;
} t_proceso;

void laGarra(int tripulanteBuscado, Segmento unSegmento);

void eliminarTripulante(int idTripulante);

void *gestionarCliente(int cliente);

//Crea cada uno de los segmentos que necesita una patota
void crear_segmentos(t_list* paquete, int tamanio, t_list* tabla_segmentos);

//Crea un segmento con la estructura de PCB
Segmento* crear_segmento_pcb(int tamanio, t_list* tabla_segmentos);

//Crea un segmento con la estructura de TCB
Segmento* crear_segmento_tcb(int numero_tripulante, uint32_t posX, uint32_t posY ,int tamanio, t_list* tabla_segmentos);

//Obtiene la base logica del ultimo segmento que entrará a RAM
uint32_t calcular_base_logica(Segmento *segmento, t_list* tabla_segmentos);
#endif
