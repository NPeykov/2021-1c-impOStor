#include "segmentacion.h"

void compactacion(){
	uint32_t finalSegmentoAnterior = 0;
	uint32_t inicioSegmentoActual = 0;
	int i = 0;
	void _recorrerSegmentos(void *algo){
		Segmento *unSegmento = (Segmento*) algo;
		inicioSegmentoActual = unSegmento->base;
		if(finalSegmentoAnterior == 0){//El primer segmento
			finalSegmentoAnterior = unSegmento->tamanio + unSegmento->base;
		}else if(finalSegmentoAnterior == inicioSegmentoActual){//Empieza donde termina el otro
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
		}else if(inicioSegmentoActual > finalSegmentoAnterior){//Hay un espacio
			void *buffer = malloc(unSegmento->tamanio);
			memcpy(buffer, unSegmento->dato, unSegmento->tamanio);
			memcpy(memoria + finalSegmentoAnterior, buffer, unSegmento->tamanio);
			unSegmento->dato = memoria + finalSegmentoAnterior;
			free(buffer);
			unSegmento->base= finalSegmentoAnterior;
			finalSegmentoAnterior = finalSegmentoAnterior + unSegmento->tamanio;
			i++;
		}//NOTA: Se coloca en un buffer ya que memcpy no puede copiar en laa zona de origen
	}
	log_info(logs_ram, "Iniciando compactacion");
	list_iterate(memoriaPrincipal, _recorrerSegmentos);
	log_info(logs_ram, "Se compactaron %d espacios", i);
	noCompactado = false;
}

uint32_t algoritmoBestFit(Segmento *segmento){
	uint32_t tamanioNecesario =(uint32_t) segmento->tamanio;
	uint32_t finalSegmentoAnterior = 0;
	uint32_t inicioSegmentoActual = 0;
	uint32_t espacioLibreUbicacion = (uint32_t) tamaniomemoria; //Cuanto espacio queda libre si se coloca en ubicacionMasJusta
	uint32_t ubicacionMasJusta = 0; //Aca se va guardando la mejor posicion encontrada

	//Determina si hay un espacio libre entre dos segmentos
	void espacioLibre(void* segmentoActual){
	Segmento* unSegmento =(Segmento*) segmentoActual;
	inicioSegmentoActual = unSegmento->base;
		if(finalSegmentoAnterior==0){//El primer elemento de la lista
			finalSegmentoAnterior = unSegmento->tamanio + unSegmento->base;
		}else if(finalSegmentoAnterior == inicioSegmentoActual){//Empieza donde termina el anterior
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
		}else if(inicioSegmentoActual > finalSegmentoAnterior && //Hay un espacio entre ambos y es mayor o igual al necesario
				(inicioSegmentoActual-finalSegmentoAnterior)>= tamanioNecesario){
			if(( inicioSegmentoActual - finalSegmentoAnterior - tamanioNecesario) < espacioLibreUbicacion){
				ubicacionMasJusta = finalSegmentoAnterior;
				espacioLibreUbicacion = inicioSegmentoActual - finalSegmentoAnterior - tamanioNecesario;
			}
		}else{
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
		}
	}
	pthread_mutex_lock(&listaMemoriaPrincipal);
	//Si no hay nada en memoria principal la dir es 0 por ser primero
	if(list_is_empty(memoriaPrincipal)){
		log_info(logs_ram, "Se coloco el segmento en 0 hasta %d",tamanioNecesario);
		pthread_mutex_unlock(&listaMemoriaPrincipal);
		return (uint32_t) 0;
	}

	list_iterate(memoriaPrincipal, espacioLibre);//Este es el mayor cambio entre FF y BF
	pthread_mutex_unlock(&listaMemoriaPrincipal);
	if(tamaniomemoria >= ubicacionMasJusta + tamanioNecesario && ubicacionMasJusta != 0){
		log_info(logs_ram,"Se coloco el segmento en %d hasta %d", finalSegmentoAnterior,tamanioNecesario);
		return ubicacionMasJusta;
	}else if(tamaniomemoria >= finalSegmentoAnterior + tamanioNecesario){
		log_info(logs_ram,"Se coloco el segmento en %d hasta %d", finalSegmentoAnterior,tamanioNecesario);
		return finalSegmentoAnterior;//Para el ultimo segmento
	}else{
		if(noCompactado){
			compactacion();//Se compacta y se hace de nuevo
			ubicacionMasJusta =  algoritmoBestFit(segmento);
			noCompactado = true;
			return ubicacionMasJusta;
		}else{
			return -1;//Hubo un error
		}
	}
}

uint32_t calcular_base_logica(Segmento *segmento){
	if(esFF){
		return algoritmoFirstFit(segmento);
	}else{
		return algoritmoBestFit(segmento);
		//return algoritmoBestFit(segmento);
	}
}

uint32_t algoritmoFirstFit(Segmento *segmento){
	uint32_t tamanioNecesario =(uint32_t) segmento->tamanio;
	uint32_t finalSegmentoAnterior = 0;
	uint32_t inicioSegmentoActual = 0;

	//ESTO SERIA FIRST FIT
	//Determina si hay un espacio libre entre dos segmentos
	bool espacioLibre(void* segmentoActual){
	Segmento* unSegmento =(Segmento*) segmentoActual;
	inicioSegmentoActual = unSegmento->base;
		if(finalSegmentoAnterior==0){//El primer elemento de la lista
			finalSegmentoAnterior = unSegmento->tamanio + unSegmento->base;
			return 0;
		}else if(finalSegmentoAnterior == inicioSegmentoActual){
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
			return 0;//El anterior termina donde empieza este
		}else if(inicioSegmentoActual > finalSegmentoAnterior &&
				(inicioSegmentoActual-finalSegmentoAnterior)>= tamanioNecesario){
			return 1; //Si hay una diferencia entre el segmento actual y el anterior.
		}else{
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
			return 0; //No hay espacio
		}
	}

	//Si no hay nada en memoria principal la dir es 0 por ser primero
	if(list_is_empty(memoriaPrincipal)){
		log_info(logs_ram,"Se coloco el segmento en 0 hasta %d",tamanioNecesario);
		return (uint32_t) 0;
	}
	pthread_mutex_lock(&listaMemoriaPrincipal);
	list_find(memoriaPrincipal, espacioLibre);
	pthread_mutex_unlock(&listaMemoriaPrincipal);
	if(tamaniomemoria >= finalSegmentoAnterior + tamanioNecesario){
		log_info(logs_ram,"Se coloco el segmento en %d hasta %d", finalSegmentoAnterior,tamanioNecesario);
		return finalSegmentoAnterior;
	}else{
		if(noCompactado){
			compactacion();//Se compacta y se hace de nuevo
			finalSegmentoAnterior =  algoritmoFirstFit(segmento);
			log_info(logs_ram,"Se coloco el segmento en %d hasta %d", finalSegmentoAnterior,tamanioNecesario);
			noCompactado = true;
			return finalSegmentoAnterior;
		}else{
			return -1;//Hubo un error
		}
	}
}

int crear_segmento_tareas(char *tareas, t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(Segmento));

	//Se llena el segmento
	segmento->idSegmento = list_size(tabla_segmentos);
	segmento->tipo = TAREAS;
	segmento->dato = tareas;
	segmento->valorRepresentacion = 0;
	log_info(logs_ram,"Tareas guardadas: %s", tareas);
	segmento->tamanio = string_length(tareas);
	sem_wait(&direcciones);
	segmento->base = calcular_base_logica(segmento);
	segmento->idSegmentoUnico = idSegmentoSiguiente;
	idSegmentoSiguiente++;

	if(segmento->base == -1){
		sem_post(&direcciones);
		free(segmento);
		return -1;//Por si hay error retorna -1
	}else{
		list_add(tabla_segmentos, segmento);
		return 0;
	}
	//Se lo agrega a la tabla de Segmentos del proceso actual



}

int crear_segmento_pcb(uint32_t inicioTareas, t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(Segmento));

	//Se llena la estructura de PatotaCB
	PatotaCB *pcb = (PatotaCB*) malloc(sizeof(PatotaCB));
	pcb->pid = numero_patota;
	sem_post(&numeroPatotas);
	pcb->tareas = inicioTareas;

	//Se llena la informacion del Segmento
	segmento->idSegmento = list_size(tabla_segmentos);
	segmento->idSegmentoUnico = idSegmentoSiguiente;
	idSegmentoSiguiente++;
	segmento->tipo = PCB;
	segmento->dato = pcb;
	segmento->tamanio = sizeof(PatotaCB);
	segmento->valorRepresentacion = 0;
	sem_wait(&direcciones);
	segmento->base = calcular_base_logica(segmento);

	//Se lo agrega a la tabla de Segmentos del proceso actual


	if(segmento->base == -1){
		sem_post(&direcciones);
		free(segmento);
		free(pcb);
		return -1;//Por si hay error retorna -1
	}else{
		list_add(tabla_segmentos, segmento);
		return 0;
	}
}

void crear_segmento_tcb(void* elTripulante) {
	TripulanteConSocket *tripulanteConSocket = (TripulanteConSocket *) elTripulante;
	t_tripulante_iniciado *unTripulante = tripulanteConSocket->tripulante;
	int _socket_cliente = tripulanteConSocket->socket;

	Segmento *segmento = (Segmento*) malloc(sizeof(Segmento));
	//Para buscar su patota
	int patota = (int) unTripulante->numPatota;

	bool _esLaPatota(void *proceso){
		t_proceso *unProceso = (t_proceso*) proceso;
		return unProceso->pid == patota;
	}

	//Para obtener los valores de la patota
	log_info(logs_ram, "Inicializando tripulante");
	pthread_mutex_lock(&listaPatotasEnUso);
	t_proceso *miPatota = (t_proceso*) list_find(patotas, _esLaPatota);
	pthread_mutex_unlock(&listaPatotasEnUso);
	t_list *tabla_segmentos = miPatota->tabla;
	Segmento *laPatota = (Segmento*) list_get(tabla_segmentos, 1);//Porque sabemos que se crea tarea->patota
	log_info(logs_ram, "Inicializando estructura del tripulante");
	//Se asignan los valores a la TCB
	TripuCB *tcb = (TripuCB*) malloc(sizeof(TripuCB));
	tcb->tid = unTripulante->tid;
	tcb->pcb = laPatota->idSegmentoUnico;
	tcb->posX = unTripulante->posX;
	tcb->posY = unTripulante->posY;
	tcb->status = unTripulante->status;
	tcb->proxIns = (uint32_t) 0;
	//Se asigna el acceso rapido de t_proceso
	log_info(logs_ram, "Inicializando segmento del tripulante");
	segmento->idSegmento = list_size(tabla_segmentos);
	segmento->idSegmentoUnico = idSegmentoSiguiente;
	idSegmentoSiguiente++;
	segmento->tipo = TCB;
	segmento->dato = tcb;
	segmento->tamanio = sizeof(TripuCB);
	sem_wait(&direcciones);
	segmento->base = calcular_base_logica(segmento);
	if(segmento->base == -1){
		idSegmentoSiguiente--;
		sem_post(&direcciones);
		enviar_mensaje_simple("no", _socket_cliente);
		liberar_cliente(_socket_cliente);
		eliminarPatota(miPatota);
		free(segmento);
		free(tcb);
		free(tripulanteConSocket);
		pthread_exit(NULL);
	}else{
		list_add(tabla_segmentos, segmento);
		segmento->valorRepresentacion = nuevoTripuMapa(tcb->posX,tcb->posY);
		agregar_a_memoria(segmento);
		sem_post(&direcciones);
		//sem_post(&tripulantesDisponibles);
		enviar_mensaje_simple("ok", _socket_cliente);
		log_info(logs_ram, "Se creo al tripulante %d de la patota %d",tcb->tid, unTripulante->numPatota);
		free(unTripulante);
		liberar_cliente(_socket_cliente);
		free(tripulanteConSocket);
		pthread_exit(NULL);
	}
}

void agregar_a_memoria(Segmento* unSegmento){
	bool _laBaseEsMenor(void* segmento1, void* segmento2){
		Segmento* unSegmento = (Segmento*) segmento1;
		Segmento* otroSegmento = (Segmento*) segmento2;

		return (unSegmento->base < otroSegmento->base);
	}

	//Se copia la estructura en el malloc de memoria
	memcpy(memoria + unSegmento->base, unSegmento->dato, unSegmento->tamanio);
	//Se libera el anterior y se coloca el puntero en la nueva direccion de memoria
	free(unSegmento->dato);
	unSegmento->dato = (memoria + unSegmento->base);
	pthread_mutex_lock(&listaMemoriaPrincipal);
	list_add_sorted(memoriaPrincipal, unSegmento, _laBaseEsMenor );
	pthread_mutex_unlock(&listaMemoriaPrincipal);
	//El t_list memoriaPrincipal se usa para hacer la compactacion
}

int contarEspacioMemoria(){
	int espacioOcupado = 0;

	void _sumarTamanios(Segmento* unSegmento){
		espacioOcupado +=unSegmento->tamanio;
	}

	pthread_mutex_lock(&listaMemoriaPrincipal);
	list_iterate(memoriaPrincipal, (void*)_sumarTamanios);
	pthread_mutex_unlock(&listaMemoriaPrincipal);
	return (tamaniomemoria - espacioOcupado);
}

bool alcanzaElEspacio(int espacioNecesario){
	int espacioDisponible = contarEspacioMemoria();
	return espacioDisponible>=espacioNecesario;
}

void crear_proceso(void *data){


	t_list* tabla_de_segmentos = list_create();
	t_datos_inicio_patota *datos_patota = (t_datos_inicio_patota*)data;
	char* contenido = datos_patota->contenido_tareas;
	int _socket_cliente = datos_patota->socket;
	int tamanioPatota = string_length(contenido) + 8 + 21*datos_patota->cantidad_tripulantes;

	if(!alcanzaElEspacio(tamanioPatota)){
		enviar_mensaje_simple("no", _socket_cliente);
		liberar_cliente(_socket_cliente);
		free(datos_patota);
		pthread_exit(NULL);
	}

	//Se crea el segmento de tareas
	int result_tareas =crear_segmento_tareas(contenido, tabla_de_segmentos);
	if(result_tareas == -1){
		idSegmentoSiguiente--;
		enviar_mensaje_simple("no", _socket_cliente);
		liberar_cliente(_socket_cliente);
		free(datos_patota);
		pthread_exit(NULL);
		return;
	}
	Segmento *segmento_tareas =(Segmento*) list_get(tabla_de_segmentos, 0);
	uint32_t inicioTareas = segmento_tareas->idSegmentoUnico;//Sabemos que siempre se empieza por las tareas

	agregar_a_memoria(segmento_tareas);
	sem_post(&direcciones);

	//Se crea t_proceso como accedo rapido a los segmentos de la patota
	t_proceso *proceso = (t_proceso*) malloc(sizeof(t_proceso));
	sem_wait(&numeroPatotas);
	proceso->pid = numero_patota;
	numero_patota += 1;

	//Se crea el segmento PCB
	int result_pcb =crear_segmento_pcb(inicioTareas, tabla_de_segmentos);
	if(result_pcb == -1){
		idSegmentoSiguiente--;
		enviar_mensaje_simple("no", _socket_cliente);
		eliminarPatota(proceso);
		free(datos_patota);
		liberar_cliente(_socket_cliente);
		pthread_exit(NULL);
		return;
	}
	Segmento *segmento_pcb =(Segmento*) list_get(tabla_de_segmentos, 1);

	agregar_a_memoria(segmento_pcb);
	sem_post(&direcciones);

	proceso->tabla = tabla_de_segmentos;
	pthread_mutex_lock(&listaPatotasEnUso);
	list_add(patotas, proceso);
	pthread_mutex_unlock(&listaPatotasEnUso);

	log_info(logs_ram, "Se inicio una patota exitosamente.");
	enviar_mensaje_simple("ok", _socket_cliente);
	liberar_cliente(_socket_cliente);
	free(datos_patota);
	pthread_exit(NULL);
}

bool noTieneMasTripulantes(t_list *tablaDePatota){

	bool _esPCB(void *algo){
		Segmento *elSegmento = (Segmento*) algo;
		return elSegmento->tipo == TCB;
	}

	bool resul = list_any_satisfy(tablaDePatota, _esPCB);
	return !resul;
}

void eliminarPatota(t_proceso *laPatota){
	void _eliminarSegmentos(void *algo){
		Segmento *unSegmento = (Segmento*) algo;
		eliminarSegmento(unSegmento->base);
		free(unSegmento);
	}

	list_destroy_and_destroy_elements(laPatota->tabla,_eliminarSegmentos);
	log_info(logs_ram, "Se elimino la patota %d", laPatota->pid);

	bool _esLaPatota(t_proceso* unaPatota){
		return (unaPatota->pid == laPatota->pid);
	}
	list_remove_by_condition(patotas,(void*) _esLaPatota);
	free(laPatota);
}

void eliminarSegmento(uint32_t baseSegmento){

	bool _esElSegmento(void *algo){
		Segmento *unSegmento = (Segmento*) algo;
		return (unSegmento->base == baseSegmento);
	}
	pthread_mutex_lock(&listaMemoriaPrincipal);
	list_remove_by_condition(memoriaPrincipal, _esElSegmento);
	pthread_mutex_unlock(&listaMemoriaPrincipal);
}

void eliminarTripulante(void *unTripulante){
	IdentificadorTripulante *tripulanteAEliminar = (IdentificadorTripulante*) unTripulante;
	int idTripulante = tripulanteAEliminar->idTripulante;
	int idPatota = tripulanteAEliminar->idPatota;
	void *elSegmento = NULL;

	bool _chequearSegmentosTCB(void *segmento) {
		Segmento *unSegmento = (Segmento*) segmento;
		if (unSegmento->tipo == TCB) {
			TripuCB *unTripulante = (TripuCB*) (unSegmento->dato);
			if(unTripulante->tid == idTripulante){
				log_info(logs_ram, "Se expulso al tripulante %d de la patota %d",idTripulante,idPatota);
				eliminarSegmento(unSegmento->base);
				eliminarTripuMapa(unSegmento->valorRepresentacion);
				elSegmento = (void*)unSegmento;
				return 1;
			}else{
				return 0;
			}
		} else {
			return 0;
		}
	}

	void _buscarTripulantes(void *proceso){
		t_proceso* unaPatota = (t_proceso*) proceso;
		if(unaPatota->pid == idPatota){
			t_list* segmentosProceso = unaPatota->tabla;
			list_remove_by_condition(segmentosProceso, _chequearSegmentosTCB);
			if(noTieneMasTripulantes(unaPatota->tabla)){
				eliminarPatota(unaPatota);//En caso de que el tripulante eliminado haya sido el ultimo
			}
		}
	}
	pthread_mutex_lock(&listaPatotasEnUso);
	list_iterate(patotas, _buscarTripulantes);
	pthread_mutex_unlock(&listaPatotasEnUso);
	free(tripulanteAEliminar);
	free(elSegmento);
}

Segmento *buscarTripulante(int idTripulante,int idPatota){
	Segmento *segmentoDelTripulante = NULL;

	void _chequearSegmentosTCB(void *segmento) {
		Segmento *unSegmento = (Segmento*) segmento;
		if (unSegmento->tipo == TCB) {
			TripuCB *unTripulante = (TripuCB*) unSegmento->dato;
			if(unTripulante->tid == idTripulante){segmentoDelTripulante = unSegmento;};
		}
	}

	void _recorrerProcesos(void *proceso){
		t_proceso* unaPatota = (t_proceso*) proceso;
		if(unaPatota->pid == idPatota){
			t_list* segmentosProceso = unaPatota->tabla;
			list_iterate(segmentosProceso, _chequearSegmentosTCB);
		}
	}
	sleep(2);
	//Hacer que itere entre cada uno de los procesos, y luego cada uno
	//de sus segmentos
	pthread_mutex_lock(&listaPatotasEnUso);
	list_iterate(patotas, _recorrerProcesos);
	pthread_mutex_unlock(&listaPatotasEnUso);
	/*if(patotaCreada(idPatota)){
		if(segmentoDelTripulante == NULL){
			log_info(logs_ram, "Soy pelotudo y me quede en este semaforo xd");
			sem_wait(&tripulantesDisponibles);//Si no existe, espera a que se haga el post y buscar de nuevo
			segmentoDelTripulante = buscarTripulante( idTripulante, idPatota);
		}
	}*/

	return segmentoDelTripulante;
}

void actualizarTripulante(t_tripulante_iniciado *tripulanteActualizado){
	//Se espera que ubicacion vengaa en un string del estilo "1|2"
	int idTripulante = tripulanteActualizado->tid;
	int idPatota = tripulanteActualizado->numPatota;

	//Busco tripulante y su segmento
	Segmento *SegmentoDelTripulante = buscarTripulante(idTripulante, idPatota);
	if(SegmentoDelTripulante == NULL){
		log_info(logs_ram, "Este tripulante no esta en memoria");
		free(tripulanteActualizado);
		pthread_exit(NULL);
	}
	sem_wait(&direcciones);
	TripuCB *elTripulante = (TripuCB*)SegmentoDelTripulante->dato;

	//Calculo cuanto se va a mover y lo muevo en el mapa
	int difX = tripulanteActualizado->posX - elTripulante->posX;
	int difY = tripulanteActualizado->posY - elTripulante->posY;
	moverTripuMapa(SegmentoDelTripulante->valorRepresentacion,difX, difY);

	//Asigno la nueva posicion
	elTripulante->posX = tripulanteActualizado->posX;
	elTripulante->posY = tripulanteActualizado->posY;
	elTripulante->status = tripulanteActualizado->status;
	sem_post(&direcciones);
	log_info(logs_ram,"El tripulante %d de la patota %d se movio a: %d|%d. Y su estatus actual es: %c.",
			idTripulante, idPatota, tripulanteActualizado->posX,tripulanteActualizado->posY,tripulanteActualizado->status);
	free(tripulanteActualizado);
}

Segmento *buscarSegmento(uint32_t numeroSegmento){

	bool _esElSegmento(void *algo){
		Segmento *unSegmento = (Segmento*) algo;
		return (unSegmento->idSegmentoUnico == numeroSegmento);
	}


	pthread_mutex_lock(&listaMemoriaPrincipal);
	Segmento *elSegmento = (Segmento*) list_find(memoriaPrincipal, _esElSegmento);
	pthread_mutex_unlock(&listaMemoriaPrincipal);
	return elSegmento;
}

char *buscarTarea(uint32_t idSegmentoTareas, int indiceTarea){
	Segmento *segmentoTareas = buscarSegmento(idSegmentoTareas);
	char *todasLasTareas = (char*) segmentoTareas->dato;
	char **tareasSeparadas = string_split(todasLasTareas, "\n");
	//Separe el string
	return tareasSeparadas[indiceTarea];
}

void enviarTareaSiguiente(void *unTripulante){
	TripulanteConSocket *elTripuConSocket = (TripulanteConSocket*) unTripulante;
	int cliente = elTripuConSocket->socket;
	t_tripulante_iniciado *tripulante = (t_tripulante_iniciado*) elTripuConSocket->tripulante;
	int idTripulante = tripulante->tid;
	int idPatota = tripulante->numPatota;
	log_info(logs_ram, "El tripulante %d de la patota %d pidio tarea.",idTripulante,idPatota);
	Segmento *SegmentoDelTripulante = buscarTripulante(idTripulante, idPatota);
	if(SegmentoDelTripulante == NULL){
		log_info(logs_ram, "Segmento Tripu Vacio");
		liberar_cliente(cliente);
		free(elTripuConSocket);
		pthread_exit(NULL);
	}
	sem_wait(&direcciones);
	TripuCB *elTripulante = (TripuCB*)SegmentoDelTripulante->dato;
	int proximaTarea = (int) elTripulante->proxIns;
	elTripulante->proxIns +=1; //Se asigna la siguiente tarea en RAM
	Segmento *segmentoPatotaDelTripulante = buscarSegmento(elTripulante->pcb);
	PatotaCB *PatotaDelTripu = (PatotaCB*) segmentoPatotaDelTripulante->dato;
	char* tarea = buscarTarea(PatotaDelTripu->tareas, proximaTarea);
	sem_post(&direcciones);

	if(tarea == NULL){
	        log_info(logs_ram, "Tripulante %d no tiene mas tareas.", idTripulante);
	        enviar_mensaje(PEDIDO_TAREA, "null", cliente);
	        liberar_cliente(cliente);
	        pthread_exit(NULL);
	    }

	log_info(logs_ram,"Tripulante %d pidio la tarea %s.", idTripulante, tarea);
	enviar_mensaje(PEDIDO_TAREA, tarea, cliente);
	free(elTripuConSocket);
	liberar_cliente(cliente);
}
//-------------------------------------------------------------------------------
//---------------------FUNCIONES ATENCION DE CLIENTE ----------------------------
//-------------------------------------------------------------------------------

void iniciarPatotaSeg(t_list *lista, int cliente){
	lista = recibir_paquete(cliente);
	t_datos_inicio_patota *datos_inicio = malloc(sizeof(t_datos_inicio_patota));
	datos_inicio->cantidad_tripulantes = atoi(list_get(lista, 0));
	datos_inicio->contenido_tareas     = list_get(lista, 1);
	datos_inicio->socket 			   = cliente;

	pthread_t hiloCreacionPatota;

	pthread_create(&hiloCreacionPatota, NULL, (void*)crear_proceso, (void*)datos_inicio);
	pthread_detach(hiloCreacionPatota);
}

void eliminarTripulanteSeg(t_list *lista, int cliente){
	lista = recibir_paquete(cliente);

	IdentificadorTripulante *unTripulante = malloc(sizeof(IdentificadorTripulante));
	unTripulante->idTripulante = atoi(list_get(lista,0));
	unTripulante->idPatota = atoi(list_get(lista,1));

	pthread_t hiloEliminacionTripulante;
	pthread_create(&hiloEliminacionTripulante, NULL, (void*)eliminarTripulante, (void*)unTripulante);
	pthread_detach(hiloEliminacionTripulante);
	liberar_cliente(cliente);
}

void actualizarPosicionSeg(t_list *lista, int cliente){
	t_tripulante_iniciado *tripulante_desplazado = recibir_tripulante_iniciado(cliente);

	pthread_t hiloActualizacionTripulante;
	pthread_create(&hiloActualizacionTripulante, NULL, (void*)actualizarTripulante, (void*)tripulante_desplazado);
	pthread_detach(hiloActualizacionTripulante);
	liberar_cliente(cliente);
}

void crearTripulanteSeg(t_list *lista, int cliente){
	t_tripulante_iniciado *nuevo_tripulante= recibir_tripulante_iniciado(cliente);
	TripulanteConSocket *nuevo_tripulante_con_socket = malloc(sizeof(TripulanteConSocket));
	nuevo_tripulante_con_socket->tripulante = nuevo_tripulante;
	nuevo_tripulante_con_socket->socket     = cliente;

	pthread_t hiloTripulante;

	pthread_create(&hiloTripulante, NULL, (void*)crear_segmento_tcb,(void*)nuevo_tripulante_con_socket);
	pthread_detach(hiloTripulante);
}

void obtenerSgteTareaSeg(t_list *lista, int cliente){
	t_tripulante_iniciado *tripulante_tarea = recibir_tripulante_iniciado(cliente);
	TripulanteConSocket *tripulante_con_socket = malloc(sizeof(TripulanteConSocket));
	tripulante_con_socket->tripulante = tripulante_tarea;
	tripulante_con_socket->socket     = cliente;

	pthread_t hiloPedidoTarea;
	pthread_create(&hiloPedidoTarea, NULL, (void*)enviarTareaSiguiente,(void*)tripulante_con_socket);
	pthread_detach(hiloPedidoTarea);
}

void inicializarSegmentacion(){
	//Se establece el algoritmo de ubicacion
	signal(SIGINT, compactacion);
	char* algoritmoUbicacion =config_get_string_value(config, "ALGORITMO_UBICACION");
	if(strcmp(algoritmoUbicacion, "FF") == 0){
		esFF = true;
	}else{
		esFF = false; //Entonces es Best Fit (BF)
	}
	sem_init(&direcciones,0,1);
	sem_init(&numeroPatotas,0,1);
	sem_init(&tripulantesDisponibles,0,0);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void dumpMemoriaSeg(){
	log_info(logs_ram, "Ejecutando Dump de memoria...");
	int idPatota;
	char *horaActual = temporal_get_string_time("%d-%m-%y_%H:%M:%S\n\n");
	char *nombreArchivo = string_new();
	string_append(&nombreArchivo,"dump_");
	string_append(&nombreArchivo,horaActual);
	string_append(&nombreArchivo,".txt");

	char* ruta = string_new();
	ruta = string_from_format("./Dump/%s",nombreArchivo);

	FILE* archivo = txt_open_for_append(ruta);
	char* textoAEscribir = string_new();
	string_append(&textoAEscribir,"Dump :");
	string_append(&textoAEscribir,horaActual);

	void _recorrerSegmentos(void* segmento){
		Segmento *unSegmento = (Segmento*) segmento;
		int idSegmento = unSegmento->idSegmento;
		uint32_t base = unSegmento->base;
		int tamanio = unSegmento->tamanio;


		char* dumpMarco = string_from_format("Proceso:%d    Segmento:%d      Inicio:0x%d    Tamanio:%db \n",idPatota, idSegmento, base, tamanio);
		txt_write_in_file(archivo, dumpMarco);
		free(dumpMarco);
	}

	void _recorrerPatotas(void* proceso){
		t_proceso *unProceso = (t_proceso*) proceso;
		idPatota = unProceso->pid;
		list_iterate(unProceso->tabla, _recorrerSegmentos);

	}



	txt_write_in_file(archivo, "---------------------------------\n\n");
	txt_write_in_file(archivo, textoAEscribir);

	if(list_is_empty(patotas)){
		txt_write_in_file(archivo,"La memoria esta vacia.\n\n" );
	}else{
		list_iterate(patotas, _recorrerPatotas);
		txt_write_in_file(archivo,"\n");
	}

	txt_write_in_file(archivo, "---------------------------------");
	free(horaActual);
	free(nombreArchivo);
	txt_close_file(archivo);
	free(ruta);
	free(textoAEscribir);
	log_info(logs_ram, "Dump de memoria exitoso.");
}

void cerrarMemoriaSeg(){
	pthread_mutex_lock(&listaPatotasEnUso);
	list_iterate(patotas, (void*)eliminarPatota);
	pthread_mutex_unlock(&listaPatotasEnUso);
}
