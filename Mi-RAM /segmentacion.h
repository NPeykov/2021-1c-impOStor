#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"
#include <commons/string.h>
#include "mi-ram.h"

t_list* memoriaPrincipal;
t_list* patotas;

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

//Prototipos de Funciones


void *gestionarClienteSeg(int );

//
//Busca un tripulante dado un idTripulante e id de Patota.
// #eliminarTripulante(idTripulante, idPatota)
//
void eliminarTripulante(int, int);

//
//Busca entre todos los procesos y retorna el tripulante buscado
// #buscarTripulante(idTripulante, idPatota)
//
TripuCB *buscarTripulante(int ,int);

//
//Crea un proceso
// #crear_proceso(cantidadTripulantes, posicionesTripulantes, stringTareas, socket_cliente)
//
void crear_proceso(char *,char *,char *, int);

//
//Actualiza la posicion del tripulante en memoria
// #actualizarTripulante(idTripulante, idPatota, ubicacionNueva)
//
void actualizarTripulante(int ,int , char*);

//
//Crea un segmento con la estructura de PCB.
// &Retorna -1 si no hay espacio en memoria
// #crear_segmento_pcb(direccionTareas, tabla_segmentos_de_proceso)
//
int crear_segmento_pcb(uint32_t , t_list*);

//
//Crea un segmento de tareas.
// &Las tareas se guardan en un string de formato "primero;tarea1;\nsegundo;tarea2\n"
// &Retorna -1 si no hay espacio en memoria
// #crear_segmento_tareas(stringTareas, tabla_segmentos_de_proceso)
//
int crear_segmento_tareas(char *, t_list*);

//
//Crea un segmento con la estructura de TCB
// &Retorna -1 si no hay espacio en memoria
// #crear_segmento_tcb(numero_tripulante, posX, posY, dir_pcb, tabla_segmentos_proceso)
//
int crear_segmento_tcb(uint32_t , uint32_t , uint32_t , uint32_t, t_list*);

//
//Obtiene la base logica del ultimo segmento que entrará a RAM
// &El -1 de las creaciones de segmento llega desde acá.
// #calcular_base_logica(segmentoAUbicar)
//
uint32_t calcular_base_logica(Segmento *);

//
//Agrega las estructuras de un proceso al malloc de memoria inicial
// #agregarAMemoria(tabla_de_segmentos)
//
void agregarAMemoria(t_list*);


#endif
