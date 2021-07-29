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


void levantar_blocks(void) {
	int fd;
	struct stat info;

	fd = open(dirBlocks, O_RDWR);

	if (fd == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR BLOCKS.IMS");
		exit(1);
	}

	fstat(fd, &info);

	s_blocks = (char*) mmap(NULL, info.st_size, PROT_WRITE, MAP_SHARED, fd, 0);

	s_tamanio_blocks = info.st_size;

	return;
}

void levantar_superbloque(void) {
	int fd;
	struct stat info;

	fd = open(dirSuperbloque, O_RDWR);

	if (fd == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR SUPERBLOQUE");
		exit(1);
	}

	fstat(fd, &info);

	s_superbloque = (char*) mmap(NULL, info.st_size, PROT_WRITE, MAP_SHARED, fd, 0);

	s_tamanio_superbloque = info.st_size;

	return;
}


bool es_blocks_superbloque() {
	bool resultado;
	int cantidad_block_size_sb = *(s_superbloque);
	int cantidad_bloques_sb = *(s_superbloque + sizeof(uint32_t));
	int cantidad_bloques_teorico = s_tamanio_blocks / cantidad_block_size_sb;

	return cantidad_bloques_teorico != cantidad_bloques_sb;
}

bool es_bitmap_superbloque() {
	bool resultado;
	int copia_offset;
	t_bitarray *bitarray_sb;
	void *bitmap_sb;

	bitmap_sb = s_superbloque + 2*sizeof(uint32_t);
	int bytes = (int) ceil((double) *g_blocks / 8);
	bitarray_sb = bitarray_create(bitmap_sb, bytes);

	bool n_bloque_ocupado;

	for(int i=1, offset=0; i <= *g_blocks; i++, offset+=*g_block_size)
	{
		copia_offset = offset;
		n_bloque_ocupado = false;

		while (copia_offset <= copia_offset + *g_block_size) {
			if (s_blocks[copia_offset] != ' ') {
				n_bloque_ocupado = true;
			}
			copia_offset++;
		}

		if(bitarray_test_bit(bitarray_sb, i) != n_bloque_ocupado) {
			resultado = true;
			return resultado;
		}
	}

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

///////////////////

sabotaje_code obtener_tipo_sabotaje() {
	sabotaje_code tipo_sabotaje;

	levantar_superbloque();
	levantar_blocks();





	munmap(s_blocks, s_tamanio_blocks);
	munmap(s_superbloque, s_tamanio_superbloque);

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
