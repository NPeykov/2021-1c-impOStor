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



//************************************************ Tareas **********************************************

char *tareas[] = { "REGAR;3;2;7", "PLANTAR;0;1;3", "GENERAR_OXIGENO 3;1;2;1", NULL }; //ejemplo

char *dar_proxima_tarea(){
	static int i=0;
	return tareas[i] == NULL ? NULL : tareas[i++];
}

Tarea *proxima_tarea(){
	char *tarea_string = dar_proxima_tarea();
	char **tarea_dividida;
	char **tarea_IO_dividida;

	Tarea *nueva_tarea = (Tarea*) malloc(sizeof(Tarea));

	if(tarea_string == NULL)
		return NULL;

	tarea_dividida = string_split(tarea_string, ";");
	tarea_IO_dividida = string_split(tarea_dividida[0], " "); //por si es tarea I/O

	nueva_tarea -> nombre 	 = tarea_IO_dividida[0];
	if(tarea_IO_dividida[1] == NULL) {
		nueva_tarea -> parametro = -1;
		nueva_tarea -> tipo = TAREA_COMUN;
	}
	else {
		nueva_tarea -> parametro = atoi(tarea_IO_dividida[1]);
		nueva_tarea -> tipo = TAREA_IO;
	}
	nueva_tarea -> posX 	 = atoi(tarea_dividida[1]);
	nueva_tarea -> posY      = atoi(tarea_dividida[2]);
	nueva_tarea -> duracion  = atoi(tarea_dividida[3]);

	return nueva_tarea;
}

bool estoy_en_mismo_punto(int sourceX, int sourceY, int targetX, int targetY){
	return sourceX == targetX && sourceY == targetY;
}

bool completo_tarea(Tripulante_Planificando *tripulante_trabajando) {
	Tripulante *tripulante = tripulante_trabajando->tripulante;
	Tarea *tarea = tripulante_trabajando->tarea;
	int sourceX = tripulante->posicionX;
	int targetX = tarea->posX;
	int sourceY = tripulante->posicionY;
	int targetY = tarea->posY;

	switch (tarea->tipo) {
	case TAREA_COMUN:
		return estoy_en_mismo_punto(sourceX, sourceY, targetX, targetY)
				&& tarea->duracion == 0;
		break;
	case TAREA_IO:
		return estoy_en_mismo_punto(sourceX, sourceY, targetX, targetY);
		break;
	default:
		log_info(logs_discordiador,
				"ERROR en funcion 'completo_tarea', no coincide tipos..\n");
		return false;
		break;

	}
}

void moverse_una_unidad(Tripulante_Planificando *tripulante_trabajando) {
	static bool last_move_x = false;
	int targetX, targetY, sourceX, sourceY;
	targetX = tripulante_trabajando->tarea->posX;
	targetY = tripulante_trabajando->tarea->posY;
	sourceX = tripulante_trabajando->tripulante->posicionX;
	sourceY = tripulante_trabajando->tripulante->posicionY;

	if (sourceX == targetX && last_move_x == false)
		last_move_x = true;

	if (sourceY == targetY && last_move_x == true)
		last_move_x = false;

	if (sourceX < targetX && last_move_x == false) {
		tripulante_trabajando->tripulante->posicionX += 1;
		last_move_x = true;
		return;
	}

	if (sourceX > targetX && last_move_x == false) {
		tripulante_trabajando->tripulante->posicionX -= 1;
		last_move_x = true;
		return;
	}

	if (sourceY < targetY && last_move_x == true) {
		tripulante_trabajando->tripulante->posicionY += 1;
		last_move_x = false;
		return;
	}

	if (sourceY > targetY && last_move_x == true) {
		tripulante_trabajando->tripulante->posicionY -= 1;
		last_move_x = false;
		return;
	}
}

void realizar_tarea_IO(Tripulante_Planificando *tripulante_trabajando) {
	//TODO: avisar a mi-ram??
	//TODO: avisarle a mongo-store para que modifique su archivo con letra de llenado
	tripulante_trabajando->tarea->duracion -= 1;
}

void realizar_tarea_comun(Tripulante_Planificando *tripulante_trabajando){
	tripulante_trabajando -> tarea -> duracion -= 1;
}

void hacer_una_unidad_de_tarea(Tripulante_Planificando *tripulante_trabajando) {
	Tipo_Tarea tipo_tarea = tripulante_trabajando->tarea->tipo;
	int targetX, targetY, sourceX, sourceY;
	targetX = tripulante_trabajando->tarea->posX;
	targetY = tripulante_trabajando->tarea->posY;
	sourceX = tripulante_trabajando->tripulante->posicionX;
	sourceY = tripulante_trabajando->tripulante->posicionY;

	switch (tipo_tarea) {

	case TAREA_COMUN:
		if (!estoy_en_mismo_punto(sourceX, sourceY, targetX, targetY))
			moverse_una_unidad(tripulante_trabajando);
		else
			realizar_tarea_comun(tripulante_trabajando);

		break;
	case TAREA_IO:
		moverse_una_unidad(tripulante_trabajando);
		break;
	}

	log_info(logs_discordiador, "Tripulante N:%d de Patota:%d se movio a X:%d, Y:%d",
			tripulante_trabajando -> tripulante -> id, tripulante_trabajando -> tripulante -> patota,
			tripulante_trabajando -> tripulante -> posicionX, tripulante_trabajando -> tripulante -> posicionY);

}

//************************************************ PLANIFICADOR **********************************************

void planificar() {


	sem_wait(&activo_planificador);

	sem_wait(&proceso_nuevo);

	while (1) {

		sem_post(&anda_rdy);
		sem_post(&anda_exec);
		sem_post(&anda_bloq);

	}
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
			sem_post(&activo_planificador);
			//mandaria un semaforo

			break;

		case 4: //PAUSAR_PLANIFICACION

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


	//---------------------------
	//creo un tripulante con tarea y quantum

	Tripulante_Planificando *tripulante_trabajando = (Tripulante_Planificando*) malloc(sizeof(Tripulante_Planificando));
	tripulante_trabajando -> tripulante = tripulante;
	tripulante_trabajando -> quantum_disponible = quantum;
	tripulante_trabajando -> tarea = proxima_tarea();
	//---------------------------

	sem_post(&proceso_nuevo);

	/*Falta resolver lo de abjao para que sea consistente
	 * con el while(1)*/
	sem_wait(&anda_rdy);
	pthread_mutex_lock(&lockear_cambio_new_rdy);
	queue_push(lista_listo, queue_pop(lista_llegada));
	pthread_mutex_unlock(&lockear_cambio_new_rdy);

	while(1){

		sem_wait(&anda_exec);
		pthread_mutex_lock(&lockear_cambio_rdy_exec);
		queue_push(lista_trabajando, queue_pop(lista_listo));
		pthread_mutex_unlock(&lockear_cambio_rdy_exec);

		while(tripulante_trabajando->quantum_disponible >= 0 && !completo_tarea(tripulante_trabajando) && !g_pausa){
			hacer_una_unidad_de_tarea(tripulante_trabajando);
			tripulante_trabajando -> quantum_disponible -= 1;
			sleep(retardo_ciclo_cpu);
		}

		if (tripulante_trabajando->tarea->tipo == TAREA_IO && completo_tarea(tripulante_trabajando)) {
			sem_wait(&bloq_disponible);
			pthread_mutex_lock(&lockear_cambio_exec_bloq);
			queue_push(lista_bloqueado, queue_pop(lista_trabajando));
			pthread_mutex_unlock(&lockear_cambio_exec_bloq);
			sleep(retardo_ciclo_cpu);
			while(tripulante_trabajando->tarea->duracion > 0){
				hacer_una_unidad_de_tarea(tripulante_trabajando);
			}
			sem_post(&libere_bloq);
		}

		if(completo_tarea(tripulante_trabajando) /*&& no_hay_mas_tareas()*/){
			pthread_mutex_lock(&lockear_exit);
			//TODO: Falta sacar el tripulante de la cola, independientemente si esta en i/o o exec
			pthread_mutex_unlock(&lockear_exit);
		}

		if(completo_tarea(tripulante_trabajando) /*&& !no_hay_mas_tareas()*/){
			tripulante_trabajando -> tarea = proxima_tarea();
		}

	}
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
	sem_init(&activo_planificador , 0, 0);
	sem_init(&cambio_new_rdy , 0, 0);
	sem_init(&proceso_nuevo , 0, 0);
	sem_init(&quiero_rdy , 0, 0);
	sem_init(&quiero_exec , 0, 0);
	sem_init(&quiero_bloq , 0, 0);
	sem_init(&bloq_disponible , 0, 3);
	sem_init(&libere_bloq , 0, 0);


	log_info(logs_discordiador, "---DATOS INICIALIZADO---\n");
}

void liberar_memoria_discordiador(void) {
	//LISTAS
	queue_clean(lista_llegada);
	queue_destroy(lista_llegada);
	queue_clean(lista_listo);
	queue_destroy(lista_listo);
	queue_clean(lista_trabajando);
	queue_destroy(lista_trabajando);
	queue_clean(lista_bloqueado);
	queue_destroy(lista_bloqueado);
	queue_clean(lista_finalizado);
	queue_destroy(lista_finalizado);

	//LOGS
	log_destroy(logs_discordiador);

	//TODO: SEMAFOROS

	//TODO: CONEXIONES

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

	log_info(logs_discordiador, "TERMINANDO PROGRAMA - LIBERANDO ESPACIO");
	liberar_memoria_discordiador();
	printf("\n-------TERMINO-------\n");

	return EXIT_SUCCESS;
}


