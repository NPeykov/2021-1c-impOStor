#include "mi-ram.h"

uint32_t calcular_base_logica(Segmento *segmento, t_list* tabla_segmentos){
	int pos_seg = segmento->idSegmento;
	Segmento* segmento_anterior = (Segmento*) list_get(tabla_segmentos, pos_seg - 1);

	if(!segmento_anterior) {
		return 0;
	}

	return (*segmento_anterior).base + (*segmento_anterior).tamanio;
}

Segmento* crear_segmento_tareas(char *tareas[], t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(Segmento));

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = TAREAS;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);
	segmento->dato = tareas;
	segmento->tamanio = sizeof(segmento);

	return segmento;
}

Segmento* crear_segmento_pcb(uint32_t segmento_tareas, t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(Segmento));

	PatotaCB *pcb = (PatotaCB*) malloc(sizeof(PatotaCB));
	pcb->pid = numero_patota;
	pcb->tareas = segmento_tareas;

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = PCB;
	segmento->dato = pcb;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);

	return segmento;
}

Segmento* crear_segmento_tcb(uint32_t numero_tripulante, uint32_t posX, uint32_t posY, uint32_t segmento_pcb, t_list* tabla_segmentos) {
	Segmento *segmento = (Segmento*) malloc(sizeof(Segmento));

	TripuCB *tcb = (TripuCB*) malloc(sizeof(tcb));
	tcb->tid = numero_tripulante;
	tcb->pcb = segmento_pcb;
	tcb->posX = posX;
	tcb->posY = posY;
	tcb->status = 'N';

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);
	segmento->tipo = TCB;
	segmento->dato = tcb;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);
	segmento->tamanio = sizeof(segmento);

	return segmento;
}

void crear_proceso(t_list *paquete){
	t_list* tabla_de_segmentos = list_create();
	int tamanio = 0;

	Segmento *segmento_tareas=crear_segmento_tareas(list_get(paquete, 2), tabla_de_segmentos);

	//TAMANIO SUMAR TODOS LOS ELEMENTOS DEL SEGMENTO

	tamanio += sizeof(segmento_tareas);

	Segmento *segmento_pcb=crear_segmento_pcb((uint32_t*) segmento_tareas, tabla_de_segmentos);

	tamanio += sizeof(segmento_pcb);

	int cantidad_tripulantes = list_get(paquete, 0);
	char* posiciones = list_get(paquete, 1);
	char **list_pos = string_split(posiciones, " ");
	printf("%s\n", list_pos[0]);
	char **posicion_del_tripulante;

	for(int i=0; i <= cantidad_tripulantes ; i++){
		//posicion_del_tripulante = string_split(posiciones[i], "|");
		//Segmento *segmento_tcb = crear_segmento_tcb((uint32_t*) i,(uint32_t*) posicion_del_tripulante[0],(uint32_t*) posicion_del_tripulante[1], segmento_pcb->base, tabla_de_segmentos);
		//tamanio += sizeof(segmento_tcb);
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
	int cliente = esperar_cliente(socket);
	printf("Cliente: %d\n", cliente);
	//logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 1, LOG_LEVEL_DEBUG);

	int idTripulante;

	while(1) {
		operacion = recibir_operacion(cliente);
		lista = NULL;

		printf("\nLA OPERACION ES: %d\n", operacion);

		switch(operacion) {
			case INICIO_PATOTA:
				lista = recibir_paquete(cliente);
				//Agregar mutex
				crear_proceso(lista);
				break;

			case ELIMINAR_TRIPULANTE:
				lista = recibir_paquete(cliente);
				uint32_t idTripulante = (uint32_t)((char *) list_get(lista,0));
				eliminarTripulante(idTripulante);
				printf("Tripulante eliminado de la nave %d\n", idTripulante);
				//liberar_cliente(cliente);
				break;

			case ACTUALIZAR_POSICION:
				lista = recibir_paquete(cliente);
				idTripulante = atoi((char *) list_get(lista,0));
				break;

			case NUEVO_TRIPULANTE:;
				t_tripulante_iniciado *nuevo_tripulante= recibir_tripulante_iniciado(cliente);

				printf("%s\n", nuevo_tripulante->status);
				break;

			case PEDIDO_TAREA:;
				char *ejemplo_tarea = "COMER;10;14;4"; //hardcodeo un string para probar desde discordiador

				//recibo datos del tripulante para buscarlo (ignoro datos q no me sirven)
				t_tripulante_iniciado *tripulante_tarea = recibir_tripulante_iniciado(cliente);

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
