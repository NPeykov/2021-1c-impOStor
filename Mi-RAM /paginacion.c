/*
 * paginacion.c
 *
 *  Created on: 25 jun. 2021
 *      Author: utnso
 */

#include "paginacion.h"
#include "mi-ram.h"

/* Verifica que un frame existe en memoria y es válido */
bool traer_marco_valido(int frame, int tipo_memoria){
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

/* Libera el marco que recibe por parametro */
void liberar_marco(int frame, int tipo_memoria)
{
	if(tipo_memoria == MEM_PPAL) {
		lock(&mutexBitArray);
		bitarray_clean_bit(frames_ocupados_ppal, frame);
		unlock(&mutexBitArray);
	}
	else {
		log_error(logs_ram, "El frame que se quiere eliminar es invalido");
		exit(1);
	}
}

/* Verifica si el marco se encuentra utilizado por alguna pagina */
int marco_vacio(int marco){

	bool _marco_en_uso(t_proceso* patota) {

		bool _marco_usado_por_pagina(t_pagina* pagina){
			return pagina->nro_frame_mpal == marco;
		}

		return list_any_satisfy(patota->tabla,(void*) _marco_usado_por_pagina);
	}

	bool a = list_any_satisfy(patotas,(void*) _marco_en_uso);

	return !a;
}

/* Busca en la memoria un marco disponible */
uint32_t buscar_marco_disponible(int tipo_memoria){
	int size = 0;
	if(tipo_memoria == MEM_PPAL){
		size = cant_marcos_memoria;
	}

	for(uint32_t m = 0; m < size; m++){
		if(!traer_marco_valido(m, tipo_memoria) && marco_vacio(m)) {
			return m;
		}
	}

	log_info(logs_ram, "No se encontro un frame disponible");
	return -1;
}

/* Crea la estructura administrativa para manejar los tipos en la pagina y la agrega a la pagina correspondiente */
void agregarEstructAdminTipoPAG(t_pagina* pagina,int desplazamiento_pag, int bytesAlojados, tipo_estructura estructura, int flag){

	t_alojado* nueva_estructura_pag = malloc(sizeof(t_alojado));
	log_info(logs_ram, "Se va a agregar a la pagina %d , la estructura %d con base %d , y tamanio %d", pagina->nro_pagina,
			estructura,desplazamiento_pag, bytesAlojados);
	nueva_estructura_pag->nro_estructura = list_size(pagina->estructuras_alojadas);
	nueva_estructura_pag->base = desplazamiento_pag;
	nueva_estructura_pag->tamanio = bytesAlojados;
	nueva_estructura_pag->tipo = estructura;
	nueva_estructura_pag->flagid = flag;
	list_add(pagina->estructuras_alojadas, nueva_estructura_pag);
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

int insertar_en_memoria_pag(t_pagina* pagina, void* pag_mem, int tipo_memoria, int* bytesAInsertar,  tipo_estructura estructura, int* bytesEscritos, int flag){
	if(!traer_marco_valido(pagina->nro_frame_mpal, tipo_memoria)){

		int desplazamiento_pag = TAM_PAG - pagina->tam_disponible;
		int desplazamiento_mem = pagina->nro_frame_mpal * TAM_PAG + desplazamiento_pag;
		int bytesAEscribir = pagina->tam_disponible - *bytesAInsertar;

		if(bytesAEscribir < 0)
		{
			bytesAEscribir = pagina->tam_disponible;
			pagina->tam_disponible = 0;
			asignar_marco_en_uso(pagina->nro_frame_mpal,tipo_memoria);
		}
		else {
			bytesAEscribir = *bytesAInsertar;
			pagina->tam_disponible = pagina->tam_disponible - *bytesAInsertar;
		}

		agregarEstructAdminTipoPAG(pagina, desplazamiento_pag, bytesAEscribir, estructura, flag);

		if(tipo_memoria == MEM_PPAL)
		{
			lock(&mutexEscribiendoMemoria);
			memcpy(memoria + desplazamiento_mem, pag_mem, bytesAEscribir);
			unlock(&mutexEscribiendoMemoria);

		}

		log_info(logs_ram, "Se inserto en RAM: FRAME: %d | DESDE: %d | HASTA: %d | ESTRUCTURA: %d", pagina->nro_pagina, desplazamiento_pag, desplazamiento_pag + bytesAEscribir - 1, estructura);

		*bytesAInsertar -= bytesAEscribir;

		*bytesEscritos = bytesAEscribir;

		log_info(logs_ram, "Resta insertar %d Bytes", *bytesAInsertar);
		return 1;
	}
	else{
		printf("MARCO EN USO\n");
		return 0;
	}
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




