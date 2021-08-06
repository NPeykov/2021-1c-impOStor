#include "utils_disc.h"

void imprimir_respuesta_log(t_list* respuesta){
	void iterator(char* value)
		{
		log_info(logs_discordiador,"valor: %s",value);
		}
	list_iterate(respuesta, (void*) iterator);

}

void listar_discordiador() {
	listar_cola_planificacion(LLEGADA);
	listar_cola_planificacion(LISTO);
	listar_cola_planificacion(TRABAJANDO);
	listar_cola_planificacion(BLOQUEADO_IO);
	listar_cola_planificacion(BLOQUEADO_EMERGENCIA);
	listar_cola_planificacion(FINALIZADO);

}


void listar_cola_planificacion(Estado estado) {
	t_link_element *elementos;
	t_list *copia_lista = (t_list*) malloc(sizeof(t_list));
	char *nombre_estado;
	char *formato_tripulante;

	//LLEGADA, LISTO, TRABAJANDO, BLOQUEADO, FINALIZADO
	switch(estado){
	case LLEGADA:
		copia_lista->head = lista_llegada-> elements -> head;
		nombre_estado = "Llegada";
		break;
	case LISTO:
		copia_lista->head = lista_listo-> elements -> head;
		nombre_estado = "Listo";
		break;
	case TRABAJANDO:
		copia_lista->head = lista_trabajando -> head;
		nombre_estado = "Trabajando";
		break;
	case BLOQUEADO_IO:
		copia_lista->head = lista_bloqueado_IO -> head;
		nombre_estado = "Bloqueados IO";
		break;
	case BLOQUEADO_EMERGENCIA:
		copia_lista->head = lista_bloqueado_EM -> head;
		nombre_estado = "Bloqueados Emergencia";
		break;
	case FINALIZADO:
		copia_lista->head = lista_finalizado -> head;
		nombre_estado = "Finalizado";
		break;
	}
	if (copia_lista->head == NULL) {
		log_info(logs_discordiador, "No hay tripulantes en la cola de %s!",nombre_estado);
	} else {
		Tripulante_Planificando *tripulante_planificando = (Tripulante_Planificando *) malloc(sizeof(Tripulante_Planificando));
		while (copia_lista->head != NULL) {
			elementos = copia_lista->head;
			tripulante_planificando = (Tripulante_Planificando *) elementos->data;

/*		OTRO FORMA DE IMPRIMIR
			formato_tripulante = string_from_format("Patota N°: %d\tTripulante ID: %d\tPosX: %d, PosY: %d\t Estado: %s\n",
					tripulante_planificando->tripulante->patota, tripulante_planificando->tripulante->id,
					tripulante_planificando->tripulante->posicionX, tripulante_planificando->tripulante->posicionY,
					nombre_estado);

			log_info(logs_discordiador, formato_tripulante);


			printf("Patota N°: %d\t", tripulante_planificando->tripulante->patota);
			printf("Tripulante ID°: %d\t", tripulante_planificando->tripulante->id);
			printf("PosX: %d, PosY: %d\t", tripulante_planificando->tripulante->posicionX, tripulante_planificando->tripulante->posicionY);
			printf("Estado: %s\n", nombre_estado);
*/
			pthread_mutex_lock(&mutex_log);
			log_info(logs_discordiador,"Patota N°: %d   Tripulante ID°: %d   PosX: %d, PosY: %d   Estado: %s",
					tripulante_planificando->tripulante->patota,tripulante_planificando->tripulante->id,
					tripulante_planificando->tripulante->posicionX,
					tripulante_planificando->tripulante->posicionY,nombre_estado);
			pthread_mutex_unlock(&mutex_log);

			copia_lista->head = copia_lista->head->next;
		}
	}

	free(copia_lista);
}

void reanudar_hilos_lista(Estado estado){

	void reanudar(void *data){
		Tripulante_Planificando *tripulante = (Tripulante_Planificando*) data;
		sem_post(&tripulante->salir_pausa);
	}

	switch (estado) {
	case LLEGADA:
		list_iterate(lista_llegada->elements, reanudar);
		break;
	case LISTO:
		list_iterate(lista_listo->elements, reanudar);
		break;
	case TRABAJANDO:
		list_iterate(lista_trabajando, reanudar);
		break;
	case BLOQUEADO_IO:
		list_iterate(lista_bloqueado_IO, reanudar);
		break;
	case BLOQUEADO_EMERGENCIA:
		list_iterate(lista_bloqueado_EM, reanudar);
		break;
	case FINALIZADO:
		list_iterate(lista_finalizado, reanudar);
		break;
	}

}

void fijarse_si_hay_pausa_hilo(Tripulante_Planificando *tripulante){
	if(g_hay_pausa) {
		sem_wait(&tripulante->salir_pausa);
	}
}

void fijarse_si_hay_pausa_planificador(){
	if(g_hay_pausa) {
		sem_wait(&otros_inicios);
		reanudar_hilos_lista(LISTO);
		reanudar_hilos_lista(TRABAJANDO);
		reanudar_hilos_lista(BLOQUEADO_IO);
		reanudar_hilos_lista(BLOQUEADO_EMERGENCIA);
		//avisar a todos los hilos con un signal
	}
}
