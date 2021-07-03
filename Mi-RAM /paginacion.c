/*
 * paginacion.c
 *
 *  Created on: 25 jun. 2021
 *      Author: utnso
 */

#include "paginacion.h"
#include "mi-ram.h"

/* Verifica que un frame existe en memoria y es válido */
bool get_frame(int frame, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		return bitarray_test_bit(frames_ocupados_ppal, frame);
	}
	else {
		log_error(logs_ram, "Intento acceder a un frame invalido");
		exit(1);
	}
}


/* Setea el frame como bit en uso */
void asignar_marco_en_uso(int frame, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		bitarray_set_bit(frames_ocupados_ppal, frame);
	}else{
		log_error(logs_ram, "El frame que trata de poner en uso es invalido");
		exit(1);
	}
}


/* Esta función lee la pagina que esta en el frame indicado.  */
void* leer_memoria_pag(int frame, int mem){


	int desplazamiento = frame * TAM_PAG;

	void* pagina = malloc(TAM_PAG);
	if(mem == MEM_PPAL){
		log_info(logs_ram, "Se va a leer la pagina que arranca en %d", desplazamiento);
		lock(&mutexEscribiendoMemoria);
		memcpy(pagina, memoria+desplazamiento, TAM_PAG);
		unlock(&mutexEscribiendoMemoria);
	}

	return pagina;
}

int insertar_en_memoria_pag(t_pagina* pagina, void* pag_mem, int tipo_memoria, int* bytesAInsertar,  tipo_estructura estructura, int* bytesEscritos){
	if(!get_frame(pagina->nro_frame_mpal, tipo_memoria)){

		int desplazamiento_pag = TAM_PAG - pagina->tam_disponible;
		int desplazamiento_mem = pagina->nro_frame_mpal * TAM_PAG + desplazamiento_pag;
		int bytesAEscribir = pagina->tam_disponible - *bytesAInsertar;

		if(bytesAEscribir < 0){
			bytesAEscribir = pagina->tam_disponible;
			pagina->tam_disponible = 0;
			asignar_marco_en_uso(pagina->tam_disponible, tipo_memoria);
		}
	};
}



/*
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
	pagina->tam_ocupado = 0;
	return pagina;
}

void crear_proceso_paginas(t_list* paquete){
	t_list* lista_de_paginas = list_create();
	char **tareas = string_split(list_get(paquete, 2), "\n");

	t_pagina* pagina = crear_pagina();

	int i = 0;
	char* tarea = tareas[i];
	while(tarea != NULL){
		if((TAM_PAG - pagina->tam_ocupado) > sizeof(tarea)){
			t_alojado *tareaNueva = malloc(sizeof(t_alojado));
			tareaNueva->base = pagina->tam_ocupado;
			tareaNueva->tamanio = sizeof(tarea);
			tareaNueva->datos = tarea;
			list_add(pagina->estructurasAlojadas, tareaNueva); //Añado la tarea a los elementos de la pagina
			pagina->tam_ocupado += sizeof(tarea); //Resto al tamaño disponible de la pagina
			i++; //Escojo siguiente tarea
			char* tarea = tareas[i];
		} else {
			list_add(lista_de_paginas, pagina);
			pagina = crear_pagina();
		}
	}

	if((TAM_PAG - pagina->tam_ocupado) > sizeof(uint32_t)){
		t_pagina *paginaTareas = (t_pagina*) lista_de_paginas->head->data;
		t_alojado *inicioTareas = (t_alojado*) paginaTareas->estructurasAlojadas->head->data;
		t_alojado *direccionTareas = malloc(sizeof(t_alojado));
		direccionTareas->base = pagina->tam_ocupado;
		direccionTareas->tamanio = sizeof(uint32_t);
		direccionTareas->datos = inicioTareas->base;
	}else{
		list_add(lista_de_paginas, pagina);
		pagina = crear_pagina();
		t_pagina *paginaTareas = (t_pagina*) lista_de_paginas->head->data;
		t_alojado *inicioTareas = (t_alojado*) paginaTareas->estructurasAlojadas->head->data;
		t_alojado *direccionTareas = malloc(sizeof(t_alojado));
		direccionTareas->base = pagina->tam_ocupado;
		direccionTareas->tamanio = sizeof(uint32_t);
		direccionTareas->datos = inicioTareas->base;
	}

	if((TAM_PAG - pagina->tam_ocupado) > sizeof(uint32_t)){
		t_alojado *pid = malloc(sizeof(t_alojado));
		pid->base = pagina->tam_ocupado;
		pid->tamanio = sizeof(uint32_t);
		pid->datos = numero_patota;
	}

}
*/




