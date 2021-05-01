/*
 * servidor.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "servidor.h"

int main(void)
{
	void iterator(char* value)
	{
		printf("%s\n", value);
	}

// Se inicializa el log del servidor
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

// Se inicia el servidor y se almacena el socket en "server_fd"
	int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");
// El servidor espera hasta que un cliente se conecte
	//Cuando un cliente se conecta, se guarda el socket del mismo en cliente_fd
	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista;
	// El servidor constantemente verifica si se recibe una operacion
	while(1)
	{
		int cod_op = recibir_operacion(cliente_fd);
		//Segun el codigo de operacion recibido, se decide que hacer
		switch(cod_op)
		{
		case MENSAJE:
			//Recibir mensaje imprime "Me llego el mensaje:"
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			//Realiza una iteracion usando el buffer, y guarda los valores en lista
			lista = recibir_paquete(cliente_fd);
			printf("Me llegaron los siguientes valores:\n");
			// Se imprimen los valores de la lista
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			//Se recibe un codigo de operacion erroneo
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}
