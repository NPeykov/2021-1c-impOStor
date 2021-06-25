#ifndef PAGINACION_H_
#define PAGINACION_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>

int TAM_PAG;

typedef struct {
	int pid;
	t_list* tablaDePaginas;
} tabla_paginas;

typedef struct {
	void *memoria;
	int nro_frame;
} t_frame;

typedef struct {
	int nro_pagina;
	int nro_frame;
	bool bit_presencia;
	bool bit_modificado;
	bool bit_uso;
	t_frame *frame;
	t_list* elementos;
} t_pagina;
