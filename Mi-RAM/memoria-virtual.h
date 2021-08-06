#ifndef MEMORIA_VIRTUAL_H_
#define MEMORIA_VIRTUAL_H_
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <commons/bitarray.h>
#include <semaphore.h>
#include "paginacion.h"

int TAM_SWAP;
int PUNTERO_ALGORITMO;
t_bitarray *BIT_ARRAY_SWAP;
void *MEMORIA_VIRTUAL;
void *ultimoMarcoSwap;
int marcos_en_swap;

int crear_archivo_swap();
void llenarArchivo(int, int);
int posicion_libre_en_swap();
void inicializar_bitmap_swap();

#endif
