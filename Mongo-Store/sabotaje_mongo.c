#include "sabotaje_mongo.h"


void rutina(int n) {

	sabotaje_code codigo_sabotaje = obtener_tipo_sabotaje();

	if(codigo_sabotaje != NO_HAY_SABOTAJE)
	{
		log_info(mongoLogger, "SE DETECTO UN SABOTAJEEEE");




		/*TODO: chequear si el semaforo de abajo no hace nada raro
		 * porque si mandan señal sin haber sabotaje van a quedar muchos
		 * hilos esperando por esa señal
		 *
		 * */
		//sem_post(&dar_orden_sabotaje);
		enviar_aviso_sabotaje_a_discordiador();
	}

	else log_info(mongoLogger, "TIRARON LA SEÑAL DE SABOTAJE PERO NO HAY INCONSISTENCIAS");


}

sabotaje_code obtener_tipo_sabotaje() {
	sabotaje_code tipo_sabotaje;



	return tipo_sabotaje;
}


void gestionarSabotaje() {
	int operacion;
	switch (operacion) {
	case 1: //SUPERBLOQUE
		//si cambia valor Blocks constatar con tamaño archivo blocks.ims
		//si cambia valor Bitmap recorrer FILES y obtener bloques usados
		break;
	case 2: //FILES
		//si cambia el SIZE recorrer todos los bloques y asumir correcto el tamaño de los mismos
		//si son inconsistentes block_count y blocks actualizo block_count en base a la lista de blocks
		//si se altera la lista BLOCKS y no estan en orden(nos damos cuenta porque cambia el valor de md5)
		//se debe reescribir la lista hasta que se obtenga el mismo tamaño
	default:
		printf("Operacion desconocida.\n");
		break;

	}
}



//devuelve la siguiente posicion del sabotaje
char* siguiente_posicion_sabotaje() {
	char** posiciones_divididas;
	char * siguiente_posicion;
	int cantidad = 0;

	posiciones_divididas = config_get_array_value(mongoConfig,
			"POSICIONES_SABOTAJE");		//array con todas las posiciones
	while (posiciones_divididas[cantidad] != NULL) {
		cantidad++;
	}
	if (numero_sabotaje < cantidad) {
		siguiente_posicion = posiciones_divididas[numero_sabotaje];
		sem_wait(&contador_sabotaje);
		numero_sabotaje++;
		sem_post(&contador_sabotaje);
		return siguiente_posicion;
	} else {
		printf("NO HAY MAS SABOTAJES\n");
		return NULL;
	}

}

//para probar el aviso de inicio de sabotaje
void enviar_aviso_sabotaje_a_discordiador(void) {
	char** sabotaje_pos_aux;
	char* sabotaje_posY;
	//char** pos_dividida;
	char* sabotaje_posX;
	char* pos_sabotaje;
	//int socket_para_sabotaje = esperar_cliente(socket_mongo_store);

	//sem_wait(&dar_orden_sabotaje);

	pos_sabotaje = siguiente_posicion_sabotaje();

	if (pos_sabotaje == NULL) //no hay mas sabotajes
		return;

	sabotaje_pos_aux = string_split(pos_sabotaje, "|");	//tomo la posicion i del array y lo paso a otro
	printf("ESTOY POR ENVIAR SABOTAJE\n");
	sabotaje_posX = sabotaje_pos_aux[0];		//agarrar la posicion x
	sabotaje_posY = sabotaje_pos_aux[1];		//agarrar la posicion y
	printf("posicion en X de sabotaje: %s\n", sabotaje_posX);
	printf("posicion en Y de sabotaje: %s\n", sabotaje_posY);

	t_paquete* paquete_sabotaje = crear_paquete(INICIO_SABOTAJE);
	agregar_a_paquete(paquete_sabotaje, sabotaje_posX,
			strlen(sabotaje_posX) + 1);
	agregar_a_paquete(paquete_sabotaje, sabotaje_posY,
			strlen(sabotaje_posY) + 1);
	enviar_paquete(paquete_sabotaje, socket_sabotaje_cliente);
	eliminar_paquete(paquete_sabotaje);

//	liberar_cliente(socket_sabotaje_cliente);

	return;
	//pthread_exit(NULL);
}
