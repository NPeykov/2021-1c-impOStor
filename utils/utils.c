#include "../utils/utils.h"

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		return -1;

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_conexion(int server_target, t_config *config) {
	char *ip_server, *puerto_server;
	int conexion;

	switch (server_target) {
	case I_MONGO_STORE:
		ip_server = config_get_string_value(config, "IP_I_MONGO_STORE");
		puerto_server = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
		if ((conexion = crear_conexion(ip_server, puerto_server)) == -1){
			printf("ERROR AL INTENTAR CONECTARSE A MONGO_STORE!!\n");
			return -1;
		}
		printf("Se conecto a MONGO_STORE, socket: %d..\n", conexion);
		return conexion;
	case MI_RAM_HQ:
		ip_server = config_get_string_value(config, "IP_MI_RAM_HQ");
		puerto_server = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
		if ((conexion = crear_conexion(ip_server, puerto_server)) == -1) {
			printf("ERROR AL INTENTAR CONECTARSE A MI_RAM!!\n");
			return -1;
		}
		printf("Se conecto a MI RAM, socket: %d..\n", conexion);
		return conexion;
	default:
		printf("No es un server valido para conectarse!\n");
		return -1;
	}
}

int crear_servidor(char *ip, char *puerto) {
	int estado, socket_server;
	int yes=1;
	struct addrinfo hints, *server_info, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = AI_PASSIVE;

	if((estado = getaddrinfo(ip, puerto, &hints, &server_info)) != 0){
		perror("Error estado: ");
		exit(1);
	}

	for (p = server_info; p != NULL; p = p->ai_next) {
		if ((socket_server = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		////
		if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes)
					== -1) {
				perror("setsockopt");
				exit(1);
			}
		////
		if (bind(socket_server, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_server);
			continue;
		}
		break;
	}

	listen(socket_server, SOMAXCONN);

	freeaddrinfo(server_info);

	return socket_server;
}


int levantar_servidor(int server_a_abrir) {
	int socket_servidor;
	t_config *config;
	char *puerto, *ip;
	config = config_create(PATH_DISCORDIADOR_CONFIG);

	switch (server_a_abrir) {
	case MI_RAM_HQ:
		ip = config_get_string_value(config, "IP_MI_RAM_HQ");
		puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
		break;
	case I_MONGO_STORE:
		ip = config_get_string_value(config, "IP_I_MONGO_STORE");
		puerto = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
		break;
	default:
		printf("Server no valido!");
		return 1;

	}

	socket_servidor = crear_servidor(ip, puerto);

	config_destroy(config);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor,t_log *log)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(log,"Se conecto un cliente con socket: %d\n", socket_cliente);

	return socket_cliente;
}

void liberar_cliente(int socket_cliente){
	close(socket_cliente);
}

/*------------------------------------------------------*/



void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(op_code operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
	crear_buffer(paquete);
	return paquete;
}


void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;

}

void* recibir_buffer(int *size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

char *recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);

	return buffer;
}


int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		return cod_op;
	}else
	{
		close(socket_cliente);
		return -1;
	}
}


void enviar_mensaje_simple(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}



void enviar_mensaje(op_code cod_op, char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = cod_op;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


t_tripulante_iniciado *recibir_tripulante_iniciado(int socket_cliente){
	t_tripulante_iniciado *tripulante = malloc(sizeof(t_tripulante_iniciado));
	t_buffer *buffer = malloc(sizeof(t_buffer));
	int offset = 0;

	recv(socket_cliente, &(buffer->size), sizeof(int), 0);
	buffer->stream = malloc(buffer->size);
	recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

	memcpy(&(tripulante->numPatota), buffer->stream + offset, sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(&(tripulante->tid), buffer->stream + offset, sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(&(tripulante->posX), buffer->stream + offset, sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(&(tripulante->posY), buffer->stream + offset, sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(&(tripulante->size_status), buffer->stream + offset, sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	//tripulante->status =(char) malloc(sizeof(char));
	memcpy(&tripulante->status, buffer->stream + offset, sizeof(char));

	//tripulante->status[tripulante->size_status] = '\0';

	return tripulante;
}

m_estado_tarea_tripulante *recibirNuevoEstadoTareaTripulante(int socket_cliente){
	m_estado_tarea_tripulante *tripulante_con_tarea = malloc(sizeof(m_estado_tarea_tripulante));
	t_list *lista = recibir_paquete(socket_cliente);
	char *nombre_tarea;

	tripulante_con_tarea->idTripulante = atoi(list_get(lista, 0));
	tripulante_con_tarea->numPatota    = atoi(list_get(lista, 1));
	tripulante_con_tarea->nombreTarea  = list_get(lista, 2);
	tripulante_con_tarea->duracionTarea= atoi(list_get(lista, 3));
	tripulante_con_tarea->tipo_tarea   = atoi(list_get(lista, 4));
	tripulante_con_tarea->parametro    = atoi(list_get(lista, 5));

	nombre_tarea = tripulante_con_tarea->nombreTarea;

	if (strcmp(nombre_tarea, "GENERAR_OXIGENO") == 0) {
		tripulante_con_tarea->codigo_tarea = GENERAR_OXIGENO;
	}

	if (strcmp(nombre_tarea, "CONSUMIR_OXIGENO") == 0) {
		tripulante_con_tarea->codigo_tarea = CONSUMIR_OXIGENO;
	}

	if (strcmp(nombre_tarea, "GENERAR_COMIDA") == 0) {
		tripulante_con_tarea->codigo_tarea = GENERAR_COMIDA;
	}

	if (strcmp(nombre_tarea, "CONSUMIR_COMIDA") == 0) {
		tripulante_con_tarea->codigo_tarea = CONSUMIR_COMIDA;
	}

	if (strcmp(nombre_tarea, "GENERAR_BASURA") == 0) {
		tripulante_con_tarea->codigo_tarea = GENERAR_BASURA;
	}

	if (strcmp(nombre_tarea, "DESCARTAR_BASURA") == 0) {
		tripulante_con_tarea->codigo_tarea = DESCARTAR_BASURA;
	}

	return tripulante_con_tarea;
}

m_movimiento_tripulante *recibirMovimientoTripulante(int socket_cliente){
	m_movimiento_tripulante *movimiento_tripulante = (m_movimiento_tripulante*)malloc(sizeof(m_movimiento_tripulante));
	t_list *lista = recibir_paquete(socket_cliente);

	movimiento_tripulante->origenX 		= atoi(list_get(lista, 0));
	movimiento_tripulante->origenY 		= atoi(list_get(lista, 1));
	movimiento_tripulante->destinoX 	= atoi(list_get(lista, 2));
	movimiento_tripulante->destinoY 	= atoi(list_get(lista, 3));
	movimiento_tripulante->idTripulante = atoi(list_get(lista, 4));
	movimiento_tripulante->idPatota     = atoi(list_get(lista, 5));

	return movimiento_tripulante;
}
