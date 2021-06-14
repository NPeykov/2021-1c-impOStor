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

		switch (valor) {
		case 0: //INICIAR_PATOTA
			;
			int cantidad_tripulantes = atoi(comando_separado[1]);
			char *lista_tareas = strdup(comando_separado[2]); //LIBERAR ESPACIO

			log_info(logs_discordiador, "Iniciando %d tripulantes de patota numero %d..\n",cantidad_tripulantes, g_numero_patota);

			//AVISAR A MI RAM QUE SE CREO LA PATOTA

			iniciar_patota(comando_separado); //CAPAZ SE PODRIA SACAR NUMERO PATOTA SI SINCRONIZO BIEN LO OTRO

			g_numero_patota += 1; //PARA LA PROX VEZ QUE SEA INICIALIZADO
			break;

		case 1: //LISTAR_TRIPULANTE
			printf("--------LISTAR TRIPULANTES---------\n");
			t_link_element *elementos;
			t_list *copia_llegada = (t_list* )malloc(sizeof(t_list));
			copia_llegada -> head = llegada -> head;
			if(copia_llegada -> head == NULL){
				printf("No hay tripulantes en la cola de llegada!\n");
				break;
			}
			else{
				Tripulante *tripulante = (Tripulante *) malloc(sizeof(Tripulante));
				while(copia_llegada -> head != NULL){
					elementos = copia_llegada -> head;
					tripulante = (Tripulante *) elementos -> data;
					printf("Patota N°: %d\n", tripulante->patota);
					printf("Tripulante ID°: %d\n\n", tripulante->id);
					copia_llegada->head = copia_llegada->head->next;
				}
			}
			free(copia_llegada);

			break;

		case 2: //EXPULSAR_TRIPULANTE
			printf("ES EXPULSAR\n");
			break;

		case 3: //INICIAR_PLANIFICACION
			break;

		case 4: //PAUSAR_PLANIFICACION
			break;

		case 5: //OBTENER_BITACORA
			break;

		case 6: //SALIR
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


void iniciar_patota(char **datos_tripulantes) {
	int cantidad_tripulantes = atoi(datos_tripulantes[1]);
	char *posiciones[cantidad_tripulantes];
	int retorno_thread;
	pthread_t tripulantes[cantidad_tripulantes - 1];
	char **posicion_del_tripulante;
	argumentos_creacion_tripulantes *args = (argumentos_creacion_tripulantes*)malloc(sizeof(argumentos_creacion_tripulantes));

	for (int i = 0, j = 3; i < cantidad_tripulantes; i++, j++) {
		if (datos_tripulantes[j] != NULL)
			posiciones[i] = datos_tripulantes[j];
		else {
			while (i < cantidad_tripulantes) {
				posiciones[i] = "0|0";
				i++;
			}
			posiciones[i] = NULL;
		}
	}

	for (int i = 0; i < cantidad_tripulantes; i++) {
		pthread_mutex_lock(&lockear_creacion_tripulante);
		posicion_del_tripulante = string_split(posiciones[i], "|");
		args -> numero_tripulante = i + 1;
		args -> posicionX = atoi(posicion_del_tripulante[0]);
		args -> posicionY = atoi(posicion_del_tripulante[1]);
		args -> patota_actual = g_numero_patota;
		pthread_create(&tripulantes[i], NULL, (void *)tripulante, (void *)args);
	}

}

void tripulante(void *argumentos){

	argumentos_creacion_tripulantes *args = argumentos;
	Tripulante *tripulante = (Tripulante*)malloc(sizeof(Tripulante));
	tripulante -> id = args->numero_tripulante;
	tripulante -> patota = args->patota_actual;
	tripulante -> posicionX = args->posicionX;
	tripulante -> posicionY = args->posicionY;
	tripulante -> estado = LLEGADA;

	list_add(llegada, tripulante);
	pthread_mutex_unlock(&lockear_creacion_tripulante);

	while(1)
		;
	//TODO: AVISAR A MI-RAM
	//TODO: AGREGR A COLA

}


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


