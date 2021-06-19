#include "mi-ram.h"


void crearProceso(t_list *paquete){
	t_proceso *proceso = (t_proceso*) malloc(sizeof(proceso));
	//Falta semaforo
	proceso->id = numero_patota;

	proceso->tabla_de_segmentos = list_create(); //Tabla de Patota

	Segmento *segmento_tareas=crear_segmento_tareas(list_get(paquete, 2),tamanio, proceso->tabla_de_segmentos);

	Segmento *segmento_pcb=crear_segmento_pcb(tamanio, proceso->tabla_de_segmentos);
	numero_patota += 1;

	int cantidad_tripulantes = list_get(paquete, 0);
	t_list* posiciones = list_get(paquete, 1);
	char **posicion_del_tripulante;

	for(int i=0; i <= cantidad_tripulantes ; i++){
		posicion_del_tripulante = string_split(posiciones[i], "|");
		Segmento *segmento_tcb=crear_segmento_tcb(i, posicion_del_tripulante[0], posicion_del_tripulante[1], tamanio, tabla_segmentos);
	}
}

uint32_t calcular_base_logica(Segmento *segmento, t_list* tabla_segmentos){
	int pos_seg = segmento->idSegmento;
	Segmento* segmento_anterior = (Segmento*) list_get(tabla_segmentos, pos_seg - 1);
	if(!segmento_anterior) {
		return 0;
	}

	return (*segmento_anterior)->base + (*segmento_anterior)->tamanio;
}


void crear_segmentos(t_list* paquete, int tamanio, t_list* tabla_segmentos){
	Segmento *segmento_tareas=crear_segmento_tareas(list_get(paquete, 2),tamanio, tabla_segmentos);

	Segmento *segmento_pcb=crear_segmento_pcb(tamanio, tabla_segmentos);
	numero_patota += 1;

	int cantidad_tripulantes = list_get(paquete, 0);
	t_list* posiciones = list_get(paquete, 1);
	char **posicion_del_tripulante;

	for(int i=0; i <= cantidad_tripulantes ; i++){
		posicion_del_tripulante = string_split(posiciones[i], "|");
		Segmento *segmento_tcb=crear_segmento_tcb(i, posicion_del_tripulante[0], posicion_del_tripulante[1], tamanio, tabla_segmentos);
	}
}



Segmento* crear_segmento_pcb(int tamanio, t_list* tabla_segmentos){
	Segmento* segmento = (Segmento*) malloc(sizeof(segmento));

	PatotaCB pcb = (PatotaCB*) malloc(sizeof(pcb));
	pcb->pid = numero_patota;
	pcb->tareas = get_segmento_patota(TAREAS, numero_patota);

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);

	segmento->tamanio = tamanio;
	segmento->tipo = PCB;
	segmento->dato = pcb;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);

	return segmento;
}

Segmento* crear_segmento_tcb(int numero_tripulante, uint32_t posX, uint32_t posY ,int tamanio, t_list* tabla_segmentos) {
	Segmento* segmento = (Segmento*) malloc(sizeof(segmento));

	TripuCB tcb = (TripuCB*) malloc(sizeof(tcb));
	tcb->pcb = get_segmento_patota(PCB, numero_patota);

	tcb->tid = numero_tripulante;
	tcb->posX = posX;
	tcb->posY = posY;

	segmento->idSegmento = tabla_segmentos->elements_count;
	list_add(tabla_segmentos, segmento);

	segmento->tamanio = tamanio;
	segmento->tipo = TCB;
	segmento->dato = tcb;
	segmento->base = calcular_base_logica(segmento, tabla_segmentos);

	return segmento;
}

void eliminarTripulante(int idTripulante){
	t_list *tabla_segmentos = proceso->tabla_de_segmentos;
	list_remove_by_condition(tabla_segmentos, (tipo_segmento == TCB && dato->id == idTripulante));
}

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
					crear_proceso(lista);
					break;
				case EXPULSAR_TRIPULANTE:
					lista = recibir_paquete(cliente);
					int idTripulante = atoi((char *) list_get(lista,0));
					eliminarTripulante(idTripulante);
					printf("Tripulante eliminado de la nave %d\n", idTripulante);
					//liberar_cliente(cliente);
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
	if(tipoMemoria == "SEGMENTACION"){
		//Agregar Hilos
		gestionarClienteSeg(socket_mi_ram);
	}else{
		//Agregar Hilos
		gestionarClientePag(socket_mi_ram);
	}
}



int main(){
  inicializar_ram();

  return EXIT_SUCCESS;
}
