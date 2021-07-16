#ifndef PAGINACION_H_
#define PAGINACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <bits/semaphore.h>

#include "mi-ram.h"

int TAM_PAG;
int TAM_MEM;

typedef enum {
	OCUPADO, LIBRE
} estado_frame;

typedef enum {
	MEM_PPAL, MEM_VIRT
} tipo_memoria;

typedef struct {
	estado_frame estado;
	uint32_t memoria;
	int nro_frame;
} t_frame;

typedef struct {
	int nro_pagina;
	int nro_frame_mpal;
	//int nro_frame_swap;
	//bool bit_presencia;
	bool bit_uso;
	t_list* estructuras_alojadas;
	int tam_disponible;
} t_pagina;

typedef struct {
	int nro_estructura;
	int base;
	int tamanio;
	tipo_estructura tipo;
	int flagid;
} t_alojado;

t_bitarray* frames_ocupados_ppal;
int cant_marcos_memoria;

pthread_mutex_t mutexEscribiendoMemoria;
pthread_mutex_t mutexBitArray;

bool traer_marco_valido(int, int);
void asignar_marco_en_uso(int, int);
void liberar_marco(int, int);
void* leer_memoria_pag(int, int);
void agregarEstructAdminTipoPAG(t_pagina*, int ,int , int , int);
int insertar_en_memoria_pag(t_pagina*, void*, int, int*, int , int*);
uint32_t buscar_marco_disponible(int );
int marco_vacio(int );
void* buscar_pagina(t_pagina* );
t_pagina* crear_pagina_en_tabla(t_proceso* ,int);
//SIN HACER
int asignarPaginasEnTabla(void* , t_tablaPaginasPatota* , tipoEstructura );
t_tablaPaginasPatota* buscarTablaDePaginasDePatota(int );
t_info_pagina* buscarUltimaPaginaDisponible(t_tablaPaginasPatota* );
t_tarea* guardarTCBPag(tcb*, int);
int guardarPCBPag(pcb*, char*);
uint32_t estimarDLTareasPag();
uint32_t buscarInicioDLTareas(t_tablaPaginasPatota* );
t_tarea* irABuscarSiguienteTareaPag(t_tablaPaginasPatota* , tcb* );
bool tieneEstructuraAlojada(t_list* , tipoEstructura);
bool tieneTripulanteAlojado(t_list* , int);
t_alojado* obtenerAlojadoPagina(t_list* , int);
int actualizarTripulanteEnMemPag(t_tablaPaginasPatota* , tcb*);
int frameTotalmenteLibre(int );
t_list* paginasConTripu(t_list*, uint32_t );
int sobreescribirTripu(t_list* , tcb* );
int actualizarTripulantePag(tcb* , int);
tcb* obtenerTripulante(t_tablaPaginasPatota* ,int );
t_tarea* asignarProxTareaPag(int , int);
t_list_iterator* iterarHastaIndice(t_list*, int);
void existenciaDeTablaParaPatota(t_tablaPaginasPatota*);
void chequearUltimoTripulante(t_tablaPaginasPatota*);
bool tieneTripulantesPag(t_tablaPaginasPatota*);
t_tablaPaginasPatota* patotaConFrame(int);
t_info_pagina* paginaConFrame(int ,t_tablaPaginasPatota*);
void expulsarTripulantePag(int ,int);
void dumpPag();

#endif
