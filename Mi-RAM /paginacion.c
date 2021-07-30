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

bool marco_vacio(int marco){
	int estadoFrame = bitarray_test_bit(frames_ocupados_ppal, marco);
	return (estadoFrame == LIBRE);
}


uint32_t buscar_marco_disponible(int tipo_memoria){
	int size = 0;
	if(tipo_memoria == MEM_PPAL){
		size = cantidadDeFrames;
	}else{
		size = marcos_en_swap;
	}

	for(uint32_t m = 0; m < size; m++){
		if(!traer_marco_valido(m, tipo_memoria) && marco_vacio(m)) {
			return m;
		}
	}

	log_info(logs_ram, "No se encontro un frame disponible");
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
	pagina->nro_frame_mpal = buscar_marco_disponible(MEM_PPAL);

	log_info(logs_ram, "Se creo el t_pagina de estructura: %d", estructura);

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
		//pthread_mutex_lock(&mutexEscribiendoMemoria);
		memcpy(pagina, memoria+desplazamiento, TAM_PAG);
		//pthread_mutex_unlock(&mutexEscribiendoMemoria);
	}

	return pagina;
}

int insertar_en_memoria_pag(t_pagina* pagina, void* datos, int tipo_memoria, int* bytesAInsertar,  int estructura, int* bytesEscritos, int flag){
	if(marco_vacio(pagina->nro_frame_mpal)){
		pthread_mutex_lock(&mutexAlojados);
		int desplazamiento_pag = TAM_PAG - pagina->tam_disponible;
		int desplazamiento_mem = pagina->nro_frame_mpal * TAM_PAG + desplazamiento_pag;
		int bytesAEscribir = pagina->tam_disponible - *bytesAInsertar;


		if(bytesAEscribir < 0){
			bytesAEscribir = pagina->tam_disponible;
			pagina->tam_disponible = 0;
			asignar_marco_en_uso(pagina->nro_frame_mpal,tipo_memoria);
		}else {
			if(estructura == TAREAS){ flag= 1;}
			bytesAEscribir = *bytesAInsertar;
			pagina->tam_disponible = pagina->tam_disponible - *bytesAInsertar;
		}

		agregar_estructura_a_pagina(pagina, desplazamiento_pag, bytesAEscribir, estructura, flag);
		pthread_mutex_unlock(&mutexAlojados);
		if(tipo_memoria == MEM_PPAL)
		{
			pthread_mutex_lock(&mutexEscribiendoMemoria);
			memcpy(memoria + desplazamiento_mem, datos, bytesAEscribir);
			pthread_mutex_unlock(&mutexEscribiendoMemoria);
		}else{
			pthread_mutex_lock(&mutexEscribiendoSwap);
			log_info(logs_ram, "Copie %d", estructura);
			memcpy(MEMORIA_VIRTUAL + desplazamiento_mem, datos, bytesAEscribir);
			pthread_mutex_unlock(&mutexEscribiendoSwap);
		}

		*bytesAInsertar -= bytesAEscribir;

		*bytesEscritos = bytesAEscribir;

		return 1;
	}else{
		return 0;
	}
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
		if(estructura != PCB){
			pagina = buscar_ultima_pagina_disponible(proceso);

			if(pagina != NULL){
				insertar_en_memoria_pag(pagina, siguienteAEscribir, MEM_PPAL, &aMeter, estructura, &bytesEscritos, flag);
			}else{
				pagina = crear_pagina_en_tabla(proceso,estructura);

				if(pagina->nro_frame_mpal != -1){
					insertar_en_memoria_pag(pagina, siguienteAEscribir, MEM_PPAL, &aMeter, estructura, &bytesEscritos,flag);
				}else{
					insertar_en_memoria_pag(pagina, siguienteAEscribir, MEM_VIRT, &aMeter, estructura, &bytesEscritos, flag);
					log_info(logs_ram, "Memoria principal llena");
					return 0;
				}
			}
		}else{
			//Si es PCB se hace esto ya que sabemos que la PCB es lo primero que se guarda
			pagina = crear_pagina_en_tabla(proceso, estructura);

			pagina->nro_frame_mpal = buscar_marco_disponible(MEM_PPAL);
			flag = -1;

			if(pagina->nro_frame_mpal != -1){
				insertar_en_memoria_pag(pagina, siguienteAEscribir, MEM_PPAL, &aMeter ,estructura ,&bytesEscritos,flag);
			}else{
				insertar_en_memoria_pag(pagina, siguienteAEscribir, MEM_VIRT, &aMeter, estructura, &bytesEscritos, flag);
				return 0;
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

char* obtener_siguiente_tarea_pag(t_proceso* proceso, TripuCB* tcb) {
	char* tarea = string_new();
	char* aux = malloc(2);
	*(aux+1) = '\0';
	void* paginaAGuardar;
	void* recorredorPagina;
	int indicePagina = (int) tcb->proxIns / 100.0;
	int desplazamiento = tcb->proxIns % 100;
	t_pagina* pagina;

	pthread_mutex_lock(&mutexEscribiendoMemoria);
	for(int i = 0; i <= indicePagina; i++){
		pagina = list_get(proceso->tabla, i);

		if(tiene_pagina_estructura_alojadas(pagina->estructuras_alojadas, TAREAS))
		{
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
				log_info(logs_ram,"Sacando tarea: %s",tarea);
			}

			tcb->proxIns = pagina->nro_pagina * 100 + desplazamiento;

			desplazamiento = 0;
			paginaAGuardar=NULL;
		}
		if(*aux == '\n' || *aux == '\0') break;

	}
	pthread_mutex_unlock(&mutexEscribiendoMemoria);
	free(paginaAGuardar);

	t_list* paginasConTripulante = lista_paginas_tripulantes(proceso->tabla, tcb->tid);
	sobreescribir_tripulante(paginasConTripulante, tcb);
	free(aux);

	return tarea;
}

//-----------------------------------------------------------------------
//---------------------MANEJO DE DIRECCIONES LOGICAS---------------------
//-----------------------------------------------------------------------

uint32_t buscar_inicio_tareas(t_proceso* proceso) {


    bool buscarDLTarea(t_pagina* pagina) {

    	//pthread_mutex_lock(&mutexAlojados);
    	bool a = tiene_pagina_estructura_alojadas(pagina->estructuras_alojadas, TAREAS);
    	//pthread_mutex_unlock(&mutexAlojados);

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
	caracterRepresentativo = 0;
	//caracterRepresentativo = nuevoTripuMapa(elTripulante->posX, elTripulante->posY);
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

	//Podria devolver la siguiente tarea para ir cargando secuencialmente la proxIns para los tripulantes creados.

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

	//pthread_mutex_lock(&mutexAlojados);
	t_alojado* alojadoConTarea = list_find(estructuras_alojadas, (void*) contieneTipo);
	//pthread_mutex_unlock(&mutexAlojados);

	return alojadoConTarea != NULL;
}

//TODO: NUNCA SE USA ESTA FUNCION
/*bool tiene_pagina_tripu_alojado(t_list* estructuras_alojadas, int id_tripulante){
	//pthread_mutex_lock(&mutexAlojados);
	t_alojado* alojadoConTarea = obtener_tripulante_pagina(estructuras_alojadas, id_tripulante);
	//pthread_mutex_unlock(&mutexAlojados);

	return alojadoConTarea != NULL;
}*/

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


int sobreescribir_tripulante(t_list* lista_paginas_tripulantes, TripuCB* tcb) {

	int aMeter, relleno, offset = 0;
	void* bufferAMeter = (void*)tcb;

	int cantPaginasConTripu = list_size(lista_paginas_tripulantes);

	if(cantPaginasConTripu == 0) {
		log_error(logs_ram, "No hay paginas que contengan al tripulante %d en memoria" , tcb->tid);
		return 0;
	}

	//log_info(logs_ram, "La cantidad de paginas que contienen al tripulante %d son %d", tcb->tid, cantPaginasConTripu);
	int i = 0;

	while(i < cantPaginasConTripu)
	{
		t_pagina* pagina = list_get(lista_paginas_tripulantes,i);
		t_alojado* alojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, tcb->tid);

		//log_info(logs_ram, "Se va a sobreescrbir el tripulante: ID: %d | ESTADO: %c | X: %d | Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
			//	tcb->tid, tcb->status, tcb->posX, tcb->posY, tcb->proxIns, tcb->pcb);

		sobreescribir_memoria(pagina->nro_frame_mpal, bufferAMeter + offset, MEM_PPAL, alojado->base, alojado->tamanio);
		offset += alojado->tamanio;
		i++;
	}
	list_destroy(lista_paginas_tripulantes);
	free(tcb);

	return 1;
}

void actualizar_tripulante_pag(t_tripulante_iniciado *tripulanteActualizado) {
	int idTripulante = tripulanteActualizado->tid;
	int idPatota = tripulanteActualizado->numPatota;

	t_proceso* proceso = buscar_patota(idPatota);
	log_info(logs_ram, "Se va a actualizar al tripu %d", idTripulante);
	existencia_patota(proceso);

	t_list *paginasConTripulante = lista_paginas_tripulantes(proceso->tabla,(uint32_t) idTripulante);
	t_pagina* pagina = list_get(paginasConTripulante,0);
	t_alojado* alojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, idTripulante);


	TripuCB *elTripulante = obtener_tripulante(proceso, idTripulante);
	int difX = tripulanteActualizado->posX-elTripulante->posX;
	int difY = tripulanteActualizado->posY-elTripulante->posY;
	elTripulante->posX = tripulanteActualizado->posX;
	elTripulante->posY = tripulanteActualizado->posY;
	elTripulante->status = tripulanteActualizado->status;


	log_info(logs_ram,"El tripulante %d se ha actualizado: Estado: %c %d|%d",idTripulante,elTripulante->status,elTripulante->posX,elTripulante->posY);

	sobreescribir_tripulante(paginasConTripulante, elTripulante);
	//moverTripuMapa(alojado->caracterRep,  difX,  difY);
	return;
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

	void* bufferTripu = malloc(sizeof(TripuCB));
	int offset = 0;

	while(i < cantPaginasConTripu)
	{
		t_pagina* pagina = list_get(paginasConTripulante,i);
		t_alojado* alojado = obtener_tripulante_de_la_pagina(pagina->estructuras_alojadas, tid);

		pthread_mutex_lock(&mutexEscribiendoMemoria);
		void* pagina_memoria = leer_memoria_pag(pagina->nro_frame_mpal, MEM_PPAL);
		pthread_mutex_unlock(&mutexEscribiendoMemoria);

		log_info(logs_ram, "SE LEE DEL TRIPU: %d - FRAME: %d | D_INCIAL: %d | TAMANIO: %d", tid,
				pagina->nro_frame_mpal, alojado->base, alojado->tamanio);

		memcpy(bufferTripu + offset,pagina_memoria + alojado->base, alojado->tamanio);
		offset += alojado->tamanio;

		i++;
		pagina = NULL;
	}
	TripuCB* tcb = transformarEnTripulante(bufferTripu);
	list_destroy(paginasConTripulante);


	return tcb;
}

void asignar_prox_tarea_pag(void *unTripulante) {
	TripulanteConSocket *elTripuConSocket = (TripulanteConSocket*) unTripulante;
	int cliente = elTripuConSocket->socket;
	t_tripulante_iniciado *tripulante = (t_tripulante_iniciado*) elTripuConSocket->tripulante;
	int idTripulante = tripulante->tid;
	int idPatota = tripulante->numPatota;

	t_proceso* proceso = buscar_patota(idPatota);

	existencia_patota(proceso);

	TripuCB* tcb = obtener_tripulante(proceso, idTripulante);

	char* tarea = obtener_siguiente_tarea_pag(proceso, tcb);

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

	int caracterRepresentativo;

	t_list* paginasTripu = lista_paginas_tripulantes(proceso->tabla, tid);
	pthread_mutex_lock(&mutexTablaPaginas);
	log_info(logs_ram, "Se va a eliminar el tripulante %d de la patota %d",tid, pid);

	void _recorrer_paginas_tripulante(void* algo){
		t_pagina *paginaActual = (t_pagina*) algo;
		if(paginaActual->estructuras_alojadas == NULL) {
			log_error(logs_ram,"No hay na aca");
		}

		t_alojado* tripuAlojado = obtener_tripulante_de_la_pagina(paginaActual->estructuras_alojadas, tid);
		paginaActual->tam_disponible += tripuAlojado->tamanio;

		log_info(logs_ram,"Se va a sacar de la lista de alojados de cant %d el tripu %d",
				list_size(paginaActual->estructuras_alojadas), tripuAlojado->flagid);

		bool tripuConID(t_alojado* alojado) {
			if(alojado->tipo == TCB && alojado->flagid == tid){
				caracterRepresentativo = alojado->flagid + 64;
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
			liberar_marco(paginaActual->nro_frame_mpal, MEM_PPAL);

			bool paginaConID(t_alojado* pagina)
			{
				return pagina->nro_estructura == paginaActual->nro_pagina;
			}
			list_remove_by_condition(proceso->tabla, (void*) paginaConID);

			list_destroy(paginaActual->estructuras_alojadas);

			free(paginaActual);
		}
	}

	if(paginasTripu != NULL){
		list_iterate(paginasTripu,_recorrer_paginas_tripulante);
		pthread_mutex_unlock(&mutexTablaPaginas);
		//eliminarTripuMapa(caracterRepresentativo);
		chequear_ultimo_tripulante(proceso);
	}else{
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

		pthread_mutex_lock(&mutexTablaPatotas);
		list_remove_by_condition(patotas, (void*) _esElProceso);
		pthread_mutex_unlock(&mutexTablaPatotas);

		pthread_mutex_lock(&mutexTablaPaginas);
		list_destroy_and_destroy_elements(proceso->tabla, (void*) borrarProceso);
		pthread_mutex_unlock(&mutexTablaPaginas);
		free(proceso);
	}
}

TripuCB* transformarEnTripulante(void* buffer){
	TripuCB *elTripulante = (TripuCB*) buffer;
	return elTripulante;
}

void sobreescribir_memoria(int nro_frame, void* datos, int tipo_memoria, int desplazEnPagina, int bytesAEscribir) {


	int desplazamiento = nro_frame * TAM_PAG + desplazEnPagina;

	if(tipo_memoria == MEM_PPAL)
	{
		pthread_mutex_lock(&mutexEscribiendoMemoria);
		memcpy(memoria+desplazamiento, datos, bytesAEscribir);
		pthread_mutex_unlock(&mutexEscribiendoMemoria);

		log_info(logs_ram, "Se sobreescribio en RAM: FRAME: %d | DESDE: %d | HASTA: %d ", nro_frame,
				desplazEnPagina, bytesAEscribir + desplazEnPagina -1);
	}else if(tipo_memoria == MEM_VIRT){
		pthread_mutex_lock(&mutexEscribiendoSwap);
		memcpy(MEMORIA_VIRTUAL + desplazamiento, datos, bytesAEscribir);
		pthread_mutex_unlock(&mutexEscribiendoSwap);
	}
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

	sem_wait(&tripulantesDisponibles);
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

	marcos_en_swap = TAM_SWAP/TAM_PAG;

	char* dirSwap =config_get_string_value(config, "PATH_SWAP");

	crear_archivo_swap();


	log_info(logs_ram, "El tamaño de las paginas es: %s", tamanioPag);
	if(strcmp(algoritmoReemplazo, "LRU") == 0){
		esLRU = true;
	}else{
		esLRU = false; //Entonces es Clock (CK)
	}
	sem_init(&tripulantesDisponibles,0,0);
	dividir_memoria_en_frames();
}


//-----------------------------------------------------------------------
//-----------------FUNCIONES DE SWAPPING---------------------------------
//-----------------------------------------------------------------------

void asignar_marco_en_swap(t_pagina* pag){
	int posicionLibre = posicion_libre_en_swap();
	bitarray_set_bit(BIT_ARRAY_SWAP, (off_t) posicionLibre);
	pag->nro_frame_mpal = -1;
	pag->bit_uso = false;
	pag->bit_presencia = false;
	pag->nro_frame_swap = posicionLibre;
}

void swap_pages(t_pagina* victima, t_pagina* paginaPedida){
	//datos de la victima
	log_info(logs_ram,"Se swapea la pagina %d con la pagina %d",paginaPedida->nro_pagina,victima->nro_pagina);
	int nroFrame = victima->nro_frame_mpal;
	void *frameVictima = memoria+(nroFrame*TAM_PAG);

	int posicionEnSwap = paginaPedida->nro_frame_swap*TAM_PAG;


	void* bufferAux = (void*)malloc(TAM_PAG);

	pthread_mutex_lock(&mutex_swap_file);
	memcpy(bufferAux, MEMORIA_VIRTUAL+posicionEnSwap, TAM_PAG); //Swap mappeado como variable global por ahora
	memcpy(MEMORIA_VIRTUAL+posicionEnSwap, frameVictima, TAM_PAG);
	memcpy(frameVictima, bufferAux, TAM_PAG);
	pthread_mutex_unlock(&mutex_swap_file);

	victima->bit_presencia = false;
	victima->nro_frame_mpal = -1;
	victima->nro_frame_swap = paginaPedida->nro_frame_swap;

	paginaPedida->bit_presencia = true;
	paginaPedida->nro_frame_swap = -1 ;
	paginaPedida->nro_frame_mpal = nroFrame;
	paginaPedida->bit_uso = true;

	free(bufferAux);
}

void traer_pagina(t_pagina* pagina){
	//cada vez que referencian
	//una pagina si no esta en memoria la buscamos
	//y cargamos, si esta en memoria seteamos el bit de uso

	if (!pagina->bit_presencia){//Si la pagina no esta presente
		log_info(logs_ram,"Se produce un PF (pagina %d)",pagina->nro_pagina);
		uint32_t marco_libre = buscar_marco_disponible(MEM_PPAL);
		int offsetPpal = marco_libre * TAM_PAG;
		if(marco_libre){
			log_info(logs_ram,"Se procede a asignar el marco %d a la pagina %d",marco_libre,pagina->nro_pagina);
			pthread_mutex_lock(&mutex_swap_file);
			memcpy(memoria+offsetPpal, MEMORIA_VIRTUAL + pagina->nro_frame_swap* TAM_PAG, TAM_PAG); //Swap mappeado como variable global por ahora
			pthread_mutex_unlock(&mutex_swap_file);
			bitarray_clean_bit(BIT_ARRAY_SWAP,(off_t) pagina->nro_frame_mpal);
			bitarray_set_bit(frames_ocupados_ppal, (off_t) marco_libre);
			pagina->nro_frame_mpal = marco_libre;
			pagina->bit_presencia = true;
			pagina->bit_uso = true;
		}else{
			t_pagina* victima = algoritmo_clock();//TODO: Agregar para LRU
			swap_pages(victima, pagina);
		}
	}
	pagina->bit_uso = true;
}

void escribir_en_archivo_swap(void *file, t_list *tabla_de_paginas, size_t tam_a_mappear,size_t tam_arch){
	int offset = tam_a_mappear;
	int tam_archivo = tam_arch;
	void *padding;
	bool archivo_completo=false;
	void _escribir_en_frame_de_swap (void *element){
		t_pagina *pagina = (t_pagina*)element;
		int pag_pos = pagina->nro_pagina;
		int posicion_en_swap = TAM_PAG * pagina->nro_frame_swap;
		if(offset >0 && archivo_completo){
			padding = malloc(TAM_PAG);
			memset(padding,'\0',TAM_PAG);
			pthread_mutex_lock(&mutex_swap_file);
			memcpy(MEMORIA_VIRTUAL+posicion_en_swap,padding,TAM_PAG);
			pthread_mutex_unlock(&mutex_swap_file);
			offset -= TAM_PAG;
			free(padding);
		}
		if(tam_archivo>=TAM_PAG && offset > 0 && !archivo_completo){
			pthread_mutex_lock(&mutex_swap_file);
			memcpy(MEMORIA_VIRTUAL+posicion_en_swap,file+(pag_pos*TAM_PAG),TAM_PAG);
			pthread_mutex_unlock(&mutex_swap_file);
			offset -= TAM_PAG;
			tam_archivo-= TAM_PAG;
		}else if (offset > 0 && tam_archivo>0){
			pthread_mutex_lock(&mutex_swap_file);
			memcpy(MEMORIA_VIRTUAL+posicion_en_swap,file+(pag_pos*TAM_PAG),tam_archivo);
			padding = malloc(TAM_PAG-tam_archivo);
			memset(padding,'\0',TAM_PAG-tam_archivo);
			memcpy(MEMORIA_VIRTUAL+posicion_en_swap+tam_archivo,padding,TAM_PAG-tam_archivo);
			pthread_mutex_unlock(&mutex_swap_file);
			offset-=TAM_PAG;
			tam_archivo-= TAM_PAG;
			archivo_completo=true;
			free(padding);
		}

	}
	list_iterate(tabla_de_paginas,_escribir_en_frame_de_swap);
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
