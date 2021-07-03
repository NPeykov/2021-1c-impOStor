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
	t_list* estructurasAlojadas;
	int tam_disponible;
} t_pagina;

typedef struct {
	int base;
	int tamanio;
	tipo_estructura tipo;
	void* datos;
} t_alojado;

t_bitarray* frames_ocupados_ppal;

pthread_mutex_t mutexEscribiendoMemoria;

bool get_frame(int, int);
void asignar_marco_en_uso(int, int);
void* leer_memoria_pag(int, int);
int insertar_en_memoria_pag(t_pagina*, void*, int, int*, tipo_estructura, int*);
#endif
