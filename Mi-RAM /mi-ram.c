#include "mi-ram.h"

void *gestionarCliente(int socket) {
		int conexionCliente;
		t_list* lista;
		int operacion;
		t_paquete *paquete;
		int respuesta;

		while(1) {
			int cliente = esperar_cliente(socket);
			printf("Cliente: %d\n", cliente);
			operacion = recibir_operacion(cliente);
			lista = NULL;

			printf("\nLA OPERACION ES: %d\n", operacion);

			switch(operacion) {
				case EXPULSAR_TRIPULANTE:
					lista = recibir_paquete(cliente);
					printf("Tripulante eliminado de la nave %s\n", (char *) list_get(lista,0));
					//liberar_cliente(cliente);
					break;
				case -1:
					printf("El cliente %d se desconecto.\n", cliente);
					//liberar_cliente(cliente);
					break;
				default:
					printf("Operacion desconocida.\n");
					break;

			}

		}
	}

void crearRAM(t_config *config, struct nodoMemoria *memoria){

	char* tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	printf("Paso 2");
	/*
	//Segun que esquema de memoria se arma la memoria a eso
	switch(atoi(tipoMemoria)){
	case 0:
		printf("Se intentó paginación");
		//TODO: PAGINACION
		break;
	case 1:
		printf("Se intento segmentación");
		//TODO: SEGMENTACION
		break;
	default:
		printf("Error, esquema de memoria desconocido.\n");
		break;
	}*/
}

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");
	//logger = log_create(archivoDeLog, "CoMAnda", 1, LOG_LEVEL_DEBUG);

	socket_mi_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	printf("Pasa 1");
	crearRAM(config, memoria);
	printf("Pasa 3");
	gestionarCliente(socket_mi_ram);
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
*/


  return EXIT_SUCCESS;
}
