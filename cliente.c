#include "cliente.h"


int main(void){
	t_config *config;
	int conexion_mi_ram_hq, conexion_i_mongo_store;

	config = config_create(PATH_CONFIG);

	conexion_mi_ram_hq = iniciar_conexion(SERVER_MI_RAM_HQ, config);

	conexion_i_mongo_store = iniciar_conexion(SERVER_I_MONGO_STORE, config);










	//PRUEBA MANDAR MSJS A MI-RAM
	char *saludo = "Hola MIRAM!";
	send(conexion_mi_ram_hq, saludo, sizeof(saludo),0);

	//PRUEBA ESCRIBIR EN MI PROPIA CONSOLA DESPUES MANDAR MSJ
	char s[100];
	printf("Escribi algo.. ");
	gets(s);
	printf("Escribi en mi propia consola: %s\n", s);







	//---------------------AL FINAL-------------//
	config_destroy(config);
	close(conexion_mi_ram_hq);
	close(conexion_i_mongo_store);

	return EXIT_SUCCESS;
}


