#include "discordiador.h"

//************************************************ Manejo de Estados **********************************************

bool tripulanteBloqueadoIO(void* elemento){
	Tripulante* tripulante = (Tripulante*)elemento;
	return tripulante->estado == BLOQUEADO_IO;
}
bool tripulanteBloqueadoEmergencia(void* elemento){
	Tripulante* tripulante = (Tripulante*)elemento;
	return tripulante->estado == BLOQUEADO_EMERGENCIA;
}
bool tripulanteFinalizado(void* elemento){
	Tripulante* tripulante = (Tripulante*)elemento;
	return tripulante->estado == FINALIZADO;
}

//************************************************ Inciar Discordiador **********************************************

void iniciarPlanificacion(){
	discordiador = (Discordiador*)malloc(sizeof(Discordiador));

	char *limiteTripulantesEjecutando = config_get_string_value(config, "GRADO_MULTITAREA");


}

//************************************************ PLANIFICADOR **********************************************

void fifo(){
	while(!teamTermino){
		sem_wait(&sem_proceso);
		//puts("sem_proceso");
		sem_wait(&sem_mx_proceso);
		//puts("sem_mx_proceso");
		//printf("-------cantidad de procesos: %d----\n",team->procesos->elements_count);
		if(team->procesos->elements_count > 0){

			irAAtrapar((Proceso*)team->procesos->head->data);
		}
		else
			return;
	}
}

void* consolaDiscordiador(){
	char* opcion_r = malloc(8);
	strcpy(opcion_r, "0");

	int mi_ram_hq_socket;
	int mongo_store_socket;
	t_paquete* paquete;
	int id_tripulante;

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
			printf("Expulsar Tripulante\n");
			mi_ram_hq_socket = iniciar_conexion(MI_RAM_HQ, config);
			paquete = crear_paquete(EXPULSAR_TRIPULANTE);
			id_tripulante = readline("Ingrese id del tripulante a eyectar: ");
			//id_tripulante = atoi(opcion_r);
			agregar_a_paquete(paquete, id_tripulante, sizeof(int));
			enviar_paquete(paquete, mi_ram_hq_socket);
			eliminar_paquete(paquete);
			break;
		case 4 : // "INICIAR_PLANIFICACION":
			printf("Iniciando Planificacion");

			break;
		case 5 : //"PAUSAR_PLANIFICACION":
			printf("Pauso Planificacion");


			break;
		case 6 : //"OBTENER_BITACORA":
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
	config = config_create(PATH_DISCORDIADOR_CONFIG);

	consolaDiscordiador();

	return EXIT_SUCCESS;
}


