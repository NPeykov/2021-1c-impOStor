#include "sabotaje_mongo.h"
#include "mongo-store.h"


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

bool es_blocks_superbloque() {
	bool resultado;
	char *contenido_sb;
	struct stat stat_superbloque;
	struct stat stat_blocks;
	int cant_bloques_sb;
	int cant_bloques_reales;

	int archivo_super_b = open(dirSuperbloque, O_RDWR);
	int archivo_blocks  = open(dirBlocks, O_RDWR);

	if (archivo_super_b == -1 || archivo_blocks == -1)
	{
		log_error(mongoLogger,"ERROR AL ABRIR EL SUPERBLOQUE O BLOCKS");
		exit(1);
	}

	fstat(archivo_super_b, &stat_superbloque);

	contenido_sb = (char*) mmap(NULL, stat_superbloque.st_size, PROT_WRITE, MAP_SHARED, archivo_super_b, 0);



	return resultado;
}

bool es_file_size(files file) {
	bool esta_saboteado=false;
	char* ruta;
	switch(file){
	case OXIGENO:ruta = rutaOxigeno;
		break;
	case COMIDA: ruta= rutaComida;
		break;
	case BASURA: ruta= rutaBasura;
		break;
	}
	if(ruta!=NULL){
		char* tamanio_del_archivo=size_de_archivo(ruta);
		char* bloques_del_archivo=bloques_de_archivo(ruta);
		char* texto_total=contenido_de_bloques(bloques_del_archivo);
		if(string_length(texto_total)!=atoi(tamanio_del_archivo)){
			esta_saboteado=true;
		}

	}
	else {
		printf("no existe el archivo");
	}
	return esta_saboteado;

}

bool es_file_block_count(files file){
	bool esta_saboteado=false;
	char* ruta;
	switch(file){
	case OXIGENO:ruta = rutaOxigeno;
		break;
	case COMIDA: ruta= rutaComida;
		break;
	case BASURA: ruta= rutaBasura;
		break;
	}
	if(ruta!=NULL && open(ruta,O_RDWR)!=-1){
		char* cant_bloques= cantidad_de_bloques_de_archivo(ruta);
		char* bloques= bloques_de_archivo(ruta);
		char** bloques_divididos = string_split(bloques,",");
		int i=0;
		int cantidad=0;
		while(bloques_divididos[i]){
			cantidad++;
			i++;
		}
		if(atoi(cant_bloques)!=cantidad){
			esta_saboteado=true;
		}
	}

	return esta_saboteado;
}


bool es_file_Blocks(files file){
	bool esta_saboteado=false;
	char* ruta;
	switch(file){
	case OXIGENO:ruta = rutaOxigeno;
		break;
	case COMIDA: ruta= rutaComida;
		break;
	case BASURA: ruta= rutaBasura;
		break;
	}
	if(ruta!=NULL && open(ruta,O_RDWR)!=-1){
		char* bloques= bloques_de_archivo(ruta);
		char** bloques_divididos = string_split(bloques,",");
		int tamanio_del_archivo=atoi(size_de_archivo(ruta));
		t_bloque* ultimo_bloque= recuperar_ultimo_bloque_file(ruta);
		char* ultimo_bloque_id=string_itoa(ultimo_bloque->id_bloque);
		char* texto_todos_los_bloques=string_new();
		char* md5Original=leer_md5file(ruta);

		int i = 0;

		while(bloques_divididos[i]!=ultimo_bloque_id){
			t_bloque* bloque_recuperado=malloc(sizeof(t_bloque));
			bloque_recuperado=list_get(disco_logico->bloques,atoi(bloques_divididos[i])-1);
			char* texto= leo_el_bloque_incluyendo_espacios(bloque_recuperado);
			string_append(&texto_todos_los_bloques,texto);
			i++;
			free(bloque_recuperado);
		}
		int lo_que_lei=string_length(texto_todos_los_bloques);
		int lo_que_falta=tamanio_del_archivo-lo_que_lei;
		int inicio=ultimo_bloque->inicio;
		while(inicio<ultimo_bloque->inicio+lo_que_falta){
			char *aux=string_from_format("%c", block_mmap[inicio]);
			string_append(&texto_todos_los_bloques,aux);
			inicio++;
		}

		char* nuevo_md5=generarMD5(texto_todos_los_bloques);
		if(nuevo_md5 != md5Original){
			esta_saboteado=true;
		}

	}


	return esta_saboteado;
}

sabotaje_code obtener_tipo_sabotaje() {
	sabotaje_code tipo_sabotaje;


	/*
    -CANT_BLOQUES
		- consiste en cambiar el vlaor de 'blocks' de superbloque
		- deteccion: constratar con el tamaño de blocks.ims (usando block_size de superbloque)
		- solucion: corregir el valor en caso que no concuerde

	-BITMAP
		- consiste en cambiar el valor binario de un bit del bitmap
		- deteccion: recorrer los files y bitacoras sacando los bloques usados y constatar con el bitmap
		- solucion: cambiar los valores del bitmap en base a los bloques detectados
	 * */

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
