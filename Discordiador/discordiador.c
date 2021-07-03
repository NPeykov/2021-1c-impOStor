#include "discordiador.h"

///************************************************ Sabotajes **********************************************
///TODO: ver como se recibe la señal de mongo
void esperar_sabotaje(void){ //este es un hilo
	int _socket_mongo;
	int codigo_recibido;
	t_list *lista;
	int sabotaje_posX;
	int sabotaje_posY;
	char *mensaje;

	_socket_mongo = iniciar_conexion(I_MONGO_STORE, config);

	while(1){
		codigo_recibido = recibir_operacion(_socket_mongo);
		mensaje = recibir_mensaje(_socket_mongo);

		printf("SABOTAJEEEE\n");

		if(codigo_recibido == INICIO_SABOTAJE){

			pthread_mutex_lock(&sabotaje_lock);
			g_hay_sabotaje = true; //activo para q todos detengan su ejecucion
			pthread_mutex_unlock(&sabotaje_lock);

			lista = recibir_paquete(_socket_mongo);
			sabotaje_posX = atoi(list_get(lista, 0));
			sabotaje_posY = atoi(list_get(lista, 1));
			log_info(logs_discordiador, "SABOTAJE EN (%d, %d)!", sabotaje_posX, sabotaje_posY);
			atender_sabotaje(sabotaje_posX, sabotaje_posY);
		}

		else {
			log_info(logs_discordiador, "RECIBI UNA OPERACION DIFERENTE DE SABOTAJE!");
			lista = recibir_paquete(_socket_mongo); //para vaciar el buffer
		}
	}

}

//TODO: rehacer este metodo con un atender_sabotaje(x, y)
void atender_sabotaje(int x, int y){
	t_list *lista_mergeada = (t_list *)malloc(sizeof(t_list));
    Tripulante_Planificando *tripulante_cercano = (Tripulante_Planificando*)malloc(sizeof(Tripulante_Planificando));

    pthread_mutex_lock(&sabotaje_lock);
	g_hay_sabotaje = true; //activo para q todos detengan su ejecucion
	pthread_mutex_unlock(&sabotaje_lock);

    bool sigue_activo(void *data){
    	Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;

    	if(tripulante == NULL)
    		return false;

    	return tripulante->sigo_planificando;
    }

    //si no hay tripulantes en ninguna de las listas
	if (list_size(lista_trabajando) == 0 && list_size(lista_listo->elements) == 0) {
		log_info(logs_discordiador, "No hay tripulantes disponibles para resolver el sabotaje");

		pthread_mutex_lock(&sabotaje_lock);
		g_hay_sabotaje = false;
		pthread_mutex_unlock(&sabotaje_lock);

		sem_post(&termino_sabotaje_planificador);
	}

    while(list_any_satisfy(lista_trabajando, sigue_activo) ||
    		list_any_satisfy(lista_bloqueado_IO, sigue_activo))
			; //espera a que todos los tripulantes terminen el su 'sleep'



    bool sorter(void *data1, void *data2){
    	int resta_patotas;
    	int resta_tripulantes;
    	bool resultado;
    	Tripulante_Planificando *tripulante1 = (Tripulante_Planificando *) data1;
    	Tripulante_Planificando *tripulante2 = (Tripulante_Planificando *) data2;

    	resta_patotas     = tripulante1->tripulante->patota - tripulante2->tripulante->patota;
    	resta_tripulantes = tripulante1->tripulante->id     - tripulante2->tripulante->id;

    	if(resta_patotas > 0)
    		return false;
    	if(resta_patotas < 0)
    		return true;

    	else
    		if(resta_tripulantes > 0)
    			return false;
    		else
    			return true;
    }



	pthread_mutex_lock(&lock_lista_exec);
	list_sort(lista_trabajando, sorter);
	pthread_mutex_unlock(&lock_lista_exec);

	pthread_mutex_lock(&lock_lista_listo);
	list_sort(lista_listo->elements, sorter);
	pthread_mutex_unlock(&lock_lista_listo);

	tripulante_cercano = (Tripulante_Planificando*) mas_cercano_al_sabotaje(x, y);

	void destroyer(void *data){
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		free(tripulante->tripulante);
		sem_destroy(&tripulante->termino_sabotaje);
		sem_destroy(&tripulante->ir_exec);
		sem_destroy(&tripulante->salir_pausa);
		free(tripulante->tarea->nombre);
	}

	if (list_size(lista_trabajando) > 0) {
		pthread_mutex_lock(&lock_lista_exec);
		list_add_all(lista_bloqueado_EM, lista_trabajando);
		//list_clean_and_destroy_elements(lista_trabajando, destroyer);
		list_clean(lista_trabajando); //sigue con referencias
		pthread_mutex_unlock(&lock_lista_exec);
	}

	if (list_size(lista_listo->elements) > 0) {
		pthread_mutex_lock(&lock_lista_listo);
		list_add_all(lista_bloqueado_EM, lista_listo->elements);
		//list_clean_and_destroy_elements(lista_listo->elements, destroyer);
		queue_clean(lista_listo); // no se elimininan
		pthread_mutex_unlock(&lock_lista_listo);
	}

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return tripulante_cercano->tripulante->id == un_tripulante->tripulante->id
				&& tripulante_cercano->tripulante->patota == un_tripulante->tripulante->patota;
	}

	pthread_mutex_lock(&lock_lista_bloq_em);
	list_remove_by_condition(lista_bloqueado_EM, soy_yo);
	list_add(lista_bloqueado_EM, tripulante_cercano); //seria para agregarlo a lo ultimo
	pthread_mutex_unlock(&lock_lista_bloq_em);


	//SIGUEN HABIENDO TRIPULANTES EN LAS OTRAS COLAS
	printf("CANTIDAD TRIPULANTES EN TRABAJANDO: %d", list_size(lista_trabajando));
	printf("CANTIDAD TRIPULANTES EN READY: %d", list_size(lista_listo->elements));
	printf("CANTIDAD TRIPULANTES EN BLOQUEADOS EM: %d", list_size(lista_bloqueado_EM));

	void cambiar_estado_a_bloqueado_emergencia(void *data) {
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		tripulante->tripulante->estado = BLOQUEADO_EMERGENCIA;
	}

	list_iterate(lista_bloqueado_EM, cambiar_estado_a_bloqueado_emergencia);

	resolver_sabotaje(tripulante_cercano, x, y);



	sem_wait(&resolvi_sabotaje);
    //cuando termina sabotaje


    void reanudar_tripulantes(void *data) {
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		sem_post(&tripulante->termino_sabotaje);
	}

	void cambiar_estado_a_ready(void *data) {
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		tripulante->tripulante->estado = LISTO;
	}

	void printear_estado(void *data) {
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		log_info(logs_discordiador, "ESTADO EN SABOTAJE: %d",
				tripulante->tripulante->estado);
	}

	pthread_mutex_lock(&mutex);

	list_iterate(lista_bloqueado_EM, cambiar_estado_a_ready);

	pthread_mutex_lock(&lock_lista_bloq_em);
	pthread_mutex_lock(&lock_lista_listo);
	list_add_all(lista_listo->elements, lista_bloqueado_EM);
	pthread_mutex_unlock(&lock_lista_listo);
	list_clean(lista_bloqueado_EM);
	pthread_mutex_unlock(&lock_lista_bloq_em);

	pthread_mutex_unlock(&mutex);

	list_iterate(lista_listo->elements, printear_estado);

	pthread_mutex_lock(&sabotaje_lock);
	g_hay_sabotaje = false;
	pthread_mutex_unlock(&sabotaje_lock);

    //no creo q estos mutexs son necesarios
    pthread_mutex_lock(&sabotaje_lock);
	list_iterate(lista_listo->elements, reanudar_tripulantes);
	pthread_mutex_unlock(&sabotaje_lock);

	pthread_mutex_lock(&sabotaje_lock);
	sem_post(&termino_sabotaje_planificador);
	pthread_mutex_unlock(&sabotaje_lock);

}

void *mas_cercano_al_sabotaje(int x, int y){
  t_list *tripulantes_mergeados = list_create();
  void *tripulante;

  if(list_size(lista_trabajando) > 0){
	  list_add_all(tripulantes_mergeados, lista_trabajando);
  }
  if(queue_size(lista_listo) > 0){
	  list_add_all(tripulantes_mergeados, lista_listo->elements);
  }
  if(queue_size(lista_listo) <= 0 && list_size(lista_trabajando) <= 0) {
	  return NULL;
  }


  void *maximo(void *data1, void *data2){
    float distancia1;
    float distancia2;

    Tripulante_Planificando *t1 = (Tripulante_Planificando *)data1;
    Tripulante_Planificando *t2 = (Tripulante_Planificando *)data2;

    distancia1 = (t1->tripulante->posicionX - x) * (t1->tripulante->posicionX - x)
      + (t1->tripulante->posicionY - y) * (t1->tripulante->posicionY - y);
    distancia2 = (t2->tripulante->posicionX - x) * (t2->tripulante->posicionX - x)
      + (t2->tripulante->posicionY - y) * (t2->tripulante->posicionY - y);

    if(distancia1 < distancia2) {
      return t1;
    }
    
    else return t2;
  }

  tripulante = list_get_maximum(tripulantes_mergeados, maximo);

  free(tripulantes_mergeados);

  return tripulante;
}

bool llegue_al_sabotaje(Tripulante_Planificando *tripulante, int x, int y){
	int sourceX = tripulante->tripulante->posicionX;
	int sourceY = tripulante->tripulante->posicionY;

	log_info(logs_discordiador, "Me movi en SABOTAJE de (%d, %d) a (%d, %d)",
			sourceX, sourceY, x, y);

	return estoy_en_mismo_punto(sourceX, sourceY, x, y);
}

void moverse_al_sabotaje(Tripulante_Planificando *tripulante, int targetX, int targetY){
	int sourceX = tripulante->tripulante->posicionX;
	int sourceY = tripulante->tripulante->posicionY;
	static bool last_move_x = false;

	if (sourceX == targetX && last_move_x == false)
			last_move_x = true;

		if (sourceY == targetY && last_move_x == true)
			last_move_x = false;


		//mucho codigo repetido
		if (sourceX < targetX && last_move_x == false) {
			tripulante->tripulante->posicionX += 1;
			last_move_x = true;
			return;
		}

		if (sourceX > targetX && last_move_x == false) {
			tripulante->tripulante->posicionX -= 1;
			last_move_x = true;
			return;
		}

		if (sourceY < targetY && last_move_x == true) {
			tripulante->tripulante->posicionY += 1;
			last_move_x = false;
			return;
		}

		if (sourceY > targetY && last_move_x == true) {
			tripulante->tripulante->posicionY -= 1;
			last_move_x = false;
			return;
		}
}

void resolver_sabotaje(Tripulante_Planificando *tripulante, int x, int y){
	while(!llegue_al_sabotaje(tripulante, x, y)){
		moverse_al_sabotaje(tripulante, x, y);
	}

	sleep(duracion_sabotaje);

	sem_post(&resolvi_sabotaje);
	log_info(logs_discordiador, "El tripulante N: %d resolvio el sabotaje!", tripulante->tripulante->id);
}


///************************************************ Tareas **********************************************

char *tareas[] = {
		"REGAR 2;1;1;7",
		"PLANTAR;2;1;3",
		"GENERAR_OXIGENO 3;1;2;7",
		"GENERAR_COMIDA 5;3;5;3",
		"ESPERAR;7;8;3",
		"COMER;10;14;4",
		"MATAR;0;0;1",
		"JUGAR;9;7;2",
		NULL }; //ejemplo

/*char *dar_proxima_tarea(int patota){
	/*t_list respuesta;
	t_paquete* paquete=crear_paquete(SIGUIENTE_TAREA);
	agregar_a_paquete(paquete,patota,sizeof(int));
	enviar_paquete(paquete,socket_ram);
	eliminar_paquete(paquete);
	respuesta=recibir_paquete(socket_ram);
	return list_get(respuesta, 0);

	static int i=0;
	return i<=7 ? tareas[i++] : NULL;*/

char *dar_proxima_tarea(Tripulante *tripulante){
	char *tarea;
	int _socket_ram;
	int operacion_retorno;

	_socket_ram = iniciar_conexion(MI_RAM_HQ, config);

	serializar_y_enviar_tripulante(tripulante, PEDIDO_TAREA, _socket_ram);
	operacion_retorno = recibir_operacion(_socket_ram);

	if(operacion_retorno == PEDIDO_TAREA){
		tarea = recibir_mensaje(_socket_ram);

	}
	//liberar_cliente(_socket_ram);
	log_info(logs_discordiador, "Proxima tarea del tripulante %d: %s", tripulante->id, tarea);

	return tarea;
}

Tarea *proxima_tarea(Tripulante *tripulante){
	char *tarea_string = dar_proxima_tarea(tripulante);
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

	avisar_a_mongo_estado_tarea(nueva_tarea, tripulante, INICIO_TAREA);


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
	bool resultado;

	switch (tarea->tipo) {
	case TAREA_COMUN:
		resultado = estoy_en_mismo_punto(sourceX, sourceY, targetX, targetY)
				&& tarea->duracion == 0;
		break;
	case TAREA_IO:
		resultado = estoy_en_mismo_punto(sourceX, sourceY, targetX, targetY);
		break;
	default:
		log_info(logs_discordiador,
				"ERROR en funcion 'completo_tarea', no coincide tipos..\n");
		return false;
		break;
	}

	if(resultado == true && tripulante_trabajando->tarea->tipo == TAREA_COMUN)
		avisar_a_mongo_estado_tarea(tarea, tripulante, FIN_TAREA);

	return resultado;
}

void moverse_una_unidad(Tripulante_Planificando *tripulante_trabajando) {
	int _socket_ram;
	static bool last_move_x = false;
	int targetX, targetY, sourceX, sourceY;
	targetX = tripulante_trabajando->tarea->posX;
	targetY = tripulante_trabajando->tarea->posY;
	sourceX = tripulante_trabajando->tripulante->posicionX;
	sourceY = tripulante_trabajando->tripulante->posicionY;


	_socket_ram   = iniciar_conexion(MI_RAM_HQ, config);

	if (sourceX == targetX && last_move_x == false)
		last_move_x = true;

	if (sourceY == targetY && last_move_x == true)
		last_move_x = false;


	//mucho codigo repetido
	if (sourceX < targetX && last_move_x == false) {
		tripulante_trabajando->tripulante->posicionX += 1;
		last_move_x = true;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		return;
	}

	if (sourceX > targetX && last_move_x == false) {
		tripulante_trabajando->tripulante->posicionX -= 1;
		last_move_x = true;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		return;
	}

	if (sourceY < targetY && last_move_x == true) {
		tripulante_trabajando->tripulante->posicionY += 1;
		last_move_x = false;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		return;
	}

	if (sourceY > targetY && last_move_x == true) {
		tripulante_trabajando->tripulante->posicionY -= 1;
		last_move_x = false;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		return;
	}
}

void realizar_tarea_IO(Tripulante_Planificando *tripulante_trabajando) {
	t_paquete *paquete;
	sleep(retardo_ciclo_cpu);
	tripulante_trabajando->tarea->duracion -= 1;


	if (tripulante_trabajando->tarea->duracion == 0) {
		avisar_a_mongo_estado_tarea(tripulante_trabajando->tarea, tripulante_trabajando->tripulante, FIN_TAREA);
		//llenar con los caracteres



		/*int socket_store = iniciar_conexion(I_MONGO_STORE, config);
		paquete = crear_paquete(tripulante_trabajando->tarea->tarea_code);
		char parametro = (char) tripulante_trabajando->tarea->parametro;
		agregar_a_paquete(paquete, parametro, strlen(parametro));
		enviar_paquete(paquete, socket_store);
		eliminar_paquete(paquete);
		liberar_cliente(socket_store);*/

		//avisar aca
	}

	log_info(logs_discordiador, "Tripulante N:%d - REALIZO una unidad de tarea IO, le quedan %d.",
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
	log_info(logs_discordiador, "Tripulante:%d de Patota:%d esta en (%d, %d) con %d unidades de tarea %s",
			tripulante_trabajando -> tripulante -> id, tripulante_trabajando -> tripulante -> patota,
			tripulante_trabajando -> tripulante -> posicionX, tripulante_trabajando -> tripulante -> posicionY,
			tripulante_trabajando -> tarea -> duracion, tripulante_trabajando -> tarea -> nombre);
}


//************************************************ PAUSA/SABOTAJE **********************************************



void fijarse_si_hay_pausa_hilo(Tripulante_Planificando *tripulante){
	if(g_hay_pausa) {
		sem_wait(&tripulante->salir_pausa);
	}
}

void fijarse_si_hay_pausa_planificador(){
	if(g_hay_pausa) {
		sem_wait(&otros_inicios);
		reanudar_hilos_lista(LISTO);
		reanudar_hilos_lista(TRABAJANDO);
		reanudar_hilos_lista(BLOQUEADO_IO);
		reanudar_hilos_lista(BLOQUEADO_EMERGENCIA);
		//avisar a todos los hilos con un signal
	}
}

//************************************************ CAMBIOS DE COLA **********************************************


void moverse_a_ready(Tripulante_Planificando *tripulante_trabajando){
	Estado estado_actual = tripulante_trabajando->tripulante->estado;

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante = (Tripulante_Planificando *) data;
			return tripulante_trabajando -> tripulante ->id == un_tripulante->tripulante->id
					&& tripulante_trabajando->tripulante->patota == un_tripulante->tripulante->patota;
	}

	switch (estado_actual) {
	case TRABAJANDO:
		pthread_mutex_lock(&lock_lista_listo);
		queue_push(lista_listo, tripulante_trabajando);
		pthread_mutex_unlock(&lock_lista_listo);
		tripulante_trabajando->tripulante->estado = LISTO;
		pthread_mutex_lock(&lock_lista_exec);
		list_remove_by_condition(lista_trabajando, soy_yo);
		pthread_mutex_unlock(&lock_lista_exec);
		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de EXEC a READY",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		break;

	case BLOQUEADO_IO:
		pthread_mutex_lock(&lock_lista_listo);
		queue_push(lista_listo, tripulante_trabajando);
		pthread_mutex_unlock(&lock_lista_listo);
		tripulante_trabajando->tripulante->estado = LISTO;
		pthread_mutex_lock(&lock_lista_bloq_io);
		list_remove_by_condition(lista_bloqueado_IO, soy_yo);
		pthread_mutex_unlock(&lock_lista_bloq_io);
		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de BLOQUEADO_IO a READY",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		break;

	}
}

void moverse_a_bloq(Tripulante_Planificando *tripulante_trabajando) {
	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante = (Tripulante_Planificando *) data;
		return tripulante_trabajando->tripulante->id == un_tripulante->tripulante->id
				&& tripulante_trabajando->tripulante->patota == un_tripulante->tripulante->patota;
	}

	//capaz borre algo
	
	pthread_mutex_lock(&lock_lista_bloq_io);
	list_add(lista_bloqueado_IO, tripulante_trabajando);
	pthread_mutex_unlock(&lock_lista_bloq_io);
	tripulante_trabajando->tripulante->estado = BLOQUEADO_IO;
	pthread_mutex_lock(&lock_lista_exec);
	list_remove_by_condition(lista_trabajando, soy_yo);
	pthread_mutex_unlock(&lock_lista_exec);
	log_info(logs_discordiador,
			"Tripulante:%d de Patota:%d pasa de EXEC a BLOQUEADO_IO",
			tripulante_trabajando->tripulante->id,
			tripulante_trabajando->tripulante->patota);

}

void mover_tripulante_a_exit(Tripulante_Planificando *tripulante_trabajando){
	Estado estado = tripulante_trabajando->tripulante->estado;

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return tripulante_trabajando->tripulante->id == un_tripulante->tripulante->id
				&& tripulante_trabajando->tripulante->patota == un_tripulante->tripulante->patota;
	}


	switch(estado){
	case TRABAJANDO:
		pthread_mutex_lock(&lock_lista_exit);
		list_add(lista_finalizado, tripulante_trabajando);
		pthread_mutex_unlock(&lock_lista_exit);
		tripulante_trabajando->tripulante->estado = FINALIZADO;

		pthread_mutex_lock(&lock_lista_exec);
		list_remove_by_condition(lista_trabajando, soy_yo);
		pthread_mutex_unlock(&lock_lista_exec);
		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de EXEC a EXIT",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		break;


	case BLOQUEADO_IO:
		pthread_mutex_lock(&lock_lista_exit);					//estos se repetirian
		list_add(lista_finalizado, tripulante_trabajando);		//
		pthread_mutex_unlock(&lock_lista_exit);					//
		tripulante_trabajando->tripulante->estado = FINALIZADO; //

		pthread_mutex_lock(&lock_lista_bloq_io);
		list_remove_by_condition(lista_bloqueado_IO, soy_yo);
		pthread_mutex_unlock(&lock_lista_bloq_io);
		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de BLOQUEADO_IO a EXIT",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		break;

	default:
		//TODO: modificar esto, porque despues se tienen que mandar a exit tripulantes 'Expulsados'
		log_info(logs_discordiador, "Se intento mandar a exit un tripulante de otro estado");
		break;
	}


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
		while (!completo_tarea(tripulante) && !tripulante->fui_expulsado) {
			fijarse_si_hay_pausa_hilo(tripulante);

			if (g_hay_sabotaje) { //la logica de esperar esta en la funcion tripulante
				pthread_mutex_lock(&lock_grado_multitarea);
				lugares_en_exec++;
				pthread_mutex_unlock(&lock_grado_multitarea);
				return;
			}
			
			hacer_una_unidad_de_tarea(tripulante);
		}

		if(tripulante->fui_expulsado){
			sem_post(&ya_sali_de_exec);
			return;
		}

		pthread_mutex_lock(&lock_grado_multitarea);
		lugares_en_exec++;
		pthread_mutex_unlock(&lock_grado_multitarea);

		if (completo_tarea && tripulante->tarea->tipo == TAREA_IO) {
			return;
		}

		else { //tengo que pedir la proxima tarea
			pthread_mutex_lock(&mutex_tarea);
			tripulante->tarea = proxima_tarea(tripulante->tripulante);
			pthread_mutex_unlock(&mutex_tarea);
			if (tripulante->tarea == NULL) {
				log_info(logs_discordiador, "EL TRIPULANTE N %d FINALIZO", tripulante->tripulante->id);
				mover_tripulante_a_exit(tripulante);
				sem_wait(&tripulantes_hermanos); //falta implementar que un proceso espere en exit a los otros de su patota
			}
		}
		break;
	case RR:
		while (!completo_tarea(tripulante) && tripulante->quantum_disponible > 0 && !tripulante->fui_expulsado) {
			fijarse_si_hay_pausa_hilo(tripulante);

			if (g_hay_sabotaje) { //la logica de esperar esta en la funcion tripulante
				pthread_mutex_lock(&lock_grado_multitarea);
				lugares_en_exec++;
				pthread_mutex_unlock(&lock_grado_multitarea);
				tripulante->quantum_disponible = quantum;
				return;
			}

			hacer_una_unidad_de_tarea(tripulante);
			tripulante->quantum_disponible -= 1;
		}


		if (tripulante->fui_expulsado) {
			sem_post(&ya_sali_de_exec);
			return;
		}

		pthread_mutex_lock(&lock_grado_multitarea);
		lugares_en_exec++;
		pthread_mutex_unlock(&lock_grado_multitarea);

		if (completo_tarea(tripulante) && tripulante->tarea->tipo == TAREA_IO)
			return;

		if (completo_tarea(tripulante)) {
			pthread_mutex_lock(&mutex_tarea);
			tripulante->tarea = proxima_tarea(tripulante->tripulante);
			pthread_mutex_unlock(&mutex_tarea);
			if (tripulante->tarea == NULL) {
				log_info(logs_discordiador, "EL TRIPULANTE N %d FINALIZO", tripulante->tripulante->id);
				mover_tripulante_a_exit(tripulante);
				sem_wait(&tripulantes_hermanos); //falta implementar que un proceso espere en exit a los otros de su patota
			}
		}

		tripulante->quantum_disponible = quantum; //se resetea el quantum

		break;
	}
}


//************************************************ PLANIFICADOR **********************************************

void planificar() {
	Tripulante_Planificando *tripulante;


	sem_wait(&primer_inicio);

	while (1) {
		if (g_hay_sabotaje) {
			sem_wait(&termino_sabotaje_planificador);
		}

		fijarse_si_hay_pausa_planificador();

		if (queue_size(lista_llegada) > 0) {
			pthread_mutex_lock(&lock_lista_llegada);
			tripulante = queue_pop(lista_llegada);
			pthread_mutex_unlock(&lock_lista_llegada);
			pthread_mutex_lock(&lock_lista_listo);
			queue_push(lista_listo, tripulante);
			pthread_mutex_unlock(&lock_lista_listo);
			tripulante->tripulante->estado = LISTO;
		}

		if (queue_size(lista_listo) > 0 && lugares_en_exec > 0) {
			pthread_mutex_lock(&lock_lista_llegada);
			tripulante = queue_pop(lista_listo);
			pthread_mutex_unlock(&lock_lista_llegada);
			tripulante->tripulante->estado = TRABAJANDO;
			pthread_mutex_lock(&lock_lista_exec);
			list_add(lista_trabajando, tripulante);
			pthread_mutex_unlock(&lock_lista_exec);
			pthread_mutex_lock(&lock_grado_multitarea);
			lugares_en_exec--;
			pthread_mutex_unlock(&lock_grado_multitarea);
			sem_post(&tripulante->ir_exec); //3
		}

	}

}



//************************************************ CONSOLA **********************************************


void atender_comandos_consola(void) {
	t_list *respuesta= list_create();
	static int num_pausas = 0; //para manejar el tipo de signal
	int socket_ram;
	int socket_store;

	while (1) {

		char **comando_separado;
		char **comando_separado_para_ram;
		tipo_comando valor = -1;
		char *comando_ingresado;


		comando_ingresado = readline(">");

		comando_separado_para_ram = string_n_split(comando_ingresado, 4," ");

		comando_separado = string_split(comando_ingresado, " "); // para pruebas

		for (int j = 0; j < CANT_COMANDOS; j++)
			if (strcmp(comando_separado[0], comandos_validos[j]) == 0)
				valor = j;

		switch (valor) {
		case INICIAR_PATOTA: //INICIAR_PATOTA 2 dd
			socket_ram = iniciar_conexion(MI_RAM_HQ, config);
			char *cantidad_tripulantes = comando_separado_para_ram[1];
			char *lista_tareas = comando_separado_para_ram[2];
			char *posiciones;
			char *respuesta2;

			posiciones = comando_separado_para_ram[3] == NULL ? "vacio" : comando_separado_para_ram[3];

			log_info(logs_discordiador, "Aviso a ram que deseo iniciar %s tripulantes..\n",cantidad_tripulantes);

			crear_y_enviar_inicio_patota(cantidad_tripulantes, lista_tareas, posiciones, socket_ram);

			int a = recibir_operacion(socket_ram);
			respuesta2 = recibir_mensaje(socket_ram);
			printf("codigo: %d", a);
			printf("RESPUESTA: %s", respuesta2);

			if(strcmp(respuesta2, "ok") == 0){
				printf("ME LLEGO OK");
				iniciar_patota(comando_separado);
			}


			else log_info(logs_discordiador, "NO SE PUDO CREAR PATOTA");

			 //capaz inicio de patota no necesita las posiciones

			g_numero_patota += 1; //la mandaria ram

			break;

		case LISTAR_TRIPULANTES: //LISTAR_TRIPULANTE
			printf("--------LISTANDO TRIPULANTES---------\n");
			listar_cola_planificacion(LLEGADA);
			listar_cola_planificacion(LISTO);
			listar_cola_planificacion(TRABAJANDO);
			listar_cola_planificacion(BLOQUEADO_IO);
			listar_cola_planificacion(BLOQUEADO_EMERGENCIA);
			listar_cola_planificacion(FINALIZADO);
			break;

		case EXPULSAR_TRIPULANTE: //EXPULSAR_TRIPULANTE id patota
			socket_ram = iniciar_conexion(MI_RAM_HQ, config);

			t_paquete* paquete_expulsar = crear_paquete(ELIMINAR_TRIPULANTE);
			agregar_a_paquete(paquete_expulsar, comando_separado[1], string_length(comando_separado[1]) + 1);
			agregar_a_paquete(paquete_expulsar, comando_separado[2], string_length(comando_separado[2]) + 1);
			enviar_paquete(paquete_expulsar, socket_ram);
			eliminar_paquete(paquete_expulsar);

			int numero_tripulante = atoi(comando_separado[1]);
			int numero_patota = atoi(comando_separado[2]);

			printf("Tripulante N: %d\n", numero_tripulante);
			printf("PATOTA N: %d\n", numero_patota);

			expulsar_tripulante(numero_tripulante, numero_patota);

			break;
		case INICIAR_PLANIFICACION: //INICIAR_PLANIFICACION
			;
			pthread_mutex_lock(&pausa_lock);
			g_hay_pausa = false;
			pthread_mutex_unlock(&pausa_lock);

			if(num_pausas == 0){
				sem_post(&primer_inicio);
				num_pausas++; //vuelve a pedir otro comando
			}

			else sem_post(&otros_inicios);


			break;
		case PAUSAR_PLANIFICACION: //PAUSAR_PLANIFICACION
			pthread_mutex_lock(&pausa_lock);
			g_hay_pausa = true;
			pthread_mutex_unlock(&pausa_lock);
			break;

		case OBTENER_BITACORA: //OBTENER_BITACORA
			socket_store = iniciar_conexion(I_MONGO_STORE, config);


			t_paquete *paquete_bitacora = crear_paquete(OBTENGO_BITACORA);
			agregar_a_paquete(paquete_bitacora, comando_separado[1],
						string_length(comando_separado[1]) + 1);
			agregar_a_paquete(paquete_bitacora, comando_separado[2],
						string_length(comando_separado[2]) + 1);
			enviar_paquete(paquete_bitacora, socket_store);
			eliminar_paquete(paquete_bitacora);

			int cod_op = recibir_operacion(socket_store);
			respuesta=recibir_paquete(socket_store);
			log_info(logs_discordiador,"INICIO DE BITACORA DEL TRIPULANTE %s",comando_separado[1]);
			imprimir_respuesta_log(respuesta);
			log_info(logs_discordiador,"FIN DE BITACORA DEL TRIPULANTE %s",comando_separado[1]);
			list_destroy_and_destroy_elements(respuesta,free);
			//liberar_cliente(socket_store);
			//liberar_cliente(socket_store);

			break;

		case EXIT: //EXIT
			;
			pthread_mutex_t unM;

			pthread_mutex_lock(&unM);
			atender_sabotaje(5, 7);
			pthread_mutex_unlock(&unM);
			/*
			printf("Si realmente deseas salir apreta 'S'..\n");
			char c = getchar();
			if(c == 's' || c == 'S'){
				//programa_activo = false;
				return;
			}*/
			break;
		default:
			printf("COMANDO INVALIDO\n");
			break;
		}
	}
}

//************************************************ COMUNICACIONES **********************************************

void crear_y_enviar_inicio_patota(char *cantidad, char *path_tareas, char *posiciones, int socket){
	t_paquete *paquete = crear_paquete(INICIO_PATOTA);
	FILE *tareas_file;
	char *contenido_tareas = NULL;
	uint32_t size_contenido_tareas;

	if ((tareas_file = fopen(path_tareas, "r")) == NULL) {
		printf("Error al abrir el archivo de tareas.");
		exit(1);
	}

	ssize_t bytes = getdelim(&contenido_tareas, &size_contenido_tareas, '\0',
			tareas_file);

	if (bytes == -1) {
		printf("Error leyendo archivo!\n");
	}

	contenido_tareas[size_contenido_tareas] = '\0'; //posible error

	agregar_a_paquete(paquete, cantidad, string_length(cantidad) + 1);
	agregar_a_paquete(paquete, posiciones, string_length(posiciones) + 1);
	agregar_a_paquete(paquete, contenido_tareas, string_length(contenido_tareas) + 1);

	enviar_paquete(paquete, socket);

	fclose(tareas_file);
}

void avisar_a_mongo_estado_tarea(Tarea *nueva_tarea, Tripulante *tripulante, op_code operacion){
	t_paquete *paquete = crear_paquete(operacion);
	char *nombreTarea = strdup(nueva_tarea->nombre);
	char tipo_tarea[5];
	char duracion[5];
	char idTripulante[5];
	char idPatota[5];
	char parametro[5];
	int _socket_store;

	_socket_store = iniciar_conexion(I_MONGO_STORE, config);

	sprintf(parametro,    "%d", nueva_tarea->parametro);
	sprintf(tipo_tarea,   "%d", nueva_tarea->tipo);
	sprintf(duracion,     "%d", nueva_tarea->duracion);
	sprintf(idTripulante, "%d", tripulante->id);
	sprintf(idPatota,     "%d", tripulante->patota);

	/*printf("\n-------: %s\n", nombreTarea);
	printf("\n-------: %s y tamaño: %d\n", duracion, string_length(duracion));*/

	agregar_a_paquete(paquete, idTripulante, string_length(idTripulante) + 1);
	agregar_a_paquete(paquete, idPatota,     string_length(idPatota) + 1);
	agregar_a_paquete(paquete, nombreTarea,  string_length(nombreTarea) + 1);
	agregar_a_paquete(paquete, duracion,     string_length(duracion) + 1);
	agregar_a_paquete(paquete, tipo_tarea,   string_length(tipo_tarea) +1 );
	agregar_a_paquete(paquete, parametro,    string_length(parametro) + 1);

	enviar_paquete(paquete, _socket_store);

	//liberar_cliente(_socket_store);
	eliminar_paquete(paquete);
}

void avisar_movimiento_a_mongo(int sourceX, int sourceY, Tripulante* tripulante){
	t_paquete *paquete = crear_paquete(ACTUALIZAR_POSICION);
	int _socket_store;
	char origenX[5], origenY[5], destinoX[5], destinoY[5], idPatota[5], idTripulante[5];

	_socket_store = iniciar_conexion(I_MONGO_STORE, config);

	sprintf(origenX, 	  "%d", sourceX);
	sprintf(origenY,      "%d", sourceY);
	sprintf(destinoX,	  "%d", tripulante->posicionX);
	sprintf(destinoY,	  "%d", tripulante->posicionY);
	sprintf(idTripulante, "%d", tripulante->id);
	sprintf(idPatota,     "%d", tripulante->patota);

	agregar_a_paquete(paquete, origenX,  string_length(origenX) + 1);
	agregar_a_paquete(paquete, origenY,  string_length(origenY) + 1);
	agregar_a_paquete(paquete, destinoX, string_length(destinoX) + 1);
	agregar_a_paquete(paquete, destinoY, string_length(destinoY) + 1);
	agregar_a_paquete(paquete, idTripulante, string_length(idTripulante) + 1);
	agregar_a_paquete(paquete, idPatota, string_length(idPatota) + 1);

	enviar_paquete(paquete, _socket_store);

	//liberar_cliente(_socket_store);
	eliminar_paquete(paquete);
}

void serializar_y_enviar_tripulante(Tripulante *tripulante, op_code tipo_operacion, int socket){
	t_paquete *paquete = crear_paquete(tipo_operacion);
	t_tripulante_iniciado *tripulante_enviado = malloc(sizeof(t_tripulante_iniciado));
	char estado;

	switch (tripulante->estado) {
	case LLEGADA:
		estado = 'N';
		break;
	case LISTO:
		estado = 'R';
		break;
	case TRABAJANDO:
		estado = 'E';
		break;
	case BLOQUEADO_IO:
		estado = 'B';
		break;
	case BLOQUEADO_EMERGENCIA:
		estado = 'B';
		break;
	case FINALIZADO:
		estado = 'F';
		break;
	}

	tripulante_enviado->numPatota   = tripulante->patota;
	tripulante_enviado->tid         = tripulante->id;
	tripulante_enviado->posX	    = tripulante->posicionX;
	tripulante_enviado->posY	    = tripulante->posicionY;
	tripulante_enviado->size_status = sizeof(estado);
	tripulante_enviado->status	    = estado;

	paquete->buffer->size = sizeof(uint32_t) * 5 + tripulante_enviado->size_status;
	void *stream = malloc(paquete->buffer->size);
	int offset = 0;

	memcpy(stream + offset, &(tripulante_enviado->numPatota), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante_enviado->tid), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante_enviado->posX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante_enviado->posY), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante_enviado->size_status), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &tripulante_enviado->status, tripulante_enviado->size_status);

	paquete->buffer->stream = stream;

	void *envio = malloc(sizeof(int) + paquete->buffer->size + sizeof(int));
	offset = 0;
	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(int));
	offset+=sizeof(int);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(int));
	offset+=sizeof(int);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	send(socket, envio, sizeof(int) + paquete->buffer->size + sizeof(int), 0);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


//************************************************ OTROS **********************************************

void imprimir_respuesta_log(t_list* respuesta){
	void iterator(char* value)
		{
		log_info(logs_discordiador,"valor: %s",value);
		}
	list_iterate(respuesta, (void*) iterator);

}


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
	//t_paquete* paquete=crear_paquete(NUEVO_TRIPULANTE);
	//t_list *respuesta;
	int _socket_ram;

	_socket_ram = iniciar_conexion(MI_RAM_HQ, config);


	argumentos_creacion_tripulantes *args = argumentos;
	Tripulante *tripulante = (Tripulante*)malloc(sizeof(Tripulante));
	tripulante -> id = args->numero_tripulante;
	tripulante -> patota = args->patota_actual;
	tripulante -> posicionX = args->posicionX;
	tripulante -> posicionY = args->posicionY;
	tripulante -> estado = LLEGADA;

	serializar_y_enviar_tripulante(tripulante, NUEVO_TRIPULANTE, _socket_ram); //aviso a ram

	//---------------------------
	Tripulante_Planificando *tripulante_trabajando =
			(Tripulante_Planificando*) malloc(sizeof(Tripulante_Planificando));
	tripulante_trabajando->tripulante = tripulante;
	tripulante_trabajando->quantum_disponible = quantum;
	pthread_mutex_lock(&mutex_tarea);
	tripulante_trabajando->tarea = proxima_tarea(tripulante_trabajando->tripulante);
	pthread_mutex_unlock(&mutex_tarea);
	sem_init(&tripulante_trabajando->ir_exec, 0, 0);
	sem_init(&tripulante_trabajando->salir_pausa, 0, 0);
	sem_init(&tripulante_trabajando->termino_sabotaje, 0, 0);
	tripulante_trabajando->sigo_planificando = true;
	tripulante_trabajando->fui_expulsado = false;

	pthread_mutex_lock(&lock_lista_llegada);
	queue_push(lista_llegada, tripulante_trabajando);
	pthread_mutex_unlock(&lock_lista_llegada);
	pthread_mutex_unlock(&lockear_creacion_tripulante);


	while(1){
		//arrancar_de_nuevo:

		sem_wait(&tripulante_trabajando -> ir_exec);
		realizar_trabajo(tripulante_trabajando);



		if(completo_tarea(tripulante_trabajando) && tripulante_trabajando -> tarea -> tipo == TAREA_IO && !g_hay_sabotaje
				&& !tripulante_trabajando->fui_expulsado){

			log_info(logs_discordiador, "Tripulante:%d de Patota:%d ESPERA LUGAR EN BLOQUEADO IO",
									tripulante_trabajando ->tripulante->id,
									tripulante_trabajando ->tripulante->patota);
			sleep(retardo_ciclo_cpu); //piden que espere antes de entrar I/O
			moverse_a_bloq(tripulante_trabajando);
			sem_wait(&bloq_disponible);
			log_info(logs_discordiador, "Tripulante:%d de Patota:%d ARRANCA A HACER TAREAS DE IO",
												tripulante_trabajando ->tripulante->id,
												tripulante_trabajando ->tripulante->patota);


			while(tripulante_trabajando->tarea->duracion > 0 && !tripulante_trabajando->fui_expulsado){ //haciendo tareas IO
				fijarse_si_hay_pausa_hilo(tripulante_trabajando);
				if (g_hay_sabotaje) {
					tripulante_trabajando->sigo_planificando = false;
					sem_wait(&tripulante_trabajando->termino_sabotaje);
					tripulante_trabajando->sigo_planificando = true;
				}
				realizar_tarea_IO(tripulante_trabajando);
			}

			sem_post(&bloq_disponible);

			if (!tripulante_trabajando->fui_expulsado) {
				pthread_mutex_lock(&mutex_tarea);
				tripulante_trabajando->tarea = proxima_tarea(
						tripulante_trabajando->tripulante);
				pthread_mutex_unlock(&mutex_tarea);
				if (tripulante_trabajando->tarea == NULL) {
					log_info(logs_discordiador,
							"Tripulante:%d de Patota:%d se movio de BLOQ_IO a EXIT",
							tripulante_trabajando->tripulante->id,
							tripulante_trabajando->tripulante->patota);
					mover_tripulante_a_exit(tripulante_trabajando);
					sem_wait(&tripulantes_hermanos); //falta implementar que un proceso espere en exit a los otros de su patota
				}
			}
		}

		if(tripulante_trabajando->fui_expulsado){
			pthread_exit(NULL);
		}

		if(g_hay_sabotaje){
			tripulante_trabajando->sigo_planificando = false;
			sem_wait(&tripulante_trabajando->termino_sabotaje);
			log_info(logs_discordiador, "SOY LIBRE! Pat; %d, Trip: %d ",
					tripulante_trabajando->tripulante->id,
					tripulante_trabajando->tripulante->patota);
			log_info(logs_discordiador, "Mi estado es: %d", tripulante_trabajando->tripulante->estado);
			tripulante_trabajando->sigo_planificando = true;
		}

		moverse_a_ready(tripulante_trabajando);
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

void reanudar_hilos_lista(Estado estado){

	void reanudar(void *data){
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		sem_post(&tripulante->salir_pausa);
	}

	switch (estado) {
	case LISTO:
		list_iterate(lista_listo->elements, reanudar);
		break;
	case TRABAJANDO:
		list_iterate(lista_trabajando, reanudar);
		break;
	case BLOQUEADO_IO:
		list_iterate(lista_bloqueado_IO, reanudar);
		break;
	case BLOQUEADO_EMERGENCIA:
		list_iterate(lista_bloqueado_EM, reanudar);
		break;
	}

}

void expulsar_tripulante(int id , int patota){
	t_list *lista_entera = list_create();
	Tripulante_Planificando *tripulante = malloc(sizeof(Tripulante_Planificando));

    bool soy_yo(void *data) { //funcion para buscar un tripulante
    	Tripulante_Planificando *un_tripulante = (Tripulante_Planificando *) data;
        return id == un_tripulante->tripulante->id
               && patota == un_tripulante->tripulante->patota;
    }


	list_add_all(lista_entera, lista_llegada->elements);
	list_add_all(lista_entera, lista_listo->elements);
	list_add_all(lista_entera, lista_bloqueado_IO);
	list_add_all(lista_entera, lista_trabajando);
	list_add_all(lista_entera, lista_bloqueado_EM);



    tripulante = (Tripulante_Planificando *)list_find(lista_entera, soy_yo);

    printf("-------Quiero eliminar a N: %d, pat N:%d\n",
    		tripulante->tripulante->id, tripulante->tripulante->patota);

    if(tripulante == NULL){
    	printf("Cantidad elementos: %d\n", list_size(lista_entera));
    	printf("El tripulante es NULL\n");

    	sleep(5);
    }

    switch(tripulante->tripulante->estado){
        case LLEGADA:
            pthread_mutex_lock(&lock_lista_llegada);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_llegada->elements, soy_yo);
            pthread_mutex_unlock(&lock_lista_llegada);
            break;

        case LISTO:
            pthread_mutex_lock(&lock_lista_listo);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_listo->elements, soy_yo);
            pthread_mutex_unlock(&lock_lista_listo);
            break;

        case TRABAJANDO:
            pthread_mutex_lock(&lock_lista_exec);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_trabajando, soy_yo);
            pthread_mutex_unlock(&lock_lista_exec);

            tripulante->fui_expulsado = true;

            sem_wait(&ya_sali_de_exec);

            pthread_mutex_lock(&lock_grado_multitarea);
            lugares_en_exec++;
            pthread_mutex_unlock(&lock_grado_multitarea);
            break;

        case BLOQUEADO_IO:
            pthread_mutex_lock(&lock_lista_bloq_io);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_bloqueado_IO, soy_yo);
            pthread_mutex_unlock(&lock_lista_bloq_io);

            tripulante->fui_expulsado = true;


            break;

        case BLOQUEADO_EMERGENCIA:
        	pthread_mutex_lock(&lock_lista_bloq_em);
        	tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_bloqueado_EM, soy_yo);
        	pthread_mutex_unlock(&lock_lista_bloq_em);
            break;

        case FINALIZADO:
        	//ver si hace algo mas
        	//tienen q esperar a los q son de su patota
            break;
    }
    if(tripulante->tripulante->estado!=FINALIZADO){
    	tripulante->tripulante->estado=FINALIZADO;
        pthread_mutex_lock(&lock_lista_exit);
        list_add(lista_finalizado, tripulante);
        pthread_mutex_unlock(&lock_lista_exit);
    }

    void destroyer(void *data){
    	Tripulante_Planificando *trip = (Tripulante_Planificando *)data;

    	free(trip->tripulante);
    	free(trip->tarea->nombre);
    	free(trip->tarea);
    	sem_destroy(&trip->ir_exec);
    	sem_destroy(&trip->salir_pausa);
    	sem_destroy(&trip->termino_sabotaje);
    	free(trip);
    }



    list_clean(lista_entera);
    list_destroy(lista_entera);
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

	//socket_ram = iniciar_conexion(MI_RAM_HQ, config);
	//log_info(logs_discordiador, "CONECTANDOSE A RAM EN SOCKET %d..", socket_ram);

	ip_mongo_store = config_get_string_value(config, "IP_I_MONGO_STORE");
	log_info(logs_discordiador, "IP STORE: %s", ip_mongo_store);

	puerto_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	log_info(logs_discordiador, "PUERTO STORE: %s", puerto_mongo_store);

	//socket_store = iniciar_conexion(I_MONGO_STORE, config);
	//log_info(logs_discordiador, "CONECTANDOSE A MONGO STORE EN SOCKET %d..", socket_store);

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
	sem_init(&bloq_disponible, 0, 1);
	sem_init(&tripulantes_hermanos, 0, 0); //revisar si se borra
	sem_init(&moverse_a_em, 0, 0); //binario
	sem_init(&se_movio_a_em, 0, 0);//binario
	sem_init(&primer_inicio, 0, 0); //primer inicio plani
	sem_init(&otros_inicios, 0, 0); //otras inicios plani
	sem_init(&termino_sabotaje_planificador, 0, 0); //desbloquear sabotaje en plani
	sem_init(&resolvi_sabotaje, 0, 0); //para la funcion de resolver sabotaje
	sem_init(&ya_sali_de_exec, 0, 0); //para tripulante expulsado
	
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
	sem_destroy(&bloq_disponible);
	sem_destroy(&tripulantes_hermanos);

}


int main(void){

	inicializar_recursos_necesarios();

	pthread_t hilo_consola;
	pthread_t hilo_planificador;
	pthread_t hilo_para_sabotaje; //falta

	pthread_create(&hilo_planificador, NULL, (void *)planificar, NULL);
	pthread_detach(hilo_planificador);

	//pthread_create(&hilo_para_sabotaje, NULL, (void *)esperar_sabotaje, NULL);
	//pthread_detach(hilo_para_sabotaje);

	pthread_create(&hilo_consola, NULL, (void *)atender_comandos_consola, NULL);
	pthread_join(hilo_consola, NULL);

	log_info(logs_discordiador, "TERMINANDO PROGRAMA - LIBERANDO ESPACIO");
	liberar_memoria_discordiador();
	printf("\n-------TERMINO-------\n");

	return EXIT_SUCCESS;
}


