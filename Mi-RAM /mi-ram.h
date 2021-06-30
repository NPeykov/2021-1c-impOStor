#ifndef MiRAM_H_
#define MiRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"
#include <commons/string.h>

int servidor;
int socket_mi_ram;
t_config *config;
char* puerto;
int socket_cliente;
t_list* memoriaPrincipal;//Lista de tablas de segmentos (TABLA DE PROCESOS)
t_list* patotas;
int numero_patota = 1;
char* tipoMemoria;
void *memoria;
int tamaniomemoria;
t_log *logs_ram;

// Todos los int de 32bits hacen referencia a una direccion en la memoria
typedef enum tipo_segmento {
	PCB, TCB, TAREAS
} tipo_segmento;


typedef struct{
	int idSegmento;
	tipo_segmento tipo;//PCB TCB Tareas
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

//Prototipos de Funciones

void *gestionarClienteSeg(int );
void *gestionarClientePag(int );
void eliminarTripulante(int );
void crear_proceso(char *,char *,char *, int cliente);
//Crea un segmento con la estructura de PCB
int crear_segmento_pcb(uint32_t , t_list*);

int crear_segmento_tareas(char *, t_list*);

//Crea un segmento con la estructura de TCB
int crear_segmento_tcb(uint32_t , uint32_t , uint32_t , uint32_t, t_list*);

//Obtiene la base logica del ultimo segmento que entrará a RAM
uint32_t calcular_base_logica(Segmento *);

//Verifica si un segmento se creo y entro a memoria correctamente. Si no lo hizo y retorno -1
//debe ir y avisar a Discordiador que no hay suficiente memoria
void verificarSegmento(int);
#endif
