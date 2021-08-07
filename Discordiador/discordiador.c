#include "discordiador.h"

//************************************************ CAMBIOS DE COLA **********************************************

void moverse_a_ready(Tripulante_Planificando *tripulante_trabajando) {
	Estado estado_actual = tripulante_trabajando->tripulante->estado;
	int _socket_ram;

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return tripulante_trabajando->tripulante->id
				== un_tripulante->tripulante->id
				&& tripulante_trabajando->tripulante->patota
						== un_tripulante->tripulante->patota;
	}

	_socket_ram = iniciar_conexion(MI_RAM_HQ, config);

	switch (estado_actual) {
	case TRABAJANDO:
		pthread_mutex_lock(&lock_lista_listo);
		queue_push(lista_listo, tripulante_trabajando);
		pthread_mutex_unlock(&lock_lista_listo);
		tripulante_trabajando->tripulante->estado = LISTO;
		pthread_mutex_lock(&lock_lista_exec);
		list_remove_by_condition(lista_trabajando, soy_yo);
		pthread_mutex_unlock(&lock_lista_exec);

		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante,
				ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);

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

		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante,
				ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);

		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de BLOQUEADO_IO a READY",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		break;

	}
}

void moverse_a_bloq(Tripulante_Planificando *tripulante_trabajando) {
	int _socket_ram;
	_socket_ram = iniciar_conexion(MI_RAM_HQ, config);

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return tripulante_trabajando->tripulante->id
				== un_tripulante->tripulante->id
				&& tripulante_trabajando->tripulante->patota
						== un_tripulante->tripulante->patota;
	}

	//capaz borre algo

	pthread_mutex_lock(&lock_lista_bloq_io);
	list_add(lista_bloqueado_IO, tripulante_trabajando);
	pthread_mutex_unlock(&lock_lista_bloq_io);
	tripulante_trabajando->tripulante->estado = BLOQUEADO_IO;
	pthread_mutex_lock(&lock_lista_exec);
	list_remove_by_condition(lista_trabajando, soy_yo);
	pthread_mutex_unlock(&lock_lista_exec);

	serializar_y_enviar_tripulante(tripulante_trabajando->tripulante,
			ACTUALIZAR_POSICION, _socket_ram);
	liberar_cliente(_socket_ram);

	log_info(logs_discordiador,
			"Tripulante:%d de Patota:%d pasa de EXEC a BLOQUEADO_IO",
			tripulante_trabajando->tripulante->id,
			tripulante_trabajando->tripulante->patota);

}

void mover_tripulante_a_exit(Tripulante_Planificando *tripulante_trabajando) {
	Estado estado = tripulante_trabajando->tripulante->estado;
	int _socket_ram = iniciar_conexion(MI_RAM_HQ, config);

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return tripulante_trabajando->tripulante->id
				== un_tripulante->tripulante->id
				&& tripulante_trabajando->tripulante->patota
						== un_tripulante->tripulante->patota;
	}

	switch (estado) {
	case TRABAJANDO:
		pthread_mutex_lock(&lock_lista_exit);
		list_add(lista_finalizado, tripulante_trabajando);
		pthread_mutex_unlock(&lock_lista_exit);
		tripulante_trabajando->tripulante->estado = FINALIZADO;
		pthread_mutex_lock(&lock_lista_exec);
		list_remove_by_condition(lista_trabajando, soy_yo);
		pthread_mutex_unlock(&lock_lista_exec);

		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante,
				ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);

		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de EXEC a EXIT",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		sem_post(&voy_a_ready);

		break;

	case BLOQUEADO_IO:
		pthread_mutex_lock(&lock_lista_exit);			//estos se repetirian
		list_add(lista_finalizado, tripulante_trabajando);		//
		pthread_mutex_unlock(&lock_lista_exit);					//

		tripulante_trabajando->tripulante->estado = FINALIZADO; //

		pthread_mutex_lock(&lock_lista_bloq_io);
		list_remove_by_condition(lista_bloqueado_IO, soy_yo);
		pthread_mutex_unlock(&lock_lista_bloq_io);

		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante,
				ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);

		log_info(logs_discordiador,
				"Tripulante:%d de Patota:%d pasa de BLOQUEADO_IO a EXIT",
				tripulante_trabajando->tripulante->id,
				tripulante_trabajando->tripulante->patota);
		sem_post(&voy_a_ready);
		break;

	default:
		//TODO: modificar esto, porque despues se tienen que mandar a exit tripulantes 'Expulsados'
		log_info(logs_discordiador,
				"Se intento mandar a exit un tripulante de otro estado");
		break;
	}

	avisar_a_tripulantes_hermanos(tripulante_trabajando);
	esperar_tripulantes_hermanos(tripulante_trabajando);

}

void sacarlo_en_caso_de_expulsion(Tripulante_Planificando *tripulante) {
	avisar_a_ram_expulsion_tripulante(tripulante->tripulante->id,
			tripulante->tripulante->patota);

	sem_destroy(&tripulante->ir_exec);
	sem_destroy(&tripulante->salir_pausa);
	sem_destroy(
			list_get(tripulante->semaforos, tripulante->tripulante->id - 1));
	sem_destroy(&tripulante->termino_sabotaje);
	free(tripulante->tripulante);
	free(tripulante->tarea);
	free(tripulante);

	log_info(logs_discordiador,
			"Se liberaron los datos del tripulante expulsado");
}

void sacarlo_de_finalizado(Tripulante_Planificando *tripulante) {
	Tripulante_Planificando *retorno;

	avisar_a_ram_expulsion_tripulante(tripulante->tripulante->id,
			tripulante->tripulante->patota);

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return tripulante->tripulante->id == un_tripulante->tripulante->id
				&& tripulante->tripulante->patota
						== un_tripulante->tripulante->patota;
	}
/*
	if(tripulante->fue_sacado_de_fin){
		pthread_exit(NULL);
	}

	pthread_mutex_lock(&lock_lista_exit);
	retorno = (Tripulante_Planificando*) list_remove_by_condition(
			lista_finalizado, soy_yo);
	pthread_mutex_unlock(&lock_lista_exit);
*/

	if(!tripulante->fue_sacado_de_fin){
		pthread_mutex_lock(&lock_lista_exit);
		retorno = (Tripulante_Planificando*) list_remove_by_condition(lista_finalizado, soy_yo);
		pthread_mutex_unlock(&lock_lista_exit);

		if (retorno == NULL) {
			log_error(logs_discordiador, "No se encontro el tripulante en la lista de finalizado para eliminarlo");
			return;
		}
	}


	sem_destroy(&tripulante->ir_exec);
	sem_destroy(&tripulante->salir_pausa);
	sem_destroy(
			list_get(tripulante->semaforos, tripulante->tripulante->id - 1));
	sem_destroy(&tripulante->termino_sabotaje);
	free(tripulante->tripulante);
	free(tripulante->tarea);
	free(tripulante);

	log_info(logs_discordiador, "Se liberaron los datos del tripulante");

	pthread_exit(NULL);

}
//************************************************ ALGORITMO PLANIFICACION **********************************************

void realizar_trabajo(Tripulante_Planificando *tripulante) {
	Algoritmo algoritmo;

	if (strcmp("RR", algoritmo_planificacion) == 0)
		algoritmo = RR;
	else
		algoritmo = FIFO;

	switch (algoritmo) {
	case FIFO:
		while (!completo_tarea(tripulante) && !tripulante->fui_expulsado) {

			fijarse_si_hay_pausa_hilo(tripulante);

			if (tripulante->fui_expulsado)
				continue;

			if (g_hay_sabotaje) { //la logica de esperar esta en la funcion tripulante
				pthread_mutex_lock(&lock_grado_multitarea);
				lugares_en_exec++;
				log_info(logs_discordiador, "Se libero un lugar en exec, ahora hay %d", lugares_en_exec);
				pthread_mutex_unlock(&lock_grado_multitarea);
				return;
			}

			hacer_una_unidad_de_tarea(tripulante);

		}

		pthread_mutex_lock(&lock_grado_multitarea);
		lugares_en_exec++;
		log_info(logs_discordiador, "Se libero un lugar en exec, ahora hay %d", lugares_en_exec);
		pthread_mutex_unlock(&lock_grado_multitarea);

		if (tripulante->fui_expulsado) {
			//sem_post(&ya_sali_de_exec);
			return;
		}

		if (completo_tarea(tripulante) && tripulante->tarea->tipo == TAREA_IO) {
			return;
		}

		else { //tengo que pedir la proxima tarea
			avisar_a_mongo_estado_tarea(tripulante->tarea, tripulante->tripulante, FIN_TAREA);
			pthread_mutex_lock(&mutex_tarea);
			tripulante->tarea = proxima_tarea(tripulante->tripulante);
			pthread_mutex_unlock(&mutex_tarea);
			if (tripulante->tarea == NULL) {
				log_info(logs_discordiador, "EL TRIPULANTE N %d FINALIZO",
						tripulante->tripulante->id);
				mover_tripulante_a_exit(tripulante);
			}
		}

		break;
	case RR:
		while (!completo_tarea(tripulante) && tripulante->quantum_disponible > 0
				&& !tripulante->fui_expulsado) {
			fijarse_si_hay_pausa_hilo(tripulante);

			if (tripulante->fui_expulsado)
				continue;



			if (g_hay_sabotaje) { //la logica de esperar esta en la funcion tripulante
				pthread_mutex_lock(&lock_grado_multitarea);
				lugares_en_exec++;
				log_info(logs_discordiador, "Se libero un lugar en exec, ahora hay %d", lugares_en_exec);
				pthread_mutex_unlock(&lock_grado_multitarea);
				tripulante->quantum_disponible = quantum;
				return;
			}

			hacer_una_unidad_de_tarea(tripulante);
			tripulante->quantum_disponible -= 1;
		}

		pthread_mutex_lock(&lock_grado_multitarea);
		lugares_en_exec++;
		log_info(logs_discordiador, "Se libero un lugar en exec, ahora hay %d", lugares_en_exec);
		pthread_mutex_unlock(&lock_grado_multitarea);
		log_info(logs_discordiador, "Tripulante numero %d de patota %d deja lugar en EXEC",
				tripulante->tripulante->id, tripulante->tripulante->patota);

		if (tripulante->fui_expulsado) {
			//sem_post(&ya_sali_de_exec);
			return;
		}

		if (completo_tarea(tripulante) && tripulante->tarea->tipo == TAREA_IO)
			return;

		if (completo_tarea(tripulante)) {
			avisar_a_mongo_estado_tarea(tripulante->tarea, tripulante->tripulante, FIN_TAREA);
			pthread_mutex_lock(&mutex_tarea);
			tripulante->tarea = proxima_tarea(tripulante->tripulante);
			pthread_mutex_unlock(&mutex_tarea);
			if (tripulante->tarea == NULL) {
				log_info(logs_discordiador, "EL TRIPULANTE N %d FINALIZO",
						tripulante->tripulante->id);
				mover_tripulante_a_exit(tripulante);
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

		sem_wait(&voy_a_ready);

		if (g_hay_sabotaje) {
			sem_wait(&termino_sabotaje_planificador);
		}

		//fijarse_si_hay_pausa_planificador();

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

			pthread_mutex_lock(&lock_grado_multitarea);
			lugares_en_exec--;
			pthread_mutex_unlock(&lock_grado_multitarea);

			tripulante->tripulante->estado = TRABAJANDO;
			pthread_mutex_lock(&lock_lista_exec);
			list_add(lista_trabajando, tripulante);
			pthread_mutex_unlock(&lock_lista_exec);
			sem_post(&tripulante->ir_exec); //3
		}

	}

}

//************************************************ CONSOLA **********************************************

void atender_comandos_consola(void) {
	t_list *respuesta = list_create();
	static int num_pausas = 0; //para manejar el tipo de signal
	int socket_ram;
	int socket_store;

	while (1) {

		char **comando_separado;
		char **comando_separado_para_ram;
		tipo_comando valor = -1;
		char *comando_ingresado;

		comando_ingresado = readline(">");

		comando_separado = string_split(comando_ingresado, " "); // para pruebas

		for (int j = 0; j < CANT_COMANDOS; j++)
			if (strcmp(comando_separado[0], comandos_validos[j]) == 0)
				valor = j;

		switch (valor) {
		case INICIAR_PATOTA: //INICIAR_PATOTA 2 dd
			socket_ram = iniciar_conexion(MI_RAM_HQ, config);
			char *cantidad_tripulantes = comando_separado[1];
			char *lista_tareas = comando_separado[2];
			char *respuesta_inicio_patota;
			int operacion;

			log_info(logs_discordiador,
					"Aviso a ram que deseo iniciar %s tripulantes..\n",
					cantidad_tripulantes);

			crear_y_enviar_inicio_patota(cantidad_tripulantes, lista_tareas,
					socket_ram);

			operacion = recibir_operacion(socket_ram);
			respuesta_inicio_patota = recibir_mensaje(socket_ram);

			if (strcmp(respuesta_inicio_patota, "ok") == 0) {
				log_info(logs_discordiador, "ME DIERON OK PARA INICIAR PATOTA");
				iniciar_patota(comando_separado);
				g_numero_patota += 1; //la mandaria ram
			}
			else {
				log_error(logs_discordiador, "NO SE PUDO CREAR PATOTA");
			}

			liberar_cliente(socket_ram);
			break;

		case LISTAR_TRIPULANTES: //LISTAR_TRIPULANTE

			log_info(logs_discordiador,
					"\n--------LISTANDO TRIPULANTES---------");
			listar_discordiador();

			break;

		case EXPULSAR_TRIPULANTE:
			; //EXPULSAR_TRIPULANTE id patota

			int numero_tripulante = atoi(comando_separado[1]);
			int numero_patota = atoi(comando_separado[2]);

			expulsar_tripulante(numero_tripulante, numero_patota);

			break;
		case INICIAR_PLANIFICACION: //INICIAR_PLANIFICACION
			;
			pthread_mutex_lock(&pausa_lock);
			g_hay_pausa = false;
			pthread_mutex_unlock(&pausa_lock);

			if (num_pausas == 0) {
				sem_post(&primer_inicio);
				num_pausas++; //vuelve a pedir otro comando
			}

			else {
				reanudar_hilos_lista(LLEGADA);
				reanudar_hilos_lista(LISTO);
				reanudar_hilos_lista(TRABAJANDO);
				reanudar_hilos_lista(BLOQUEADO_IO);
				reanudar_hilos_lista(BLOQUEADO_EMERGENCIA);
				reanudar_hilos_lista(FINALIZADO);
			}
			//sem_post(&otros_inicios);

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
			respuesta = recibir_paquete(socket_store);
			log_info(logs_discordiador, "INICIO DE BITACORA DEL TRIPULANTE %s",
					comando_separado[1]);
			imprimir_respuesta_log(respuesta);
			log_info(logs_discordiador, "FIN DE BITACORA DEL TRIPULANTE %s",
					comando_separado[1]);
			list_destroy_and_destroy_elements(respuesta, free);
			liberar_cliente(socket_store);
			break;

		case EXIT: //EXIT
			;
			printf("Si realmente deseas salir apreta 'S'..\n");
			char c = getchar();

			if (c == 's' || c == 'S') {
				//programa_activo = false;
				log_info(logs_discordiador, "FINALIZANDO DISCORDIADOR");
				return;
			}
			break;
		default:

			log_error(logs_discordiador, "COMANDO INVALIDO");

			break;
		}

		free(comando_separado);
	}
}

//************************************************ OTROS **********************************************

void avisar_que_no_pudo_iniciar(int cantidad_hermanos, sem_t *semaforo) {

	for(int i = 0; i < cantidad_hermanos; i++){
		sem_post(semaforo);
	}
}

void avisar_a_tripulantes_hermanos(Tripulante_Planificando *tripulante) {
	pthread_mutex_lock(&mutex_lista_semaforos);
	sem_t *su_semaforo = list_get(tripulante->semaforos,
			tripulante->tripulante->id - 1);
	pthread_mutex_unlock(&mutex_lista_semaforos);

	for (int i = 0; i < tripulante->cant_trip; i++) {
		sem_post(su_semaforo);
	}

	tripulante->fui_expulsado = true;
}

void esperar_tripulantes_hermanos(Tripulante_Planificando *tripulante) {

	void esperar_hermanos(void *data) {
		sem_t *un_semaforo = (sem_t*) data;
		sem_wait(un_semaforo);
	}

	list_iterate(tripulante->semaforos, esperar_hermanos);

	if(!tripulante->fue_sacado_de_fin)
		log_info(logs_discordiador, "LA PATOTA NUMERO: %d FINALIZO",tripulante->tripulante->patota);


	sacarlo_de_finalizado(tripulante);

}

void iniciar_patota(char **datos_tripulantes) {
	int cantidad_tripulantes = atoi(datos_tripulantes[1]);
	char *posiciones[cantidad_tripulantes];
	int retorno_thread;
	pthread_t tripulantes[cantidad_tripulantes - 1];
	char **posicion_del_tripulante;
	argumentos_creacion_tripulantes *args =
			(argumentos_creacion_tripulantes*) malloc(
					sizeof(argumentos_creacion_tripulantes));

	t_list *semaforos = list_create();

	for (int i = 0; i < cantidad_tripulantes; i++) {
		sem_t *un_semaforo = (sem_t*) malloc(sizeof(sem_t));
		sem_init(un_semaforo, 0, 0);
		list_add(semaforos, un_semaforo);
	}

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
		args->numero_tripulante = i + 1;
		args->posicionX = atoi(posicion_del_tripulante[0]);
		args->posicionY = atoi(posicion_del_tripulante[1]);
		args->patota_actual = g_numero_patota;
		args->cantidad_tripulantes = cantidad_tripulantes;
		args->semaforos = semaforos;
		pthread_create(&tripulantes[i], NULL, (void *) tripulante,
				(void *) args);
	}

}

void tripulante(void *argumentos) {
	int _socket_ram;
	int codigo_operacion;
	char *respuesta_pedido;
	int cantidad_tripulantes;

	_socket_ram = iniciar_conexion(MI_RAM_HQ, config);

	argumentos_creacion_tripulantes *args = argumentos;
	Tripulante *tripulante = (Tripulante*) malloc(sizeof(Tripulante));
	tripulante->id = args->numero_tripulante;
	tripulante->patota = args->patota_actual;
	tripulante->posicionX = args->posicionX;
	tripulante->posicionY = args->posicionY;
	tripulante->estado = LLEGADA;
	cantidad_tripulantes = args->cantidad_tripulantes;

	//free(argumentos);

	serializar_y_enviar_tripulante(tripulante, NUEVO_TRIPULANTE, _socket_ram); //aviso a ram

	codigo_operacion = recibir_operacion(_socket_ram);
	respuesta_pedido = recibir_mensaje(_socket_ram);


	if(strcmp(respuesta_pedido, "ok") == 0){
		log_info(logs_discordiador, "Puedo iniciar tripulante numero: %d", tripulante->id);
	}
	else {
		log_error(logs_discordiador, "NO PUEDO REALIZAR INICIO TRIPULANTE %d", tripulante->id);
		liberar_cliente(_socket_ram);
		avisar_que_no_pudo_iniciar(cantidad_tripulantes, list_get(args->semaforos,tripulante->id -1));
		pthread_exit(NULL);
	}
	liberar_cliente(_socket_ram);

	//---------------------------

	Tripulante_Planificando *tripulante_trabajando =
			(Tripulante_Planificando*) malloc(sizeof(Tripulante_Planificando));
	tripulante_trabajando->tripulante = tripulante;
	tripulante_trabajando->quantum_disponible = quantum;
	pthread_mutex_lock(&mutex_tarea);
	tripulante_trabajando->tarea = proxima_tarea(
			tripulante_trabajando->tripulante);
	pthread_mutex_unlock(&mutex_tarea);
	sem_init(&tripulante_trabajando->ir_exec, 0, 0);
	sem_init(&tripulante_trabajando->salir_pausa, 0, 0);
	sem_init(&tripulante_trabajando->termino_sabotaje, 0, 0);
	tripulante_trabajando->sigo_planificando = true;
	tripulante_trabajando->fui_expulsado = false;
	tripulante_trabajando->fue_sacado_de_fin = false;
	tripulante_trabajando->cant_trip = cantidad_tripulantes;
	tripulante_trabajando->semaforos = args->semaforos;

	pthread_mutex_lock(&lock_lista_llegada);
	queue_push(lista_llegada, tripulante_trabajando);
	pthread_mutex_unlock(&lock_lista_llegada);
	pthread_mutex_unlock(&lockear_creacion_tripulante);

	while (1) {
		//arrancar_de_nuevo:
		sem_post(&voy_a_ready);
		sem_wait(&tripulante_trabajando->ir_exec);
		realizar_trabajo(tripulante_trabajando);

		/*
		 if (tripulante_trabajando->fui_expulsado) {
		 sem_post(&voy_a_ready);
		 esperar_tripulantes_hermanos(tripulante_trabajando);
		 log_info(logs_discordiador, "El tripulante %d sale del mapa",
		 tripulante_trabajando->tripulante->id);

		 sacarlo_de_finalizado(tripulante_trabajando);
		 }
		 */

		if (completo_tarea(tripulante_trabajando)
				&& tripulante_trabajando->tarea->tipo == TAREA_IO
				&& !g_hay_sabotaje && !tripulante_trabajando->fui_expulsado) {

			log_info(logs_discordiador,
					"Tripulante:%d de Patota:%d ESPERA LUGAR EN BLOQUEADO IO",
					tripulante_trabajando->tripulante->id,
					tripulante_trabajando->tripulante->patota);
			sleep(retardo_ciclo_cpu); //piden que espere antes de entrar I/O
			moverse_a_bloq(tripulante_trabajando);
			sem_post(&voy_a_ready); //para agregar grado multi cuando pasa a I/O
			sem_wait(&bloq_disponible);
			log_info(logs_discordiador,
					"Tripulante:%d de Patota:%d ARRANCA A HACER TAREAS DE IO",
					tripulante_trabajando->tripulante->id,
					tripulante_trabajando->tripulante->patota);

			while (tripulante_trabajando->tarea->duracion > 0
					&& !tripulante_trabajando->fui_expulsado) { //haciendo tareas IO
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
				}
			}
		}

		if (tripulante_trabajando->fui_expulsado) {

			sem_post(&voy_a_ready);

			avisar_a_tripulantes_hermanos(tripulante_trabajando);
			/*esperar_tripulantes_hermanos(tripulante_trabajando);
			 log_info(logs_discordiador, "El tripulante %d sale del mapa",
			 tripulante_trabajando->tripulante->id);*/

			sacarlo_de_finalizado(tripulante_trabajando);
		}

		if (g_hay_sabotaje) {
			tripulante_trabajando->sigo_planificando = false;
			sem_wait(&tripulante_trabajando->termino_sabotaje);
			tripulante_trabajando->sigo_planificando = true;
		}

		moverse_a_ready(tripulante_trabajando);
	}
}

void expulsar_tripulante(int id, int patota) {
	t_list *lista_entera = list_create();
	Tripulante_Planificando *tripulante = malloc(
			sizeof(Tripulante_Planificando));

	bool soy_yo(void *data) { //funcion para buscar un tripulante
		Tripulante_Planificando *un_tripulante =
				(Tripulante_Planificando *) data;
		return id == un_tripulante->tripulante->id
				&& patota == un_tripulante->tripulante->patota;
	}

	list_add_all(lista_entera, lista_llegada->elements);
	list_add_all(lista_entera, lista_listo->elements);
	list_add_all(lista_entera, lista_bloqueado_IO);
	list_add_all(lista_entera, lista_trabajando);
	list_add_all(lista_entera, lista_bloqueado_EM);
	list_add_all(lista_entera, lista_finalizado);

	tripulante = (Tripulante_Planificando *) list_find(lista_entera, soy_yo);

	if (tripulante == NULL) {
		log_error(logs_discordiador,
				"No se encontro el tripulante a expulsar.\n");
		return;
	}

    tripulante = (Tripulante_Planificando *)list_find(lista_entera, soy_yo);

    if(tripulante == NULL){
    	log_error(logs_discordiador, "No se encontro el tripulante a expulsar.\n");
    	return;
    }

    log_info(logs_discordiador, "Expulsando al tripulante %d de patota %d",
    		tripulante->tripulante->id, tripulante->tripulante->patota);

    switch(tripulante->tripulante->estado){
        case LLEGADA:
            pthread_mutex_lock(&lock_lista_llegada);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_llegada->elements, soy_yo);
            pthread_mutex_unlock(&lock_lista_llegada);
            avisar_a_tripulantes_hermanos(tripulante);
            sacarlo_en_caso_de_expulsion(tripulante);
            break;

        case LISTO:
            pthread_mutex_lock(&lock_lista_listo);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_listo->elements, soy_yo);
            pthread_mutex_unlock(&lock_lista_listo);
            avisar_a_tripulantes_hermanos(tripulante);
            sacarlo_en_caso_de_expulsion(tripulante);
            break;

        case TRABAJANDO:
            pthread_mutex_lock(&lock_lista_exec);
            tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_trabajando, soy_yo);
            pthread_mutex_unlock(&lock_lista_exec);

            tripulante->fui_expulsado = true;

            //sem_wait(&ya_sali_de_exec);

            //pthread_mutex_lock(&lock_grado_multitarea);
            //lugares_en_exec++;
            //pthread_mutex_unlock(&lock_grado_multitarea);
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
        	avisar_a_tripulantes_hermanos(tripulante);
        	sacarlo_en_caso_de_expulsion(tripulante);
            break;

        case FINALIZADO:
        	pthread_mutex_lock(&lock_lista_exit);
        	tripulante = (Tripulante_Planificando *)list_remove_by_condition(lista_finalizado, soy_yo);
        	pthread_mutex_unlock(&lock_lista_exit);
        	tripulante->fue_sacado_de_fin = true;
        	//sacarlo_en_caso_de_expulsion(tripulante);
            break;
    }

    if(tripulante->tripulante->estado==BLOQUEADO_IO || tripulante->tripulante->estado==TRABAJANDO){
    	tripulante->tripulante->estado=FINALIZADO;
        pthread_mutex_lock(&lock_lista_exit);
        list_add(lista_finalizado, tripulante);
        pthread_mutex_unlock(&lock_lista_exit);

        int _socket_ram = iniciar_conexion(MI_RAM_HQ, config);
        serializar_y_enviar_tripulante(tripulante->tripulante, ACTUALIZAR_POSICION, _socket_ram);
        liberar_cliente(_socket_ram);
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



void inicializar_recursos_necesarios(void) {

	comandos_validos[0] = "INICIAR_PATOTA";
	comandos_validos[1] = "LISTAR_TRIPULANTES";
	comandos_validos[2] = "EXPULSAR_TRIPULANTE";
	comandos_validos[3] = "INICIAR_PLANIFICACION";
	comandos_validos[4] = "PAUSAR_PLANIFICACION";
	comandos_validos[5] = "OBTENER_BITACORA";
	comandos_validos[6] = "EXIT";
	comandos_validos[7] = NULL;

	g_numero_patota = 1;
	g_hay_pausa = true;
	g_hay_sabotaje = false;

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

	puerto_mongo_store = config_get_string_value(config,
			"PUERTO_I_MONGO_STORE");
	log_info(logs_discordiador, "PUERTO STORE: %s", puerto_mongo_store);

	//socket_store = iniciar_conexion(I_MONGO_STORE, config);
	//log_info(logs_discordiador, "CONECTANDOSE A MONGO STORE EN SOCKET %d..", socket_store);

	algoritmo_planificacion = config_get_string_value(config, "ALGORITMO");
	log_info(logs_discordiador, "ALGORITMO DE PLANIFICACION: %s",
			algoritmo_planificacion);

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
	lista_llegada = queue_create();
	lista_listo = queue_create();
	lista_trabajando = list_create();
	lista_bloqueado_IO = list_create();
	lista_bloqueado_EM = list_create();
	lista_finalizado = list_create();
	log_info(logs_discordiador, " COLAS DE PLANIFICACION INICIALIZADAS..");

	//inicios semaforos
	sem_init(&bloq_disponible, 0, 1);
	sem_init(&moverse_a_em, 0, 0); //binario
	sem_init(&se_movio_a_em, 0, 0); //binario
	sem_init(&primer_inicio, 0, 0); //primer inicio plani
	sem_init(&otros_inicios, 0, 0); //otras inicios plani
	sem_init(&termino_sabotaje_planificador, 0, 0); //desbloquear sabotaje en plani
	sem_init(&resolvi_sabotaje, 0, 0); //para la funcion de resolver sabotaje
	sem_init(&ya_sali_de_exec, 0, 0); //para tripulante expulsado
	sem_init(&semaforo_tarea, 0, 1);
	sem_init(&voy_a_ready, 0, 0);

	log_info(logs_discordiador, "---DATOS INICIALIZADO---");
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

}

int main(void) {

	inicializar_recursos_necesarios();

	pthread_t hilo_consola;
	pthread_t hilo_planificador;
	pthread_t hilo_para_sabotaje;

	pthread_create(&hilo_planificador, NULL, (void *) planificar, NULL);
	pthread_detach(hilo_planificador);

	pthread_create(&hilo_para_sabotaje, NULL, (void *) esperar_sabotaje, NULL);
	pthread_detach(hilo_para_sabotaje);

	pthread_create(&hilo_consola, NULL, (void *) atender_comandos_consola,
			NULL);
	pthread_join(hilo_consola, NULL);

	log_info(logs_discordiador, "TERMINANDO PROGRAMA - LIBERANDO ESPACIO");
	liberar_memoria_discordiador();

	log_info(logs_discordiador, "-------TERMINO-------");

	return EXIT_SUCCESS;
}

