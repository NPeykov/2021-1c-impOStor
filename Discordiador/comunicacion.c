#include "comunicacion.h"

void enviar_mensaje_mongo(op_code codigo){
	int socket_store = iniciar_conexion(I_MONGO_STORE, config);
	enviar_mensaje(codigo,"",socket_store);
	liberar_cliente(socket_store);
}

void crear_y_enviar_inicio_patota(char *cantidad, char *path_tareas, int socket){
	t_paquete *paquete = crear_paquete(INICIO_PATOTA);
	FILE *tareas_file;
	char *contenido_tareas = NULL;
	uint32_t size_contenido_tareas;

	if ((tareas_file = fopen(path_tareas, "r")) == NULL) {
		log_error(logs_discordiador, "Error al abrir el archivo de tareas.");
		exit(1);
	}

	ssize_t bytes = getdelim(&contenido_tareas, &size_contenido_tareas, '\0',
			tareas_file);

	if (bytes == -1) {
		log_error(logs_discordiador, "Error leyendo archivo!\n");
	}

	contenido_tareas[size_contenido_tareas] = '\0'; //posible error

	agregar_a_paquete(paquete, cantidad, string_length(cantidad) + 1);
	//agregar_a_paquete(paquete, posiciones, string_length(posiciones) + 1);
	agregar_a_paquete(paquete, contenido_tareas, string_length(contenido_tareas) + 1);

	enviar_paquete(paquete, socket);

	fclose(tareas_file);
}

void avisar_a_ram_expulsion_tripulante(int id, int patota) {

	int _socket_ram = iniciar_conexion(MI_RAM_HQ, config);

	char *num_id = string_new();
	num_id = string_from_format("%d", id);
	char *num_patota = string_new();
	num_patota = string_from_format("%d", patota);

	t_paquete* paquete_expulsar = crear_paquete(ELIMINAR_TRIPULANTE);
	agregar_a_paquete(paquete_expulsar, num_id, string_length(num_id) + 1);
	agregar_a_paquete(paquete_expulsar, num_patota, string_length(num_patota) + 1);

	enviar_paquete(paquete_expulsar, _socket_ram);

	eliminar_paquete(paquete_expulsar);

	liberar_cliente(_socket_ram);

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

	liberar_cliente(_socket_store);
	eliminar_paquete(paquete);
}

void avisar_estado_sabotaje_a_mongo(int posX, int posY, Tripulante* tripulante, op_code estado) {

	t_paquete *paquete = crear_paquete(estado);
	int _socket_store;
	char origenX[5], origenY[5], destinoX[5], destinoY[5], idPatota[5],
			idTripulante[5];

	_socket_store = iniciar_conexion(I_MONGO_STORE, config);

	sprintf(origenX, "%d", posX);
	sprintf(origenY, "%d", posY);
	sprintf(destinoX, "%d", tripulante->posicionX);
	sprintf(destinoY, "%d", tripulante->posicionY);
	sprintf(idTripulante, "%d", tripulante->id);
	sprintf(idPatota, "%d", tripulante->patota);

	agregar_a_paquete(paquete, origenX, string_length(origenX) + 1);
	agregar_a_paquete(paquete, origenY, string_length(origenY) + 1);
	agregar_a_paquete(paquete, destinoX, string_length(destinoX) + 1);
	agregar_a_paquete(paquete, destinoY, string_length(destinoY) + 1);
	agregar_a_paquete(paquete, idTripulante, string_length(idTripulante) + 1);
	agregar_a_paquete(paquete, idPatota, string_length(idPatota) + 1);

	enviar_paquete(paquete, _socket_store);
	liberar_cliente(_socket_store);
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

	liberar_cliente(_socket_store);
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

