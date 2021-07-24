#include "segmentacion.c"
#include "mapa.c"
#include "paginacion.c"

void inicializar_ram(){
	logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 0, LOG_LEVEL_INFO);

	log_info(logs_ram,"################# Modulo Mi-RAM #################\n");

	socket_principal_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");

	tamaniomemoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	memoria = malloc(tamaniomemoria);
	memoriaPrincipal = list_create();
	patotas = list_create();

	//crear_nivel();
}

void *gestionarCliente(int socket) {

	int operacion;
	t_list *lista;

	bool esSegmentacion = strcmp(tipoMemoria, "SEGMENTACION") == 0;

	while(1) {
		int cliente = esperar_cliente(socket, logs_ram);

		operacion = recibir_operacion(cliente);
		lista = NULL;

		log_info(logs_ram,"\nSe recibio una operacion: %d\n", operacion);

		switch(operacion) {
			case INICIO_PATOTA:
				if(esSegmentacion){
					iniciarPatotaSeg(lista, cliente);
				}else{
					iniciarPatotaPag(lista, cliente);
				}
				break;
			case ELIMINAR_TRIPULANTE:
				if(esSegmentacion){
					eliminarTripulanteSeg(lista, cliente);
				}else{
					eliminarTripulantePag(lista, cliente);
				}
				break;
			case ACTUALIZAR_POSICION:;
				if(esSegmentacion){
					actualizarPosicionSeg(lista, cliente);
				}else{
					actualizarPosicionPag(lista, cliente);
				}
				break;
			case NUEVO_TRIPULANTE:;
				if(esSegmentacion){
					crearTripulanteSeg(lista, cliente);
				}else{
					crearTripulantePag(lista, cliente);
				}
				break;

			case PEDIDO_TAREA:;
				if(esSegmentacion){
					obtenerSgteTareaSeg(lista, cliente);
				}else{
					obtenerSgteTareaPag(lista, cliente);
				}
				break;

			case -1:
				log_info(logs_ram,"El cliente %d se desconecto.\n", cliente);
				liberar_cliente(cliente);
				break;

			default:
				log_info(logs_ram,"Operacion desconocida.\n");
				liberar_cliente(cliente);
				break;
		}
	}
}

void atenderSegunEsquema(){
	if(strcmp(tipoMemoria, "SEGMENTACION") == 0){
		inicializarSegmentacion(); //Inicializacion de semaforos y variables del esquema
	}else{
		inicializarPaginacion();
	}
	pthread_t hilo_cliente;

	pthread_create(&hilo_cliente, NULL, (void *)gestionarCliente(socket_principal_ram), NULL);
	pthread_join(hilo_cliente, NULL);
}

void dumpMemoria(){
	if(strcmp(tipoMemoria, "SEGMENTACION") == 0){
		dumpMemoriaSeg();
	}else{
		//dumpMemoriaPag(); TODO:Hacer dump Paginacion
	}
}



int main(){
	inicializar_ram();

	signal(SIGTSTP, dumpMemoriaSeg);

	atenderSegunEsquema();

	return EXIT_SUCCESS;
}
