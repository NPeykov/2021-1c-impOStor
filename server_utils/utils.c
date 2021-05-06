#include "utils.h"

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
	config = config_create(PATH_CLIENTE_CONFIG);

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

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	printf("SE CONECTO UN CLIENTE CON SOCKET: %d\n", socket_cliente);

	return socket_cliente;
}
