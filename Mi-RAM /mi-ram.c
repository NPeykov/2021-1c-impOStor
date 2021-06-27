#include "mi-ram.h"

uint32_t calcular_base_logica(Segmento *segmento){
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
//TODO: ACA DEBERIA IR COMPACTACION POR SI NO SE ENCUENTRA ESPACIO.
		}else{
			return 0; //No hay espacio
		}
	}

	//Si no hay nada en memoria principal la dir es 0 por ser primero
	if(list_is_empty(memoriaPrincipal)){
		return (uint32_t) 0;
	}

	list_find(memoriaPrincipal, espacioLibre);
	if(tamaniomemoria > finalSegmentoAnterior + tamanioNecesario){
		return finalSegmentoAnterior;
	}else{
		return -1; //Hubo un error
	}

}

int crear_segmento_tareas(char *tareas[], t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(Segmento));

	//Se llena el segmento
	segmento->tipo = TAREAS;
	segmento->dato = tareas;
	segmento->tamanio = sizeof(tareas);
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

int crear_segmento_tcb(uint32_t numero_tripulante, uint32_t posX, uint32_t posY, uint32_t segmento_pcb, t_list* tabla_segmentos) {
	Segmento *segmento = (Segmento*) malloc(sizeof(Segmento));

	TripuCB *tcb = (TripuCB*) malloc(sizeof(tcb));
	tcb->tid = numero_tripulante;
	tcb->pcb = segmento_pcb;
	tcb->posX = posX;
	tcb->posY = posY;
	tcb->status = 'N';//Estado New

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = TCB;
	segmento->dato = tcb;
	segmento->tamanio = sizeof(TripuCB);
	segmento->base = calcular_base_logica(segmento);

	if(segmento->base == -1){
		return -1;//Por si hay error retorna -1
	}else{
		return 0;
	}
}

void verificarSegmento(int resultado_creacion_segmento){
	return;
	//TODO: cuando el resultado sea -1 avisar a discordiador
}

void crear_proceso(t_list *paquete){
	t_list* tabla_de_segmentos = list_create();

	int result_tareas =crear_segmento_tareas(list_get(paquete, 2), tabla_de_segmentos);
	Segmento *segmento_tareas =(Segmento*) list_get(tabla_de_segmentos, 0);
	uint32_t inicioTareas = segmento_tareas->base;//Sabemos que siempre se empieza por las tareas
	verificarSegmento(result_tareas);

	int *result_pcb =crear_segmento_pcb(inicioTareas, tabla_de_segmentos);
	Segmento *segmento_pcb =(Segmento*) list_get(tabla_de_segmentos, 1);
	verificarSegmento(result_pcb);
//Hasta aca bien

	int cantidad_tripulantes = (int) list_get(paquete, 0);
	char* posiciones = list_get(paquete, 1);
	char **list_pos = string_split(posiciones, " ");
	printf("%s\n", list_pos[0]);
	char **posicion_del_tripulante;

	for(int i=0; i <= cantidad_tripulantes ; i++){
		posicion_del_tripulante = string_split(posiciones[i], "|");
		int *result_tcb = crear_segmento_tcb((uint32_t*) i,(uint32_t*) posicion_del_tripulante[0],(uint32_t*) posicion_del_tripulante[1], segmento_pcb->base, tabla_de_segmentos);
		verificarSegmento(result_tcb);
	}

		t_proceso *proceso = (t_proceso*) malloc(sizeof(t_proceso));
		proceso->id = numero_patota;
		proceso->tabla_de_segmentos = tabla_de_segmentos;
		list_add(patotas, proceso);
		numero_patota += 1;
	//Hacer post al mutex
}



// Eliminacion de Tripulante

void eliminarTripulante(int idTripulante){

	bool chequearSegmentosTCB(void *segmento) {
		Segmento *unSegmento = (Segmento*) segmento;
		if (unSegmento->tipo == TCB) {
			TripuCB *unTripulante = unSegmento->dato;
			return unTripulante->tid == idTripulante;
		} else {
			return 0;
		}
	}

	void buscarTripulantes(t_proceso *proceso){
		t_list* segmentosProceso = proceso->tabla_de_segmentos;
		list_remove_by_condition(segmentosProceso, chequearSegmentosTCB);
	}

	list_iterate(patotas, buscarTripulantes);
}

TripuCB *buscarTripulante(int idTripulante){
	TripuCB *elTripulante;

	bool chequearSegmentosTCB(void *segmento) {
		Segmento *unSegmento = (Segmento*) segmento;
		if (unSegmento->tipo == TCB) {
			TripuCB *unTripulante = unSegmento->dato;
			return unTripulante->tid == idTripulante;
		} else {
			return 0;
		}
	}

	void recorrerProcesos(t_proceso *proceso){
		t_list* segmentosProceso = proceso->tabla_de_segmentos;
		elTripulante = (TripuCB*) list_find(segmentosProceso, chequearSegmentosTCB);
	}

	//Hacer que itere entre cada uno de los procesos, y luego cada uno
	//de sus segmentos
	list_iterate(patotas, recorrerProcesos);
	return elTripulante;
}

void *actualizarTripulante(int idTripulante, char *ubicacion){
	//Se espera que ubicacion vengaa en un string del estilo "1|2"
	uint32_t posicionX;
	uint32_t posicionY;
	posicionX = (uint32_t) (ubicacion[0]);
	posicionY = (uint32_t) (ubicacion[2]);

	TripuCB *elTripulante = buscarTripulante(idTripulante);
	elTripulante->posX = posicionX;
	elTripulante->posY = posicionY;


	return 0;
}

void *gestionarClienteSeg(int socket) {

	void iterator(char* value) {printf("%s\n", value);}

	int operacion;
	t_list *lista;
	int cliente;


	int idTripulante;

	while(1) {
		cliente = esperar_cliente(socket);
		printf("Cliente: %d\n", cliente);

		operacion = recibir_operacion(cliente);
		lista = NULL;

		printf("\nLA OPERACION ES: %d\n", operacion);

		switch(operacion) {
			case INICIO_PATOTA:
				lista = recibir_paquete(cliente);
				char *contenido;
				char *posiciones;
				char *cantidad;

				cantidad = list_get(lista, 0);
				posiciones = list_get(lista, 1);
				contenido = list_get(lista, 2);

				log_info(logs_ram, "Se iniciaron %s tripulantes", cantidad);


				/*hardcodeo esto por la respues de si se puede crear o no una patota*/
				if(true){
					enviar_mensaje_simple("ok", cliente);
					//send(cliente, "ok", string_length("ok") + 1, 0);
				}

				else enviar_mensaje_simple("no", cliente);


				printf("Contenido: %s\n", contenido);
				//Agregar mutex
				//crear_proceso(lista);
				break;

			case ELIMINAR_TRIPULANTE:
				lista = recibir_paquete(cliente);
				uint32_t idTripulante = (uint32_t)((char *) list_get(lista,0));
				//eliminarTripulante(idTripulante);
				printf("Tripulante eliminado de la nave %d\n", idTripulante);
				//liberar_cliente(cliente);
				break;

			case ACTUALIZAR_POSICION:;

				t_tripulante_iniciado *tripulante_desplazado = recibir_tripulante_iniciado(cliente);

				printf("Tripulante %d se movio a (%d, %d)",
						tripulante_desplazado->tid,
						tripulante_desplazado->posX,
						tripulante_desplazado->posY);

				//lista = recibir_paquete(cliente);
				//idTripulante = atoi((char *) list_get(lista,0));
				break;

			case NUEVO_TRIPULANTE:;
				t_tripulante_iniciado *nuevo_tripulante= recibir_tripulante_iniciado(cliente);
				printf("%s\n", nuevo_tripulante->status);
				printf("%d\n", nuevo_tripulante->tid);

				break;

			case PEDIDO_TAREA:;
				char *ejemplo_tarea = "COMER;10;14;15"; //hardcodeo un string para probar desde discordiado

				//recibo datos del tripulante para buscarlo (ignoro datos q no me sirven)
				t_tripulante_iniciado *tripulante_tarea = recibir_tripulante_iniciado(cliente);


				printf("Tripulante %d pidio tarea.\n", tripulante_tarea->tid);
				/*log_info(logs_ram, "Tripulante %d pidio tarea.",
						tripulante_desplazado->tid);*/

				enviar_mensaje(PEDIDO_TAREA, ejemplo_tarea, cliente);
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

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");

	logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 1, LOG_LEVEL_DEBUG);

	socket_mi_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");

	tamaniomemoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	memoria = malloc(tamaniomemoria);
	patotas = list_create();

	if(strcmp(tipoMemoria, "SEGMENTACION") == 0){
		//Agregar Hilos
		gestionarClienteSeg(socket_mi_ram);
	}else{
		//Agregar Hilos

		//gestionarClientePag(socket_mi_ram);
	}
}


int main(){
  inicializar_ram();

  return EXIT_SUCCESS;
}
