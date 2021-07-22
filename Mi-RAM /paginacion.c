/*
 * paginacion.c
 *
 *  Created on: 25 jun. 2021
 *      Author: utnso
 */

#include "paginacion.h"
#include "mi-ram.h"


bool traer_marco_valido(int frame, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		return bitarray_test_bit(frames_ocupados_ppal, frame);
	}
	else {
		log_error(logs_ram, "Intento acceder a un frame invalido");
		exit(1);
	}
}


void asignar_marco_en_uso(int frame, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		bitarray_set_bit(frames_ocupados_ppal, frame);
		t_frame elMarco = list_get(memoriaPrincipal, frame);
		elMarco->estado = OCUPADO;//TODO REVISAR
	}else{
		log_error(logs_ram, "El frame que trata de poner en uso es invalido");
		exit(1);
	}
}


void liberar_marco(int frame, int tipo_memoria)
{
	if(tipo_memoria == MEM_PPAL) {
		pthread_mutex_lock(&mutexBitArray);
		bitarray_clean_bit(frames_ocupados_ppal, frame);
		pthread_mutex_unlock(&mutexBitArray);
		t_frame elMarco = list_get(memoriaPrincipal, frame);
		elMarco->estado = LIBRE;//TODO REVISAR
	}
	else {
		log_error(logs_ram, "El frame que se quiere eliminar es invalido");
		exit(1);
	}
}

int marco_vacio(int marco){


	bool _marco_en_uso(void* frame) {//TODO REVISAR
		t_frame unFrame = (t_frame*) frame;
		return (unFrame->estado == OCUPADO && unFrame->nro_frame == marco);
	}

	bool a = list_any_satisfy(memoriaPrincipal,(void*) _marco_en_uso);

	return !a;
}


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



void* leer_memoria_pag(int frame, int mem){

	int desplazamiento = frame * TAM_PAG;

	void* pagina = malloc(TAM_PAG);
	if(mem == MEM_PPAL){
		log_info(logs_ram, "Se va a leer la pagina que arranca en %d", desplazamiento);
		pthread_mutex_lock(&mutexEscribiendoMemoria);
		memcpy(pagina, memoria+desplazamiento, TAM_PAG);
		pthread_mutex_unlock(&mutexEscribiendoMemoria);
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
			pthread_mutex_lock(&mutexEscribiendoMemoria);
			memcpy(memoria + desplazamiento_mem, pag_mem, bytesAEscribir);
			pthread_mutex_unlock(&mutexEscribiendoMemoria);

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

int frame_libre(int frame){
	bool estaEnUso(t_frame* unFrame) {

		if (unFrame->nro_frame == frame && unFrame->estado == LIBRE){
			return 1;
		}

		return 0;
	}

	bool a = list_any_satisfy(memoriaPrincipal,(void*) estaEnUso);

	return !a;
}

void* buscar_pagina(t_pagina* pagina_buscada) {
    void* pagina = NULL;
    int frame_ppal = pagina_buscada->nro_frame_mpal;
    //int frame_virtual = pagina->frame_m_virtual;
    if(frame_ppal != -1)
        pagina = leer_memoria_pag(frame_ppal, MEM_PPAL);

    // log_info(logger, "pagina encontrada en memoria principal");
    return pagina;
}

t_pagina* crear_pagina_en_tabla(t_proceso* proceso,int estructura){
	log_info(logs_ram, "Creando pagina en la tabla de la patota: %d", proceso->pid);

	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->nro_pagina = list_size(proceso->tabla);
	pagina->nro_frame_mpal = -1;
	pagina->tam_disponible = TAM_PAG;
	pagina->estructuras_alojadas = list_create();

	log_info(logs_ram, "Se creo el t_pagina de estructura: %d", estructura);

	list_add(proceso->tabla, pagina);

	return pagina;
}

int asignar_paginas_en_tabla(void* bytesAGuardar, t_proceso* proceso, int estructura){

	int aMeter = 0;
	int flagid;
	void* bufferAMeter = meterEnBuffer(bytesAGuardar, estructura, &aMeter, &flagid); //No entiendo como manejar el tema de los buffer aca hay que crear la funcion para meter los datos en el buffer.
	void* copiaBuffer = bufferAMeter;
	int bytesEscritos;

	t_pagina* pagina;

	while(aMeter > 0)
	{
		pthread_mutex_lock(&mutexEscribiendoMemoria);
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

				pagina->nro_frame_mpal = buscar_marco_disponible(MEM_PPAL);

				if(pagina->nro_frame_mpal != -1)
				{
					log_info(logs_ram,"Hay un frame disponible, el %d", pagina->nro_frame_mpal);
					insertar_en_memoria_pag(pagina, copiaBuffer, MEM_PPAL, &aMeter, estructura,&bytesEscritos , flagid);
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

			pagina->nro_frame_mpal = buscar_marco_disponible(MEM_PPAL);

			if(pagina->nro_frame_mpal != -1)
			{
				log_info(logs_ram,"Hay un frame disponible, el %d", pagina->nro_frame_mpal);
				insertar_en_memoria_pag(pagina, copiaBuffer, MEM_PPAL, &aMeter ,estructura ,&bytesEscritos ,flagid);
			}
			else
			{
				log_info(logs_ram, "Memoria principal llena");
				return 0;
			}
		}

		copiaBuffer += bytesEscritos;
		pthread_mutex_unlock(&mutexEscribiendoMemoria);
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
		log_error(logs_ram, "Este proceso es inexistente");
		exit(1);
	}
}

char* obtener_siguiente_tarea_pag(t_proceso* proceso, TripuCB* tcb) {

	log_info(logs_ram,"Soy el tripu %d voy a buscar tarea a %d", tcb->tid, tcb->proxIns);

	char* tarea = string_new();
	char* aux = malloc(2);
	*(aux+1) = '\0';
	char* proximoALeer = malloc(2);
	*(proximoALeer+1) = '\0';
	void* paginaAGuardar;
	void* recorredorPagina;
	int indicePagina = (int) tcb->proxIns / 100.0;
	int desplazamiento = tcb->proxIns % 100;
	t_pagina* pagina;

	pthread_mutex_lock(&mutexTablaPatota);
	log_info(logs_ram, "PAGINAS EN TABLA: %d - ME MUEVO %d PAGINAS", list_size(proceso->tabla), indicePagina);
	pthread_mutex_unlock(&mutexTablaPatota);

	*proximoALeer = '0';

	log_info(logs_ram,"Sacando tarea arrancando de indice: %d - desplazamiento: %d ", indicePagina, desplazamiento);

	pthread_mutex_lock(&mutexTablaPatota);
	for(int i = 0; i <= indicePagina; i++){

		pagina = list_get(proceso->tabla, i);

		if(tiene_pagina_estructura_alojadas(pagina->estructuras_alojadas, TAREAS))
		{
			paginaAGuardar = leer_memoria_pag(pagina->nro_frame_mpal, MEM_PPAL);
			recorredorPagina = paginaAGuardar;
			recorredorPagina += desplazamiento;

			memcpy(aux,recorredorPagina,1);

			log_info(logs_ram, "VALOR DE PROXIMA A LEER: %s", aux);

			if(*aux == '|' && !string_is_empty(tarea))
			{
				log_info(logs_ram, "LLEGUE", aux);
				break;
			}

			log_info(logs_ram, "ME CHUPO UN HUEVO", proximoALeer);

			while(desplazamiento != TAM_PAG && *proximoALeer != '\n'  && *proximoALeer != '\0')
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
			free(paginaAGuardar);
		}

		if(*proximoALeer == '\n' || *proximoALeer == '\0') break;

	}
	pthread_mutex_unlock(&mutexTablaPatota);

	if(*aux == '|' && !string_is_empty(tarea))
	{

		log_info(logs_ram, "SE VUELVE A CARGAR LA DL");
		log_info(logs_ram,"Asignando al TCB prox a ejecutar - indice: %d - desplazamiento: %d ", pagina->nro_pagina, desplazamiento);

		tcb->proxIns = pagina->nro_pagina * 100 + desplazamiento;
	}


	actualizar_tripulante_EnMem_pag(proceso, tcb);


	if(*tarea == '|') tarea = string_substring_from(tarea,1);

	char* tareaAMandar = armarTarea(tarea);
	free(aux);
	free(proximoALeer);
	free(tarea);

	return tareaAMandar;
}

uint32_t buscar_inicio_tareas(t_proceso* proceso) {


    bool buscarDLTarea(t_pagina* pagina) {

    	//pthread_mutex_lock(&mutexAlojados);
    	bool a = tiene_pagina_estructura_alojadas(pagina->estructuras_alojadas, TAREAS);
    	//pthread_mutex_unlock(&mutexAlojados);

    	return a;
    }

    pthread_mutex_lock(&mutexTablaPatota);
    t_pagina* paginaConTarea = list_find(proceso->tabla, (void*) buscarDLTarea);
    pthread_mutex_unlock(&mutexTablaPatota);

    bool tieneTarea(t_alojado* estructuraAlojada) {
    	return estructuraAlojada->tipo == TAREAS;
    }

    pthread_mutex_lock(&mutexAlojados);
    t_alojado* alojadoConTarea = list_find(paginaConTarea->estructuras_alojadas, (void*) tieneTarea);
    pthread_mutex_unlock(&mutexAlojados);


    //Retornar un struct DL de las tareas que tiene el indice de la pagina y el desplazamiento en esta
    //Se guarda en algun lado cuanto pesa el string

    pthread_mutex_lock(&mutexAlojados);
    int a = paginaConTarea->nro_pagina + alojadoConTarea->base;
    pthread_mutex_unlock(&mutexAlojados);

    return a;
}

uint32_t calcuar_DL_tareas_pag(){
	int pagina;
	int desplazamiento;
	int espacioRestante = TAM_PAG-8;//Tamaño de PCB es 8 bytes
	//Sabemos que lo primero que se guarda de un proceso es la PCB
	//Por tanto tiene un porcentaje de la primera pagina ocupada
	if(espacioRestante>0){
		pagina = 0;
		desplazamiento = espacioRestante;
	}else{//Sabemos que las paginas seran de minimo 8 bytes
		pagina=1;
		desplazamiento = 0;
	}
	uint32_t direccionLogica = (uint32_t) (pagina*100 + desplazamiento);
	return direccionLogica;
}

t_proceso* buscar_patota(int id_patota) {

	bool idIgualA(void* algo)
	    {
		t_proceso *patotaBuscada = (t_proceso*) algo;
	        bool a;

	        a = patotaBuscada->pid == id_patota;

	        return a;
	    }

		pthread_mutex_lock(&mutexTablaProcesos);
	    t_proceso* patota = list_find(patotas, idIgualA);
	    pthread_mutex_unlock(&mutexTablaProcesos);

	    if(patota == NULL)
	    {
	        log_error(logs_ram,"Tabla de pagina de patota %d no encontrada!!", id_patota);
	        exit(1);
	    }

	    return patota;
}

int guardar_TCB_pag(TripuCB* tcb, int idPatota) {
	t_proceso* proceso = buscar_patota(idPatota);
	tcb->pcb = 00;//Habria que revisar esto pero entiendo que la PCB es siempre 00 el inicio de cada tabla de pagina.
	tcb->proxIns = buscar_inicio_tareas(proceso);
	existencia_patota(proceso);

	int res = asignar_paginas_en_tabla((void*) tcb, proceso,TCB);
	if(res == 0) {
		return -1;
	} else {
		return 1;
	}

	//Podria devolver la siguiente tarea para ir cargando secuencialmente la proxIns para los tripulantes creados.

}

int guardar_PCB_pag(PatotaCB* pcbAGuardar, char* tareas){
	int pcbGuardado, tareasGuardadas;
	t_proceso* patota = malloc(sizeof(t_proceso));
	patota->pid = pcbAGuardar->pid;
	patota->tabla = list_create();

	pthread_mutex_lock(&mutexTablaProcesos);
	list_add(patotas, patota);
	pthread_mutex_unlock(&mutexTablaProcesos);

	log_info(logs_ram, "Se creo la tabla de paginas para la patota: %d", pcbAGuardar->pid);

	pcbAGuardar->tareas = calcuar_DL_tareas_pag();

	pcbGuardado = asignar_paginas_en_tabla((void*) pcbAGuardar, patota,PCB);

	log_info(logs_ram, "La direccion logica de las tareas es: %d", pcbAGuardar->tareas);

	tareasGuardadas = asignar_paginas_en_tabla((void*) tareas, patota,TAREAS);

	free(pcbAGuardar);
	free(tareas);

	return pcbGuardado && tareasGuardadas;
}

bool tiene_pagina_estructura_alojadas(t_list* estructuras_alojadas, int estructura){
	bool contieneTipo(t_alojado* estructuraAlojada){
		return estructuraAlojada->tipo == estructura;
	}

	//pthread_mutex_lock(&mutexAlojados);
	t_alojado* alojadoConTarea = list_find(estructuras_alojadas, (void*) contieneTipo);
	//pthread_mutex_unlock(&mutexAlojados);

	return alojadoConTarea != NULL;
}


bool tiene_pagina_tripu_alojado(t_list* estructuras_alojadas, int id_tripulante){
	//pthread_mutex_lock(&mutexAlojados);
	t_alojado* alojadoConTarea = obtener_tripulante_pagina(estructuras_alojadas, id_tripulante);
	//pthread_mutex_unlock(&mutexAlojados);

	return alojadoConTarea != NULL;
}

t_alojado* obtener_tripulante_pagina(t_list* estructuras_alojadas, int id_tripulante){

	bool contieneTipo(t_alojado* estructuraAlojada){
		return estructuraAlojada->tipo == TCB && estructuraAlojada->flagid == id_tripulante;
	}

	//pthread_mutex_lock(&mutexAlojados);
	t_alojado* alojadoConTarea = list_find(estructuras_alojadas, (void*) contieneTipo);
	//pthread_mutex_unlock(&mutexAlojados);

	return alojadoConTarea;
}

int actualizar_tripulante_EnMem_pag(t_proceso* proceso, TripuCB* tcb) {

	t_list* tablaPaginasConTripu = lista_paginas_tripulantes(proceso->tabla, tcb->tid);

	return sobreescribir_tripulante(tablaPaginasConTripu, tcb);
}

t_list* lista_paginas_tripulantes(t_list* tabla_paginas_proceso, uint32_t id_tripulante){
	bool tieneTripu(t_pagina* pagina)
	{
		return obtener_tripulante_pagina(pagina->estructuras_alojadas, id_tripulante);
	}

	pthread_mutex_lock(&mutexTablaPatota);
	t_list* tablaPaginasConTripu = list_filter(tabla_paginas_proceso, (void*) tieneTripu);
	pthread_mutex_unlock(&mutexTablaPatota);

	return tablaPaginasConTripu;
}


int sobreescribir_tripulante(t_list* lista_paginas_tripulantes, TripuCB* tcb) {

	int aMeter, relleno, offset = 0;
	void* bufferAMeter = meterEnBuffer(tcb, TCB, &aMeter, &relleno);

	int cantPaginasConTripu = list_size(lista_paginas_tripulantes);

	if(cantPaginasConTripu == 0) {
		log_error(logs_ram, "No hay paginas que contengan al tripulante %d en memoria" , tcb->tid);
		free(lista_paginas_tripulantes);
		return 0;
	}

	log_info(logs_ram, "La cantidad de paginas que contienen al tripulante %d son %d", tcb->tid, cantPaginasConTripu);
	int i = 0;

	while(i < cantPaginasConTripu)
	{
		t_pagina* pagina = list_get(lista_paginas_tripulantes,i);
		pthread_mutex_lock(&mutexAlojados);
		t_alojado* alojado = obtener_tripulante_pagina(pagina->estructuras_alojadas, tcb->tid);
		pthread_mutex_unlock(&mutexAlojados);

		log_info(logs_ram, "Se va a sobreescrbir el tripulante: ID: %d | ESTADO: %c | X: %d | Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
				tcb->tid, tcb->status, tcb->posX, tcb->posY, tcb->proxIns, tcb->pcb);

		sobreescribir_memoria(pagina->nro_frame_mpal, bufferAMeter + offset, MEM_PPAL, alojado->base, alojado->tamanio);//TODO
		offset += alojado->tamanio;
		i++;
	}
	list_destroy(lista_paginas_tripulantes);
	free(bufferAMeter);

	return 1;
}

int actualizar_tripulante_pag(TripuCB* tcb, int idPatota) {

	t_proceso* proceso = buscar_patota(idPatota);

	existencia_patota(proceso);

	t_list* paginasConTripulante = lista_paginas_tripulantes(proceso->tabla, tcb->tid);

	int cantPaginasConTripu = list_size(paginasConTripulante);

		if(cantPaginasConTripu == 0) {
			log_error(logs_ram, "No hay paginas que contengan al tripulante %d en memoria" , tcb->tid);
			free(paginasConTripulante);
			return 0;
		}

		log_info(logs_ram, "La cantidad de paginas que contienen al tripulante %d son %d", tcb->tid, cantPaginasConTripu);
		int i = 0;

		void* bufferTripu = malloc(21);
		int offset = 0;

		while(i < cantPaginasConTripu)
		{
			t_pagina* pagina = list_get(paginasConTripulante,i);
			pthread_mutex_lock(&mutexAlojados);
			t_alojado* alojado = obtener_tripulante_pagina(pagina->estructuras_alojadas, tcb->tid);
			pthread_mutex_unlock(&mutexAlojados);


			pthread_mutex_lock(&mutexTablaPatota);
			void* pagina_mem = leer_memoria_pag(pagina->nro_frame_mpal, MEM_PPAL);
			pthread_mutex_unlock(&mutexTablaPatota);

			log_info(logs_ram, "SE LEE DEL TRIPULANTE: %d - FRAME: %d | D_INCIAL: %d | TAMANIO: %d", tcb->tid,
					pagina->nro_frame_mpal, alojado->base, alojado->tamanio);

			if(pagina_mem != NULL) {
			memcpy(bufferTripu + offset,pagina_mem + alojado->base, alojado->tamanio);
			offset += alojado->tamanio;
			i++;
			free(pagina_mem);
			}
			else {
				log_error(logs_ram, "Se leyo mal la pagina mi bro");
				free(paginasConTripulante);
				free(bufferTripu);
				return 0;
			}
		}

		cargarDLTripulante(bufferTripu, tcb);//TODO

		log_info(logs_ram, "Se va a actualizar el tripulante: ID: %d | ESTADO: %c | X: %d | Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
		tcb->tid, tcb->status, tcb->posX, tcb->posY, tcb->proxIns, tcb->pcb);

		free(bufferTripu);
		list_destroy(paginasConTripulante);
		return actualizar_tripulante_EnMem_pag(proceso, tcb);
}

TripuCB* obtener_tripulante(t_proceso* proceso, int tid) {

	t_list* paginasConTripulante = lista_paginas_tripulantes(proceso->tabla, tid);

	int cantPaginasConTripu = list_size(paginasConTripulante);

	if(cantPaginasConTripu == 0) {
		log_error(logs_ram, "No hay paginas que contengan al tripulante %d en memoria" , tid);
		return 0;
	}

	log_info(logs_ram, "La cantidad de paginas que contienen al tripulante %d son %d", tid, cantPaginasConTripu);
	int i = 0;

	void* bufferTripu = malloc(21);
	int offset = 0;

	while(i < cantPaginasConTripu)
	{
		t_pagina* pagina = list_get(paginasConTripulante,i);
		pthread_mutex_lock(&mutexAlojados);
		t_alojado* alojado = obtener_tripulante_pagina(pagina->estructuras_alojadas, tid);
		pthread_mutex_unlock(&mutexAlojados);

		pthread_mutex_lock(&mutexTablaProcesos);
		void* pagina_memoria = leer_memoria_pag(pagina->nro_frame_mpal, MEM_PPAL);
		pthread_mutex_unlock(&mutexTablaProcesos);

		log_info(logs_ram, "SE LEE DEL TRIPU: %d - FRAME: %d | D_INCIAL: %d | TAMANIO: %d", tid,
				pagina->nro_frame_mpal, alojado->base, alojado->tamanio);

		memcpy(bufferTripu + offset,pagina_memoria + alojado->base, alojado->tamanio);
		offset += alojado->tamanio;

		i++;
		free(pagina);
	}

	TripuCB* tcb = transformarEnTripulante(bufferTripu);
	free(bufferTripu);
	list_destroy(paginasConTripulante);

	return tcb;
}

char* asignar_prox_tarea_pag(int pid,int tid) {

	t_proceso* proceso = buscar_patota(pid);

	existencia_patota(proceso);

	pthread_mutex_lock(&mutexTablaProcesos);
	log_info(logs_ram,"Se encontro la tabla de paginas_ PATOTA: %d - CANT PAGINAS: %d",
					proceso->pid, list_size(proceso->tabla));
	pthread_mutex_unlock(&mutexTablaProcesos);

	TripuCB* tcb = obtener_tripulante(proceso, tid);

	log_info(logs_ram, "Tripulante a asignar proxima tarea: ID: %d | ESTADO: %c | POS_X: %d | POS_Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
			tcb->tid, tcb->status, tcb->posX, tcb->posY, tcb->proxIns, tcb->pcb);

	char* tarea = obtener_siguiente_tarea_pag(proceso, tcb);

	free(tcb);
	return tarea;
}

void expulsar_tripulante_pag(int tid,int pid) {

	t_proceso* proceso = buscar_patota(pid);
	existencia_patota(proceso);
	t_list* paginasTripu = lista_paginas_tripulantes(proceso->tabla, tid);

	log_info(logs_ram, "Se va a eliminar el tripulante %d de la patota %d",tid, proceso-pid);

	if(paginasTripu != NULL)
	{
		t_list_iterator* iteradorPaginas = list_iterator_create(paginasTripu);

		while(list_iterator_has_next(iteradorPaginas))
		{
			t_pagina* paginaActual = list_iterator_next(iteradorPaginas);
			pthread_mutex_lock(&mutexAlojados);
			t_alojado* tripuAlojado = obtener_tripulante_pagina(paginaActual->estructuras_alojadas, tid);

			paginaActual->tam_disponible += tripuAlojado->tamanio;

			log_info(logs_ram,"Se va a sacar de la lista de alojados de cant %d el tripu %d",
					list_size(paginaActual->estructuras_alojadas), tripuAlojado->flagid);
			pthread_mutex_unlock(&mutexAlojados);

			if(paginaActual->estructuras_alojadas == NULL) {
				log_error(logs_ram,"No hay na aca");
			}

			bool tripuConID(t_alojado* alojado) {
				return alojado->tipo == TCB && alojado->flagid == tid;
			}

			pthread_mutex_lock(&mutexAlojados);
			list_remove_by_condition(paginaActual->estructuras_alojadas,(void*) tripuConID);
			pthread_mutex_unlock(&mutexAlojados);

			//reducirIndiceAlojados(paginaActual->estructurasAlojadas);

			log_info(logs_ram, "Se elimino el dato del TRIPULANTE %d en la PAGINA: %d",tid, paginaActual->nro_pagina);

			pthread_mutex_lock(&mutexTablaProcesos);
			log_info(logs_ram,"PAGINA: %d - BYTES DISPONIBLES: %d",paginaActual->nro_pagina,paginaActual->tam_disponible);
			pthread_mutex_unlock(&mutexTablaProcesos);


			free(tripuAlojado);

			pthread_mutex_lock(&mutexTablaProcesos);
			if(paginaActual->tam_disponible == 32)
			{
				pthread_mutex_unlock(&mutexTablaProcesos);
				log_info(logs_ram,"Pagina %d vacia se procede a liberar el frame y borrarla de tabla",paginaActual->nro_pagina);
				liberar_marco(paginaActual->nro_frame_mpal, MEM_PPAL);

				bool paginaConID(t_alojado* pagina)
				{
					return pagina->nro_estructura == paginaActual->nro_pagina;
				}

				pthread_mutex_lock(&mutexTablaProcesos);
				list_remove_by_condition(proceso->tabla, (void*) paginaConID);
				pthread_mutex_unlock(&mutexTablaProcesos);

				pthread_mutex_lock(&mutexAlojados);
				list_destroy(paginaActual->estructuras_alojadas);
				pthread_mutex_unlock(&mutexAlojados);

				free(paginaActual);
			}
			else {
				pthread_mutex_unlock(&mutexTablaProcesos);
			}
		}

		list_destroy(paginasTripu);
		chequear_ultimo_tripulante(proceso);
		list_iterator_destroy(iteradorPaginas);
	}
}

bool proceso_tiene_tripulantes_(t_proceso* proceso) {

	bool _esTCB(void* alojado){
		t_alojado *unaEstructura = (t_alojado*) alojado;
		return unaEstructura->tipo == TCB;
	}

	bool _tieneAlgunTripulante(void* pagina){
		t_pagina *unaPagina = (t_pagina*) pagina;
		return list_any_satisfy(unaPagina->estructuras_alojadas, _esTCB);
	}

	pthread_mutex_lock(&mutexTablaPatota);
	bool a = list_any_satisfy(proceso->tabla, (void*) _tieneAlgunTripulante);
	pthread_mutex_unlock(&mutexTablaPatota);

	return a;
}

void chequear_ultimo_tripulante(t_proceso* proceso) {

	if(!proceso_tiene_tripulantes(proceso)) {

		log_info(logs_ram,"La patota %d no tiene mas tripulantes. Se procede a borrarla de memoria", proceso->pid);

		void borrarProceso(void* algo)
		{
			t_pagina *unaPagina = (t_pagina*) algo;

			void borrarAlojados(void* alojado)
			{
				t_alojado *unaEstructura = (t_alojado*) alojado;
				free(unaEstructura);
			}

			pthread_mutex_lock(&mutexAlojados);
			list_destroy_and_destroy_elements(unaPagina->estructuras_alojadas, borrarAlojados);
			pthread_mutex_unlock(&mutexAlojados);
			log_info(logs_ram,"SE LIBERA  EL FRAME: %d",unaPagina->nro_pagina);
			liberar_marco(unaPagina->nro_frame_mpal, MEM_PPAL);
			free(unaPagina);
		}

		bool _esElProceso(void* algo) {
			t_proceso *unProceso = (t_proceso*) algo;
			return unProceso->pid == proceso->pid;
		}

		pthread_mutex_lock(&mutexTablaPatota);
		list_remove_by_condition(patotas, (void*) _esElProceso);
		pthread_mutex_unlock(&mutexTablaPatota);

		pthread_mutex_lock(&mutexTablaProcesos);
		list_destroy_and_destroy_elements(proceso->tabla, (void*) borrarProceso);
		pthread_mutex_unlock(&mutexTablaProcesos);

		free(proceso);
	}
}

t_proceso* patota_que_tiene_el_frame(int frame){
		//Antes llamada frame_con_patota
	bool _tieneElFrame(void* algo){
		t_pagina *unaPagina = (t_pagina*) algo;
		return unaPagina->nro_frame_mpal == frame;
	}

	bool _frameEnUso(void* algo) {
		t_proceso *unProceso = (t_proceso*) algo;
		return list_any_satisfy(unProceso->tabla, _tieneElFrame);
	}

	pthread_mutex_lock(&mutexTablaPatota);
	t_proceso* procesoEnFrame = list_find(patotas,(void*) _frameEnUso);
	pthread_mutex_unlock(&mutexTablaPatota);

	return procesoEnFrame;
}


t_pagina* frame_con_pagina(int frame, t_proceso* proceso) {

	bool esLaPagina(void* algo){
		t_pagina *unaPagina = (t_pagina*)algo;
		return unaPagina->nro_frame_mpal == frame;
	}

	return list_find(proceso->tabla, esLaPagina);
}

/*
void dumpPag() {

	char* nombreArchivo = temporal_get_string_time("DUMP_%y%m%d%H%M%S%MS.dmp");
	char* rutaAbsoluta = string_from_format("/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/Dump/%s",nombreArchivo);

	FILE* archivoDump = txt_open_for_append(rutaAbsoluta);

	if(archivoDump == NULL){
		log_error(logs_ram, "No se pudo abrir el archivo correctamente");
		exit(1);
	}

	char* fechaYHora = temporal_get_string_time("%d/%m/%y %H:%M:%S \n");
	char* dump = string_from_format("DUMP: %s",fechaYHora);

	txt_write_in_file(archivoDump, "--------------------------------------------------------------------------\n");
	txt_write_in_file(archivoDump, dump);

	for(int i=0; i< cantidadDeFrames; i++) { //cant_frames en memoria principal tiene que ser una variable global cuando se inicializa la memoria.

		t_proceso* proceso = patotaConFrame(i);

		if(proceso != NULL)
		{
			t_pagina* info_pagina = paginaConFrame(i,proceso);

			char* dumpMarco = string_from_format("Marco:%d    Estado:Ocupado    Proceso:%d    Pagina:%d \n",
					i, proceso->pid, info_pagina->nro_pagina);

			txt_write_in_file(archivoDump, dumpMarco);
			free(dumpMarco);
		}
		else
		{
			char* dumpMarco = string_from_format("Marco:%d    Estado:Libre      Proceso:-    Pagina:- \n",i);
			txt_write_in_file(archivoDump, dumpMarco);
			free(dumpMarco);
		}
	}

	txt_write_in_file(archivoDump, "--------------------------------------------------------------------------\n");

	txt_close_file(archivoDump);
	free(rutaAbsoluta);
	free(dump);
}*/


void dividir_memoria_en_frames() {
	cantidadDeFrames = TAM_MEM/TAM_PAG;
}


void* meterEnBuffer(void* bytesAGuardar, int estructura, int* aMeter, int* flagid){
	if(estructura==PCB){
		*flagid = -1;
		*aMeter = sizeof(PatotaCB);
	}else if(estructura == TAREAS){
		*flagid = -1;
		//int tamañoTareas = sizeof(bytesAGuardar);
		*aMeter = sizeof(bytesAGuardar);
	}else{
		*flagid = 1;
		//*flagid = ???
		*aMeter = sizeof(TripuCB);
	}
	void *buffer = bytesAGuardar;
	return buffer;
}

TripuCB* transformarEnTripulante(void* buffer){
	TripuCB *elTripulante = NULL;
	if(sizeof(buffer) == 21){
		elTripulante = (TripuCB*) buffer;
	}
	return elTripulante;
}

void sobreescribir_memoria(int frame, void* buffer, int mem, int desplazPagina, int bytesAEscribir) {


	int desplazamiento = frame * TAM_PAG + desplazPagina;

	if(mem == MEM_PPAL)
	{
		pthread_mutex_lock(&mutexEscribiendoMemoria);
		memcpy(memoria+desplazamiento, buffer, bytesAEscribir);
		pthread_mutex_unlock(&mutexEscribiendoMemoria);

		log_info(logs_ram, "Se sobreescribio en RAM: FRAME: %d | DESDE: %d | HASTA: %d ", frame,
				desplazPagina, bytesAEscribir + desplazPagina -1);
	}else if(mem == MEM_VIRT){
		//TODO:Hacer para memoria virtual
	}
}

/*

t_pagina* crear_pagina(){
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->bit_uso=1;
	pagina->tam_disponible = TAM_PAG;
	return pagina;
}



/*
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





