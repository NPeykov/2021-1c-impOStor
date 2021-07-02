#ifndef PAGINACION_H_
#define PAGINACION_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>

int TAM_PAG;
int TAM_MEM;

typedef enum {
	OCUPADO, LIBRE
} estado_frame;

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
	t_list* elementos; //aca van los t_alojado
	int tam_disponible;
} t_pagina;

typedef struct {
	int base;
	int tamanio;
	tipo_segmento tipo;
	void* datos;
} t_alojado;

#endif
