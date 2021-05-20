#include "discordiador.h"


void* consolaDiscordiador(){
	char* opcion_r = malloc(8);
	strcpy(opcion_r, "0");

	int mi_ram_hq_socket;
	t_paquete* paquete;
	int id_tripulante;
	t_config *config;

	config = config_create(PATH_DISCORDIADOR_CONFIG);

	int operacion;

	while(1) {
		printf("------ MODULO DISCORDIADOR ------\n");
		printf("INICIAR_PATOTA\n");
		printf("LISTAR_TRIPULANTES\n");
		printf("EXPULSAR_TRIPULANTES\n");
		printf("INICIAR_PLANIFICACION\n");
		printf("PAUSAR_PLANIFICACION\n\n");
		printf("OBTENER_BITACORA\n\n");
		printf("Salir <intro>\n\n\n");

		opcion_r = readline(">");
		if(strncmp(opcion_r, "", 1) == 0) {
			free(opcion_r);
			exit(-1);
		}

		switch(atoi(opcion_r)) {
		case 1: //"INICIAR_PATOTA":
			printf("Creo una patota");

			break;
		case 2: //"LISTAR_TRIPULANTES":
			printf("Estado de la Nave: 09/07/21 10:15:01\n");
			printf("Tripulante:1 Patota:1 Estado:EJECUTANDO\n");
			printf("Tripulante:2 Patota:2 Estado:EJECUTANDO\n");
			printf("Tripulante:3 Patota:2 Estado:BLOQUEADO I/O\n");
			break;
		case 3: //"EXPULSAR_TRIPULANTES":
			printf("Expulsar Tripulante");


//			mi_ram_hq_socket = levantar_servidor(MI_RAM_HQ);
			mi_ram_hq_socket = iniciar_conexion(MI_RAM_HQ,config);
			paquete = crear_paquete(ELIMINAR_TRIPULANTE);
			opcion_r = readline("Ingrese id del tripulante a eyectar: ");
			id_tripulante = atoi(opcion_r);
			agregar_a_paquete(paquete, &id_tripulante, sizeof(int));
			enviar_paquete(paquete, mi_ram_hq_socket);
			eliminar_paquete(paquete);


			break;
		case 4 : // "INICIAR_PLANIFICACION":
			printf("Iniciando Planificacion");

			break;
		case  5 : //"PAUSAR_PLANIFICACION":
			printf("Pauso Planificacion");


			break;
		case  6 : //"OBTENER_BITACORA":
			printf("Bitacora....");

				mongo_socket = levantar_servidor(I_MONGO_STORE);
				paquete = crear_paquete(OBTENER_BITACORA);
				opcion_r = readline("Ingrese id tripulante: ");
				id_tripulante = atoi(opcion_r);
				agregar_a_paquete(paquete, &id_tripulante, sizeof(int));
				enviar_paquete(paquete, mongo_socket);
				eliminar_paquete(paquete);

			break;
		default:
			printf("ERROR: opción no válida");
			break;
		}
	}

	free(opcion_r);
}

int main(void){
	consolaDiscordiador();


//	t_config *config;
//	int conexion_mi_ram_hq, conexion_i_mongo_store;
//
//	config = config_create(PATH_DISCORDIADOR_CONFIG);
//
//	conexion_mi_ram_hq = iniciar_conexion(SERVER_MI_RAM_HQ, config);
//
//	conexion_i_mongo_store = iniciar_conexion(SERVER_I_MONGO_STORE, config);

	/*
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
	*/

	return EXIT_SUCCESS;
}


