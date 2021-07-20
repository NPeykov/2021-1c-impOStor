#include "segmentacion.c"
#include "mapa.c"
//#include "paginacion.c"

void inicializar_ram(){
	logs_ram = log_create("../logs_files/ram.log", "Mi-RAM", 0, LOG_LEVEL_WARNING);

	log_info(logs_ram,"################# Modulo Mi-RAM #################\n");

	socket_principal_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");

	tamaniomemoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	memoria = malloc(tamaniomemoria);
	memoriaPrincipal = list_create();
	patotas = list_create();

	crear_nivel();
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
		sem_init(&direcciones,0,1); //Estos semaforos pueden iniciarse en segmentacion.c
		sem_init(&numeroPatotas,0,1);
		sem_init(&tripulantesDisponibles,0,0);

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
	inicializar_ram();

	signal(SIGTSTP, dumpMemoriaSeg);

	atenderSegunEsquema();

	return EXIT_SUCCESS;
}
