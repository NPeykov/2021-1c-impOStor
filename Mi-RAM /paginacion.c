/*
 * paginacion.c
 *
 *  Created on: 25 jun. 2021
 *      Author: utnso
 */

#include "paginacion.h"
#include "mi-ram.h"

void dividir_memoria_en_frames(void * memoria, int tammem) {
	t_frame *frame_ptr;
	int cantidadDeFrames = tammem/TAM_PAG;
	for (int i = 0; i < cantidadDeFrames; i++) {
		frame_ptr = (t_frame*) malloc(sizeof(t_frame));
		frame_ptr->memoria = memoria + i * TAM_PAG;
		frame_ptr->nro_frame = i;
		list_add(memoriaPrincipal, frame_ptr);
}

t_pagina* crear_pagina(){
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->bit_presencia = 0;
	pagina->bit_uso=1;
	pagina->bit_modificado=0;
	pagina->frame = NULL;
	return pagina;
}

void crear_proceso(t_list *paquete){

}





