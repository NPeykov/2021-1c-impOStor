/*
 * paginacion.c
 *
 *  Created on: 25 jun. 2021
 *      Author: utnso
 */

#include "paginacion.h"
#include "mi-ram.h"

void dividir_memoria_en_frames() {
	t_frame *frame_ptr;
	uint32_t memoria = 0;
	int cantidadDeFrames = TAM_MEM/TAM_PAG;
	for (int i = 0; i < cantidadDeFrames; i++) {
		frame_ptr = (t_frame*) malloc(sizeof(t_frame));
		memoria += i * TAM_PAG;
		frame_ptr->memoria = memoria;
		frame_ptr->estado = LIBRE;
		frame_ptr->nro_frame = i;
		list_add(memoriaPrincipal, frame_ptr);
	}
}

t_pagina* crear_pagina(){
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->bit_uso=1;
	pagina->tam_disponible = TAM_PAG;
	return pagina;
}

void crear_proceso_paginas(t_list* paquete){
	t_list* lista_de_paginas = list_create();
	char **tareas = string_split(list_get(paquete, 2), "\n");

	t_pagina* pagina = crear_pagina();

	int i = 0;
	char* tarea = tareas[i];
	while(tarea != NULL){
		if(pagina->tam_disponible > sizeof(tarea)){
			t_alojado *tareaNueva = malloc(sizeof(t_alojado));
			tareaNueva->base = ;
			tareaNueva->tamanio = sizeof(tarea);
			tareaNueva->datos = tarea;
			list_add(pagina->elementos, tareaNueva); //Añado la tarea a los elementos de la pagina
			pagina->tam_disponible -= sizeof(tarea); //Resto al tamaño disponible de la pagina
			i++; //Escojo siguiente tarea
			char* tarea = tareas[i];
		} else {
			list_add(lista_de_paginas, pagina);
			pagina = crear_pagina();
		}
	}

}





