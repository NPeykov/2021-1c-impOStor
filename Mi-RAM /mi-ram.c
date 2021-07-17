#include "segmentacion.c"
//#include "paginacion.c"
#include "mi-ram.h"

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");

	logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 1, LOG_LEVEL_DEBUG);

	socket_principal_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");

	tamaniomemoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	memoria = malloc(tamaniomemoria);
	memoriaPrincipal = list_create();
	patotas = list_create();
}

void atenderSegunEsquema(){
	if(strcmp(tipoMemoria, "SEGMENTACION") == 0){
		//Se establece el algoritmo de ubicacion
		char* algoritmoUbicacion =config_get_string_value(config, "ALGORITMO_UBICACION");
		if(strcmp(algoritmoUbicacion, "FF") == 0){
			esFF = true;
		}else{
			esFF = false; //Entonces es Best Fit (BF)
		}
		sem_init(&creacion_tripulante,0,1); //Estos semaforos pueden iniciarse en segmentacion.c
		sem_init(&tripulantesRestantes,0,0);
		pthread_t hilo_cliente;

		pthread_create(&hilo_cliente, NULL, (void *)gestionarClienteSeg(socket_principal_ram), NULL);
		pthread_join(hilo_cliente, NULL);
	}else{
		//Agregar Hilos

		//gestionarClientePag(socket_mi_ram);
	}
}

void dumpMemoria(){
	if(strcmp(tipoMemoria, "SEGMENTACION") == 0){
		dumpMemoriaSeg();
	}else{
		//dumpMemoriaPag(); TODO:Hacer dump Paginacion
	}
}



int main(){
	signal(SIGTSTP, dumpMemoriaSeg);

	inicializar_ram();

	atenderSegunEsquema();

	return EXIT_SUCCESS;
}
