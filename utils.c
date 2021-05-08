#include "utils.h"

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
	case SERVER_I_MONGO_STORE:
		ip_server = config_get_string_value(config, "IP_I_MONGO_STORE");
		puerto_server = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
		if ((conexion = crear_conexion(ip_server, puerto_server)) == -1){
			printf("ERROR AL INTENTAR CONECTARSE A MONGO_STORE!!\n");
			return -1;
		}
		printf("Se conecto a MONGO_STORE, socket: %d..\n", conexion);
		return conexion;
	case SERVER_MI_RAM_HQ:
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
