#include "mi-ram.h"

void *gestionarCliente(int cliente) {

		printf("Cliente: %d\n", cliente);
		int conexionCliente;
		t_list* lista;
		int operacion;
		t_paquete *paquete;
		int respuesta;

		printf("Servidor: %d\n", servidor);

		while(1) {
			operacion = recibir_operacion(cliente);
			lista = NULL;

			printf("\nLA OPERACION ES: %d\n", operacion);

			switch(operacion) {

				case ELIMINAR_TRIPULANTE:

					/*
					lista = recibir_paquete(cliente);
					printf("Conexion con Cliente...\n");
					printf("Los datos del cliente son:\n");
					printf("ID: %s\n", (char*) list_get(lista,0));
					printf("Posicion X: %d\n", *(int*) list_get(lista,1));
					printf("Posicion Y: %d\n", *(int*) list_get(lista,2));
					ipCliente = (char*) list_get(lista, 3);
					puertoEscuchaCliente = (char*) list_get(lista, 4);
					paquete = crear_paquete(COMANDA);
					enviar_paquete(paquete, cliente);
					eliminar_paquete(paquete);
					liberar_cliente(cliente);
					break;
					*/

				case -1:
					printf("El cliente %d se desconecto.\n", cliente);
					//liberar_cliente(cliente);

				default:
					printf("Operacion desconocida.\n");
					break;

			}
		}
	}
}

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");
	//logger = log_create(archivoDeLog, "CoMAnda", 1, LOG_LEVEL_DEBUG);



	socket_mi_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);

	servidor = esperar_cliente(socket_cliente);

	//memoriaPrincipal = malloc(tamanioMemoria);
	//memoriaSwap = malloc(tamanioSwap);
	//restaurantes = list_create();

}

int main(){
  inicializar_ram();


  /*
  int socket_mi_ram;
  t_config *config;
  char* puerto;
  int socket_cliente;

  socket_mi_ram = levantar_servidor(MI_RAM_HQ);

  //-------------------------------------------------------//

  config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

  puerto = config_get_string_value(config, "PUERTO");

  printf("MI_RAM escuchando en PUERTO:%s \n", puerto);

  socket_cliente = esperar_cliente(socket_mi_ram);
  */

  while(1) {
	int cliente=-1;
	cliente = esperar_cliente(servidor);
	if(cliente!=-1) {
	   gestionarCliente(cliente);
	   //pthread_create(&thread0, NULL, gestionarCliente, cliente);
	   //pthread_detach(thread0);
	}
  }

  //pthread_join(thread0, NULL);



  return EXIT_SUCCESS;
}
