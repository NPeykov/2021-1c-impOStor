#include "segmentacion.h"

void compactacion(){
	uint32_t finalSegmentoAnterior = 0;
	uint32_t inicioSegmentoActual = 0;

	void _recorrerSegmentos(void *algo){
		Segmento *unSegmento = (Segmento*) algo;
		inicioSegmentoActual = unSegmento->base;
		if(finalSegmentoAnterior == 0){//El primer segmento
			finalSegmentoAnterior = unSegmento->tamanio + unSegmento->base;
		}else if(finalSegmentoAnterior == inicioSegmentoActual){//Empieza donde termina el otro
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
		}else if(inicioSegmentoActual > finalSegmentoAnterior){//Hay un espacio
			memcpy(memoria + finalSegmentoAnterior, unSegmento->dato, unSegmento->tamanio);
			printf("Se compacto un espacio");
		}
	}

	list_iterate(memoriaPrincipal, _recorrerSegmentos);
	noCompactado = false;
}

uint32_t algoritmoBestFit(Segmento *segmento){
	uint32_t tamanioNecesario =(uint32_t) segmento->tamanio;
	uint32_t finalSegmentoAnterior = 0;
	uint32_t inicioSegmentoActual = 0;
	uint32_t espacioLibreUbicacion = (uint32_t) tamaniomemoria; //Cuanto espacio queda libre si se coloca en ubicacionMasJusta
	uint32_t ubicacionMasJusta = 0; //Aca se va guardando la mejor posicion encontrada
	printf("Calcule una base logica\n");
	//ESTO SERIA FIRST FIT
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
			if(( inicioSegmentoActual - finalSegmentoAnterior + tamanioNecesario) < espacioLibreUbicacion){
				ubicacionMasJusta = inicioSegmentoActual + unSegmento->tamanio;
				espacioLibreUbicacion = inicioSegmentoActual - finalSegmentoAnterior + tamanioNecesario;
			}
		}else{
			finalSegmentoAnterior = inicioSegmentoActual + unSegmento->tamanio;
		}
	}

	//Si no hay nada en memoria principal la dir es 0 por ser primero
	if(list_is_empty(memoriaPrincipal)){
		return (uint32_t) 0;
	}

	list_iterate(memoriaPrincipal, espacioLibre);//Este es el mayor cambio entre FF y BF
	if(tamaniomemoria >= ubicacionMasJusta + tamanioNecesario){
		return ubicacionMasJusta;
	}else if(tamaniomemoria >= finalSegmentoAnterior + tamanioNecesario){
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
	printf("Calcule una base logica\n");
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
		return (uint32_t) 0;
	}

	list_find(memoriaPrincipal, espacioLibre);
	if(tamaniomemoria >= finalSegmentoAnterior + tamanioNecesario){
		return finalSegmentoAnterior;
	}else{
		if(noCompactado){
			compactacion();//Se compacta y se hace de nuevo
			finalSegmentoAnterior =  algoritmoFirstFit(segmento);
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
	segmento->tipo = TAREAS;
	segmento->dato = tareas;
	segmento->tamanio = string_length(tareas);
	segmento->base = calcular_base_logica(segmento);
	segmento->idSegmento = tabla_segmentos->elements_count;

	//Se lo agrega a la tabla de Segmentos del proceso actual
	list_add(tabla_segmentos, segmento);

	if(segmento->base == -1){
		return -1;//Por si hay error retorna -1
	}else{
		return 0;
	}
}

int crear_segmento_pcb(uint32_t inicioTareas, t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(Segmento));

	//Se llena la estructura de PatotaCB
	PatotaCB *pcb = (PatotaCB*) malloc(sizeof(PatotaCB));
	pcb->pid = numero_patota;
	pcb->tareas = inicioTareas;

	//Se llena la informacion del Segmento
	segmento->idSegmento = tabla_segmentos->elements_count;
	segmento->tipo = PCB;
	segmento->dato = pcb;
	segmento->tamanio = sizeof(PatotaCB);
	segmento->base = calcular_base_logica(segmento);

	//Se lo agrega a la tabla de Segmentos del proceso actual
	list_add(tabla_segmentos, segmento);

	if(segmento->base == -1){
		return -1;//Por si hay error retorna -1
	}else{
		return 0;
	}
}

int crear_segmento_tcb(t_tripulante_iniciado *unTripulante, int cliente) {
	Segmento *segmento = (Segmento*) malloc(sizeof(Segmento));

	//Para buscar su patota
	int patota = (int) unTripulante->numPatota;
	bool _esLaPatota(void *proceso){
		t_proceso *unProceso = (t_proceso*) proceso;
		return unProceso->pid == patota;
	}

	//Para obtener los valores de la patota
	t_proceso *miPatota = (t_proceso*) list_find(patotas, _esLaPatota);
	t_list *tabla_segmentos = miPatota->tabla;
	Segmento *laPatota = (Segmento*) list_get(tabla_segmentos, 1);//Porque sabemos que se crea tarea->patota

	//Se asignan los valores a la TCB
	TripuCB *tcb = (TripuCB*) malloc(sizeof(TripuCB));
	tcb->tid = unTripulante->tid;
	tcb->pcb = laPatota->base;
	tcb->posX = unTripulante->posX;
	tcb->posY = unTripulante->posY;
	tcb->status = unTripulante->status;
	tcb->proxIns = (uint32_t) 0;

	//Se asigna el acceso rapido de t_proceso

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = TCB;
	segmento->dato = tcb;
	segmento->tamanio = sizeof(TripuCB);
	segmento->base = calcular_base_logica(segmento);

	if(segmento->base == -1){
		enviar_mensaje_simple("no", cliente);
		return -1;//Por si hay error retorna -1
	}else{
		enviar_mensaje_simple("ok", cliente);
		return 0;
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
	list_add_sorted(memoriaPrincipal, unSegmento, _laBaseEsMenor );
	//El t_list memoriaPrincipal se usa para hacer la compactacion
}

void crear_proceso(char* contenido, int cliente){
	t_list* tabla_de_segmentos = list_create();

	//Se crea el segmento de tareas
	int result_tareas =crear_segmento_tareas(contenido, tabla_de_segmentos);
	Segmento *segmento_tareas =(Segmento*) list_get(tabla_de_segmentos, 0);
	uint32_t inicioTareas = segmento_tareas->base;//Sabemos que siempre se empieza por las tareas
	if(result_tareas == -1){
		enviar_mensaje_simple("no", cliente);
		return;
	}

	//Se crea el segmento PCB
	int result_pcb =crear_segmento_pcb(inicioTareas, tabla_de_segmentos);
	Segmento *segmento_pcb =(Segmento*) list_get(tabla_de_segmentos, 1);
	//verificarSegmento(result_pcb, cliente);
	if(result_pcb == -1){
		enviar_mensaje_simple("no", cliente);
		return;
	}

	//Se crea t_proceso como accedo rapido a los segmentos de la patota
	t_proceso *proceso = (t_proceso*) malloc(sizeof(t_proceso));
	proceso->pid = numero_patota;
	proceso->tabla = tabla_de_segmentos;
	list_add(patotas, proceso);
	numero_patota += 1;

	//Se agregan ambos segmentos al malloc
	agregar_a_memoria(segmento_tareas);
	agregar_a_memoria(segmento_pcb);
	enviar_mensaje_simple("ok", cliente);
}

void eliminarTripulante(int idTripulante,int idPatota){

	bool _chequearSegmentosTCB(void *segmento) {
		Segmento *unSegmento = (Segmento*) segmento;
		if (unSegmento->tipo == TCB) {
			TripuCB *unTripulante = (TripuCB*) (unSegmento->dato);
			if(unTripulante->tid == idTripulante){
				free(unSegmento); //Para que se borre el nodo del segmento
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
		}
	}

	list_iterate(patotas, _buscarTripulantes);
}

TripuCB *buscarTripulante(int idTripulante,int idPatota){
	TripuCB *elTripulante;

	bool _chequearSegmentosTCB(void *segmento) {
		Segmento *unSegmento = (Segmento*) segmento;
		if (unSegmento->tipo == TCB) {
			TripuCB *unTripulante = unSegmento->dato;
			return unTripulante->tid == idTripulante;
		} else {
			return 0;
		}
	}

	void _recorrerProcesos(void *proceso){
		t_proceso* unaPatota = (t_proceso*) proceso;
		if(unaPatota->pid == idPatota){
			t_list* segmentosProceso = unaPatota->tabla;
			elTripulante = (TripuCB*) list_find(segmentosProceso, _chequearSegmentosTCB);
		}
	}

	//Hacer que itere entre cada uno de los procesos, y luego cada uno
	//de sus segmentos
	list_iterate(patotas, _recorrerProcesos);
	return elTripulante;
}

void actualizarTripulante(t_tripulante_iniciado *tripulanteActualizado){
	//Se espera que ubicacion vengaa en un string del estilo "1|2"
	uint32_t posicionX;
	uint32_t posicionY;
	posicionX = tripulanteActualizado->posX;
	posicionY = tripulanteActualizado->posY;
	int idTripulante = tripulanteActualizado->tid;
	int idPatota = tripulanteActualizado->numPatota;

	TripuCB *elTripulante = buscarTripulante(idTripulante, idPatota);
	elTripulante->posX = posicionX;
	elTripulante->posY = posicionY;
	elTripulante->status = tripulanteActualizado->status;
}

Segmento *buscarSegmento(uint32_t baseSegmento){

	bool _esElSegmento(void *algo){
		Segmento *unSegmento = (Segmento*) algo;
		return (unSegmento->base == baseSegmento);
	}

	Segmento *elSegmento = (Segmento*) list_find(memoriaPrincipal, _esElSegmento);
	return elSegmento;
}

char *buscarTarea(uint32_t baseSegmentoTareas, int indiceTarea){
	Segmento *segmentoTareas = buscarSegmento(baseSegmentoTareas);
	char *todasLasTareas = (char*) segmentoTareas->dato;
	char **tareasSeparadas = string_split(todasLasTareas, "\n");
	return tareasSeparadas[indiceTarea];
}

char *obtenerTareaSiguiente(t_tripulante_iniciado *tripulante){
	int idTripulante = tripulante->tid;
	int idPatota = tripulante->numPatota;

	TripuCB *elTripulante = buscarTripulante(idTripulante, idPatota);
	int proximaTarea = (int) elTripulante->proxIns;
	Segmento *segmentoPatotaDelTripulante = buscarSegmento(elTripulante->pcb);
	PatotaCB *PatotaDelTripu = (PatotaCB*) segmentoPatotaDelTripulante->dato;
	return buscarTarea(PatotaDelTripu->tareas, proximaTarea);


}

void *gestionarClienteSeg(int socket) {

	void iterator(char* value) {printf("%s\n", value);}

	int operacion;
	t_list *lista;
	int cliente;


	int idTripulante;

	while(1) {
		cliente = esperar_cliente(socket);

		operacion = recibir_operacion(cliente);
		lista = NULL;

		printf("\nLA OPERACION ES: %d\n", operacion);

		switch(operacion) {
			case INICIO_PATOTA:
				lista = recibir_paquete(cliente);
				char *contenido;
				char *cantidad;
				cantidad = list_get(lista, 0);
				contenido = list_get(lista, 2);

				crear_proceso(contenido, cliente);

				log_info(logs_ram, "Se inicio una patota.\n", cantidad);
				liberar_cliente(cliente);

				//hardcodeo esto por la respues de si se puede crear o no una patota
				//Agregar mutex
				break;

			case ELIMINAR_TRIPULANTE:
				lista = recibir_paquete(cliente);
				int idTripulante = atoi(list_get(lista,0));
				int idPatota = (int)((char *) list_get(lista,1));
				eliminarTripulante(idTripulante, idPatota);
				printf("Tripulante %d de la patota %d eliminado de la nave\n", idTripulante, idPatota);
				liberar_cliente(cliente);
				break;

			case ACTUALIZAR_POSICION:;

				t_tripulante_iniciado *tripulante_desplazado = recibir_tripulante_iniciado(cliente);

				actualizarTripulante(tripulante_desplazado);
				liberar_cliente(cliente);
				break;

			case NUEVO_TRIPULANTE:;
				t_tripulante_iniciado *nuevo_tripulante= recibir_tripulante_iniciado(cliente);
				crear_segmento_tcb(nuevo_tripulante, cliente);
				printf("Se creo un nuevo tripulante.\n");
				liberar_cliente(cliente);
				break;

			case PEDIDO_TAREA:;
				 //char *ejemplo_tarea = "COMER;10;14;15";hardcodeo un string para probar desde discordiado

				//recibo datos del tripulante para buscarlo (ignoro datos q no me sirven)
				t_tripulante_iniciado *tripulante_tarea = recibir_tripulante_iniciado(cliente);

				char* tarea = obtenerTareaSiguiente(tripulante_tarea);

				printf("Tripulante %d pidio tarea.\n", tripulante_tarea->tid);
				/*log_info(logs_ram, "Tripulante %d pidio tarea.",
						tripulante_desplazado->tid);*/

				enviar_mensaje(PEDIDO_TAREA, tarea, cliente);
				liberar_cliente(cliente);
				break;

			case -1:
				printf("El cliente %d se desconecto.\n", cliente);
				//liberar_cliente(cliente);
				break;

			default:
				printf("Operacion desconocida.\n");
				break;
		}

		liberar_cliente(cliente);
	}
}
/*
void dumpMemoriaSeg(){
	int idPatota;
	char *horaActual = *temporal_get_string_time("%d-%m-%y_%H:%M:%S");
	char *nombreArchivo = "dump_"+horaActual+".dmp";

	char* ruta = string_from_format("./Dump/%s",nombreArchivo);

	FILE* archivo = txt_open_for_append(ruta);
	char* textoAEscribir = "Dump :" + horaActual;

	void _recorrerSegmentos(void* segmento){
		Segmento *unSegmento = (Segmento*) segmento;
		int idSegmento = unSegmento->idSegmento;
		uint32_t base = unSegmento->base;
		int tamanio = unSegmento->tamanio;


		char* dumpMarco = string_from_format("Proceso:%d    Segmento:%d      Inicio:0x%x    Tamanio:%db \n",idPatota, idSegmento, base, tamanio);
		txt_write_in_file(archivo, dumpMarco);
	}

	void _recorrerPatotas(void* proceso){
		t_proceso *unProceso = (t_proceso*) proceso;
		idPatota = unProceso->pid;
		list_iterate(unProceso->tabla, _recorrerSegmentos);

	}



	txt_write_in_file(archivo, "---------------------------------\n");
	txt_write_in_file(textoAEscribir, textoAEscribir);

	list_iterate(patotas, _recorrerPatotas);

	txt_write_in_file(archivo, "---------------------------------\n");
}
*/

