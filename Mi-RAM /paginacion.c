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
void agregarEstructAdminTipoPAG(t_pagina* pagina,int desplazamiento_pag, int bytesAlojados, int estructura, int flag){

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

int insertar_en_memoria_pag(t_pagina* pagina, void* pag_mem, int tipo_memoria, int* bytesAInsertar,  int estructura, int* bytesEscritos, int flag){
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

void* buscar_pagina(t_pagina* pagina_buscada) {
    void* pagina = NULL;
    int frame_ppal = pagina_buscada->nro_frame_mpal;
    //int frame_virtual = pagina->frame_m_virtual;
    if(frame_ppal != NULL)
        pagina = leer_memoria_pag(frame_ppal, MEM_PPAL);

        // log_info(logger, "pagina encontrada en memoria principal");
    return pagina;
}

t_pagina* crear_pagina_en_tabla(t_proceso* proceso,int estructura){
	log_info(logs_ram, "Creando pagina en la tabla de la patota: %d", proceso->pid);

	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->nro_pagina = list_size(proceso->tabla);
	pagina->nro_frame_mpal = NULL;
	pagina->tam_disponible = TAM_PAG;
	pagina->estructuras_alojadas = list_create();

	log_info(logs_ram, "Se creo el t_pagina de estructura: %d", estructura);

	list_add(proceso->tabla, pagina);

	return pagina;
}

int asignar_paginas_en_tabla(void* bytesAGuardar, t_proceso* proceso, int estructura){

	int aMeter = 0;
	int flagid;
	void* bufferAMeter = meterEnBuffer(bytesAGuardar, estructura, &aMeter, &flagid);
	void* copiaBuffer = bufferAMeter;
	int bytesEscritos;

	t_pagina* pagina;

	while(aMeter > 0)
	{
		lock(&mutexEscribiendoMemoria);
		log_info(logs_ram, "HAY QUE METER %d BYTES",aMeter);

		if(estructura != PCB)
		{
			pagina = buscar_ultima_pagina_disponible(proceso);

			if(pagina != NULL)
			{
				log_info(logs_ram, "La pagina en el frame %d tiene lugar y se va a aprovechar", pagina->nro_frame_mpal);
				insertar_en_memoria_pag(pagina, copiaBuffer, MEM_PPAL, &aMeter, estructura, &bytesEscritos, flagid);
			} else
			{
				log_info(logs_ram, "No se encontro una pagina con espacio restante");

				pagina = crear_pagina_en_tabla(proceso,estructura);

				pagina->nro_frame_mpal = buscar_frame_disponible(MEM_PPAL);

				if(pagina->nro_frame_mpal != NULL)
				{
					log_info(logs_ram,"Hay un frame disponible, el %d", pagina->nro_frame_mpal);
					insertar_en_memoria_pag(pagina, copiaBuffer, MEM_PPAL, &aMeter, estructura, flagid, &bytesEscritos);
				}
				else
				{
					log_info(logs_ram, "Memoria principal llena");
					return 0;
				}
			}
		}
		else
		{
			pagina = crear_pagina_en_tabla(proceso, estructura);

			pagina->nro_frame_mpal = buscar_frame_disponible(MEM_PPAL);

			if(pagina->nro_frame_mpal != NULL)
			{
				log_info(logs_ram,"Hay un frame disponible, el %d", pagina->nro_frame_mpal);
				insertar_en_memoria_pag(pagina, copiaBuffer, MEM_PPAL, &aMeter, estructura, flagid, &bytesEscritos);
			}
			else
			{
				log_info(logs_ram, "Memoria principal llena");
				return 0;
			}
		}

		copiaBuffer += bytesEscritos;
		unlock(&mutexEscribiendoMemoria);
	}
	log_info(logs_ram,"Se insertaron todos los bytes en ram");
	free(bufferAMeter);
	return 1;
}

t_pagina* buscar_ultima_pagina_disponible(t_proceso* proceso) {

    int nro_ult_pag = list_size(proceso->tabla) - 1;

    t_pagina* ultima_pagina = list_get(proceso->tabla, nro_ult_pag);

    if(ultima_pagina->tam_disponible == 0)
    {
        return NULL;
    }

    return ultima_pagina;
}

void existencia_patota(t_proceso* proceso) {
	if(proceso == NULL) {
		log_error(logs_ram, "No existe TCB para ese PCB");
		exit(1);
	}
}

char* irABuscarSiguienteTareaPag(t_proceso* proceso, TripuCB* tcb) {

	log_info(logs_ram,"Soy el tripu %d voy a buscar tarea a %d", tcb->tid, tcb->proxIns);

	char* tarea = string_new();
	char* aux = malloc(2);
	*(aux+1) = '\0';
	char* proximoALeer = malloc(2);
	*(proximoALeer+1) = '\0';
	void* paginaAGuardar;
	void* recorredorPagina;
	int indicePagina = (int) floor((double) tcb->proxIns / 100.0);
	int desplazamiento = tcb->proxIns % 100;
	t_pagina* pagina;

	lock(&mutexTablaPatota);
	log_info(logs_ram, "PAGINAS EN TABLA: %d - ME MUEVO %d PAGINAS", list_size(proceso->tabla), indicePagina);
	unlock(&mutexTablaPatota);

	t_list_iterator* iteradorTablaPaginas = iterarHastaIndice(proceso->tabla, indicePagina);

	// pagina = list_get(proceso->tablaDePaginas, indicePagina);
	*proximoALeer = '0';

	log_info(logs_ram,"Sacando tarea arrancando de indice: %d - desplazamiento: %d ", indicePagina, desplazamiento);

	lock(&mutexTablaPatota);
	while(list_iterator_has_next(iteradorTablaPaginas))
	{
		pagina = list_iterator_next(iteradorTablaPaginas);

		if(tieneEstructuraAlojada(pagina->estructuras_alojadas, TAREAS))
		{
		pagina = leer_memoria_pag(pagina->nro_frame_mpal, MEM_PPAL);
		recorredorPagina = pagina;
		recorredorPagina += desplazamiento;

		memcpy(aux,recorredorPagina,1);

		log_info(logs_ram, "VALOR DE PROXIMA A LEER: %s", aux);

		if(*aux == '|' && !string_is_empty(tarea))
		{
			log_info(logs_ram, "LLEGUE", aux);
			break;
		}

		log_info(logs_ram, "ME CHUPO UN HUEVO", proximoALeer);

			while(desplazamiento != TAM_PAG && *proximoALeer != '|'  && *proximoALeer != '\0')
				{
					memcpy(aux,recorredorPagina,1);
					string_append(&tarea,aux);
					recorredorPagina++;
					desplazamiento++;

					if(desplazamiento != TAM_PAG)
					{
						memcpy(proximoALeer,recorredorPagina,1);
					}

					log_info(logs_ram,"Sacando tarea: %s",tarea);
					log_info(logs_ram,"Proximo a leer: %s",proximoALeer);
				}


		log_info(logs_ram,"Asignando al TCB prox a ejecutar - indice: %d - desplazamiento: %d ", pagina->nro_pagina, desplazamiento);

		tcb->proxIns = pagina->nro_pagina * 100 + desplazamiento;

		desplazamiento = 0;
		free(pagina);
		}

		if(*proximoALeer == '|' || *proximoALeer == '\0') break;
	}
	unlock(&mutexTablaPatota);

	if(*aux == '|' && !string_is_empty(tarea))
	{

		log_info(logs_ram, "SE VUELVE A CARGAR LA DL");
		log_info(logs_ram,"Asignando al TCB prox a ejecutar - indice: %d - desplazamiento: %d ", pagina->nro_pagina, desplazamiento);

		tcb->proxIns = pagina->nro_pagina * 100 + desplazamiento;
	}


	actualizarTripulanteEnMemPag(proceso, tcb);


	if(*tarea == '|') tarea = string_substring_from(tarea,1);

	char* tareaAMandar = armarTarea(tarea);
	free(aux);
	free(proximoALeer);
	list_iterator_destroy(iteradorTablaPaginas);
	free(tarea);

	return tareaAMandar;
}

int guardar_TCB_pag(TripuCB* tcb,int idPatota) {
	t_proceso* proceso = buscar_tabla_de_paginas_de_patota(idPatota);
	tcb->pcb = 00;
	tcb->proxIns = buscar_inicio_tareas(proceso);
	existencia_patota(proceso);

	int res = asignar_paginas_en_tabla((void*) tcb, proceso,TCB);
	if(res == 0) {
		return NULL;
	} else {
		return 1;
	}

}

uint32_t buscar_inicio_tareas(t_proceso* proceso) {


    bool buscarDLTarea(t_pagina* pagina) {

    	//lock(&mutexAlojados);
    	bool a = tieneEstructuraAlojada(pagina->estructuras_alojadas, TAREAS);
    	//unlock(&mutexAlojados);

    	return a;
    }

    lock(&mutexTablaPatota);
    t_pagina* paginaConTarea = list_find(proceso->tabla, (void*) buscarDLTarea);
    unlock(&mutexTablaPatota);

    bool tieneTarea(t_alojado* estructuraAlojada) {
    	return estructuraAlojada->tipo == TAREAS;
    }

    lock(&mutexAlojados);
    t_alojado* alojadoConTarea = list_find(paginaConTarea->estructuras_alojadas, (void*) tieneTarea);
    unlock(&mutexAlojados);


    //Retornar un struct DL de las tareas que tiene el indice de la pagina y el desplazamiento en esta
    //Se guarda en algun lado cuanto pesa el string

    lock(&mutexAlojados);
    int a = paginaConTarea->nro_pagina + alojadoConTarea->base;
    unlock(&mutexAlojados);

    return a;
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




