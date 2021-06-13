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
/*
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
}*/

//************************************************ CONSOLA **********************************************


void atender_comandos_consola(void) {

	while (1) {

		char **comando_separado;
		tipo_comando valor = -1;
		char *comando_ingresado;

		comando_ingresado = readline(">");

		comando_separado = string_split(comando_ingresado, " ");

		for (int j = 0; j < CANT_COMANDOS; j++)
			if (strcmp(comando_separado[0], comandos_validos[j]) == 0)
				valor = j;

		if(valor == INICIAR_PATOTA){
			pthread_t tripulante[comando_separado[1]];
		}


		switch (valor) {
		case INICIAR_PATOTA: //INICIAR_PATOTA
			log_info(logs_discordiador, "Iniciando %s tripulantes de patota numero %d..\n",comando_separado[1] ,numero_patota);
			int patota = numero_patota;
			int resultado_crear_patota;
			int cantidadTripulantes = atoi(comando_separado[1]);
			char *lista_tareas = strdup(comando_separado[2]); //LIBERAR ESPACIO
			argumentos_creacion_tripulantes *args = (argumentos_creacion_tripulantes*)malloc(sizeof(argumentos_creacion_tripulantes));
			printf("TAREA EN: %s\n", lista_tareas);
			printf("CANTIDAD: %d\n", cantidadTripulantes);

			//AVISAR A MI RAM QUE SE CREO LA PATOTA


			for (int i = 3, num_tripulante = 1; num_tripulante <= cantidadTripulantes; i++, num_tripulante++) {
				if (comando_separado[i] != NULL) {

					args->numero_tripulante = num_tripulante;
					char **posicion = string_split(comando_separado[i], "|");
					args->posicionX = atoi(posicion[0]);
					args->posicionY = atoi(posicion[1]);
					printf("posicion %d: %s\n", num_tripulante, comando_separado[i]);
					resultado_crear_patota = pthread_create(&tripulante[num_tripulante], NULL, (void *)crear_tripulante, args);
					pthread_detach(tripulante[num_tripulante]);
					if (resultado_crear_patota != 0) {
						log_info(logs_discordiador, "ERROR al iniciar tripulante %d de patota %d\n", num_tripulante, numero_patota);
					}
				}
				else{
					while (num_tripulante <= cantidadTripulantes){
						args->numero_tripulante = num_tripulante;
						args->posicionX = 0;
						args->posicionY = 0;
						printf("posicion %d: 0|0\n", num_tripulante++);
						resultado_crear_patota = pthread_create(&tripulante[num_tripulante], NULL, (void *)crear_tripulante, args);
						pthread_detach(tripulante[num_tripulante]);
						if(resultado_crear_patota != 0){
							log_info(logs_discordiador, "ERROR al iniciar tripulante %d de patota %d\n", num_tripulante, numero_patota);
						}
					}
				}
			}
			/*for (int num_tripulante = 1, j = 3;
					num_tripulante <= cantidadTripulantes;
					num_tripulante++, j++) {
				pthread_t tripulante;
				args->numero_tripulante = num_tripulante;
				if (comando_separado[j] == NULL){
					while(j <= cantidadTripulantes){
					args->posicionX = 0;
					args->posicionY = 0;
					}
				}
				else {
					char **posicion = string_split(comando_separado[j], "|");
					args->posicionX = atoi(posicion[0]);
					args->posicionY = atoi(posicion[1]);
				}


				resultado_crear_patota = pthread_create(&tripulante, NULL, (void *)crear_tripulante, args);
				if(resultado_crear_patota != 0){
					log_info(logs_discordiador, "ERROR al iniciar tripulante %d de patota %d\n", num_tripulante, numero_patota);
				}
			}*/




			numero_patota += 1; //PARA LA PROX VEZ QUE SEA INICIALIZADO
			break;

		case LISTAR_TRIPULANTES: //LISTAR_TRIPULANTE
			printf("--------LISTAR TRIPULANTES---------\n");
			t_link_element *elementos;
			if(llegada -> head == NULL){
				printf("No hay tripulantes en la cola de llegada!\n");
				break;
			}
			else{
				t_list *copia_llegada = (t_list* )malloc(sizeof(t_list));
				Tripulante *tripulante = (Tripulante *) malloc(sizeof(Tripulante));
				copia_llegada = llegada;
				while(copia_llegada -> head != NULL){
					elementos = copia_llegada -> head;
					tripulante = (Tripulante *) elementos -> data;
					printf("Tripulante N°: %d\n", tripulante->patota);
					printf("Tripulante ID°: %d\n\n", tripulante->id);
					copia_llegada->head = copia_llegada->head->next;
				}
			}
			/*for( = llegada -> head; llegada -> head != NULL; llegada -> head = llegada -> head -> next){
				Tripulante *tripulante = (Tripulante *) elementos -> data;
				printf("Tripulante N°: %d", tripulante->patota);
				elementos = llegada -> head;
			}*/
			printf("TAMAÑO LISTA: %d", list_size(llegada));
			break;

		case EXPULSAR_TRIPULANTE: //EXPULSAR_TRIPULANTE
			printf("ES EXPULSAR\n");
			break;
		case INICIAR_PLANIFICACION: //INICIAR_PLANIFICACION
			break;
		case PAUSAR_PLANIFICACION: //PAUSAR_PLANIFICACION
			break;
		case OBTENER_BITACORA: //OBTENER_BITACORA
			break;
		case EXIT: //SALIR
			printf("Si realmente deseas salir apreta 'S'..\n");
			char c;
			c = getchar();
			if(c == 's' || c == 'S')
				return;
			break;
		default:
			printf("COMANDO INVALIDO\n");
			break;
		}
	}
}

//************************************************ OTROS **********************************************

void crear_tripulante(void *argumentos){
	argumentos_creacion_tripulantes *args = argumentos;

	Tripulante *tripulante = (Tripulante*)malloc(sizeof(Tripulante));
	tripulante -> id = args->numero_tripulante;
	tripulante -> patota = numero_patota;
	tripulante -> posicionX = args->posicionX;
	tripulante -> posicionY = args->posicionY;
	tripulante -> estado = LLEGADA;
	printf("CREADA: id %d, patota %d, posx: %d", tripulante->id, tripulante->patota, tripulante->posicionX);

	list_add(llegada, tripulante);

	while(1)
		;
	//TODO: AVISAR A MI-RAM
	//TODO: AGREGR A COLA

}

/*
tripulantes_iniciados* crear_lista_tripulantes(char **resultado){
	tripulantes_iniciados *tripulantes = NULL;

	for(int i=2, j=0; resultado[i] != NULL; i++, j++){
		Tripulante *nuevo = (Tripulante*)malloc(sizeof(Tripulante));
		nuevo -> id = j;
		nuevo -> patota = numero_patota;
		nuevo -> estado = LLEGADA;

		if(resultado[i] != NULL){
			nuevo -> posicion -> x = atoi(*resultado[0]);
			nuevo -> posicion -> y = atoi(*resultado[2]);
		}
		else {
			nuevo->posicion->x = 0;
			nuevo->posicion->y = 0;
		}
		//TODO: IMPLEMENTAR LISTA
		//tripulantes -> tripulante = nuevo;
		//tripulantes -> proximo_tripulante = NULL;
	}

	return tripulantes;
}*/

void inicializar_recursos_necesarios(void){
	logs_discordiador = log_create("../logs_files/discordiador.log",
			"DISCORDIADOR", 1, LOG_LEVEL_INFO);
	log_info(logs_discordiador, "INICIANDO DISCORDIADOR..\n");

	log_info(logs_discordiador, "Generando configuraciones..\n");
	config = config_create(PATH_DISCORDIADOR_CONFIG);

	ip_mi_ram = config_get_string_value(config, "IP_MI_RAM_HQ");
	log_info(logs_discordiador, "IP RAM: %s\n", ip_mi_ram);

	puerto_mi_ram = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
	log_info(logs_discordiador, "PUERTO RAM: %s\n", puerto_mi_ram);

	ip_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	log_info(logs_discordiador, "IP STORE: %s\n", ip_mongo_store);

	puerto_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	log_info(logs_discordiador, "PUERTO STORE: %s\n", puerto_mongo_store);

	algoritmo_planificacion = config_get_string_value(config, "ALGORITMO");
	log_info(logs_discordiador, "ALGORITMO DE PLANIFICACION: %s\n", algoritmo_planificacion);

	if (strcmp(algoritmo_planificacion, "RR") == 0) {
			char *q = config_get_string_value(config, "QUANTUM");
			log_info(logs_discordiador, "QUANTUM UTILIZADO: %s\n", q);
			quantum = atoi(q);
		}

	char *grado_mt = config_get_string_value(config, "GRADO_MULTITAREA");
	log_info(logs_discordiador, "GRADO MULTITAREA PERMITIDO: %s\n", grado_mt);
	grado_multitarea = atoi(grado_mt);

	char *duracion = config_get_string_value(config, "DURACION_SABOTAJE");
	log_info(logs_discordiador, "DURACION DE LOS SABOTAJES: %s\n", duracion);
	duracion_sabotaje = atoi(duracion);

	char *retardo_cpu = config_get_string_value(config, "RETARDO_CICLO_CPU");
	log_info(logs_discordiador, "RETARDO DEL CICLO DE CPU: %s\n", retardo_cpu);
	retardo_ciclo_cpu = atoi(retardo_cpu);

	//falta avisar?
	llegada = list_create();


	log_info(logs_discordiador, "TERMINO INICIALIZACION DEL DISCORDIADOR..\n\n");
}

int main(void){

	inicializar_recursos_necesarios();

	pthread_t hilo_consola;

	pthread_create(&hilo_consola, NULL, (void *)atender_comandos_consola, NULL);

	pthread_detach(hilo_consola);

	while(1)
		;

	return EXIT_SUCCESS;
}


