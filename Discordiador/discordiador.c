#include "discordiador.h"

//************************************************ Manejo de Estados **********************************************
/* por ahora no lo uso
 *
bool tripulanteBloqueadoIO(void* elemento){
	Tripulante* tripulante = (Tripulante*)elemento;
	return tripulante->estado == BLOQUEADO_IO;
}
bool tripulanteBloqueadoEmergencia(void* elemento){
	Tripulante* tripulante = (Tripulante*)elemento;
	return tripulante->estado == BLOQUEADO_EMERGENCIA;
}
bool tripulanteFinalizado(void* elemento){
	Tripulante* tripulante = (Tripulante*)elemento;
	return tripulante->estado == FINALIZADO;
}*/

//************************************************ Inciar Discordiador **********************************************

void iniciarPlanificacion(){
	discordiador = (Discordiador*)malloc(sizeof(Discordiador));

	char *limiteTripulantesEjecutando = config_get_string_value(config, "GRADO_MULTITAREA");


}

//************************************************ PLANIFICADOR **********************************************
void planificar(){


	//TODO
}

/*
void fifo(){
	while(!teamTermino){
		sem_wait(&sem_proceso);
		//puts("sem_proceso");
		sem_wait(&sem_mx_proceso);
		//puts("sem_mx_proceso");
		//printf("-------cantidad de procesos: %d----\n",team->procesos->elements_count);
		if(team->procesos->elements_count > 0){

			irAAtrapar((Proceso*)team->procesos->head->data);
		}
		else
			return;
	}
}*/

//************************************************ CONSOLA **********************************************


void atender_comandos_consola(void) {

	while (1) {

		char **comando_separado;
		tipo_comando valor = -1;
		char *comando_ingresado;

		comando_ingresado = readline(">");

		comando_separado = string_split(comando_ingresado, " ");

		for (int j = 0; j < CANT_COMANDOS; j++)
			if (strcmp(comando_separado[0], comandos_validos[j]) == 0)
				valor = j;

		switch (valor) {
		case 0: //INICIAR_PATOTA
			;
			int cantidad_tripulantes = atoi(comando_separado[1]);
			char *lista_tareas = strdup(comando_separado[2]); //LIBERAR ESPACIO

			log_info(logs_discordiador, "Iniciando %d tripulantes de patota numero %d..\n",cantidad_tripulantes, g_numero_patota);

			//AVISAR A MI RAM QUE SE CREO LA PATOTA

			iniciar_patota(comando_separado);

			g_numero_patota += 1; //PARA LA PROX VEZ QUE SEA INICIALIZADO
			break;

		case 1: //LISTAR_TRIPULANTE
			printf("--------LISTANDO TRIPULANTES---------\n");
			listar_cola_planificacion(LLEGADA);
			listar_cola_planificacion(LISTO);
			listar_cola_planificacion(TRABAJANDO);
			listar_cola_planificacion(BLOQUEADO);
			listar_cola_planificacion(FINALIZADO);
			break;

		case 2: //EXPULSAR_TRIPULANTE
			printf("ES EXPULSAR\n");
			break;

		case 3: //INICIAR_PLANIFICACION
			puede_planificar = true;
			sem_post(&iniciar_planificacion);
			sem_post(&iniciar_planificacion);
			sem_post(&iniciar_planificacion);
			//mandaria un semaforo

			break;

		case 4: //PAUSAR_PLANIFICACION
			puede_planificar = false;
			break;

		case 5: //OBTENER_BITACORA
			break;

		case 6: //SALIR
			printf("Si realmente deseas salir apreta 'S'..\n");
			char c = getchar();
			if(c == 's' || c == 'S'){
				programa_activo = false;
				return;
			}
			printf("NO VOY S SALIRRR");
			break;
		default:
			printf("COMANDO INVALIDO\n");
			break;
		}
	}
}

//************************************************ OTROS **********************************************


void iniciar_patota(char **datos_tripulantes) {
	int cantidad_tripulantes = atoi(datos_tripulantes[1]);
	char *posiciones[cantidad_tripulantes];
	int retorno_thread;
	pthread_t tripulantes[cantidad_tripulantes - 1];
	char **posicion_del_tripulante;
	argumentos_creacion_tripulantes *args = (argumentos_creacion_tripulantes*)malloc(sizeof(argumentos_creacion_tripulantes));

	for (int i = 0, j = 3; i < cantidad_tripulantes; i++, j++) {
		if (datos_tripulantes[j] != NULL)
			posiciones[i] = datos_tripulantes[j];
		else {
			while (i < cantidad_tripulantes)
				posiciones[i++] = "0|0";
			posiciones[i] = NULL;
		}
	}

	for (int i = 0; i < cantidad_tripulantes; i++) {
		pthread_mutex_lock(&lockear_creacion_tripulante);
		posicion_del_tripulante = string_split(posiciones[i], "|");
		args -> numero_tripulante = i + 1;
		args -> posicionX = atoi(posicion_del_tripulante[0]);
		args -> posicionY = atoi(posicion_del_tripulante[1]);
		args -> patota_actual = g_numero_patota;
		pthread_create(&tripulantes[i], NULL, (void *)tripulante, (void *)args);
	}

}

void tripulante(void *argumentos){

	argumentos_creacion_tripulantes *args = argumentos;
	Tripulante *tripulante = (Tripulante*)malloc(sizeof(Tripulante));
	tripulante -> id = args->numero_tripulante;
	tripulante -> patota = args->patota_actual;
	tripulante -> posicionX = args->posicionX;
	tripulante -> posicionY = args->posicionY;
	tripulante -> estado = LLEGADA;

	queue_push(lista_llegada, tripulante);
	pthread_mutex_unlock(&lockear_creacion_tripulante);

	//TODO: ACA DEBERIA AVISAR A MI-RAM PARA TCB



	sem_wait(&iniciar_planificacion);
	pthread_mutex_lock(&lockear_cambio_new_rdy);
	queue_push(lista_listo, queue_pop(lista_llegada));
	pthread_mutex_unlock(&lockear_cambio_new_rdy);



	/*sem_post(&tripulante_listo); //avisa al planificador que quiere entrar

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante *un_tripulante = (Tripulante *) data;
		return tripulante->id == un_tripulante->id
				&& tripulante->patota == un_tripulante->patota;
	}*/


/*
	//sem_wait(&planificacion_activa); //me avisa el planificador que puedo meterme a ready
	while (programa_activo) {
		while (puede_planificar) {

			sem_wait(&metete_a_listo);
			pthread_mutex_lock(&lockear_cambio_new_rdy);
			list_add(lista_listo, tripulante);
			list_remove_by_condition(lista_llegada, soy_yo);
			pthread_mutex_unlock(&lockear_cambio_new_rdy);

			sem_wait(&metete_a_exec);
			pthread_mutex_lock(&lockear_cambio_rdy_exec);
			list_add(lista_trabajando, tripulante);
			list_remove_by_condition(lista_listo, soy_yo);
			pthread_mutex_unlock(&lockear_cambio_rdy_exec);
		}
	}*/
	//TODO: AVISAR A MI-RAM
	//TODO: AGREGR A COLA

}


void listar_cola_planificacion(Estado estado) {
	t_link_element *elementos;
	t_list *copia_lista = (t_list*) malloc(sizeof(t_list));
	char *nombre_estado;

	//LLEGADA, LISTO, TRABAJANDO, BLOQUEADO, FINALIZADO
	switch(estado){
	case LLEGADA:
		copia_lista->head = lista_llegada-> elements -> head;
		nombre_estado = "Llegada";
		break;
	case LISTO:
		copia_lista->head = lista_listo-> elements -> head;
		nombre_estado = "Listo";
		break;
	case TRABAJANDO:
		copia_lista->head = lista_trabajando-> elements -> head;
		nombre_estado = "Trabajando";
		break;
	case BLOQUEADO:
		copia_lista->head = lista_bloqueado-> elements -> head;
		nombre_estado = "Bloqueado";
		break;
	case FINALIZADO:
		copia_lista->head = lista_finalizado-> elements -> head;
		nombre_estado = "Finalizado";
		break;
	}
	if (copia_lista->head == NULL) {
		printf("No hay tripulantes en la cola de %s!\n",nombre_estado);
	} else {
		Tripulante *tripulante = (Tripulante *) malloc(sizeof(Tripulante));
		while (copia_lista->head != NULL) {
			elementos = copia_lista->head;
			tripulante = (Tripulante *) elementos->data;
			printf("Patota N°: %d\t", tripulante->patota);
			printf("Tripulante ID°: %d\t", tripulante->id);
			printf("PosX: %d, PosY: %d\t", tripulante->posicionX, tripulante->posicionY);
			printf("Estado: %s\n", nombre_estado);
			copia_lista->head = copia_lista->head->next;
		}
	}
	free(copia_lista);
}

void inicializar_recursos_necesarios(void){
	logs_discordiador = log_create("../logs_files/discordiador.log",
			"DISCORDIADOR", 1, LOG_LEVEL_INFO);
	log_info(logs_discordiador, "INICIANDO DISCORDIADOR..");

	log_info(logs_discordiador, "Generando configuraciones..");
	config = config_create(PATH_DISCORDIADOR_CONFIG);

	ip_mi_ram = config_get_string_value(config, "IP_MI_RAM_HQ");
	log_info(logs_discordiador, "IP RAM: %s", ip_mi_ram);

	puerto_mi_ram = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
	log_info(logs_discordiador, "PUERTO RAM: %s", puerto_mi_ram);

	ip_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	log_info(logs_discordiador, "IP STORE: %s", ip_mongo_store);

	puerto_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	log_info(logs_discordiador, "PUERTO STORE: %s", puerto_mongo_store);

	algoritmo_planificacion = config_get_string_value(config, "ALGORITMO");
	log_info(logs_discordiador, "ALGORITMO DE PLANIFICACION: %s", algoritmo_planificacion);

	if (strcmp(algoritmo_planificacion, "RR") == 0) {
			char *q = config_get_string_value(config, "QUANTUM");
			log_info(logs_discordiador, "QUANTUM UTILIZADO: %s", q);
			quantum = atoi(q);
		}

	char *grado_mt = config_get_string_value(config, "GRADO_MULTITAREA");
	log_info(logs_discordiador, "GRADO MULTITAREA PERMITIDO: %s", grado_mt);
	grado_multitarea = atoi(grado_mt);

	char *duracion = config_get_string_value(config, "DURACION_SABOTAJE");
	log_info(logs_discordiador, "DURACION DE LOS SABOTAJES: %s", duracion);
	duracion_sabotaje = atoi(duracion);

	char *retardo_cpu = config_get_string_value(config, "RETARDO_CICLO_CPU");
	log_info(logs_discordiador, "RETARDO DEL CICLO DE CPU: %s", retardo_cpu);
	retardo_ciclo_cpu = atoi(retardo_cpu);

	//falta avisar?
	lista_llegada    = queue_create();
	lista_listo      = queue_create();
	lista_trabajando = queue_create();
	lista_bloqueado  = queue_create();
	lista_finalizado = queue_create();
	log_info(logs_discordiador, " COLAS DE PLANIFICACION INICIALIZADAS..");


	//inicios semaforos

	sem_init(&metete_a_exec, 0, 0);
	sem_init(&iniciar_planificacion, 0, 0);
	sem_init(&tripulante_listo, 0, 0);
	sem_init(&metete_a_listo, 0, 0);


	log_info(logs_discordiador, "---DATOS INICIALIZADO---\n");
}

void liberar_memoria_discordiador(void){
	//TODO
}

int main(void){

	inicializar_recursos_necesarios();

	pthread_t hilo_consola;
	pthread_t hilo_planificador;
	pthread_t hilo_para_sabotaje; //falta

	pthread_create(&hilo_planificador, NULL, (void *)planificar, NULL);
	pthread_detach(hilo_planificador);


	pthread_create(&hilo_consola, NULL, (void *)atender_comandos_consola, NULL);
	pthread_join(hilo_consola, NULL);

	printf("\n-------TERMINAR-------\n");

	liberar_memoria_discordiador();

	return EXIT_SUCCESS;
}


