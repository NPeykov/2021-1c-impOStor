#include "mi-ram.h"
#include "segmentacion.c"
//#include "paginacion.c"

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");

	logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 1, LOG_LEVEL_DEBUG);

	socket_mi_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");

	tamaniomemoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	memoria = malloc(tamaniomemoria);
	memoriaPrincipal = list_create();
	patotas = list_create();

	if(strcmp(tipoMemoria, "SEGMENTACION") == 0){
		//Agregar Hilos
		gestionarClienteSeg(socket_mi_ram);
	}else{
		//Agregar Hilos

		//gestionarClientePag(socket_mi_ram);
	}
}


int main(){
  inicializar_ram();

  return EXIT_SUCCESS;
}
