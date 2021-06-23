#include "mi-ram.h"
/*
void crearProceso(t_list *paquete){
	t_list* tabla_de_segmentos = list_create();
	int tamanio = 0;

	Segmento *segmento_tareas=crear_segmento_tareas(list_get(paquete, 2), tabla_de_segmentos);

	tamanio += sizeof(segmento_tareas);

	Segmento *segmento_pcb=crear_segmento_pcb((uint32_t*) segmento_tareas, tabla_de_segmentos);

	tamanio += sizeof(segmento_pcb);

	int cantidad_tripulantes = list_get(paquete, 0);
	t_list* posiciones = list_get(paquete, 1);
	char **posicion_del_tripulante;

	for(int i=0; i <= cantidad_tripulantes ; i++){
		posicion_del_tripulante = string_split(posiciones[i], "|");
		Segmento *segmento_tcb = crear_segmento_tcb((uint32_t*) segmento_pcb, i, posicion_del_tripulante[0], posicion_del_tripulante[1], tabla_de_segmentos);
		tamanio += sizeof(segmento_tcb);
	}


	if(tamaniomemoria >= tamanio){
		t_proceso *proceso = (t_proceso*) malloc(sizeof(tamanio));
		proceso->id = numero_patota;
		proceso->tabla_de_segmentos = tabla_de_segmentos;
		proceso->memoriaPedida = tamanio; //creo que no hace falta tal vez sirve para liberar memoria
		tamaniomemoria -= tamanio;
		list_add(patotas, proceso);
		numero_patota += 1;
	}else{
		printf("Espacio en memoria insuficiente");
	}

	//Hacer post al mutex
}

uint32_t calcular_base_logica(Segmento *segmento, t_list* tabla_segmentos){
	int pos_seg = segmento->idSegmento;
	Segmento* segmento_anterior = (Segmento*) list_get(tabla_segmentos, pos_seg - 1);
	if(!segmento_anterior) {
		return 0;
	}

	return (*segmento_anterior)->base + (*segmento_anterior)->tamanio;
}

Segmento* crear_segmento_tareas(char *tareas[], t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(segmento));

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = TAREAS;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);
	segmento->dato = tareas;
	segmento->tamanio = sizeof(segmento);

	return segmento;
}

Segmento* crear_segmento_pcb(uint32_t segmento_tareas,t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(segmento));

	PatotaCB pcb = (PatotaCB*) malloc(sizeof(pcb));
	pcb->pid = numero_patota;
	pcb->tareas = segmento_tareas;

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = PCB;
	segmento->dato = pcb;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);

	return segmento;
}

Segmento* crear_segmento_tcb(uint32_t segmento_pcb, int numero_tripulante, uint32_t posX, uint32_t posY ,int tamanio, t_list* tabla_segmentos) {
	Segmento* segmento = (Segmento*) malloc(sizeof(segmento));

	TripuCB tcb = (TripuCB*) malloc(sizeof(tcb));
	tcb->tid = numero_tripulante;
	tcb->pcb = segmento_pcb;
	tcb->posX = posX;
	tcb->posY = posY;

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = TCB;
	segmento->dato = tcb;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);
	segmento->tamanio = sizeof(segmento);

	return segmento;
}

// Eliminacion de Tripulante



void eliminarTripulante(int idTripulante){

	bool laGarra(void *segmento){
		Segmento *unSegmento = (Segmento*) segmento;
		if(unSegmento->tipo == TCB){
			TripuCB tripulante = unSegmento->dato;
			return tripulante->tid == idTripulante;
		}else{
			return 0;
		}
	}
	void buscarTripulantes(t_proceso *proceso){
		t_list* segmentosProceso = proceso->tabla_de_segmentos;
		list_remove_by_condition(segmentosProceso, laGarra );
	}

	list_iterate(patotas, buscarTripulantes);




}

TripuCB *buscarTripulante(int idTripulante){
	TripuCB *elTripulante;

	TripuCB chequearSegmentosTCB(){
		Segmento *unSegmento = (Segmento*) segmento;
		if(unSegmento->tipo == TCB){
			elTripulante = unSegmento->dato;
			return tripulante->tid == idTripulante;
		}else{
			return 0;
		}
	}

	TripuCB *recorrerProcesos(t_proceso *proceso){
		t_list* segmentosProceso = proceso->tabla_de_segmentos;

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


// Gestion de Discordiador
void *gestionarClienteSeg(int socket) {
		int conexionCliente;
		t_list* lista;
		int operacion;
		t_paquete *paquete;
		int respuesta;

		while(1) {
			int cliente = esperar_cliente(socket);
			printf("Cliente: %d\n", cliente);
			operacion = recibir_operacion(cliente);
			lista = NULL;

			printf("\nLA OPERACION ES: %d\n", operacion);

			switch(operacion) {
				case INICIAR_PATOTA:
					lista = recibir_paquete(cliente);
					//Agregar mutex
					crear_proceso(lista);
					break;
				case EXPULSAR_TRIPULANTE:
					lista = recibir_paquete(cliente);
					int idTripulante = atoi((char *) list_get(lista,0));
					eliminarTripulante(idTripulante);
					printf("Tripulante eliminado de la nave %d\n", idTripulante);
					//liberar_cliente(cliente);
					break;
				case ACTUALIZAR_TRIPULANTE:
					lista = recibir_paquete(cliente);
					int idTripulante = atoi((char *) list_get(lista,0));
					break;
				case -1:
					printf("El cliente %d se desconecto.\n", cliente);
					//liberar_cliente(cliente);
					break;
				default:
					printf("Operacion desconocida.\n");
					break;

			}

		}
	}

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");
	//logger = log_create(archivoDeLog, "CoMAnda", 1, LOG_LEVEL_DEBUG);

	socket_mi_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");

	tamaniomemoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	memoria =malloc(tamaniomemoria);
	patotas = list_create();

	if(tipoMemoria == "SEGMENTACION"){
		//Agregar Hilos
		gestionarClienteSeg(socket_mi_ram);
	}else{
		//Agregar Hilos
		gestionarClientePag(socket_mi_ram);
	}
}*/

void atender_discordiador(){

	void iterator(char* value) {printf("%s\n", value);}

	int socket_ram = levantar_servidor(MI_RAM_HQ);
	printf("Escuchando..");
	int socket_cliente = esperar_cliente(socket_ram);
	logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 1, LOG_LEVEL_DEBUG);
	t_list *lista;

	while (1) {
		op_code cod_op;
		cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE: //MENSAJE (ENUM NO FUNCA)
			recibir_mensaje(socket_cliente);
			break;
		case PAQUETE: //PAQUETE (ENUM NO FUNCA)
			lista = recibir_paquete(socket_cliente);
			printf("Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case INICIO_PATOTA:
			;
			t_list* lista = recibir_paquete(socket_cliente);

			char *cantidad 		   = list_get(lista, 0);
			char *posiciones 	   = list_get(lista, 1);
			char *contenido_tareas = list_get(lista, 2);

			printf("\n-------ME LLEGARON DATOS DE PATOTA---------\n");
			printf("Cantidad: %s\n", cantidad);
			printf("Posiciones: %s\n", posiciones);
			printf("Tareas: %s\n", contenido_tareas);

			/*t_inicio_patota *datos_inicio_patota = recibir_datos_patota(socket_cliente);

			printf("\n-------ME LLEGARON DATOS DE PATOTA---------\n");
			printf("Cantidad: %d\n", datos_inicio_patota->cantidad);
			printf("Posiciones: %s\n", datos_inicio_patota->posiciones);
			printf("Tareas: %s\n", datos_inicio_patota->contenido_tareas);*/
			break;
		case -1:
			log_error(logs_ram, "el cliente se desconecto. Terminando servidor");
			return; //error
		default:
			for (int i = 0; i < 50; i++) //agregue esto, la ultima vez me genero 2gb de log
				log_warning(logs_ram,
						"Operacion desconocida. No quieras meter la pata");
			return; //error
		}
	}

}


int main(){
  //inicializar_ram();

	pthread_t hilo_discordiador;


	pthread_create(&hilo_discordiador, NULL, (void *) atender_discordiador, NULL);
	pthread_join(hilo_discordiador, NULL);

  return EXIT_SUCCESS;
}
