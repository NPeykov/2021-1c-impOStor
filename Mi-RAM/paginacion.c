#include "paginacion.h"
#include "mi-ram.h"
#include "memoria-virtual.c"

//----------------------------------------------------------
//--------------FUNCIONES DE MARCOS-------------------------
//----------------------------------------------------------

bool traer_marco_valido(int frame, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		return bitarray_test_bit(frames_ocupados_ppal,(off_t) frame);
	}
	else {
		return bitarray_test_bit(BIT_ARRAY_SWAP,(off_t) frame);
	}
}


void asignar_marco_en_uso(int frame, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		bitarray_set_bit(frames_ocupados_ppal, frame);
	}else{
		bitarray_set_bit(BIT_ARRAY_SWAP, frame);
	}
}


void liberar_marco(int frame, int tipo_memoria)
{
	if(tipo_memoria == MEM_PPAL) {
		bitarray_clean_bit(frames_ocupados_ppal, frame);
	}
	else {
		bitarray_clean_bit(BIT_ARRAY_SWAP, frame);
	}
}

bool marco_vacio(int marco, int tipo_memoria){
	if(tipo_memoria == MEM_PPAL){
		int estadoFrame = bitarray_test_bit(frames_ocupados_ppal, marco);
		return (estadoFrame == LIBRE);
	}else{
		int estadoFrame = bitarray_test_bit(BIT_ARRAY_SWAP, marco);
		return (estadoFrame == LIBRE);
	}

}


uint32_t buscar_marco_disponible(int tipo_memoria){
	int size;
	if(tipo_memoria == MEM_PPAL){
		size = cantidadDeFrames;
	}else{
		size = marcos_en_swap;
	}

	for(int m = 0; m < size; m++){
		if(!traer_marco_valido(m, tipo_memoria)) {
			return m;
		}
	}

	log_info(logs_ram, "No se encontro un frame disponible para memoria %d. (0=Ppal - 1=Virtual)",tipo_memoria );
	return -1;
}

void dividir_memoria_en_frames() {
	TAM_MEM = tamaniomemoria;

	cantidadDeFrames = TAM_MEM/TAM_PAG;
	int cantBytes = (double)(cantidadDeFrames/8);
	if(cantBytes<1){
		cantBytes=1;
	}
	void* puntero_a_bits = malloc(cantBytes);

	frames_ocupados_ppal = bitarray_create(puntero_a_bits, cantBytes);
	for(int i = 0; i < cantidadDeFrames; i++){
		bitarray_clean_bit(frames_ocupados_ppal, i);
	}
}

//-------------------------------------------------------------------------
//-----------------------FUNCIONES DE PAGINAS------------------------------
//-------------------------------------------------------------------------

void agregar_estructura_a_pagina(t_pagina* pagina,int desplazamiento_pag, int bytesAlojados, int estructura, int flag){

	t_alojado* nueva_estructura_pag = malloc(sizeof(t_alojado));
	log_info(logs_ram, "Se va a agregar a la pagina %d , la estructura %d con base %d , y tamanio %d", pagina->nro_pagina,
			estructura,desplazamiento_pag, bytesAlojados);
	nueva_estructura_pag->nro_estructura = list_size(pagina->estructuras_alojadas);
	nueva_estructura_pag->base = desplazamiento_pag;
	nueva_estructura_pag->tamanio = bytesAlojados;
	nueva_estructura_pag->tipo = estructura;
	nueva_estructura_pag->flagid = flag;
	if(estructura == TCB){
		nueva_estructura_pag->caracterRep = caracterRepresentativo;
		pthread_mutex_unlock(&charRepresentativo);
	}
	list_add(pagina->estructuras_alojadas, nueva_estructura_pag);
}

t_pagina* crear_pagina_en_tabla(t_proceso* proceso,int estructura){

	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->nro_pagina = list_size(proceso->tabla);
	pagina->pid = proceso->pid;
	pagina->tam_disponible = TAM_PAG;
	pagina->estructuras_alojadas = list_create();
	pagina->nro_frame_mpal = buscar_marco_disponible(MEM_PPAL);
	log_info(logs_ram, "Se asigno el frame %d a la pagina %d",pagina->nro_frame_mpal,pagina->nro_pagina);
	if(pagina->nro_frame_mpal == -1){
		pagina->bit_presencia = false;
		pagina->bit_uso=false;
	}else{
		if(esLRU){
			log_info(logs_ram, "Agregue la pagina %d al LRU", pagina->nro_pagina);
			pthread_mutex_lock(&listaLRU);
			list_add(paginas_lru, pagina);
			pthread_mutex_unlock(&listaLRU);
		}
		pagina->bit_presencia = true;
		pagina->bit_uso=true;
		pagina->nro_frame_swap=-1;
		asignar_marco_en_uso(pagina->nro_frame_mpal,MEM_PPAL);
	}


	log_info(logs_ram, "Se creo la pagina de la estructura: %d", estructura);

	//Agrega la pagina al final de la lista
	list_add(proceso->tabla, pagina);

	return pagina;
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

//------------------------------------------------------------------------------------
//-------------------------FUNCIONES DE MANEJO DE MEMORIA-----------------------------
//------------------------------------------------------------------------------------

void* leer_memoria_pag(int frame, int mem){

	int desplazamiento = frame * TAM_PAG;

	void* pagina = malloc(TAM_PAG);
	if(mem == MEM_PPAL){
		log_info(logs_ram, "Se va a leer la pagina que arranca en %d", desplazamiento);
		memcpy(pagina, memoria+desplazamiento, TAM_PAG);
	}

	return pagina;
}

int insertar_en_memoria_pag(t_pagina* pagina, void* datos, int* bytesAInsertar,  int estructura, int* bytesEscritos, int flag){
		pthread_mutex_lock(&mutexAlojados);
		int desplazamiento_pag = TAM_PAG - pagina->tam_disponible;
		int desplazamiento_mem = pagina->nro_frame_mpal * TAM_PAG + desplazamiento_pag;
		int bytesAEscribir = pagina->tam_disponible - *bytesAInsertar;

		if(bytesAEscribir < 0){
			bytesAEscribir = pagina->tam_disponible;
			pagina->tam_disponible = 0;
		}else {
			if(estructura == TAREAS){ flag= 1;}
			bytesAEscribir = *bytesAInsertar;
			pagina->tam_disponible = pagina->tam_disponible - *bytesAInsertar;
		}

		agregar_estructura_a_pagina(pagina, desplazamiento_pag, bytesAEscribir, estructura, flag);
		pthread_mutex_unlock(&mutexAlojados);

		pthread_mutex_lock(&mutexEscribiendoMemoria);
		memcpy(memoria + desplazamiento_mem, datos, bytesAEscribir);
		pthread_mutex_unlock(&mutexEscribiendoMemoria);

		*bytesAInsertar -= bytesAEscribir;

		*bytesEscritos = bytesAEscribir;

		return 1;
}


int insertar_en_paginas(void* bytesAGuardar, t_proceso* proceso, int estructura, int aMeter){
	void* bufferAMeter = bytesAGuardar;
	void* siguienteAEscribir = bufferAMeter;
	int bytesEscritos;
	t_pagina* pagina;
	int flag;
	if(estructura == TCB){
		TripuCB *elTripu = (TripuCB*) bytesAGuardar;
		flag = elTripu->tid;
	}

	while(aMeter > 0)
	{
		if(estructura != PCB){//Para Tareas y Tripulantes
			pagina = buscar_ultima_pagina_disponible(proceso);

			if(pagina != NULL && pagina->nro_frame_mpal != -1){//Tiene espacio y esta en memoria
				insertar_en_memoria_pag(pagina, siguienteAEscribir, &aMeter, estructura, &bytesEscritos, flag);
			}else if(pagina != NULL && pagina->nro_frame_mpal == -1){//Tiene espacio pero esta en swap
				traer_pagina(pagina);
				insertar_en_memoria_pag(pagina, siguienteAEscribir, &aMeter, estructura, &bytesEscritos, flag);
			}else{
				pagina = crear_pagina_en_tabla(proceso,estructura);

				if(pagina->nro_frame_mpal != -1){//No tiene espacio y hay espacio en mmpal
					insertar_en_memoria_pag(pagina, siguienteAEscribir, &aMeter, estructura, &bytesEscritos,flag);
				}else{//No tiene espacio y NO hay espacio en mmpal
					log_info(logs_ram, "Memoria principal llena, realizando swap.");
					asignar_marco_en_swap(pagina);
					traer_pagina(pagina);
					insertar_en_memoria_pag(pagina, siguienteAEscribir, &aMeter, estructura, &bytesEscritos, flag);
				}
			}
		}else{//Para patota
			//Si es PCB se hace esto ya que sabemos que la PCB es lo primero que se guarda
			pagina = crear_pagina_en_tabla(proceso, estructura);
			flag = -1;

			if(pagina->nro_frame_mpal != -1){
				insertar_en_memoria_pag(pagina, siguienteAEscribir, &aMeter ,estructura ,&bytesEscritos,flag);
			}else{
				asignar_marco_en_swap(pagina);
				traer_pagina(pagina);
				insertar_en_memoria_pag(pagina, siguienteAEscribir, &aMeter, estructura, &bytesEscritos, flag);
			}
		}
		siguienteAEscribir += bytesEscritos;
	}
	return 1;
}

void existencia_patota(t_proceso* proceso) {
	if(proceso == NULL) {
		log_error(logs_ram, "Este proceso es inexistente");
		exit(1);
	}
}

char* obtener_siguiente_tarea_pag(t_proceso* proceso, uint32_t *proxIns,int idTripulante) {
	char* tarea = string_new();
	char* aux = malloc(2);
	*(aux+1) = '\0';
	void* paginaAGuardar;
	void* recorredorPagina;
	int indicePagina = (int) *proxIns / 100.0;
	int desplazamiento = *proxIns % 100;
	t_pagina* pagina;
	log_info(logs_ram, "La proxima instruccion esta en la pagina %d y desplazamiento %d", indicePagina, desplazamiento);
	if(*proxIns == -1){
		log_info(logs_ram, "El tripulante no tiene mas tareas");
		return "null";
	}

	pthread_mutex_lock(&mutexEscribiendoMemoria);
	while(true){
		pagina = list_get(proceso->tabla, indicePagina);
		if(tiene_pagina_estructura_alojadas(pagina->estructuras_alojadas, TAREAS))
		{
			traer_pagina(pagina);
			paginaAGuardar = leer_memoria_pag(pagina->nro_frame_mpal, MEM_PPAL);
			recorredorPagina = paginaAGuardar;
			recorredorPagina += desplazamiento;
			memcpy(aux,recorredorPagina,1);

			while(desplazamiento != TAM_PAG && *aux != '\n'  && *aux != '\0')
			{
				string_append(&tarea,aux);
				recorredorPagina++;
				desplazamiento++;
				memcpy(aux,recorredorPagina,1);
			}
			free(paginaAGuardar);
		}
		if(desplazamiento == TAM_PAG){desplazamiento=0;indicePagina++;*aux = 'a';}
		if(*aux == '\n' || *aux == '\0') break;
	}
	if(*aux == '\0'){
		*proxIns = -1;
	}else{
		*proxIns = indicePagina*100 + desplazamiento+1;
	}

	log_info(logs_ram, "La proxima tarea es %d",*proxIns);
	pthread_mutex_unlock(&mutexEscribiendoMemoria);

	t_list* paginasConTripulante = lista_paginas_tripulantes(proceso->tabla, idTripulante);
	escribir_dato_tripulante(paginasConTripulante,desplazAProxIns,sizeof(uint32_t),idTripulante,(void*)proxIns);
	//sobreescribir_tripulante(paginasConTripulante, tcb,proceso->pid);
	free(aux);
	free(proxIns);

	return tarea;
}

//-----------------------------------------------------------------------
//---------------------MANEJO DE DIRECCIONES LOGICAS---------------------
//-----------------------------------------------------------------------

uint32_t buscar_inicio_tareas(t_proceso* proceso) {


    bool buscarDLTarea(t_pagina* pagina) {
    	bool a = tiene_pagina_estructura_alojadas(pagina->estructuras_alojadas, TAREAS);

    	return a;
    }

    pthread_mutex_lock(&mutexTablaPaginas);
    t_pagina* paginaConTarea = list_find(proceso->tabla, (void*) buscarDLTarea);
    pthread_mutex_unlock(&mutexTablaPaginas);

    bool tieneTarea(t_alojado* estructuraAlojada) {
    	return estructuraAlojada->tipo == TAREAS;
    }

    pthread_mutex_lock(&mutexAlojados);
    t_alojado* alojadoConTarea = list_find(paginaConTarea->estructuras_alojadas, (void*) tieneTarea);
    pthread_mutex_unlock(&mutexAlojados);


    //Retornar un struct DL de las tareas que tiene el indice de la pagina y el desplazamiento en esta
    //Se guarda en algun lado cuanto pesa el string

    int a = paginaConTarea->nro_pagina*100 + alojadoConTarea->base;
    return (uint32_t)a;
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
	t_proceso *patota = NULL;

	bool idIgualA(void* algo)
	{
		t_proceso *patotaBuscada = (t_proceso*) algo;
		bool a;

		a = patotaBuscada->pid == id_patota;

		return a;
	}

	pthread_mutex_lock(&mutexTablaPatotas);
	patota = list_find(patotas, idIgualA);
	pthread_mutex_unlock(&mutexTablaPatotas);

	if(patota == NULL)
	{
		log_error(logs_ram,"Tabla de pagina de patota %d no encontrada!!", id_patota);
		exit(1);
	}

	return patota;
}

//-----------------------------------------------------------------------
//----------------------CREACION Y GUARDADO DE ESTRUCTURAS---------------
//-----------------------------------------------------------------------

int guardar_TCB_pag(void* algo) {
	TripulanteConSocket *tripulanteConSocket = (TripulanteConSocket *) algo;
	t_tripulante_iniciado *unTripulante = tripulanteConSocket->tripulante;
	int _socket_cliente = tripulanteConSocket->socket;

	t_proceso* proceso = buscar_patota(unTripulante->numPatota);

	TripuCB *elTripulante = (TripuCB*) malloc(sizeof(TripuCB));
	elTripulante->tid = unTripulante->tid;
	elTripulante->status = unTripulante->status;
	elTripulante->pcb = 00;
	elTripulante->posX = unTripulante->posX;
	elTripulante->posY = unTripulante->posY;
	elTripulante->proxIns = buscar_inicio_tareas(proceso);
	existencia_patota(proceso);
	pthread_mutex_lock(&charRepresentativo);
	caracterRepresentativo = nuevoTripuMapa(elTripulante->posX, elTripulante->posY);
	int res = insertar_en_paginas((void*) elTripulante, proceso,TCB, sizeof(TripuCB));
	if(res == 0) {
		enviar_mensaje_simple("no", _socket_cliente);
		liberar_cliente(_socket_cliente);
		free(elTripulante);
		free(algo);
		return -1;
	} else {

		sem_post(&tripulantesDisponibles);
		enviar_mensaje_simple("ok", _socket_cliente);
		liberar_cliente(_socket_cliente);
		free(algo);
		return 1;
	}
}

int guardar_PCB_pag(void* data){
	t_datos_inicio_patota *datos_patota = (t_datos_inicio_patota*)data;
	char* contenido = datos_patota->contenido_tareas;
	int _socket_cliente = datos_patota->socket;

	int pcbGuardado, tareasGuardadas;
	t_proceso* patota = malloc(sizeof(t_proceso));

	PatotaCB *laPatota = (PatotaCB*) malloc(sizeof(PatotaCB));

	pthread_mutex_lock(&mutexNumeroPatotas);
	patota->pid = numero_patota;
	numero_patota++;
	pthread_mutex_unlock(&mutexNumeroPatotas);

	patota->tabla = list_create();

	pthread_mutex_lock(&mutexTablaPatotas);
	list_add(patotas, patota);
	pthread_mutex_unlock(&mutexTablaPatotas);

	laPatota->tareas = patota->pid;
	laPatota->tareas = calcuar_DL_tareas_pag();

	pcbGuardado = insertar_en_paginas((void*) laPatota, patota,PCB, 8);
	if(!pcbGuardado){
		free(patota);
		free(laPatota);
		free(data);
		enviar_mensaje_simple("no", _socket_cliente);
	}


	int tamanioTareas = string_length(contenido);

	tareasGuardadas = insertar_en_paginas((void*) contenido, patota,TAREAS,tamanioTareas);

	if(!tareasGuardadas){
		free(patota);
		free(laPatota);
		free(data);
		enviar_mensaje_simple("no", _socket_cliente);
	}

	free(data);
	log_info(logs_ram, "Se creo la patota %d correctamente.",patota->pid);
	enviar_mensaje_simple("ok", _socket_cliente);
	liberar_cliente(_socket_cliente);
	return 1;
}

bool tiene_pagina_estructura_alojadas(t_list* estructuras_alojadas, int estructura){
	bool contieneTipo(t_alojado* estructuraAlojada){
		return estructuraAlojada->tipo == estructura;
	}

	t_alojado* alojadoConTarea = list_find(estructuras_alojadas, (void*) contieneTipo);

	return alojadoConTarea != NULL;
}


t_alojado* obtener_tripulante_de_la_pagina(t_list* estructuras_alojadas, int id_tripulante){

	bool _esElTripu(t_alojado* estructuraAlojada){
		return estructuraAlojada->tipo == TCB && estructuraAlojada->flagid == id_tripulante;
	}

	pthread_mutex_lock(&mutexAlojados);
	t_alojado* tripulanteAlojado = list_find(estructuras_alojadas, (void*) _esElTripu);
	pthread_mutex_unlock(&mutexAlojados);

	return tripulanteAlojado;
}

t_list* lista_paginas_tripulantes(t_list* tabla_paginas_proceso, uint32_t id_tripulante){
	bool _tieneTripu(void* algo){
		t_pagina *pagina = (t_pagina*)algo;
		t_alojado *unAlojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, id_tripulante);
		return unAlojado !=NULL;
	}

	pthread_mutex_lock(&mutexTablaPaginas);
	t_list* tablaPaginasConTripu = list_filter(tabla_paginas_proceso, _tieneTripu);
	pthread_mutex_unlock(&mutexTablaPaginas);

	return tablaPaginasConTripu;
}

void actualizar_tripulante_pag(t_tripulante_iniciado *tripulanteActualizado) {
	int idTripulante = tripulanteActualizado->tid;
	int idPatota = tripulanteActualizado->numPatota;

	t_proceso* proceso = buscar_patota(idPatota);
	log_info(logs_ram, "Se va a actualizar al tripu %d de la patota %d", idTripulante, proceso->pid);
	existencia_patota(proceso);

	t_list *paginasConTripulante = lista_paginas_tripulantes(proceso->tabla,(uint32_t) idTripulante);
	if(list_is_empty(paginasConTripulante)){
		log_info(logs_ram, "El tripulante %d de la patota %d no existe", idTripulante, idPatota);
		free(tripulanteActualizado);
		pthread_exit(NULL);
	}
	t_pagina* pagina = list_get(paginasConTripulante, 0);
	t_alojado* alojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, idTripulante);

	pthread_mutex_lock(&mutexEscribiendoMemoria);
	uint32_t *posX = (uint32_t*)obtener_dato_tripulante(paginasConTripulante,desplazAPosX,sizeof(uint32_t),idTripulante);
	uint32_t *posY = (uint32_t*)obtener_dato_tripulante(paginasConTripulante,desplazAPosY,sizeof(uint32_t),idTripulante);
	char* estado =(char*) obtener_dato_tripulante(paginasConTripulante,desplazAEstado,sizeof(char),idTripulante);
	log_info(logs_ram, "El tripulante tenia Estado %c y Posiciones %d|%d",estado[0],*posX,*posY);
	int difX = tripulanteActualizado->posX-*posX;
	int difY = tripulanteActualizado->posY-*posY;
	void *nuevoEstado, *newPosX, *newPosY;
	newPosX = &tripulanteActualizado->posX;
	newPosY = &tripulanteActualizado->posY;
	nuevoEstado = &tripulanteActualizado->status;
	if(difX==0 && difY==0){//Si solo cambio el estado
		escribir_dato_tripulante(paginasConTripulante,desplazAEstado, sizeof(char),idTripulante,nuevoEstado);
	}else if(estado[0] == tripulanteActualizado->status){//Si solo cambiaron las posiciones
		escribir_dato_tripulante(paginasConTripulante,desplazAPosX, sizeof(uint32_t),idTripulante,newPosX);
		escribir_dato_tripulante(paginasConTripulante,desplazAPosY, sizeof(uint32_t),idTripulante,newPosY);
		moverTripuMapa(alojado->caracterRep,  difX,  difY);
	}else{//Si cambiaron ambos
		escribir_dato_tripulante(paginasConTripulante,desplazAPosX, sizeof(uint32_t),idTripulante,newPosX);
		escribir_dato_tripulante(paginasConTripulante,desplazAPosY, sizeof(uint32_t),idTripulante,newPosY);
		escribir_dato_tripulante(paginasConTripulante,desplazAEstado, sizeof(char),idTripulante,nuevoEstado);
		moverTripuMapa(alojado->caracterRep,  difX,  difY);
	}
	pthread_mutex_unlock(&mutexEscribiendoMemoria);
	log_info(logs_ram,"El tripulante %d se ha actualizado: Estado: %c %d|%d",idTripulante,tripulanteActualizado->status,tripulanteActualizado->posX,tripulanteActualizado->posY);

	free(tripulanteActualizado);
	free(posX);
	free(posY);
	free(estado);
	return;
}

void escribir_dato_tripulante(t_list *paginas_del_tripulante, int desplazamientoEnTripulante, int tamanioDato, int idTripulante, void* elDato){
	int i =0;
	void *direccion;
	int espacioPosDesplaz;
	void* punteroRestante = elDato;

	while(tamanioDato >0){
		t_pagina* pagina = list_get(paginas_del_tripulante,i);
		t_alojado* alojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, idTripulante);
		traer_pagina(pagina);
		//Direccion respecto del desplazamiento de ese dato en memoria
		direccion = memoria + pagina->nro_frame_mpal * TAM_PAG+alojado->base +desplazamientoEnTripulante;
		espacioPosDesplaz = TAM_PAG - (alojado->base+desplazamientoEnTripulante);

		if(espacioPosDesplaz > tamanioDato){
			memcpy(direccion,punteroRestante , tamanioDato);
			tamanioDato -= tamanioDato;
		}else if(espacioPosDesplaz >0){
			memcpy(direccion,punteroRestante ,espacioPosDesplaz);
			punteroRestante +=espacioPosDesplaz;
			tamanioDato -=espacioPosDesplaz;
		}
		desplazamientoEnTripulante -=alojado->tamanio;
		direccion=NULL;
		espacioPosDesplaz = 0;
		i++;
	}

}

void *obtener_dato_tripulante(t_list *paginas_del_tripulante, int desplazamientoEnTripulante, int tamanioDato, int idTripulante){
	int i =0;
	void* bufferDato = malloc(tamanioDato);
	void *direccion;
	int espacioPosDesplaz;


	while(tamanioDato >0){
		t_pagina* pagina = list_get(paginas_del_tripulante,i);
		t_alojado* alojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, idTripulante);
		traer_pagina(pagina);

		//Direccion respecto del desplazamiento de ese dato en memoria
		direccion = memoria + pagina->nro_frame_mpal * TAM_PAG+alojado->base +desplazamientoEnTripulante;
		espacioPosDesplaz = TAM_PAG - (alojado->base+desplazamientoEnTripulante);

		if(espacioPosDesplaz > tamanioDato){
			memcpy(bufferDato, direccion, tamanioDato);
			tamanioDato -= tamanioDato;
		}else if(espacioPosDesplaz >0){
			memcpy(bufferDato, direccion,espacioPosDesplaz);
			tamanioDato -=espacioPosDesplaz;
		}
		desplazamientoEnTripulante -=alojado->tamanio;
		direccion=NULL;
		espacioPosDesplaz = 0;
		i++;
	}

	return bufferDato;
}

void asignar_prox_tarea_pag(void *unTripulante) {
	TripulanteConSocket *elTripuConSocket = (TripulanteConSocket*) unTripulante;
	int cliente = elTripuConSocket->socket;
	t_tripulante_iniciado *tripulante = (t_tripulante_iniciado*) elTripuConSocket->tripulante;
	int idTripulante = tripulante->tid;
	int idPatota = tripulante->numPatota;

	t_proceso* proceso = buscar_patota(idPatota);

	existencia_patota(proceso);

	t_list *paginasConTripulante = lista_paginas_tripulantes(proceso->tabla,(uint32_t) idTripulante);
	uint32_t *proximaInstruccion = (uint32_t*) obtener_dato_tripulante(paginasConTripulante,desplazAProxIns,sizeof(uint32_t), idTripulante);
	if(proximaInstruccion == NULL){
		log_info(logs_ram, "entre aca");
		sem_wait(&tripulantesDisponibles);
		proximaInstruccion = (uint32_t*) obtener_dato_tripulante(paginasConTripulante,desplazAProxIns,sizeof(uint32_t),idTripulante);
	}
	log_info(logs_ram,"Insutrccion:%d", *proximaInstruccion);

	char* tarea = obtener_siguiente_tarea_pag(proceso, proximaInstruccion,idTripulante);

	log_info(logs_ram, "El tripulante %d pidio la tarea %s", idTripulante, tarea);

	enviar_mensaje(PEDIDO_TAREA, tarea, cliente);
	liberar_cliente(cliente);
}

void expulsar_tripulante_pag(void* algo) {
	IdentificadorTripulante *tripulanteAEliminar = (IdentificadorTripulante*) algo;
	int tid = tripulanteAEliminar->idTripulante;
	int pid = tripulanteAEliminar->idPatota;

	t_proceso* proceso = buscar_patota(pid);
	existencia_patota(proceso);

	t_list* paginasTripu = lista_paginas_tripulantes(proceso->tabla, tid);
	pthread_mutex_lock(&mutexTablaPaginas);
	log_info(logs_ram, "Se va a eliminar el tripulante %d de la patota %d",tid, pid);

	void _recorrer_paginas_tripulante(void* algo){
		t_pagina *paginaActual = (t_pagina*) algo;
		if(paginaActual->estructuras_alojadas == NULL) {
			log_error(logs_ram,"No tiene estructuras alojadas esta pagina");
		}

		t_alojado* tripuAlojado = obtener_tripulante_de_la_pagina(paginaActual->estructuras_alojadas, tid);
		paginaActual->tam_disponible += tripuAlojado->tamanio;

		log_info(logs_ram,"Se va a sacar de la lista de alojados de cant %d el tripu %d",
				list_size(paginaActual->estructuras_alojadas), tripuAlojado->flagid);
		bool eliminadoMapa = false;
		bool tripuConID(t_alojado* alojado) {
			if(alojado->tipo == TCB && alojado->flagid == tid){
				if(!eliminadoMapa){
					eliminarTripuMapa(alojado->caracterRep);
					eliminadoMapa = true;
				}
				return true;
			}else{
				return false;
			}
		}
		list_remove_by_condition(paginaActual->estructuras_alojadas,(void*) tripuConID);

		log_info(logs_ram, "Se elimino el dato del TRIPULANTE %d en la PAGINA: %d",tid, paginaActual->nro_pagina);
		log_info(logs_ram,"PAGINA: %d - BYTES DISPONIBLES: %d",paginaActual->nro_pagina,paginaActual->tam_disponible);

		free(tripuAlojado);

		if(paginaActual->tam_disponible == 32)
		{

			log_info(logs_ram,"Pagina %d vacia se procede a liberar el frame y borrarla de tabla",paginaActual->nro_pagina);
			if(paginaActual->nro_frame_mpal != -1){
				liberar_marco(paginaActual->nro_frame_mpal, MEM_PPAL);
			}else{
				liberar_marco(paginaActual->nro_frame_swap, MEM_VIRT);
			}


			bool paginaConID(t_pagina* pagina)
			{
				return pagina->nro_pagina == paginaActual->nro_pagina;
			}
			list_remove_by_condition(proceso->tabla, (void*) paginaConID);

			list_destroy(paginaActual->estructuras_alojadas);

			free(paginaActual);
		}
	}

	if(paginasTripu != NULL){
		list_iterate(paginasTripu,_recorrer_paginas_tripulante);
		pthread_mutex_unlock(&mutexTablaPaginas);
		if(!proceso_tiene_tripulantes(proceso)){limpiarProceso(proceso);};
	}else{
		pthread_mutex_unlock(&mutexTablaPaginas);
		log_error(logs_ram,"Este tripulante no existe en esta patota");
	}
}

bool proceso_tiene_tripulantes(t_proceso* proceso) {

	bool _esTCB(void* alojado){
		t_alojado *unaEstructura = (t_alojado*) alojado;
		return unaEstructura->tipo == TCB;
	}

	bool _tieneAlgunTripulante(void* pagina){
		t_pagina *unaPagina = (t_pagina*) pagina;
		return list_any_satisfy(unaPagina->estructuras_alojadas, _esTCB);
	}

	pthread_mutex_lock(&mutexTablaPaginas);
	bool a = list_any_satisfy(proceso->tabla, (void*) _tieneAlgunTripulante);
	pthread_mutex_unlock(&mutexTablaPaginas);

	return a;
}

void limpiarProceso(t_proceso* proceso) {

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
			if(unaPagina->nro_frame_mpal !=-1){
				liberar_marco(unaPagina->nro_frame_mpal, MEM_PPAL);
			}else{
				liberar_marco(unaPagina->nro_frame_swap, MEM_VIRT);
			}


			free(unaPagina);
		}

		bool _esElProceso(void* algo) {
			t_proceso *unProceso = (t_proceso*) algo;
			return unProceso->pid == proceso->pid;
		}

		pthread_mutex_lock(&mutexTablaPatotas);
		list_remove_by_condition(patotas, (void*) _esElProceso);
		pthread_mutex_unlock(&mutexTablaPatotas);

		pthread_mutex_lock(&mutexTablaPaginas);
		list_destroy_and_destroy_elements(proceso->tabla, (void*) borrarProceso);
		pthread_mutex_unlock(&mutexTablaPaginas);
		free(proceso);
}

TripuCB* transformarEnTripulante(void* buffer){
	TripuCB *elTripulante = (TripuCB*) buffer;
	return elTripulante;
}

void sobreescribir_memoria(int nro_frame, void* datos, int tipo_memoria, int desplazEnPagina, int bytesAEscribir) {

	int desplazamiento = nro_frame * TAM_PAG + desplazEnPagina;
	memcpy(memoria+desplazamiento, datos, bytesAEscribir);
	pthread_mutex_unlock(&mutexEscribiendoMemoria);

	log_info(logs_ram, "Se sobreescribio en RAM: FRAME: %d | DESDE: %d | HASTA: %d ", nro_frame,
			desplazEnPagina, bytesAEscribir + desplazEnPagina -1);
}

//---------------------------------------------------------------------------
//-----------------FUNCIONES DE VISUALIZACION DE DATOS-----------------------
//---------------------------------------------------------------------------

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

	pthread_mutex_lock(&mutexTablaPatotas);
	t_proceso* procesoEnFrame = list_find(patotas,(void*) _frameEnUso);
	pthread_mutex_unlock(&mutexTablaPatotas);

	return procesoEnFrame;
}


t_pagina* pagina_que_tiene_el_frame(int frame, t_proceso* proceso) {

	bool esLaPagina(void* algo){
		t_pagina *unaPagina = (t_pagina*)algo;
		return unaPagina->nro_frame_mpal == frame;
	}

	return list_find(proceso->tabla, esLaPagina);
}

void dumpMemoriaPag() {

	char* horaActual = temporal_get_string_time("%d-%m-%y_%H:%M:%S\n\n");
	char *nombreArchivo = string_new();
	string_append(&nombreArchivo,"dump_");
	string_append(&nombreArchivo,horaActual);
	string_append(&nombreArchivo,".txt");
	char* rutaRelativa = string_from_format("./Dump/%s",nombreArchivo);

	FILE* archivoDump = txt_open_for_append(rutaRelativa);

	if(archivoDump == NULL){
		log_error(logs_ram, "No se pudo abrir el archivo correctamente");
		exit(1);
	}

	char* dump = string_from_format("DUMP: %s\n\n",horaActual);

	txt_write_in_file(archivoDump, "--------------------------------------------------------------------------\n\n");
	txt_write_in_file(archivoDump, dump);

	for(int i=0; i< cantidadDeFrames; i++) { //cant_frames en memoria principal tiene que ser una variable global cuando se inicializa la memoria.

		t_proceso* proceso = patota_que_tiene_el_frame(i);

		if(proceso != NULL)
		{
			t_pagina* info_pagina = pagina_que_tiene_el_frame(i,proceso);

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
	free(rutaRelativa);
	free(dump);
}

//-----------------------------------------------------------------------
//-----------------FUNCIONES DE GESTION DE CLIENTE-----------------------
//-----------------------------------------------------------------------

void iniciarPatotaPag(t_list *lista, int cliente){
	lista = recibir_paquete(cliente);
	t_datos_inicio_patota *datos_inicio = malloc(sizeof(t_datos_inicio_patota));
	datos_inicio->cantidad_tripulantes = atoi(list_get(lista, 0));
	datos_inicio->contenido_tareas     = list_get(lista, 1);
	datos_inicio->socket 			   = cliente;

	pthread_t hiloCreacionPatota;

	pthread_create(&hiloCreacionPatota, NULL, (void*)guardar_PCB_pag, (void*)datos_inicio);
	pthread_detach(hiloCreacionPatota);
}

void crearTripulantePag(t_list *lista, int cliente){
	t_tripulante_iniciado *nuevo_tripulante= recibir_tripulante_iniciado(cliente);
	TripulanteConSocket *nuevo_tripulante_con_socket = malloc(sizeof(TripulanteConSocket));
	nuevo_tripulante_con_socket->tripulante = nuevo_tripulante;
	nuevo_tripulante_con_socket->socket     = cliente;

	pthread_t hiloTripulante;

	pthread_create(&hiloTripulante, NULL, (void*)guardar_TCB_pag,(void*)nuevo_tripulante_con_socket);
	pthread_detach(hiloTripulante);
}

void eliminarTripulantePag(t_list *lista, int cliente){
	lista = recibir_paquete(cliente);

	IdentificadorTripulante *unTripulante = malloc(sizeof(IdentificadorTripulante));
	unTripulante->idTripulante = atoi(list_get(lista,0));
	unTripulante->idPatota = atoi(list_get(lista,1));

	pthread_t hiloEliminacionTripulante;
	pthread_create(&hiloEliminacionTripulante, NULL, (void*)expulsar_tripulante_pag, (void*)unTripulante);
	pthread_detach(hiloEliminacionTripulante);
	liberar_cliente(cliente);
}

void actualizarPosicionPag(t_list *lista, int cliente){
	t_tripulante_iniciado *tripulante_desplazado = recibir_tripulante_iniciado(cliente);

	pthread_t hiloActualizacionTripulante;
	pthread_create(&hiloActualizacionTripulante, NULL, (void*)actualizar_tripulante_pag, (void*)tripulante_desplazado);
	pthread_detach(hiloActualizacionTripulante);
	liberar_cliente(cliente);
}

void obtenerSgteTareaPag(t_list *lista, int cliente){
	t_tripulante_iniciado *tripulante_tarea = recibir_tripulante_iniciado(cliente);
	TripulanteConSocket *tripulante_con_socket = malloc(sizeof(TripulanteConSocket));
	tripulante_con_socket->tripulante = tripulante_tarea;
	tripulante_con_socket->socket     = cliente;

	pthread_t hiloPedidoTarea;
	pthread_create(&hiloPedidoTarea, NULL, (void*)asignar_prox_tarea_pag,(void*)tripulante_con_socket);
	pthread_detach(hiloPedidoTarea);
}

void inicializarPaginacion(){
	//Se establece el algoritmo de reemplazo
	char* algoritmoReemplazo =config_get_string_value(config, "ALGORITMO_REEMPLAZO");

	char* tamanioPag =config_get_string_value(config, "TAMANIO_PAGINA");
	TAM_PAG = atoi(tamanioPag);

	char* tamanioSwap =config_get_string_value(config, "TAMANIO_SWAP");
	TAM_SWAP = atoi(tamanioSwap);

	PUNTERO_ALGORITMO = 0;
	marcos_en_swap = TAM_SWAP/TAM_PAG;
	log_info(logs_ram,"La cantidad de marcos en Swap es: %d", marcos_en_swap);

	char* dirSwap =config_get_string_value(config, "PATH_SWAP");

	crear_archivo_swap();

	paginas_lru = list_create();
	if(strcmp(algoritmoReemplazo, "LRU") == 0){
		esLRU = true;
	}else{
		esLRU = false; //Entonces es Clock (CK)
	}
	sem_init(&tripulantesDisponibles,0,0);
	dividir_memoria_en_frames();
	inicializar_bitmap_swap();
}


//-----------------------------------------------------------------------
//-----------------FUNCIONES DE SWAPPING---------------------------------
//-----------------------------------------------------------------------

void asignar_marco_en_swap(t_pagina* pag){
	int posicionLibre = (int)buscar_marco_disponible(MEM_VIRT);
	pag->nro_frame_mpal = -1;
	pag->bit_uso = false;
	pag->bit_presencia = false;
	pag->nro_frame_swap = posicionLibre;
	asignar_marco_en_uso(pag->nro_frame_swap,MEM_VIRT);
	log_info(logs_ram, "Se asigno a la pagina %d el frame %d en swap",pag->nro_pagina,posicionLibre);
}

void swap_pages(t_pagina* victima, t_pagina* paginaPedida){

	log_info(logs_ram,"La pagina PEDIDA %d de Patota %d tiene asignado frame %d en mpal y %d en swap",paginaPedida->nro_pagina, paginaPedida->pid, paginaPedida->nro_frame_mpal, paginaPedida->nro_frame_swap);
	log_info(logs_ram,"La pagina VICTIMA %d de Patota %d tiene asignado frame %d en mpal y %d en swap",victima->nro_pagina,victima->pid ,victima->nro_frame_mpal, victima->nro_frame_swap);

	int nroFrame = victima->nro_frame_mpal;
	void *frameVictima = memoria+(nroFrame*TAM_PAG);

	int posicionEnSwap = paginaPedida->nro_frame_swap*TAM_PAG;


	void* bufferAux = (void*)malloc(TAM_PAG);

	pthread_mutex_lock(&mutex_swap_file);
	memcpy(bufferAux, MEMORIA_VIRTUAL+posicionEnSwap, TAM_PAG);//Muevo lo de SWAP al buffer
	memcpy(MEMORIA_VIRTUAL+posicionEnSwap, frameVictima, TAM_PAG);//Lo de memoria a swap
	memcpy(frameVictima, bufferAux, TAM_PAG);//Y lo del buffer a memoria
	pthread_mutex_unlock(&mutex_swap_file);

	victima->bit_presencia = false;
	victima->nro_frame_mpal = -1;
	victima->nro_frame_swap = paginaPedida->nro_frame_swap;

	paginaPedida->bit_presencia = true;
	paginaPedida->nro_frame_swap = -1 ;
	paginaPedida->nro_frame_mpal = nroFrame;
	paginaPedida->bit_uso = true;
	if(esLRU){
		log_info(logs_ram, "Agregue la pagina %d de la patota %d al LRU", paginaPedida->nro_pagina, paginaPedida->pid);
		list_add(paginas_lru, paginaPedida);
	}

	free(bufferAux);
}

void traer_pagina(t_pagina* pagina){
	//cada vez que referencian
	//una pagina si no esta en memoria la buscamos
	//y cargamos, si esta en memoria seteamos el bit de uso
	if (!pagina->bit_presencia){//Si la pagina no esta presente

		log_info(logs_ram,"Se produce un PF (PAGINA %d | PROCESO %d)", pagina->nro_pagina, pagina->pid);

		uint32_t marco_libre = buscar_marco_disponible(MEM_PPAL);
		int offsetPpal = marco_libre * TAM_PAG;
		if(marco_libre!=-1){
			log_info(logs_ram,"Se procede a asignar el marco %d a la pagina %d",marco_libre,pagina->nro_pagina);
			pthread_mutex_lock(&mutex_swap_file);
			memcpy(memoria+offsetPpal, MEMORIA_VIRTUAL + pagina->nro_frame_swap* TAM_PAG, TAM_PAG);
			pthread_mutex_unlock(&mutex_swap_file);
			bitarray_clean_bit(BIT_ARRAY_SWAP,(off_t) pagina->nro_frame_swap);
			bitarray_set_bit(frames_ocupados_ppal, (off_t) marco_libre);
			pagina->nro_frame_mpal = marco_libre;
			pagina->bit_presencia = true;
			pagina->bit_uso = true;
			pagina->nro_frame_swap = -1;
			if(esLRU){
				pthread_mutex_lock(&listaLRU);
				list_add(paginas_lru, pagina);
				pthread_mutex_unlock(&listaLRU);
			}

		}else{
			reemplazarSegunAlgoritmo(pagina);
		}
	}else{
		if(esLRU){
			bool _esLaPagina(void* algo){
				t_pagina *unaPagina = (t_pagina*) algo;
				return unaPagina == pagina;
			}
			log_info(logs_ram, "Se actualizo la posicion de la pagina %d de la patota %d en LRU", pagina->nro_pagina, pagina->pid);
			pthread_mutex_lock(&listaLRU);
			list_remove_by_condition(paginas_lru, _esLaPagina);//Saco la pagina de la lista
			list_add(paginas_lru, pagina);//Y la agrego al final
			pthread_mutex_unlock(&listaLRU);
		}
	}
}


void incrementar_puntero(){
	if(PUNTERO_ALGORITMO == (cantidadDeFrames-1)){
		PUNTERO_ALGORITMO = 0;
	} else {
		PUNTERO_ALGORITMO ++;
	}
}

void* buscar_cero(){
	for(int index=0; index<cantidadDeFrames; index++){

		t_proceso* patota = patota_que_tiene_el_frame(index);

		t_pagina* pagina = pagina_que_tiene_el_frame(index, patota);

		if(pagina && index == PUNTERO_ALGORITMO && bitarray_test_bit(frames_ocupados_ppal,index)){

			if(pagina->bit_uso == 0){
				incrementar_puntero();
				return pagina;
			}
			pagina->bit_uso = 0;
			incrementar_puntero();
		}
	}
	return NULL;
}

t_pagina* algoritmo_clock(){
	pthread_mutex_lock(&mutex_clock);
	t_pagina* victima = NULL;
	log_info(logs_ram,"Se comienza a ejecutar el algoritmo clock");
	while(!victima){
		victima = (t_pagina*)buscar_cero();
	}
	log_info(logs_ram,"Se eligio como victima la pagina %d cuyo frame es %d.",victima->nro_pagina,victima->nro_frame_mpal);
	pthread_mutex_unlock(&mutex_clock);
	return victima;
}

t_pagina* algoritmo_lru(){
	t_pagina* victima;
	victima = list_remove(paginas_lru,0);//Tomo la primera pagina de la lista
	log_info(logs_ram, "Removi la pagina %d de LRU", victima->nro_pagina);
	return victima;//La primera pagina de la lista es la que se referencio hace mas tiempo
}


void reemplazarSegunAlgoritmo(t_pagina* paginaEntrada){
	t_pagina *paginaSalida;
	if(esLRU){
		pthread_mutex_lock(&listaLRU);
		paginaSalida = algoritmo_lru();
		swap_pages(paginaSalida, paginaEntrada);
		pthread_mutex_unlock(&listaLRU);
	}else{
		paginaSalida = algoritmo_clock();
		swap_pages(paginaSalida, paginaEntrada);
	}
}

void cerrarMemoriaPag(){
	list_iterate(patotas, (void*)limpiarProceso);
}
