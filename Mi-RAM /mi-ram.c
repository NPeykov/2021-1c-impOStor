#include "mi-ram.h"

Segmento* crear_segmento(char *tareasNuevas[], tipo_segmento tipo, int tamanio, t_list* tabla_segmentos){
	Segmento *segmento;

	switch(tipo){
	case(tipo == PCB):
		segmento=crear_segmento_pcb(numero_patota, tamanio, tabla_segmentos);
		numero_patota += 1;
		break;
	case(tipo == TCB):
		segmento=crear_segmento_tcb(tamanio, tabla_segmentos);
		break;
	case(tipo == TAREAS):
		segmento=crear_segmento_tareas(tamanio, tabla_segmentos);
		break;
	default:
		printf("Este es un tipo de segmento no válido");
	}

	return segmento;
}

Segmento* crear_segmento_pcb(int numero_patota,int tamanio, tabla_segmentos){
	PatotaCB pcb = (PatotaCB*) malloc(sizeof(PatotaCB));
	pcb->pid = numero_patota;
	sem_wait(&tareas_creadas);
	pcb->tareas = get_segmento_tareas_patota(numero_patota);
	sem_post(&tareas_creadas);
}

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
				case INICIAR_PATOTA:
					lista = recibir_paquete(cliente);
					t_list* tabla_segmentos = list_create(); //Tabla de Patota
					break;
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

void inicializar_ram(){
	printf("################# Modulo Mi-RAM #################\n");
	//logger = log_create(archivoDeLog, "CoMAnda", 1, LOG_LEVEL_DEBUG);

	socket_mi_ram = levantar_servidor(MI_RAM_HQ);

	config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	printf("MI_RAM escuchando en PUERTO:%s \n", puerto);
	char* tipoMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	switch(atoi(tipoMemoria)){
		case 0:
			printf("Se intentó paginación");
			//TODO: PAGINACION
			break;
		case 1:
			printf("Se intento segmentación");

			break;
		default:
			printf("Error, esquema de memoria desconocido.\n");
			break;
	}
	gestionarCliente(socket_mi_ram);
	//memoriaPrincipal = malloc(tamanioMemoria);
	//memoriaSwap = malloc(tamanioSwap);

}



int main(){
  inicializar_ram();

  sem_init(&tareas_creadas, 0, 0);

  return EXIT_SUCCESS;
}
