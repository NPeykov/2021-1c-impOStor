#include "discordiador.h"

///************************************************ Tareas **********************************************

char *tareas[] = { "REGAR 4;3;2;7", "PLANTAR;4;5;3", "GENERAR_OXIGENO 3;1;2;1", NULL }; //ejemplo

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

	log_info(logs_discordiador, "ME ASIGNARON UNA TAREA %s CON DURACION %d",
					nueva_tarea->tipo == TAREA_IO? "IO" : "COMUN", nueva_tarea->duracion);

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
	sleep(retardo_ciclo_cpu);
	tripulante_trabajando->tarea->duracion -= 1;
	log_info(logs_discordiador, "Tripulante N:%d - REALIZO una unidad de tarea IO, me quedan %d.",
				tripulante_trabajando->tripulante->id, tripulante_trabajando->tarea->duracion);
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

	sleep(retardo_ciclo_cpu);
	log_info(logs_discordiador, "Tripulante N:%d de Patota:%d se movio a X:%d, Y:%d",
			tripulante_trabajando -> tripulante -> id, tripulante_trabajando -> tripulante -> patota,
			tripulante_trabajando -> tripulante -> posicionX, tripulante_trabajando -> tripulante -> posicionY);

}


//************************************************ PAUSA/SABOTAJE **********************************************

void fijarse_si_hay_sabotaje(){
	if(g_hay_sabotaje){
		pthread_mutex_lock(&sabotaje_lock);
		while(g_hay_sabotaje) {
			pthread_cond_wait(&sabotaje_resuelto, &sabotaje_lock);
		}
		pthread_mutex_unlock(&sabotaje_lock);

		//resetear_quantum();
		//goto arrancar_de_nuevo; //antes del wait(trabajar)
	}
}

void fijarse_si_hay_pausa(){
	pthread_mutex_lock(&pausa_lock);
	while(g_hay_pausa) {
		pthread_cond_wait(&sacar_pausa, &pausa_lock);
	}
	pthread_mutex_unlock(&pausa_lock);
}

void fijarse_si_hay_pausa_plani(){
	pthread_mutex_lock(&pausa_lock_plani);
	while(g_hay_pausa) {
		pthread_cond_wait(&sacar_pausa, &pausa_lock_plani);
	}
	pthread_mutex_unlock(&pausa_lock_plani);
}
//************************************************ CAMBIOS DE COLA **********************************************


void moverse_a_ready(Tripulante_Planificando *tripulante_trabajando){
	Estado estado_actual = tripulante_trabajando->tripulante->estado;

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante = (Tripulante_Planificando *) data;
			return tripulante_trabajando -> tripulante ->id == un_tripulante->tripulante->id
					&& tripulante_trabajando->tripulante->patota == un_tripulante->tripulante->patota;
	}

	log_info(logs_discordiador,
			"Entro a moverme a READY con tripulante N: %d\n", tripulante_trabajando->tripulante->id);

	if(estado_actual == TRABAJANDO){
		log_info(logs_discordiador, "El tripulante estaba TRABAJANDO");
	}

	else if(estado_actual == BLOQUEADO_IO)
		log_info(logs_discordiador, "El tripulante estaba EN IO");

	else
		log_info(logs_discordiador, "El tripulante NO ESTABA HACIENDO NINGUNA");

	switch (estado_actual) {
	case TRABAJANDO:
		queue_push(lista_listo, tripulante_trabajando);
		tripulante_trabajando->tripulante->estado = LISTO;
		list_remove_by_condition(lista_trabajando, soy_yo);
		log_info(logs_discordiador, "En cola TRABAJANDO hay %d tripulantes", list_size(lista_trabajando));
		break;

	case BLOQUEADO_IO:
		queue_push(lista_listo, tripulante_trabajando);
		tripulante_trabajando->tripulante->estado = LISTO;
		list_remove_by_condition(lista_bloqueado_IO, soy_yo);

		break;
	}

	log_info(logs_discordiador, "En lista de listo hay %d", queue_size(lista_listo));

	log_info(logs_discordiador, "El tripulante SE FUE EN ESTADO %d", tripulante_trabajando -> tripulante->estado);

}

void moverse_a_bloq(Tripulante_Planificando *tripulante_trabajando) {
	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante = (Tripulante_Planificando *) data;
		return tripulante_trabajando->tripulante->id == un_tripulante->tripulante->id
				&& tripulante_trabajando->tripulante->patota == un_tripulante->tripulante->patota;
	}

	pthread_mutex_lock(&mutex_exec_a_bloq);
	list_add(lista_bloqueado_IO, tripulante_trabajando);
	tripulante_trabajando -> tripulante -> estado = BLOQUEADO_IO;
	list_remove_by_condition(lista_trabajando, soy_yo);
	pthread_mutex_unlock(&mutex_exec_a_bloq);
}
//************************************************ ALGORITMO PLANIFICACION **********************************************

void realizar_trabajo(Tripulante_Planificando *tripulante){
	Algoritmo algoritmo;

	if(strcmp("RR", algoritmo_planificacion) == 0)
		algoritmo = RR;
	else
		algoritmo = FIFO;

	switch (algoritmo) {
	case FIFO:
		while (!completo_tarea(tripulante)) {
			fijarse_si_hay_pausa(); //TODO
			fijarse_si_hay_sabotaje(); //TODO
			hacer_una_unidad_de_tarea(tripulante);
		}

		if (completo_tarea && tripulante->tarea->tipo == TAREA_IO) {
			break;
		}

		else { //tengo que pedir la proxima tarea
			pthread_mutex_lock(&mutex_pedido_tarea);
			tripulante->tarea = proxima_tarea();
			pthread_mutex_unlock(&mutex_pedido_tarea);
			if (tripulante->tarea == NULL) {
				log_info(logs_discordiador, "EL TRIPULANTE N %d FINALIZO", tripulante->tripulante->id);
				pthread_exit(NULL);
				//EXIT
			}
		}
		break;
	case RR:
		while (!completo_tarea(tripulante) && tripulante->quantum_disponible > 0) {
			fijarse_si_hay_pausa();
			fijarse_si_hay_sabotaje();

			hacer_una_unidad_de_tarea(tripulante);
			tripulante->quantum_disponible -= 1;
			log_info(logs_discordiador,
					"Tripulante N:%d de patota N:%d le quedan %d quantums en tarea %s",
					tripulante->tripulante->id, tripulante->tripulante->patota,
					tripulante->quantum_disponible, tripulante->tarea->nombre);
		}

		if (completo_tarea(tripulante) && tripulante->tarea->tipo == TAREA_IO)
			return;
			//NOTA: NO CREO QUE SEA NECESARIO, NO ENTRA A NINGUN IF Y LO ESPERA UN IF QUE COINCIDE


		if (completo_tarea(tripulante)) {
			pthread_mutex_lock(&mutex_pedido_tarea);
			tripulante->tarea = proxima_tarea();
			pthread_mutex_unlock(&mutex_pedido_tarea);
			if (tripulante->tarea == NULL) {
				log_info(logs_discordiador, "EL TRIPULANTE N %d FINALIZO", tripulante->tripulante->id);
				pthread_exit(NULL);
				//EXIT
			}
		}

		tripulante->quantum_disponible = quantum; //se resetea el quantum
		log_info(logs_discordiador,
							"Se reseteo el quantum del tripulante N:%d Patota N: %d, a %d",
							tripulante->tripulante->id, tripulante->tripulante->patota,
							tripulante->quantum_disponible);
		break;
	}
}


//************************************************ PLANIFICADOR **********************************************

void planificar() {
	Tripulante_Planificando *tripulante;

	fijarse_si_hay_pausa_plani();

	while (1) {
		if (g_hay_sabotaje) {
			//TODO
		}

		fijarse_si_hay_pausa_plani();

		if (queue_size(lista_llegada) > 0) {
			pthread_mutex_lock(&mutex_cambio_a_ready);
			tripulante = queue_pop(lista_llegada);
			queue_push(lista_listo, tripulante);
			tripulante->tripulante->estado = LISTO;
			pthread_mutex_unlock(&mutex_cambio_a_ready);
		}

		if (queue_size(lista_listo) > 0 && lugares_en_exec > 0) {
			pthread_mutex_lock(&mutex_rdy_exec);
			tripulante = queue_pop(lista_listo);
			tripulante->tripulante->estado = TRABAJANDO;
			list_add(lista_trabajando, tripulante);
			lugares_en_exec--;
			sem_post(&tripulante->ir_exec); //3
			pthread_mutex_unlock(&mutex_rdy_exec);
		}

	}

}



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
			listar_cola_planificacion(BLOQUEADO_IO);
			listar_cola_planificacion(BLOQUEADO_EMERGENCIA);
			listar_cola_planificacion(FINALIZADO);
			break;

		case 2: //EXPULSAR_TRIPULANTE
			printf("ES EXPULSAR\n");
			break;
		case 3: //INICIAR_PLANIFICACION
			pthread_mutex_lock(&pausa_lock);
			g_hay_pausa = false;
			pthread_cond_broadcast(&sacar_pausa); //para que los thds se desbloqueen
			pthread_mutex_unlock(&pausa_lock);

			break;
		case 4: //PAUSAR_PLANIFICACION
			pthread_mutex_lock(&pausa_lock);
			g_hay_pausa = true;
			pthread_mutex_unlock(&pausa_lock);
			break;

		case 5: //OBTENER_BITACORA

			break;

		case 6: //SALIR
			printf("Si realmente deseas salir apreta 'S'..\n");
			char c = getchar();
			if(c == 's' || c == 'S'){
				//programa_activo = false;
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

	//TODO: ACA DEBERIA AVISAR A MI-RAM PARA TCB

	//---------------------------
	Tripulante_Planificando *tripulante_trabajando =
			(Tripulante_Planificando*) malloc(sizeof(Tripulante_Planificando));
	tripulante_trabajando->tripulante = tripulante;
	tripulante_trabajando->quantum_disponible = quantum;
	pthread_mutex_lock(&mutex_tarea);
	tripulante_trabajando->tarea = proxima_tarea();
	pthread_mutex_unlock(&mutex_tarea);
	sem_init(&tripulante_trabajando->ir_exec, 0, 0);

	queue_push(lista_llegada, tripulante_trabajando);
	pthread_mutex_unlock(&lockear_creacion_tripulante);


	while(1){
		//arrancar_de_nuevo:

		sem_post(&proceso_rdy);
		sem_wait(&tripulante_trabajando -> ir_exec);
		realizar_trabajo(tripulante_trabajando);
		pthread_mutex_unlock(&mutex_dejar_exec);
		lugares_en_exec++;
		pthread_mutex_unlock(&mutex_dejar_exec);

		if(completo_tarea(tripulante_trabajando) && tripulante_trabajando -> tarea -> tipo == TAREA_IO){
			log_info(logs_discordiador,
							"REALIZANDO TAREA IO");
			moverse_a_bloq(tripulante_trabajando); //TODO
			sem_wait(&bloq_disponible);
			while(tripulante_trabajando->tarea->duracion > 0){
				realizar_tarea_IO(tripulante_trabajando);
			}
			sem_post(&bloq_disponible);
			pthread_mutex_lock(&mutex_pedido_tarea);
			tripulante_trabajando -> tarea = proxima_tarea();
			pthread_mutex_unlock(&mutex_pedido_tarea);
			if(tripulante_trabajando->tarea == NULL){
				log_info(logs_discordiador, "TERMINO IO Tripulante: %d ", tripulante_trabajando ->tripulante->id);
				pthread_exit(NULL);
				//EXIT
			}
		}
		log_info(logs_discordiador, "MANDO AL TRIPULANTE:%d A READY OTRA VEZ", tripulante_trabajando ->tripulante->id);
		pthread_mutex_lock(&mutex_cambio_a_ready);
		moverse_a_ready(tripulante_trabajando); //TODO
		pthread_mutex_unlock(&mutex_cambio_a_ready);
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
		copia_lista->head = lista_trabajando -> head;
		nombre_estado = "Trabajando";
		break;
	case BLOQUEADO_IO:
		copia_lista->head = lista_bloqueado_IO -> head;
		nombre_estado = "Bloqueados IO";
		break;
	case BLOQUEADO_EMERGENCIA:
		copia_lista->head = lista_bloqueado_EM -> head;
		nombre_estado = "Bloqueados Emergencia";
		break;
	case FINALIZADO:
		copia_lista->head = lista_finalizado -> head;
		nombre_estado = "Finalizado";
		break;
	}
	if (copia_lista->head == NULL) {
		printf("No hay tripulantes en la cola de %s!\n",nombre_estado);
	} else {
		Tripulante_Planificando *tripulante_planificando = (Tripulante_Planificando *) malloc(sizeof(Tripulante_Planificando));
		while (copia_lista->head != NULL) {
			elementos = copia_lista->head;
			tripulante_planificando = (Tripulante_Planificando *) elementos->data;
			printf("Patota N°: %d\t", tripulante_planificando->tripulante->patota);
			printf("Tripulante ID°: %d\t", tripulante_planificando->tripulante->id);
			printf("PosX: %d, PosY: %d\t", tripulante_planificando->tripulante->posicionX, tripulante_planificando->tripulante->posicionY);
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
	lugares_en_exec = grado_multitarea;


	char *duracion = config_get_string_value(config, "DURACION_SABOTAJE");
	log_info(logs_discordiador, "DURACION DE LOS SABOTAJES: %s", duracion);
	duracion_sabotaje = atoi(duracion);

	char *retardo_cpu = config_get_string_value(config, "RETARDO_CICLO_CPU");
	log_info(logs_discordiador, "RETARDO DEL CICLO DE CPU: %s", retardo_cpu);
	retardo_ciclo_cpu = atoi(retardo_cpu);

	//falta avisar?
	lista_llegada       = queue_create();
	lista_listo         = queue_create();
	lista_trabajando    = list_create();
	lista_bloqueado_IO  = list_create();
	lista_bloqueado_EM  = list_create();
	lista_finalizado    = list_create();
	log_info(logs_discordiador, " COLAS DE PLANIFICACION INICIALIZADAS..");


	//inicios semaforos

	sem_init(&proceso_rdy, 0, 0);
	sem_init(&trabajar, 0, 0);
	sem_init(&bloq_disponible, 0, 1);

	log_info(logs_discordiador, "---DATOS INICIALIZADO---\n");
}

void liberar_memoria_discordiador(void) {
	//LISTAS
	queue_clean(lista_llegada);
	queue_destroy(lista_llegada);
	queue_clean(lista_listo);
	queue_destroy(lista_listo);
	list_clean(lista_trabajando);
	list_destroy(lista_trabajando);
	list_clean(lista_bloqueado_IO);
	list_destroy(lista_bloqueado_IO);
	list_clean(lista_bloqueado_EM);
	list_destroy(lista_bloqueado_EM);
	list_clean(lista_finalizado);
	list_destroy(lista_finalizado);

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


