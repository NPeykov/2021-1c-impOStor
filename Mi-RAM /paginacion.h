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
	int pid;
	t_list* tablaDePaginas;
} tabla_procesos;

typedef struct {
	estado_frame estado;
	uint32_t memoria;
	int nro_frame;
} t_frame;

typedef struct {
	int nro_pagina;
	int nro_frame;
	bool bit_presencia;
	bool bit_modificado;
	bool bit_uso;
	t_list* elementos;
	int tam_disponible;
} t_pagina;
