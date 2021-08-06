#include "tareas.h"

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
	liberar_cliente(_socket_ram);
	return tarea;
}

Tarea *proxima_tarea(Tripulante *tripulante){
	char *tarea_string = dar_proxima_tarea(tripulante);
	char **tarea_dividida;
	char **tarea_IO_dividida;

	Tarea *nueva_tarea = (Tarea*) malloc(sizeof(Tarea));

	if(string_equals_ignore_case(tarea_string, "null")){
				return NULL;

	}

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
		liberar_cliente(_socket_ram);
		return;
	}

	if (sourceX > targetX && last_move_x == false) {
		tripulante_trabajando->tripulante->posicionX -= 1;
		last_move_x = true;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);
		return;
	}

	if (sourceY < targetY && last_move_x == true) {
		tripulante_trabajando->tripulante->posicionY += 1;
		last_move_x = false;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);
		return;
	}

	if (sourceY > targetY && last_move_x == true) {
		tripulante_trabajando->tripulante->posicionY -= 1;
		last_move_x = false;
		avisar_movimiento_a_mongo(sourceX, sourceY, tripulante_trabajando->tripulante);
		serializar_y_enviar_tripulante(tripulante_trabajando->tripulante, ACTUALIZAR_POSICION, _socket_ram);
		liberar_cliente(_socket_ram);
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

