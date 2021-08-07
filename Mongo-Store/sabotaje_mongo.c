#include "sabotaje_mongo.h"
#include "mongo-store.h"

void gestionar_sabotaje() {
	sem_wait(&semaforo_modificacion_de_datos);
	sabotaje_code codigo_sabotaje = obtener_tipo_sabotaje();

	if (codigo_sabotaje == NO_HAY_SABOTAJE) {
		log_info(mongoLogger,
				"TIRARON LA SEÑAL DE SABOTAJE PERO NO HAY INCONSISTENCIAS");
		sem_post(&semaforo_modificacion_de_datos);
		return;
	}

	log_warning(mongoLogger, "SE DETECTO UN SABOTAJE--CODIGO: %d--",
			codigo_sabotaje);
	sem_post(&dar_orden_sabotaje);

	sem_wait(&inicio_fsck); //espera q discordiador termine el sabotaje
	if(sabotaje_exito){
		iniciar_recuperacion(codigo_sabotaje);
	}else {
		log_warning(mongoLogger,"no se pudo resolver el sabotaje porque no hay tripulantes disponibles");
	}
	munmap(s_blocks, s_tamanio_blocks);
	munmap(s_superbloque, s_tamanio_superbloque);
	sem_post(&semaforo_modificacion_de_datos);
}

void rutina(int n) {

	pthread_t rutina;
	pthread_create(&rutina, NULL, (void *) gestionar_sabotaje, NULL);
	pthread_detach(rutina);
}

int levantar_blocks(void) {
	int fd;
	struct stat info;
	char* dirBlocks = conseguir_ruta(BLOCKS);
	fd = open(dirBlocks, O_RDWR);

	if (fd == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR BLOCKS.IMS");
		return -1;
	}

	fstat(fd, &info);

	s_blocks = (char*) mmap(NULL, info.st_size, PROT_WRITE, MAP_SHARED, fd, 0);

	s_tamanio_blocks = info.st_size;
	free(dirBlocks);
	return 1;
}

int levantar_superbloque(void) {
	int fd;
	struct stat info;
	char* dirSuperbloque = conseguir_ruta(SUPERBLOQUE);
	fd = open(dirSuperbloque, O_RDWR);

	if (fd == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR SUPERBLOQUE");
		return -1;
	}

	fstat(fd, &info);

	s_superbloque = (char*) mmap(NULL, info.st_size, PROT_WRITE, MAP_SHARED, fd,
			0);

	s_size_sb = superbloque;
	s_blocks_sb = superbloque + sizeof(uint32_t);

	s_tamanio_superbloque = info.st_size;
	free(dirSuperbloque);
	return 1;
}

bool es_blocks_superbloque() {
	bool resultado;

	//printf("SIZE BLOCKS: %d\n", s_tamanio_blocks);

	//printf("-----------SIZE: %d\nCANT SABOTEADA: %d\n", *(s_size_sb),
	//*(s_blocks_sb));

	int cantidad_bloques_teorico = s_tamanio_blocks / *(s_size_sb);

	//printf("CANTIDAD QUE DEBERIA SER: %d\n", cantidad_bloques_teorico);

	resultado = cantidad_bloques_teorico != *(s_blocks_sb);

	if (resultado == true)
		log_info(mongoLogger, "El sabotaje fue en blocks de superbloque");

	return resultado;
}

bool es_bitmap_superbloque() {

	bool resultado = false;
	int copia_offset;
	void *bitmap_sb;

	bitmap_sb = s_superbloque + 2 * sizeof(uint32_t);

	int bytes = (int) ceil((double) *(s_blocks_sb) / 8);

	bitarray_sb = bitarray_create(bitmap_sb, bytes);

	bool n_bloque_ocupado;

	for (int i = 0, offset = 0; i < *g_blocks; i++, offset += *g_block_size) {
		copia_offset = offset;

		n_bloque_ocupado = false;

		while (copia_offset <= offset + *g_block_size - 1) {
			if (s_blocks[copia_offset] != ' ') {
				n_bloque_ocupado = true;
			}
			copia_offset++;
		}
		if (bitarray_test_bit(bitarray_sb, i) != n_bloque_ocupado) {
			resultado = true;
			log_info(mongoLogger, "El sabotaje fue en bitmap de superbloque");
			return resultado;
		}
	}

	return resultado;
}

bool es_file_size(files file) {
	bool esta_saboteado = false;
	char* ruta;
	switch (file) {
	case OXIGENO:
		ruta = conseguir_ruta(OXIGENOO);
		break;
	case COMIDA:
		ruta = conseguir_ruta(COMIDAA);
		break;
	case BASURA:
		ruta = conseguir_ruta(BASURAA);
		break;
	}
	int archivo = open(ruta, O_RDWR);
	if (archivo != -1) {
		struct stat info;
		fstat(archivo, &info);
		archivo_saboteado = (char*) mmap(NULL, info.st_size, PROT_WRITE,
		MAP_SHARED, archivo, 0);
		char* bloques_del_archivo = bloques_de_archivo(archivo_saboteado);

		char* tamanio_del_archivo = size_de_archivo(archivo_saboteado);
		if (atoi(tamanio_del_archivo) == 0 && string_is_empty(bloques_del_archivo)) {
			return false;
		}
		char* texto_total = contenido_de_bloques_fisico(bloques_del_archivo);

		if (string_length(texto_total) != atoi(tamanio_del_archivo)) {
			esta_saboteado = true;
			log_info(mongoLogger, "El sabotaje fue en size del file: %d", file);
			close(archivo);
		} else {
			munmap(archivo_saboteado, info.st_size);
			close(archivo);
		}
	}
	free(ruta);
	return esta_saboteado;
}

bool es_file_block_count(files file) {

	bool esta_saboteado = false;
	char* ruta;
	int cantidad = 0;
	switch (file) {
	case OXIGENO:
		ruta = conseguir_ruta(OXIGENOO);
		break;
	case COMIDA:
		ruta = conseguir_ruta(COMIDAA);
		break;
	case BASURA:
		ruta = conseguir_ruta(BASURAA);
		break;
	}
	int archivo = open(ruta, O_RDWR);

	if (archivo != -1) {
		struct stat info;
		fstat(archivo, &info);
		archivo_saboteado = (char*) mmap(NULL, info.st_size, PROT_WRITE,
		MAP_SHARED, archivo, 0);

		char* cant_bloques = cantidad_de_bloques_de_archivo(archivo_saboteado);
		char* bloques = bloques_de_archivo(archivo_saboteado);
		if (string_is_empty(bloques)){
			return false;
		}
		if(string_is_empty(cant_bloques)){
			cant_bloques="0";
		}
		char* bloquesaux = string_substring_until(bloques,
				string_length(bloques) - 1);
		char** bloques_divididos = string_split(bloquesaux, ",");
		int i = 0;
		while (bloques_divididos[i]) {
			cantidad++;
			i++;
		}
		if (atoi(cant_bloques) != cantidad) {
			esta_saboteado = true;
			cantidad_bloques_file = cantidad;
			log_info(mongoLogger, "El sabotaje fue en block_count de file");
			close(archivo);
		} else {
			munmap(archivo_saboteado, info.st_size);
			close(archivo);
		}
	}
	free(ruta);
	return esta_saboteado;
}

bool es_file_Blocks(files file) {

	bool esta_saboteado = false;
	char* ruta;
	switch (file) {
	case OXIGENO:
		ruta = conseguir_ruta(OXIGENOO);
		break;
	case COMIDA:
		ruta = conseguir_ruta(COMIDAA);
		break;
	case BASURA:
		ruta = conseguir_ruta(BASURAA);
		break;
	}
	int archivo = open(ruta, O_RDWR);
	if (archivo != -1) {
		struct stat info;
		fstat(archivo, &info);
		archivo_saboteado = (char*) mmap(NULL, info.st_size, PROT_WRITE,
		MAP_SHARED, archivo, 0);
		char* bloques = bloques_de_archivo(archivo_saboteado);
		if(string_is_empty(bloques)){
			return false;
		}
		char** bloques_divididos = string_split(bloques, ",");
		int tamanio_del_archivo = atoi(size_de_archivo(archivo_saboteado));
		t_bloque* ultimo_bloque = recuperar_ultimo_bloque(archivo_saboteado);
		char* ultimo_bloque_id = string_itoa(ultimo_bloque->id_bloque);
		char* texto_todos_los_bloques = string_new();
		char* md5Original = leer_md5file(archivo_saboteado);
		int i = 0;

		while (!string_equals_ignore_case(bloques_divididos[i],
				ultimo_bloque_id)) {
			t_bloque* bloque_recuperado;
			bloque_recuperado = list_get(disco_logico->bloques,
					atoi(bloques_divididos[i]) - 1);
			char* texto = leo_el_bloque_incluyendo_espacios(bloque_recuperado);
			string_append(&texto_todos_los_bloques, texto);
			i++;
			free(texto);
		}
		int lo_que_lei = string_length(texto_todos_los_bloques);

		int lo_que_falta = tamanio_del_archivo - lo_que_lei;
		int inicio = ultimo_bloque->inicio;
		char* dirBlocks = conseguir_ruta(BLOCKS);
		int archivo_blocks = open(dirBlocks, O_RDWR);
		struct stat infoblock;
		fstat(archivo_blocks, &infoblock);
		archivo_blocks_para_sabotaje = (char*) mmap(NULL, infoblock.st_size,
		PROT_WRITE, MAP_SHARED, archivo_blocks, 0);

		while (inicio < ultimo_bloque->inicio + lo_que_falta) {
			char *aux = string_from_format("%c",
					archivo_blocks_para_sabotaje[inicio]);
			string_append(&texto_todos_los_bloques, aux);
			inicio++;
		}
		munmap(archivo_blocks_para_sabotaje, infoblock.st_size);
		close(archivo_blocks);
		char* nuevo_md5 = generarMD5(texto_todos_los_bloques);
		char* nuevo_md5_sin_salto = string_substring_until(nuevo_md5,
				string_length(nuevo_md5) - 1);
		if (!string_equals_ignore_case(nuevo_md5_sin_salto, md5Original)) {
			esta_saboteado = true;
			log_info(mongoLogger, "El sabotaje fue en blocks de file");
			close(archivo);
		} else {
			munmap(archivo_saboteado, info.st_size);
			close(archivo);
		}
	}

	free(ruta);
	return esta_saboteado;
}

sabotaje_code obtener_tipo_sabotaje() {
	sabotaje_code tipo_sabotaje = NO_HAY_SABOTAJE;
	fue_en_oxigeno = false;
	fue_en_comida = false;
	fue_en_basura = false;

	int retornoSB = levantar_superbloque();
	int retornoBS = levantar_blocks();

	if (retornoSB != -1 && retornoBS != -1)
		if (es_blocks_superbloque()){
			log_warning(mongoLogger,"se detecto sabotaje en Superbloque");
			return SB_BLOCKS;
		}


	if (retornoSB != -1 && retornoBS != -1)
		if (es_bitmap_superbloque()){
			log_warning(mongoLogger,"se detecto sabotaje en bitmap");
			return SB_BITMAP;
		}


	if (es_file_size(OXIGENO)) {
		tipo_sabotaje = FILES_SIZE;
		fue_en_oxigeno = true;
		log_warning(mongoLogger,"se detecto sabotaje en oxigeno por size");
		return tipo_sabotaje;

	}

	if (es_file_size(COMIDA)) {
		tipo_sabotaje = FILES_SIZE;
		fue_en_comida = true;
		log_warning(mongoLogger,"se detecto sabotaje en comida por size");
		return tipo_sabotaje;

	}

	if (es_file_size(BASURA)) {
		tipo_sabotaje = FILES_SIZE;
		fue_en_basura = true;
		log_warning(mongoLogger,"se detecto sabotaje en basura por size");
		return tipo_sabotaje;

	}

	if (es_file_block_count(OXIGENO)) {
		tipo_sabotaje = FILES_BLOCK_COUNT;
		fue_en_oxigeno = true;
		log_warning(mongoLogger,"se detecto sabotaje en oxigeno por cantidad de bloques");
		return tipo_sabotaje;

	}

	if (es_file_block_count(COMIDA)) {
		tipo_sabotaje = FILES_BLOCK_COUNT;
		fue_en_comida = true;
		log_warning(mongoLogger,"se detecto sabotaje en comida por cantidad de bloques");
		return tipo_sabotaje;

	}

	if (es_file_block_count(BASURA)) {
		tipo_sabotaje = FILES_BLOCK_COUNT;
		fue_en_basura = true;
		log_warning(mongoLogger,"se detecto sabotaje en basura por cantidad de bloques");
		return tipo_sabotaje;

	}

	if (es_file_Blocks(OXIGENO)) {
		tipo_sabotaje = FILES_MD5;
		fue_en_oxigeno = true;
		log_warning(mongoLogger,"se detecto sabotaje en oxigeno por MD5");
		return tipo_sabotaje;

	}

	if (es_file_Blocks(COMIDA)) {

		tipo_sabotaje = FILES_MD5;
		fue_en_comida = true;
		log_warning(mongoLogger,"se detecto sabotaje en comida por MD5");
		return tipo_sabotaje;

	}

	if (es_file_Blocks(BASURA)) {
		tipo_sabotaje = FILES_MD5;
		fue_en_basura = true;
		log_warning(mongoLogger,"se detecto sabotaje en basura por MD5");
		return tipo_sabotaje;

	}
	printf("------------------------no hay sabotaje------------------------\n");
	return tipo_sabotaje;
}

//--------------RECUPERACION

void actualizar_valor_blocks_sb() {
	uint32_t cantidad_bloques_teorico = (unsigned) s_tamanio_blocks
			/ *(s_size_sb);

	memcpy(s_superbloque + sizeof(uint32_t), &cantidad_bloques_teorico,
			sizeof(uint32_t));
}

bool esta_ocupado(int inicio, int fin) {

	while (inicio <= fin) {
		if (s_blocks[inicio] != ' ') {
			return true;
		}
		inicio++;
	}
	return false;
}

void setear_valores_a_bitmap() {

	for (int i = 0, offset = 0; i < *g_blocks; i++, offset += *g_block_size) {

		if (esta_ocupado(offset, offset + *g_block_size - 1)) {
			bitarray_set_bit(bitarray_sb, i);
		}

		else {
			bitarray_clean_bit(bitarray_sb, i);
		}

	}

}

void arreglar_valor_size(files file) {
	int tamanio_real;
	char *ruta;
	char caracter_llenado;

	switch (file) {
	case COMIDA:
		ruta = conseguir_ruta(COMIDAA);
		caracter_llenado = 'C';
		break;

	case BASURA:
		ruta = conseguir_ruta(BASURAA);
		caracter_llenado = 'B';
		break;

	case OXIGENO:
		ruta = conseguir_ruta(OXIGENOO);
		;
		caracter_llenado = 'O';
		break;
	}
	char* bloques_del_archivo = bloques_de_archivo(archivo_saboteado);
	char* bloquesaux = string_substring_until(bloques_del_archivo,
			string_length(bloques_del_archivo) - 1);
	char* texto_total = contenido_de_bloques_fisico(bloques_del_archivo);
	tamanio_real = string_length(texto_total);
	char *cantidad_bloques = cantidad_de_bloques_de_archivo(archivo_saboteado);

	char *md5 = leer_md5file(archivo_saboteado);

	char *nuevo_contenido =
			string_from_format(
					"SIZE=%d\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%c\nMD5_ARCHIVO=%s",
					tamanio_real, cantidad_bloques, bloques_del_archivo,
					caracter_llenado, md5);

	int archivo = open(ruta, O_RDWR);
	ftruncate(archivo, (off_t) string_length(nuevo_contenido));
	close(archivo);
	free(md5);
	int i = 0;
	while (nuevo_contenido[i]) {
		archivo_saboteado[i] = nuevo_contenido[i];
		i++;
	}
	munmap(archivo_saboteado, string_length(nuevo_contenido));
	free(ruta);
}

t_bloque* bloque_con_espacio(char* bloques) {
	char **bloques_divididos = string_split(bloques, ",");
	int i = 0;
	bool flag_corte = true;
	t_bloque *el_bloque_con_espacio;
	t_bloque *bloque;
	while (flag_corte) {
		bloque = list_get(disco_logico->bloques,
				atoi(bloques_divididos[i]) - 1);

		char* texto = leo_el_bloque_fisico(bloque);
		if (bloque->inicio + string_length(texto) - 1 < bloque->fin) {
			el_bloque_con_espacio = bloque;
			flag_corte = false;
		}
		i++;
	}
	return el_bloque_con_espacio;
}

void reparar_MD5(char* file, char caracter) {
	char* dirBlocks = conseguir_ruta(BLOCKS);
	int archivo = open(dirBlocks, O_RDWR);
	struct stat info;
	fstat(archivo, &info);
	archivo_blocks_para_sabotaje = (char*) mmap(NULL, info.st_size, PROT_WRITE,
	MAP_SHARED, archivo, 0);

	char* bloques = bloques_de_archivo(archivo_saboteado);
	t_bloque* bloque;
	int caracteres_agregados = 0;
	bloque = bloque_con_espacio(bloques);
	char* texto = leo_el_bloque_fisico(bloque);
	//bloque->posicion_para_escribir = string_length(texto) + 1;

	//bloque->espacio = bloque->fin - bloque->inicio + 1 - string_length(texto);
	char* caracter_aux = string_from_format("%c",caracter);
	while (bloque->espacio > 0) {
		escribir_en_block(caracter_aux,bloque);
		/*archivo_blocks_para_sabotaje[bloque->posicion_para_escribir
				+ bloque->inicio - 1] = caracter;
		bloque->espacio--;
		bloque->posicion_para_escribir++;*/
		caracteres_agregados++;
	}

	bloque = recuperar_ultimo_bloque(archivo_saboteado);

	borrar_en_block(caracteres_agregados,bloque);

	/*while (caracteres_agregados > 0) {
		bloque->posicion_para_escribir--;
		archivo_blocks_para_sabotaje[bloque->posicion_para_escribir] = ' ';
		bloque->espacio++;
		caracteres_agregados--;
	}*/
	munmap(archivo_blocks_para_sabotaje, info.st_size);
	close(archivo);
	free(dirBlocks);
}

void arreglar_valor_block_count(files file) {
	char *ruta;
	char caracter_llenado;

	switch (file) {
	case COMIDA:
		ruta = conseguir_ruta(COMIDAA);
		caracter_llenado = 'C';
		break;

	case BASURA:
		ruta = conseguir_ruta(BASURAA);
		caracter_llenado = 'B';
		break;

	case OXIGENO:
		ruta = conseguir_ruta(OXIGENOO);
		caracter_llenado = 'O';
		break;
	}
	char *tamanio = size_de_archivo(archivo_saboteado);

	char* bloques_del_archivo = bloques_de_archivo(archivo_saboteado);
	char *md5 = leer_md5file(archivo_saboteado);

	char *nuevo_contenido =
			string_from_format(
					"SIZE=%s\nBLOCK_COUNT=%d\nBLOCKS=%s\nCARACTER_LLENADO=%c\nMD5_ARCHIVO=%s",
					tamanio, cantidad_bloques_file, bloques_del_archivo,
					caracter_llenado, md5);
	int archivo = open(ruta, O_RDWR);
	ftruncate(archivo, (off_t) string_length(nuevo_contenido));
	close(archivo);
	int i = 0;
	while (nuevo_contenido[i]) {
		archivo_saboteado[i] = nuevo_contenido[i];
		i++;
	}
	munmap(archivo_saboteado, string_length(nuevo_contenido));
	free(ruta);
}

void iniciar_recuperacion(sabotaje_code sabotaje_cod) {

	switch (sabotaje_cod) {

	case SB_BLOCKS:
		actualizar_valor_blocks_sb();
		log_warning(mongoLogger,"se arreglo el sabotaje en superbloque");
		return;
		break;

	case SB_BITMAP:
		setear_valores_a_bitmap();
		log_warning(mongoLogger,"se arreglo el sabotaje en bitmap");
		return;
		break;

	case FILES_SIZE:
		if (fue_en_oxigeno) {
			arreglar_valor_size(OXIGENO);
			log_warning(mongoLogger,"se arreglo el sabotaje en oxigeno por size de archivo");
			return;
		}

		if (fue_en_comida) {
			arreglar_valor_size(COMIDA);
			log_warning(mongoLogger,"se arreglo el sabotaje en comida por size de archivo");
			return;
		}

		if (fue_en_basura) {
			arreglar_valor_size(BASURA);
			log_warning(mongoLogger,"se arreglo el sabotaje en basura por size de archivo");
			return;
		}

		break;

	case FILES_BLOCK_COUNT:
		if (fue_en_oxigeno) {
			arreglar_valor_block_count(OXIGENO);
			log_warning(mongoLogger,"se arreglo el sabotaje en oxigeno por cantidad de bloques");
			return;

		}

		if (fue_en_comida) {
			arreglar_valor_block_count(COMIDA);
			log_warning(mongoLogger,"se arreglo el sabotaje en comida por cantidad de bloques");
			return;
		}

		if (fue_en_basura) {
			arreglar_valor_block_count(BASURA);
			log_warning(mongoLogger,"se arreglo el sabotaje en basura por cantidad de bloques");
			return;
		}
		break;

	case FILES_MD5:
		if (fue_en_oxigeno) {
			char* rutaOxigeno = conseguir_ruta(OXIGENOO);
			reparar_MD5(rutaOxigeno, 'O');
			log_warning(mongoLogger,"se arreglo el sabotaje en oxigeno por alteracion en blocks");
			free(rutaOxigeno);
			return;
		}

		if (fue_en_comida) {
			char* rutaComida = conseguir_ruta(COMIDAA);

			reparar_MD5(rutaComida, 'C');
			log_warning(mongoLogger,"se arreglo el sabotaje en comida por alteracion en blocks");
			free(rutaComida);
			return;
		}

		if (fue_en_basura) {
			char* rutaBasura = conseguir_ruta(BASURAA);
			reparar_MD5(rutaBasura, 'B');
			log_warning(mongoLogger,"se arreglo el sabotaje en basura por alteracion en blocks");
			free(rutaBasura);
			return;
		}
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
void enviar_aviso_sabotaje_a_discordiador() {
	char** sabotaje_pos_aux;
	char* sabotaje_posY;
	char* sabotaje_posX;
	char* pos_sabotaje;
	while (1) {
		sem_wait(&dar_orden_sabotaje);
		pos_sabotaje = siguiente_posicion_sabotaje();
		if (pos_sabotaje == NULL) {		//no hay mas sabotajes
			sem_post(&semaforo_modificacion_de_datos);
		} else {
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
		}
	}
	return;
}
