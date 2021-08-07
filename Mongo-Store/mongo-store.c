#include "mongo-store.h"

int main() {

	iniciar_recursos_mongo();
	pthread_t hilo_bajada_a_disco;

	pthread_create(&hilo_bajada_a_disco, NULL,
			(void *) gestionar_bajadas_a_disco, NULL);
	pthread_detach(hilo_bajada_a_disco);

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);

	gestionarCliente(socket_mongo_store);

	printf("SOCKET DISCO %d\n", socket_mongo_store);
	return EXIT_SUCCESS;
}

char* conseguir_ruta(rutas ruta) {
	char* ruta_completa = string_new();
	string_append(&ruta_completa, puntoMontaje);

	switch (ruta) {
	case SUPERBLOQUE:
		string_append(&ruta_completa, "/SuperBloque.ims");
		break;
	case FILES:
		string_append(&ruta_completa, "/Files");
		break;
	case BITACORA:
		string_append(&ruta_completa, "/Files/Bitacora");
		break;
	case BLOCKS:
		string_append(&ruta_completa, "/Blocks.ims");
		break;
	case OXIGENOO:
		string_append(&ruta_completa, "/Files/oxigeno.ims");
		break;
	case COMIDAA:
		string_append(&ruta_completa, "/Files/comida.ims");
		break;
	case BASURAA:
		string_append(&ruta_completa, "/Files/basura.ims");
		break;
	}

	return ruta_completa;
}

void iniciar_recursos_mongo(void) {
	signal(SIGUSR1, rutina); //Recepcion mensaje de sabotaje

	numero_sabotaje = 0;

	sem_init(&dar_orden_sabotaje, 0, 0);
	sem_init(&contador_sabotaje, 0, 1);
	sem_init(&semaforo_bitacora, 0, 1);
	sem_init(&semaforo_para_file_oxigeno, 0, 1);
	sem_init(&semaforo_para_file_comida, 0, 1);
	sem_init(&semaforo_para_file_basura, 0, 1);
	sem_init(&inicio_fsck, 0, 0);
	sem_init(&semaforo_generar_md5, 0, 1);
	sem_init(&semaforo_modificacion_de_datos, 0, 1);

	//inicializo los booleanos de existen archivos
	g_existe_file_oxigeno = false;
	g_existe_file_comida = false;
	g_existe_file_basura = false;
	g_modificado_file_oxigeno = false;
	g_modificado_file_comida = false;
	g_modificado_file_basura = false;
	g_en_uso_file_oxigeno = false;
	g_en_uso_file_comida = false;
	g_en_uso_file_basura = false;
	g_abierto_file_oxigeno = false;
	g_abierto_file_comida = false;
	g_abierto_file_basura = false;

	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server
	puerto = config_get_string_value(mongoConfig, "PUERTO");
	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_DEBUG);

	log_info(mongoLogger, "INICIANDO RECURSOS");

	crear_estructura_filesystem();
}

void bajar_datos_blocks(void) {
	int archivo_blocks;
	char *contenido_blocks;
	struct stat statbuf;

	char* dirBlocks = conseguir_ruta(BLOCKS);
	archivo_blocks = open(dirBlocks, O_RDWR);

	if (archivo_blocks == -1) {
		log_error(mongoLogger,
				"Error al abrir el archivo para bajar los blocks");
		exit(1);
	}

	fstat(archivo_blocks, &statbuf);

	contenido_blocks = (char*) mmap(NULL, statbuf.st_size, PROT_WRITE,
	MAP_SHARED, archivo_blocks, 0);

	memcpy(contenido_blocks, block_mmap, statbuf.st_size);

	msync(contenido_blocks, statbuf.st_size, MS_SYNC);

	munmap(contenido_blocks, statbuf.st_size);

	close(archivo_blocks);
	free(dirBlocks);
}

void bajar_datos_files(char* un_file, char* ruta_file) {
	int archivo_un_file;
	char *contenido_un_file;
	struct stat statbuf;

	archivo_un_file = open(ruta_file, O_RDWR);

	if (archivo_un_file == -1) {
		log_error(mongoLogger,
				"Error al abrir el archivo para bajar el file %s", ruta_file);
		exit(1);
	}
	ftruncate(archivo_un_file, (off_t) string_length(un_file));
	fstat(archivo_un_file, &statbuf);

	contenido_un_file = (char*) mmap(NULL, statbuf.st_size, PROT_WRITE,
	MAP_SHARED, archivo_un_file, 0);

	memcpy(contenido_un_file, un_file, statbuf.st_size);

	msync(contenido_un_file, statbuf.st_size, MS_SYNC);

	munmap(contenido_un_file, statbuf.st_size);

	close(archivo_un_file);
}

void bajar_datos_superbloque(void) {
	int archivo_superbloque;
	void *contenido_superbloque;
	struct stat statbuf;

	char* dirSuperbloque = conseguir_ruta(SUPERBLOQUE);
	archivo_superbloque = open(dirSuperbloque, O_RDWR);

	if (archivo_superbloque == -1) {
		log_error(mongoLogger,
				"Error al abrir el archivo para bajar el superbloque");
		exit(1);
	}

	fstat(archivo_superbloque, &statbuf);

	contenido_superbloque = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED,
			archivo_superbloque, 0);

	memcpy(contenido_superbloque, superbloque, statbuf.st_size);

	int retorno = msync(contenido_superbloque, statbuf.st_size, MS_SYNC);

	munmap(contenido_superbloque, statbuf.st_size);

	if (retorno == -1) {
		log_error(mongoLogger, "Error al sincronizar el superbloque");
	}

	close(archivo_superbloque);
	free(dirSuperbloque);
}

void gestionar_bajadas_a_disco(void) {
	int tiempo_bajada = atoi(
			config_get_string_value(mongoConfig, "TIEMPO_SINCRONIZACION"));

	while (1) {
		sleep(tiempo_bajada);
		sem_wait(&semaforo_modificacion_de_datos);
		log_info(mongoLogger,
				"Realizando bajada a disco de blocks y superbloque..");

		bajar_datos_blocks();
		bajar_datos_superbloque();

		if (g_existe_file_oxigeno && g_modificado_file_oxigeno) {
			char* rutaOxigeno = conseguir_ruta(OXIGENOO);
			bajar_datos_files(archivoOxigeno, rutaOxigeno);
			g_modificado_file_oxigeno = false;
			free(rutaOxigeno);
			if (!g_en_uso_file_oxigeno) {
				free(archivoOxigeno);
				g_abierto_file_oxigeno = false;
			}
		}
		if (g_existe_file_comida && g_modificado_file_comida) {
			char* rutaComida = conseguir_ruta(COMIDAA);
			bajar_datos_files(archivoComida, rutaComida);
			g_modificado_file_comida = false;
			free(rutaComida);
			if (!g_en_uso_file_oxigeno) {
				free(archivoComida);
				g_abierto_file_comida = false;
			}
		}
		if (g_existe_file_basura && g_modificado_file_basura) {
			char* rutaBasura = conseguir_ruta(BASURAA);
			bajar_datos_files(archivoBasura, rutaBasura);
			g_modificado_file_basura = false;
			free(rutaBasura);
			if (!g_en_uso_file_oxigeno) {
				free(archivoBasura);
				g_abierto_file_basura = false;
			}
		}
		log_info(mongoLogger, "Finalizo la bajada a disco");
		sem_post(&semaforo_modificacion_de_datos);
	}
}

void mostrar_estado_bitarray(void) {
	int cantBytes = (double) ceil((double) *g_blocks / 8);

	mem_hexdump(superbloque + 2 * sizeof(uint32_t), cantBytes);
	/*
	 for (int i = 0; i <= *g_blocks; i++) {
	 if(i == 0)
	 printf("%d", bitarray_test_bit(bitmap, i));
	 else if (i % 8 == 0)
	 printf("%d\n", bitarray_test_bit(bitmap, i));
	 else
	 printf("%d", bitarray_test_bit(bitmap, i));
	 }
	 */
	printf("\n");

}

//genera el MD5 pasandole por parametro el contenido de un archivo de file
//por ejemplo el contenido puede ser 'OOOOOOOOOOO'
char *generarMD5(char *contenido) {
	int fd;
	char *command = string_new(); //por las dudas q el path sea largo
	int retorno;
	struct stat statbuf;
	void *data;
	char *md5;
	char *ruta_md5 = string_new();
	char *punto_montaje = config_get_string_value(mongoConfig, "PUNTO_MONTAJE");

	ruta_md5 = string_from_format("%s/Files/md5.txt", punto_montaje);
	sem_wait(&semaforo_generar_md5);
	command = string_from_format("echo -n %s | md5sum > %s", contenido,
			ruta_md5);

	retorno = system(command);

	if (retorno != 0) {
		printf("ERROR AL GENERAR EL MD5\n");
		sem_post(&semaforo_generar_md5);
		return " ";
	}

	fd = open(ruta_md5, O_RDWR);

	if (fd == -1) {
		printf("ERROR AL ABRIR ARCHIVO\n");
		sem_post(&semaforo_generar_md5);
		return " ";
	}

	fstat(fd, &statbuf);

	data = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
	md5 = strdup(data);

	munmap(data, statbuf.st_size); //cierro el archivo
	free(command);
	sem_post(&semaforo_generar_md5);
	return md5;
}

//creamos el disco logico
void crearEstructuraDiscoLogico() {
	disco_logico = (t_disco_logico *) malloc(sizeof(t_disco_logico));
	disco_logico->bloques = list_create();
}

//creamos todos los bloques del filesystem
void crearEstructurasBloques() {
	int _cantidad_de_bloques = (int) *g_blocks;

	for (int contador = 1; contador <= _cantidad_de_bloques; contador++) {
		t_bloque *bloque = (t_bloque *) malloc(sizeof(t_bloque));
		bloque->id_bloque = contador;
		bloque->inicio = (contador - 1) * *g_block_size;
		bloque->fin = bloque->inicio + (*g_block_size - 1);
		bloque->espacio = *g_block_size;
		bloque->posicion_para_escribir = bloque->inicio;
		list_add(disco_logico->bloques, bloque);

	}

	log_info(mongoLogger, "Se creo estructura bloques!");

	log_info(mongoLogger, "Cantidad de bloques generados: %d",
			list_size(disco_logico->bloques));
}

//devuelve el ultimo bloque que usa el tripulante para sus datos
t_bloque* buscar_ultimo_bloque_del_tripulante(char* rutaBitacora) {
	t_bloque* el_bloque;
	int cantidad_de_bloques = 0;
	struct stat statbuf;
	int archivo = open(rutaBitacora, O_RDWR);
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char **renglones_bitacora = string_split(archivo_addr, "\n");
	char **bloques_division = string_split(renglones_bitacora[1], "=");

	char **bloques_bitacora = string_split(bloques_division[1], ",");
	while (bloques_bitacora[cantidad_de_bloques]) {
		cantidad_de_bloques++;
	}

	int indexUltimoBloque = atoi(bloques_bitacora[cantidad_de_bloques - 1]) - 1;

	el_bloque = (t_bloque *) list_get(disco_logico->bloques, indexUltimoBloque);

	/*log_info(mongoLogger,
			"El ultimo bloque asignado en la bitacora %s es: %d y esta en indice: %d",
			rutaBitacora, el_bloque->id_bloque, indexUltimoBloque);*/

	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	return el_bloque;
}

//crea un archivo en una ruta
void crear_archivo(char* ruta) {
	FILE *archivo = fopen(ruta, "w");
	fclose(archivo);
}

//crea el archivo bitacora cuando no existe
void inicializar_bitacora(char *rutaBitacora, char *numeroDeBloque) {
	crear_archivo(rutaBitacora);
	char *cadena=string_from_format("SIZE=0\nBLOCKS=%s",numeroDeBloque);
	int archivo = open(rutaBitacora, O_RDWR);
	struct stat statbuf;
	ftruncate(archivo, string_length(cadena)+1);
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);

	for (int i = 0; i < string_length(cadena); i++) {
		archivo_addr[i] = cadena[i];
	}
	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
}

void agregar_bloque_bitacora(char *rutaBitacora, int bloque) {
	FILE *fd = fopen(rutaBitacora, "r+");

	char *nuevoBloque = string_from_format(",%d", bloque);

	if (fd == NULL) {
		log_info(mongoLogger,
				"Error al abrir la bitacora para escribir bloque");
		return;
	}

	fseek(fd, 0, SEEK_END);

	fputs(nuevoBloque, fd);

	fclose(fd);
}

//crea el bitmap logico para operar los bloques
void crearBitMapLogico() {
	int cantBloques = atoi(config_get_string_value(mongoConfig, "BLOCKS"));

	int cantBytes = (double) ceil((double) cantBloques / 8);

	void* puntero_a_bits = malloc(cantBytes);

	bitmap = bitarray_create(puntero_a_bits, cantBytes);
	for (int i = 1; i <= cantBloques; i++) {
		bitarray_clean_bit(bitmap, i);
	}

	log_info(mongoLogger, "Se creo el Bitmap logico con %d Bytes", cantBytes);
}

//crea el archivo superbloque
void crearSuperbloque(char * dirSuperbloque) {
	char *block_size = config_get_string_value(mongoConfig, "BLOCK_SIZE");
	char *blocks = config_get_string_value(mongoConfig, "BLOCKS");
	crear_archivo(dirSuperbloque);
	int archivo = open(dirSuperbloque, O_RDWR);
	struct stat statbuf;
	ftruncate(archivo,
			(off_t) 19 + string_length(block_size) + string_length(blocks));
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char *cadena = string_new();
	string_append(&cadena, "Block_size=");
	string_append(&cadena, block_size);
	string_append(&cadena, "\nBLOCKS=");
	string_append(&cadena, blocks);

	for (int i = 0; i < string_length(cadena); i++) {
		archivo_addr[i] = cadena[i];
	}
	munmap(archivo_addr, statbuf.st_size);
	free(cadena);
	close(archivo);
}

//crea el archivo blocks
void crearblocks(char* dirBlocks) {

	struct stat statbuf;
	//int block_size= atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
	int block_size = (int) *g_block_size;
	//int blocks= atoi(config_get_string_value(mongoConfig,"BLOCKS"));
	int blocks = (int) *g_blocks;
	crear_archivo(dirBlocks);
	int peso = (int) block_size * blocks;

	printf("PESO ES: %d\n", peso);

	FILE *fd;

	fd = fopen(dirBlocks, "w+");

	ftruncate(fileno(fd), peso);

	for (int i = 0; i < peso; i++) {
		fputc(' ', fd);
	}

	fclose(fd);

	log_info(mongoLogger, "Se crearon los bloques!");
}

//crea la carpeta file
void crearCarpetaFile(char * dirFiles) {

	if (mkdir(dirFiles, 0777) == 0) {
		log_info(mongoLogger, "se creo el directorio file correctamente\n");
	} else
		log_error(mongoLogger,
				"Ha ocurrido un error al crear el directorio Files.");

}

//crea la carpeta bitacora
void crearCarpetaBitacora(char * dirBitacora) {
	if (mkdir(dirBitacora, 0777) == 0) {
		log_info(mongoLogger, "se creo el directorio bitacora correctamente\n");
	} else
		log_error(mongoLogger,
				"Ha ocurrido un error al crear el directorio bitacora.");
}

//comprueba si existen todos los datos al levantar de nuevo el filesystem
int comprobar_que_todos_los_datos_existen(char* puntoMontaje) {
	bool todoBien = false;
	char *dirSuperbloque = conseguir_ruta(SUPERBLOQUE);
	int existeArchivoSuperBloque = access(dirSuperbloque, F_OK);

	char* dirFiles = conseguir_ruta(FILES);
	int existeArchivoFiles = access(dirFiles, F_OK);

	char* dirBitacora = conseguir_ruta(BITACORA);
	int existeArchivoBitacora = access(dirBitacora, F_OK);

	char* dirBlocks = conseguir_ruta(BLOCKS);
	int existeArchivoBlocks = access(dirBlocks, F_OK);

	if (!existeArchivoSuperBloque && !existeArchivoBitacora
			&& !existeArchivoFiles && !existeArchivoBlocks) {
		printf("se recupero todo perfecto\n");
		todoBien = true;
	} else {
		printf("no se recupero todo perfecto\n");

	}
	free(dirSuperbloque);
	free(dirFiles);
	free(dirBitacora);
	free(dirBlocks);
	return todoBien;
}

char* copiar_file(char* archivoEnRam, char *ruta) {
	struct stat statbuf;
	void *contenido_ruta;
	int fdm = open(ruta, O_RDWR);
	if (fdm == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR ARCHIVO %s", ruta);
		return NULL;
	}

	fstat(fdm, &statbuf);
	contenido_ruta = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fdm,
			0);

	archivoEnRam = malloc(statbuf.st_size);
	memcpy(archivoEnRam, contenido_ruta, statbuf.st_size);
	munmap(contenido_ruta, statbuf.st_size);
	close(fdm);
	return archivoEnRam;
}

void copiar_bitmap_de_disco(char* dirSuperbloque) {
	struct stat statbuf;
	void *contenido_superbloque;
	int fdm = open(dirSuperbloque, O_RDWR);

	if (fdm == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR ARCHIVO");
		exit(1);
	}

	fstat(fdm, &statbuf);
	contenido_superbloque = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED,
			fdm, 0);

	superbloque = malloc(statbuf.st_size);

	memcpy(superbloque, contenido_superbloque, statbuf.st_size);

	g_block_size = superbloque;
	g_blocks = superbloque + sizeof(uint32_t);
	bitarrayEnChar = superbloque + 2 * sizeof(uint32_t);

	int cantLeer2 = (int) ceil((double) *g_blocks / 8);

	bitmap = bitarray_create(bitarrayEnChar, cantLeer2);

	g_tamanio_superbloque = statbuf.st_size;

	//reviso las cosas
	log_info(mongoLogger, "SIZE GUARDADO EN SUPERBLOQUE: %d\n",
			(int) *g_block_size);
	log_info(mongoLogger, "CANT GUARDADO EN SUPERBLOQUE: %d\n",
			(int) *g_blocks);

	mostrar_estado_bitarray(); //usarlo cuando hay pocos bits

	log_info(mongoLogger, "Se bajaron los datos del Superbloque existente!");

	munmap(contenido_superbloque, g_tamanio_superbloque);
	close(fdm);
}

void crearSuperbloqueNuevo(char *path) {
	FILE *fd;
	fd = fopen(path, "wb+");

	if (fd == NULL) {
		log_error(mongoLogger, "ERROR AL ABRIR ARCHIVO");
		exit(1);
	}

	int cantBytes = (int) ceil((double) (*g_blocks) / 8);

	printf("%d", cantBytes);

	fwrite(g_block_size, sizeof(uint32_t), 1, fd);
	fwrite(g_blocks, sizeof(uint32_t), 1, fd);
	fwrite(bitmap->bitarray, cantBytes, 1, fd);

	log_info(mongoLogger, "Se creo el superbloque nuevo!");

	fclose(fd);
}

//devuelve la cantidad de caracteres ocupados
int caracteres_ocupados(int inicio, int fin) {
	int posicion = inicio;
	int caracterOcupado = 0;
	int ultimoCaracter = 0;
	bool tiene_caracteres_escritos = false;
	while (posicion <= fin) {
		if (block_mmap[posicion] != ' ') {
			ultimoCaracter = posicion;
			tiene_caracteres_escritos = true;
		}
		posicion++;
	}

	if (tiene_caracteres_escritos) {
		caracterOcupado = ultimoCaracter + 1 - inicio;
	}

	return caracterOcupado;
}

//copia los datos de cada bloque a los bloques logicos para recuperar la info cuando
//se levanta el filesystem
void copiar_datos_de_bloques(t_list* bloques) {
	int inicio, fin;
	t_bloque *bloque;

	for (int i = 0; i < list_size(bloques); i++) {
		//agarro un bloque
		bloque = list_get(bloques, i);
		//veo donde empieza
		int inicio = bloque->inicio;

		//veo donde termina
		int fin = bloque->fin;

		//leo desde empieza hasta termina en bloc los caracteres escritos
		int caracteresOcupados = caracteres_ocupados(inicio, fin);
		//resto espacio en bloque
		bloque->espacio = bloque->espacio - caracteresOcupados;

		//acomodo el puntero en bloque
		bloque->posicion_para_escribir = caracteresOcupados + bloque->inicio;
	}
}

void crearCopiaBlocks(char *dirBlocks) {
	struct stat statbuf;
	int archivo;
	char *contenidoArchivo;

	archivo = open(dirBlocks, O_RDWR);

	if (archivo == -1) {
		log_error(mongoLogger,
				"Error al abrir el archivo para copiar los blocks!");
	}

	fstat(archivo, &statbuf);

	contenidoArchivo = (char *) mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);

	block_mmap = (char *) malloc(statbuf.st_size);

	memcpy(block_mmap, contenidoArchivo, statbuf.st_size);

	munmap(contenidoArchivo, statbuf.st_size);
}

void crearCopiaFile(char* file, char *dir_file) {
	struct stat statbuf;
	int archivo;
	char *contenidoArchivo;

	archivo = open(dir_file, O_RDWR);

	if (archivo != -1) {
		fstat(archivo, &statbuf);

		contenidoArchivo = (char *) mmap(NULL, statbuf.st_size,
		PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);

		file = (char *) malloc(statbuf.st_size);

		memcpy(file, contenidoArchivo, statbuf.st_size);

		munmap(contenidoArchivo, statbuf.st_size);
	}
}

//crea el filesystem completo si no existe o levanta el que existe
void crear_estructura_filesystem() {

	puntoMontaje = config_get_string_value(mongoConfig, "PUNTO_MONTAJE");

	char* dirSuperbloque = conseguir_ruta(SUPERBLOQUE);
	char* dirFiles = conseguir_ruta(FILES);
	char* dirBitacora = conseguir_ruta(BITACORA);
	char* dirBlocks = conseguir_ruta(BLOCKS);
	char* rutaOxigeno = conseguir_ruta(OXIGENOO);
	char* rutaBasura = conseguir_ruta(BASURAA);
	char* rutaComida = conseguir_ruta(COMIDAA);

	if (mkdir(puntoMontaje, 0777) != 0) {
		int todoBien = comprobar_que_todos_los_datos_existen(puntoMontaje);

		if (todoBien) {
			log_info(mongoLogger, "Es un FS existente.");

			copiar_bitmap_de_disco(dirSuperbloque); //busco la info superB
			crearEstructuraDiscoLogico();
			crearEstructurasBloques();
			crearCopiaBlocks(dirBlocks);
			copiar_datos_de_bloques(disco_logico->bloques);

			int existeArchivoOxigeno = open(rutaOxigeno, O_RDWR);
			if (existeArchivoOxigeno != -1) {
				g_existe_file_oxigeno = true;
			}
			close(existeArchivoOxigeno);

			int existeArchivoComida = open(rutaComida, O_RDWR);
			if (existeArchivoComida != -1) {
				g_existe_file_comida = true;
			}
			close(existeArchivoComida);

			int existeArchivoBasura = open(rutaBasura, O_RDWR);
			if (existeArchivoBasura != -1) {
				g_existe_file_basura = true;
			}
			close(existeArchivoBasura);

		}
	}

	else {
		log_info(mongoLogger, "Generando estructura del FS");

		g_nuevo_block_size = (unsigned) atoi(
				config_get_string_value(mongoConfig, "BLOCK_SIZE"));
		g_nuevo_blocks = (unsigned) atoi(
				config_get_string_value(mongoConfig, "BLOCKS"));

		g_block_size = &g_nuevo_block_size;
		g_blocks = &g_nuevo_blocks;

		log_info(mongoLogger, "BLOCKS SIZE: %u", *g_block_size);
		log_info(mongoLogger, "BLOCKS: %u", *g_blocks);

		crearEstructuraDiscoLogico();
		crearEstructurasBloques();
		crearBitMapLogico();
		crearSuperbloqueNuevo(dirSuperbloque); //archivo nuevo
		copiar_bitmap_de_disco(dirSuperbloque);
		crearblocks(dirBlocks); //archivo
		crearCarpetaFile(dirFiles); //carpeta
		crearCarpetaBitacora(dirBitacora); //carpeta
		crearCopiaBlocks(dirBlocks);
	}
	free(dirSuperbloque);
	free(dirFiles);
	free(dirBitacora);
	free(dirBlocks);
	free(rutaOxigeno);
	free(rutaComida);
	free(rutaBasura);
}

//escribe lo que quieras en el archivo block
void escribir_en_block(char* lo_que_se_va_a_escribir, t_bloque* el_bloque) {
	pthread_mutex_lock(&mutex_disco_logico);

	int longitud_texto = string_length(lo_que_se_va_a_escribir);
	for (int i = 0; i < longitud_texto; i++) {
		block_mmap[el_bloque->posicion_para_escribir] =
				lo_que_se_va_a_escribir[i];
		el_bloque->posicion_para_escribir++;
	}
	el_bloque->espacio = el_bloque->espacio
			- string_length(lo_que_se_va_a_escribir);
	pthread_mutex_unlock(&mutex_disco_logico);

}

void borrar_en_block(int cantidad, t_bloque* bloque) {
	while (cantidad > 0) {
		bloque->posicion_para_escribir--;
		block_mmap[bloque->posicion_para_escribir] = ' ';
		bloque->espacio++;
		cantidad--;
	}
}

char* obtener_bloques_bitacora(char* ruta) {
	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char** auxiliar = string_split(archivo_addr, "\n");
	char** auxiliar2 = string_split(auxiliar[1], "=");
	char* blocks = auxiliar2[1];

	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	return blocks;
}

void obtener_bitacora_tripulante(int cliente) {
	t_list* lista = recibir_paquete(cliente);
	uint32_t idTripulante = (uint32_t) atoi(list_get(lista, 0));
	uint32_t idPatota = (uint32_t) atoi(list_get(lista, 1));
	char* dirBitacora = conseguir_ruta(BITACORA);
	char* rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
			dirBitacora, idTripulante, idPatota);
	int archivo = open(rutaBitacora, O_RDONLY);
	if (archivo != -1) {
		char* bloques_bitacora = obtener_bloques_bitacora(rutaBitacora);
		char* texto = contenido_de_bloques(bloques_bitacora);
		t_paquete* paquete = crear_paquete(OBTENGO_BITACORA);
		agregar_a_paquete(paquete, texto, string_length(texto) + 1);
		enviar_paquete(paquete, cliente);
		eliminar_paquete(paquete);

	} else {
		log_info(mongoLogger,
				"se pidio la bitacora del tripulante %d de la patota %d pero no existe",
				idTripulante, idPatota);
	}
	liberar_cliente(cliente);
	close(archivo);
	free(dirBitacora);
	free(rutaBitacora);

}

void funcion_para_llenar_con_tarea_IO(
		m_estado_tarea_tripulante* tripulanteConTareaFinalizada) {

	switch (tripulanteConTareaFinalizada->codigo_tarea) {

	case GENERAR_OXIGENO:
		printf("ES GENERAR OXIGENO\n");
		printf("Cantidad a llenar: %d\n",
				tripulanteConTareaFinalizada->parametro);
		int cantidadO = tripulanteConTareaFinalizada->parametro;
		generar_oxigeno(cantidadO);
		break;

	case CONSUMIR_OXIGENO:
		printf("ES CONSUMIR OXIGENO\n");
		printf("Cantidad a consumir: %d\n",
				tripulanteConTareaFinalizada->parametro);
		int cantidadOx = tripulanteConTareaFinalizada->parametro;
		consumir_oxigeno(cantidadOx);
		break;

	case GENERAR_COMIDA:
		printf("ES GENERAR COMIDA\n");
		printf("Cantidad a llenar: %d\n",
				tripulanteConTareaFinalizada->parametro);
		int cantidadC = tripulanteConTareaFinalizada->parametro;
		generar_comida(cantidadC);
		break;

	case CONSUMIR_COMIDA:
		printf("ES CONSUMIR COMIDA\n");
		printf("Cantidad a consumir: %d\n",
				tripulanteConTareaFinalizada->parametro);
		int cantidadCo = tripulanteConTareaFinalizada->parametro;
		consumir_comida(cantidadCo);
		break;

	case GENERAR_BASURA:
		printf("ES GENERAR BASURA \n");
		printf("Cantidad a llenar: %d\n",
				tripulanteConTareaFinalizada->parametro);
		int cantidadB = tripulanteConTareaFinalizada->parametro;
		generar_basura(cantidadB);
		break;

	case DESCARTAR_BASURA:
		printf("ES DESCARTARAR BASURA \n");
		printf("Cantidad a llenar: %d\n",
				tripulanteConTareaFinalizada->parametro);
		int cantidadBa = tripulanteConTareaFinalizada->parametro;
		descartar_basura(cantidadBa);
		break;

	}

}

void *gestionarCliente(int socket) {
	t_list* lista;
	int operacion;
	t_paquete *paquete;

	while (1) {
		int cliente = esperar_cliente(socket, mongoLogger);
		operacion = recibir_operacion(cliente);
		lista = NULL;

		switch (operacion) {
		case OBTENGO_BITACORA:
			;
			pthread_t obtener_bitacora;
			pthread_create(&obtener_bitacora, NULL,
					(void*) obtener_bitacora_tripulante, (void*) cliente);
			pthread_detach(obtener_bitacora);
			break;
		case ELIMINAR_TRIPULANTE:
			liberar_cliente(cliente);
			break;
		case ESPERANDO_SABOTAJE:
			;
			socket_sabotaje_cliente = cliente;

			log_info(mongoLogger, "El socket para sabotajes es: %d\n",
					socket_sabotaje_cliente);

			pthread_create(&hilo_sabotaje, NULL,
					(void*) enviar_aviso_sabotaje_a_discordiador,
					NULL);
			pthread_detach(hilo_sabotaje);
			break;
		case ACTUALIZAR_POSICION:
			;
			pthread_t hilo_actualizar_posicion;
			tripulanteEnMovimiento = (m_movimiento_tripulante *) malloc(
					sizeof(m_movimiento_tripulante));
			tripulanteEnMovimiento = recibirMovimientoTripulante(cliente);

			tripulante_con_su_accion *tripulanteAP =
					(tripulante_con_su_accion*) malloc(
							sizeof(tripulante_con_su_accion));
			tripulanteAP->tripulante = tripulanteEnMovimiento;
			tripulanteAP->accion = ACTUALIZAR_POSICION;

			pthread_create(&hilo_actualizar_posicion, NULL,
					(void*) escribir_en_su_bitacora_la_accion,
					(void*) tripulanteAP);
			pthread_detach(hilo_actualizar_posicion);
			liberar_cliente(cliente);
			break;

		case INICIO_TAREA:
			;
			pthread_t hilo_escribir_inicio_tarea;
			m_estado_tarea_tripulante *tripulanteConTarea =
					(m_estado_tarea_tripulante *) malloc(
							sizeof(m_estado_tarea_tripulante));
			tripulanteConTarea = recibirNuevoEstadoTareaTripulante(cliente);

			tripulante_con_su_accion *tripulanteIT =
					(tripulante_con_su_accion*) malloc(
							sizeof(tripulante_con_su_accion));
			tripulanteIT->tripulante = tripulanteConTarea;
			tripulanteIT->accion = INICIO_TAREA;

			pthread_create(&hilo_escribir_inicio_tarea, NULL,
					(void*) escribir_en_su_bitacora_la_accion,
					(void*) tripulanteIT);
			pthread_detach(hilo_escribir_inicio_tarea);

			liberar_cliente(cliente);
			break;

		case FIN_TAREA:
			;
			pthread_t hilo_escribir_fin_tarea;
			pthread_t hilo_tarea_io;
			m_estado_tarea_tripulante *tripulanteConTareaFinalizada =
					(m_estado_tarea_tripulante *) malloc(
							sizeof(m_estado_tarea_tripulante));
			tripulanteConTareaFinalizada = recibirNuevoEstadoTareaTripulante(
					cliente);

			tripulante_con_su_accion *tripulanteFT =
					(tripulante_con_su_accion*) malloc(
							sizeof(tripulante_con_su_accion));
			tripulanteFT->tripulante = tripulanteConTareaFinalizada;
			tripulanteFT->accion = FIN_TAREA;
			//aca avisaria A BITACORA que termino tarea independientemente si es IO/COMUN
			pthread_create(&hilo_escribir_fin_tarea, NULL,
					(void*) escribir_en_su_bitacora_la_accion,
					(void*) tripulanteFT);
			pthread_detach(hilo_escribir_fin_tarea);

			if (tripulanteConTareaFinalizada->tipo_tarea == TAREA_IO) {
				//aca llenaria el archivo tantas veces como el 'parametro'
				pthread_create(&hilo_tarea_io, NULL,
						(void*) funcion_para_llenar_con_tarea_IO,
						(void*) tripulanteConTareaFinalizada);
				pthread_detach(hilo_tarea_io);
			}
			liberar_cliente(cliente);
			break;

		case INICIO_SABOTAJE:
			;
			pthread_t hilo_inicio_sabotaje;
			m_movimiento_tripulante *trip_inicio_sabotaje =
					(m_movimiento_tripulante *) malloc(
							sizeof(m_movimiento_tripulante));
			trip_inicio_sabotaje = recibirMovimientoTripulante(cliente);

			tripulante_con_su_accion *tripulanteIS =
					(tripulante_con_su_accion*) malloc(
							sizeof(tripulante_con_su_accion));
			tripulanteIS->tripulante = trip_inicio_sabotaje;
			tripulanteIS->accion = INICIO_SABOTAJE;

			pthread_create(&hilo_inicio_sabotaje, NULL,
					(void*) escribir_en_su_bitacora_la_accion,
					(void*) tripulanteIS);
			pthread_detach(hilo_inicio_sabotaje);

			liberar_cliente(cliente);
			break;

		case FIN_SABOTAJE:
			;
			pthread_t hilo_fin_sabotaje;
			pthread_t tarea_IO;
			m_movimiento_tripulante *trip_fin_sabotaje =
					(m_movimiento_tripulante *) malloc(
							sizeof(m_movimiento_tripulante));
			trip_fin_sabotaje = recibirMovimientoTripulante(cliente);

			tripulante_con_su_accion *tripulanteFS =
					(tripulante_con_su_accion*) malloc(
							sizeof(tripulante_con_su_accion));
			tripulanteFS->tripulante = trip_fin_sabotaje;
			tripulanteFS->accion = FIN_SABOTAJE;
			sabotaje_exito=true;
			sem_post(&inicio_fsck);

			pthread_create(&hilo_fin_sabotaje, NULL,
					(void*) escribir_en_su_bitacora_la_accion,
					(void*) tripulanteFS);
			pthread_detach(hilo_fin_sabotaje);

			liberar_cliente(cliente);
			break;
		case FIN_SABOTAJE_ERROR:
			sabotaje_exito=false;
			sem_post(&inicio_fsck);
			liberar_cliente(cliente);
			break;
		case -1:
			log_error(mongoLogger, "El cliente %d se desconecto", cliente);
			liberar_cliente(cliente);
			break;
		default:
			log_error(mongoLogger, "Operacion desconocida");
			liberar_cliente(cliente);
			break;

		}
	}
}

//crea y escribe el archivo necesario con el texto necesario
void inicializar_archivo(char* ruta_archivo, char caracter) {

	crear_archivo(ruta_archivo);
	char* el_caracter = string_from_format("%c", caracter);

	char* cadena = string_new();
	string_append(&cadena, "SIZE=0");
	string_append(&cadena, "\nBLOCK_COUNT=");
	string_append(&cadena, "\nBLOCKS=");
	string_append(&cadena, "\nCARACTER_LLENADO=");
	string_append(&cadena, el_caracter);
	string_append(&cadena, "\nMD5_ARCHIVO=");

	int archivo = open(ruta_archivo, O_RDWR);
	struct stat statbuf;
	ftruncate(archivo, (off_t) string_length(cadena) * sizeof(char));
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);

	for (int i = 0; i < string_length(cadena); i++) {
		archivo_addr[i] = cadena[i];
	}

	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	free(cadena);

}

//devuelte el tamaño del archivo en char*
char *size_de_archivo(char* ruta) {
	char** auxiliar = string_split(ruta, "\n");
	char** auxiliar2 = string_split(auxiliar[0], "=");
	char* size = auxiliar2[1];

	return size;
}

char *size_de_archivo_fisico(char* ruta) {
	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char** auxiliar = string_split(archivo_addr, "\n");
	char** auxiliar2 = string_split(auxiliar[0], "=");
	char* size = auxiliar2[1];

	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	return size;
}

char **renglones_archivo(char* ruta) {
	char** auxiliar = string_split(ruta, "\n");

	return auxiliar;
}
//devuelve la cantidad de bloques de un archivo en char*
char* cantidad_de_bloques_de_archivo(char* ruta) {

	char** auxiliar = string_split(ruta, "\n");
	char** auxiliar2 = string_split(auxiliar[1], "=");
	char* block_count = auxiliar2[1];

	return block_count;

}

char* cantidad_de_bloques_de_archivo_fisico(char* ruta) {

	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char** auxiliar = string_split(archivo_addr, "\n");
	char** auxiliar2 = string_split(auxiliar[1], "=");
	char* block_count = auxiliar2[1];

	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	return block_count;

}

//devuelve los bloques de un archivo en char*
char* bloques_de_archivo(char* ruta) {
	char** auxiliar = string_split(ruta, "\n");
	char** auxiliar2 = string_split(auxiliar[2], "=");
	char* blocks = auxiliar2[1];

	return blocks;
}

char* bloques_de_archivo_fisico(char* ruta) {
	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char** auxiliar = string_split(archivo_addr, "\n");
	char** auxiliar2 = string_split(auxiliar[2], "=");
	char* blocks = auxiliar2[1];

	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	return blocks;
}
//lee un bloque y devuelve to do su texto
char* leo_el_bloque(t_bloque* bloque) {
	char* texto = string_new();

	int inicio = bloque->inicio;
	int fin = bloque->posicion_para_escribir;

	while (inicio < fin) {
		char *aux = string_from_format("%c", block_mmap[inicio]);

		string_append(&texto, aux);
		inicio++;
	}
	return texto;
}

char* leo_el_bloque_fisico(t_bloque* bloque) {
	char* dirBlocks = conseguir_ruta(BLOCKS);
	int archivo = open(dirBlocks, O_RDWR);
	struct stat statbuf;
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);

	char* texto = string_new();
	int inicio = bloque->inicio;
	int fin = bloque->posicion_para_escribir;
	while (inicio < fin) {
		char *aux = string_from_format("%c", archivo_addr[inicio]);
		string_append(&texto, aux);
		inicio++;
	}
	munmap(archivo_addr, statbuf.st_size);
	close(archivo);
	free(dirBlocks);
	return texto;
}

char* leo_el_bloque_incluyendo_espacios(t_bloque* bloque) {
	char* texto = string_new();
	char* dirBlocks = conseguir_ruta(BLOCKS);
	int inicio = bloque->inicio;
	int fin = bloque->fin;
	int archivo = open(dirBlocks, O_RDWR);
	struct stat info;
	fstat(archivo, &info);
	archivo_blocks_para_sabotaje = (char*) mmap(NULL, info.st_size,
	PROT_WRITE,
	MAP_SHARED, archivo, 0);
	while (inicio <= fin) {
		char *aux = string_from_format("%c",
				archivo_blocks_para_sabotaje[inicio]);

		string_append(&texto, aux);
		inicio++;
	}
	close(archivo);
	free(dirBlocks);
	return texto;
}
//devuelve el texto de TODOS los bloques
char* contenido_de_bloques(char* bloques) {
	t_bloque* bloque;
	char** auxiliar = string_split(bloques, ",");
	char* texto_todos_los_bloques = string_new();
	int i = 0;
	while (auxiliar[i]) {
		bloque = list_get(disco_logico->bloques, atoi(auxiliar[i]) - 1);
		char* texto = leo_el_bloque(bloque);

		string_append(&texto_todos_los_bloques, texto);
		i++;
		free(texto);
	}

	return texto_todos_los_bloques;
}

char* contenido_de_bloques_fisico(char* bloques) {

	t_bloque* bloque;
	char** auxiliar = string_split(bloques, ",");
	char* texto_todos_los_bloques = string_new();
	int i = 0;
	char* texto;
	while (atoi(auxiliar[i])) {

		bloque = list_get(disco_logico->bloques, atoi(auxiliar[i]) - 1);
		texto = leo_el_bloque_fisico(bloque);

		string_append(&texto_todos_los_bloques, texto);
		i++;
		free(texto);
	}
	return texto_todos_los_bloques;
}

char* leer_md5file(char* ruta) {
	char* md5 = string_new();
	char** archivo_dividido = string_split(ruta, "\n");
	char** valor_md5 = string_split(archivo_dividido[4], "=");
	string_append(&md5, valor_md5[1]);
	return md5;
}

//para comprobar si un array de bloques en char* contiene a un bloque en char*
bool contiene(char* bloques, char* bloque) {
	bool flag = false;
	char** bloques_separados = string_split(bloques, ",");
	int i = 0;
	while (bloques_separados[i]) {

		if (string_equals_ignore_case(bloques_separados[i], bloque)) {
			flag = true;
		}
		i++;
	}
	return flag;
}

//actualiza un archivo si se agregan bloques o cambia su tamaño
void actualizar_el_archivo(files file, char* cadena, t_bloque* bloque) {
	char* cantidad_bloques;
	char* bloques;
	char* size;
	bool contiene_bloque;
	char* caracter_llenado;
	char* contenido_de_los_bloques;
	char* md5;
	char* texto;

	switch (file) {
	case OXIGENO:
		;
		bloques = bloques_de_archivo(archivoOxigeno);
		contiene_bloque = contiene(bloques, string_itoa(bloque->id_bloque));

		if (!contiene_bloque) {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoOxigeno)) + 1);
		} else {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoOxigeno)));
		}

		size = string_itoa(
				atoi(size_de_archivo(archivoOxigeno)) + string_length(cadena));
		if (!contiene_bloque) {
			string_append(&bloques, string_itoa(bloque->id_bloque));
		} else {
			bloques = string_substring_until(bloques,
					string_length(bloques) - 1);
		}

		caracter_llenado = string_substring_until(cadena, 1);
		contenido_de_los_bloques = contenido_de_bloques(bloques);
		//sem_wait(&semaforo_generar_md5);
		md5 = generarMD5(contenido_de_los_bloques);
		//sem_post(&semaforo_generar_md5);
		string_append(&bloques, ",");
		texto =
				string_from_format(
						"SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
						size, cantidad_bloques, bloques, caracter_llenado, md5);
		archivoOxigeno = realloc(archivoOxigeno, string_length(texto) + 1);
		memcpy(archivoOxigeno, texto, string_length(texto) + 1);
		break;
	case COMIDA:
		;
		bloques = bloques_de_archivo(archivoComida);
		contiene_bloque = contiene(bloques, string_itoa(bloque->id_bloque));

		if (!contiene_bloque) {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoComida)) + 1);
		} else {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoComida)));
		}

		size = string_itoa(
				atoi(size_de_archivo(archivoComida)) + string_length(cadena));
		if (!contiene_bloque) {
			string_append(&bloques, string_itoa(bloque->id_bloque));
		} else {
			bloques = string_substring_until(bloques,
					string_length(bloques) - 1);
		}

		caracter_llenado = string_substring_until(cadena, 1);
		contenido_de_los_bloques = contenido_de_bloques(bloques);
		//sem_wait(&semaforo_generar_md5);
		md5 = generarMD5(contenido_de_los_bloques);
		//sem_post(&semaforo_generar_md5);
		string_append(&bloques, ",");
		texto =
				string_from_format(
						"SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
						size, cantidad_bloques, bloques, caracter_llenado, md5);
		archivoComida = realloc(archivoComida, string_length(texto) + 1);
		memcpy(archivoComida, texto, string_length(texto) + 1);
		break;
	case BASURA:
		;
		bloques = bloques_de_archivo(archivoBasura);
		contiene_bloque = contiene(bloques, string_itoa(bloque->id_bloque));

		if (!contiene_bloque) {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoBasura)) + 1);
		} else {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoBasura)));
		}

		size = string_itoa(
				atoi(size_de_archivo(archivoBasura)) + string_length(cadena));
		if (!contiene_bloque) {
			string_append(&bloques, string_itoa(bloque->id_bloque));
		} else {
			bloques = string_substring_until(bloques,
					string_length(bloques) - 1);
		}

		caracter_llenado = string_substring_until(cadena, 1);
		contenido_de_los_bloques = contenido_de_bloques(bloques);
		//sem_wait(&semaforo_generar_md5);
		md5 = generarMD5(contenido_de_los_bloques);
		//sem_post(&semaforo_generar_md5);
		string_append(&bloques, ",");
		texto =
				string_from_format(
						"SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
						size, cantidad_bloques, bloques, caracter_llenado, md5);
		archivoBasura = realloc(archivoBasura, string_length(texto) + 1);
		memcpy(archivoBasura, texto, string_length(texto) + 1);
		break;
	}
}

//borra la cantidad de caracteres que se desean de un archivo
void actualizar_archivo_borrado(files file, int cadena, bool flag,
		char caracter) {
	char* cantidad_bloques;
	char* bloquesaux;
	char* contenido_de_los_bloques;
	char* md5;
	char* bloques;
	char** bloques_divididos;
	char* size;
	char* caracter_llenado;
	char* texto;
	int tamanio_texto;
	switch (file) {

	case OXIGENO:
		bloques = bloques_de_archivo(archivoOxigeno);
		if (!flag) {
			cantidad_bloques = cantidad_de_bloques_de_archivo(archivoOxigeno);
		} else {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoOxigeno)) - 1);
			bloques_divididos = string_split(bloques, ",");
			int i = 0;
			bloques = string_new();
			while (i < atoi(cantidad_bloques)) {
				string_append(&bloques, bloques_divididos[i]);
				string_append(&bloques, ",");
				i++;
			}
		}
		size = string_itoa(atoi(size_de_archivo(archivoOxigeno)) - cadena);

		caracter_llenado = string_from_format("%c", caracter);
		if (!string_is_empty(bloques)) {
			bloquesaux = string_substring_until(bloques,
					string_length(bloques) - 1);
			contenido_de_los_bloques = contenido_de_bloques(bloquesaux);
			//sem_wait(&semaforo_generar_md5);
			md5 = generarMD5(contenido_de_los_bloques);
			//sem_post(&semaforo_generar_md5);
		}
		texto =
				string_from_format(
						"SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
						size, cantidad_bloques, bloques, caracter_llenado, md5);
		tamanio_texto = string_length(texto);
		free(bloques);
		archivoOxigeno = realloc(archivoOxigeno, string_length(texto) + 1);
		memcpy(archivoOxigeno, texto, string_length(texto) + 1);
		break;

	case COMIDA:
		bloques = bloques_de_archivo(archivoComida);
		if (!flag) {
			cantidad_bloques = cantidad_de_bloques_de_archivo(archivoComida);
		} else {
			cantidad_bloques = string_itoa(
					atoi(cantidad_de_bloques_de_archivo(archivoComida)) - 1);
			bloques_divididos = string_split(bloques, ",");
			int i = 0;
			bloques = string_new();
			while (i < atoi(cantidad_bloques)) {
				string_append(&bloques, bloques_divididos[i]);
				string_append(&bloques, ",");
				i++;
			}
		}
		size = string_itoa(atoi(size_de_archivo(archivoComida)) - cadena);

		caracter_llenado = string_from_format("%c", caracter);
		if (!string_is_empty(bloques)) {
			bloquesaux = string_substring_until(bloques,
					string_length(bloques) - 1);
			contenido_de_los_bloques = contenido_de_bloques(bloquesaux);
			//sem_wait(&semaforo_generar_md5);
			md5 = generarMD5(contenido_de_los_bloques);
			//sem_post(&semaforo_generar_md5);
		}
		texto =
				string_from_format(
						"SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
						size, cantidad_bloques, bloques, caracter_llenado, md5);
		int tamanio_texto = string_length(texto);
		free(bloques);
		archivoComida = realloc(archivoComida, string_length(texto) + 1);
		memcpy(archivoComida, texto, string_length(texto) + 1);
		break;
	}
}

//escribe lo que se desea en un archivo
void escribir_el_archivo(files file, char* cadena, t_bloque* bloque) {
	if (bloque->espacio >= string_length(cadena)) {
		escribir_en_block(cadena, bloque);
		actualizar_el_archivo(file, cadena, bloque);
	} else {
		char *lo_que_entra_en_el_bloque = string_substring_until(cadena,
				bloque->espacio);
		char *lo_que_falta_escribir = string_substring_from(cadena,
				string_length(lo_que_entra_en_el_bloque));
		t_bloque* nuevo_bloque;
		escribir_en_block(lo_que_entra_en_el_bloque, bloque);
		actualizar_el_archivo(file, lo_que_entra_en_el_bloque, bloque);
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		nuevo_bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque);
		escribir_el_archivo(file, lo_que_falta_escribir, nuevo_bloque);
	}

}

//recupera el bloque en forma de bloque de un archivo
t_bloque* recuperar_ultimo_bloque(char* ruta) {

	t_bloque* el_bloque;
	int cantidad_de_bloques = 0;
	char* sizeArchivo = size_de_archivo(ruta);
	if (atoi(sizeArchivo) == 0) {
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		el_bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque);
	} else {
		char ** renglones_file = string_split(ruta, "\n");
		char **linea_bloques_file = string_split(renglones_file[2], "=");
		char **bloques_file = string_split(linea_bloques_file[1], ",");
		while (bloques_file[cantidad_de_bloques]) {
			cantidad_de_bloques++;
		}
		el_bloque = (t_bloque *) list_get(disco_logico->bloques,
				atoi(bloques_file[cantidad_de_bloques - 2]) - 1);
	}
	return el_bloque;
}

//borra los caracteres de un archivo y ajusta los bloques
void borrar_del_archivo(files file, int cant_borrar, t_bloque* bloque,
		char caracter) {
	int caracteresOcupados = caracteres_ocupados(bloque->inicio, bloque->fin);

	if (cant_borrar < caracteresOcupados) {
		for (int i = 0; i < cant_borrar; i++) {
			block_mmap[bloque->posicion_para_escribir - 1] = ' ';
			bloque->espacio++;
			bloque->posicion_para_escribir--;
		}
		actualizar_archivo_borrado(file, cant_borrar, false, caracter);
	} else {
		//borrar_todo y actualizar el archivo
		for (int i = bloque->posicion_para_escribir - 1; i >= bloque->inicio;
				i--) {
			block_mmap[i] = ' ';
		}
		bloque->espacio = bloque->fin - bloque->inicio + 1;
		bloque->posicion_para_escribir = bloque->inicio;
		actualizar_archivo_borrado(file, cant_borrar, true, caracter);
		pthread_mutex_lock(&mutex_bitmap);
		bitarray_clean_bit(bitmap, bloque->id_bloque - 1);
		pthread_mutex_unlock(&mutex_bitmap);
	}
	log_info(mongoLogger, "Se borraron %d bytes en el bloque %d, "
			"le quedan %d bytes disponibles.", cant_borrar, bloque->id_bloque,
			bloque->espacio);
}

//avisa que el archivo no existe por log
void avisar_que_no_existe(char* ruta) {
	log_info(mongoLogger, "El archivo %s al que intenta borrar no existe",
			ruta);
}

//vacia to do un bloque
void vaciar_bloque(t_bloque* bloque) {
	for (int i = bloque->posicion_para_escribir - 1; i >= bloque->inicio; i--)
		block_mmap[i] = ' ';
}

//comprueba si el archivo existe y si existe elimina los caracteres deseados
void eliminar_del_archivo(files file, int cant_borrar, char caracter) {
	char* contenido_total;
	char* bloques;
	int lo_que_puedo_borrar;
	int lo_que_falta;
	int cantidadOcupada;
	char* bloquesaux;
	char** bloques_divididos;
	char* el_caracter;
	char* cadena;
	t_bloque* bloque;
	int cantidad_de_bloques;

	switch (file) {
	case OXIGENO:
		if (atoi(size_de_archivo(archivoOxigeno)) > 0) {
			bloques = bloques_de_archivo(archivoOxigeno);
			bloquesaux = string_substring_until(bloques,
					string_length(bloques) - 1);
			contenido_total = contenido_de_bloques(bloquesaux);
			char* rutaOxigeno = conseguir_ruta(OXIGENOO);

			if (string_length(contenido_total) >= cant_borrar) {
				//recupero el ultimo bloque
				bloque = recuperar_ultimo_bloque(archivoOxigeno);
				//int cantidad_ocupada=bloque->posicion_para_escribir-bloque->inicio-1;
				cantidadOcupada = caracteres_ocupados(bloque->inicio,
						bloque->fin);
				//me fijo si la cantidad ocupada es mayor que la cantidad a borrar
				if (cantidadOcupada >= cant_borrar) {
					borrar_del_archivo(OXIGENO, cant_borrar, bloque, caracter);
				} else {
					lo_que_puedo_borrar = cantidadOcupada;
					//sino borro lo que puedo
					borrar_del_archivo(OXIGENO, lo_que_puedo_borrar, bloque,
							caracter);
					lo_que_falta = cant_borrar - lo_que_puedo_borrar;
					//elimino del archivo lo que falta
					eliminar_del_archivo(OXIGENO, lo_que_falta, caracter);
				}
			} else {
				printf("vas a borrar mas de la cuenta \n");
				bloques_divididos = string_split(bloques, ",");
				cantidad_de_bloques = atoi(
						cantidad_de_bloques_de_archivo(archivoOxigeno));
				//inicializar_archivo(ruta,0,caracter);
				inicializar_archivo(rutaOxigeno, caracter);
				for (int i = 0; i < cantidad_de_bloques; i++) {
					bloque = list_get(disco_logico->bloques,
							atoi(bloques_divididos[i]) - 1);
					vaciar_bloque(bloque);
					bloque->espacio = bloque->fin - bloque->inicio + 1;
					bloque->posicion_para_escribir = bloque->inicio;
					pthread_mutex_lock(&mutex_bitmap);
					bitarray_clean_bit(bitmap, atoi(bloques_divididos[i]) - 1);
					pthread_mutex_unlock(&mutex_bitmap);
				}
				el_caracter = string_from_format("%c", caracter);
				cadena = string_new();
				string_append(&cadena, "SIZE=0");
				string_append(&cadena, "\nBLOCK_COUNT=");
				string_append(&cadena, "\nBLOCKS=");
				string_append(&cadena, "\nCARACTER_LLENADO=");
				string_append(&cadena, el_caracter);
				string_append(&cadena, "\nMD5_ARCHIVO=");
				archivoOxigeno = realloc(archivoOxigeno,
						string_length(cadena) + 1);
				memcpy(archivoOxigeno, cadena, string_length(cadena) + 1);
				log_info(mongoLogger, "se borro mas de la cuenta");
				free(cadena);

			}
			free(rutaOxigeno);
		}
		break;
	case COMIDA:
		if (atoi(size_de_archivo(archivoComida)) > 0) {
			char* rutaComida = conseguir_ruta(COMIDAA);
			bloques = bloques_de_archivo(archivoComida);
			bloquesaux = string_substring_until(bloques,
					string_length(bloques) - 1);
			contenido_total = contenido_de_bloques(bloquesaux);
			if (string_length(contenido_total) >= cant_borrar) {
				bloque = recuperar_ultimo_bloque(archivoComida);
				cantidadOcupada = caracteres_ocupados(bloque->inicio,
						bloque->fin);
				if (cantidadOcupada >= cant_borrar) {
					borrar_del_archivo(COMIDA, cant_borrar, bloque, caracter);
				} else {
					lo_que_puedo_borrar = cantidadOcupada;
					borrar_del_archivo(COMIDA, lo_que_puedo_borrar, bloque,
							caracter);
					lo_que_falta = cant_borrar - lo_que_puedo_borrar;
					eliminar_del_archivo(COMIDA, lo_que_falta, caracter);
				}
			} else {
				printf("vas a borrar mas de la cuenta \n");
				bloques_divididos = string_split(bloques, ",");
				cantidad_de_bloques = atoi(
						cantidad_de_bloques_de_archivo(archivoComida));
				inicializar_archivo(rutaComida, caracter);
				for (int i = 0; i < cantidad_de_bloques; i++) {
					bloque = list_get(disco_logico->bloques,
							atoi(bloques_divididos[i]) - 1);
					vaciar_bloque(bloque);
					bloque->espacio = bloque->fin - bloque->inicio + 1;
					bloque->posicion_para_escribir = bloque->inicio;
					pthread_mutex_lock(&mutex_bitmap);
					bitarray_clean_bit(bitmap, atoi(bloques_divididos[i]) - 1);
					pthread_mutex_unlock(&mutex_bitmap);
				}
				el_caracter = string_from_format("%c", caracter);
				cadena = string_new();
				string_append(&cadena, "SIZE=0");
				string_append(&cadena, "\nBLOCK_COUNT=");
				string_append(&cadena, "\nBLOCKS=");
				string_append(&cadena, "\nCARACTER_LLENADO=");
				string_append(&cadena, el_caracter);
				string_append(&cadena, "\nMD5_ARCHIVO=");
				archivoComida = realloc(archivoComida,
						string_length(cadena) + 1);
				memcpy(archivoComida, cadena, string_length(cadena) + 1);
				log_info(mongoLogger, "se borro mas de la cuenta");
				free(cadena);

			}
			free(rutaComida);
		}
		break;
	case BASURA:
		;
		char * rutaBasura = conseguir_ruta(BASURAA);
		bloques = bloques_de_archivo(archivoBasura);
		bloques_divididos = string_split(bloques, ",");
		cantidad_de_bloques = atoi(
				cantidad_de_bloques_de_archivo(archivoBasura));
		for (int i = 0; i < cantidad_de_bloques; i++) {
			bloque = list_get(disco_logico->bloques,
					atoi(bloques_divididos[i]) - 1);
			vaciar_bloque(bloque);
			bloque->espacio = bloque->fin - bloque->inicio + 1;
			bloque->posicion_para_escribir = bloque->inicio;
			pthread_mutex_lock(&mutex_bitmap);
			bitarray_clean_bit(bitmap, atoi(bloques_divididos[i]) - 1);
			pthread_mutex_unlock(&mutex_bitmap);
		}
		free(archivoBasura);
		g_existe_file_basura = false;
		remove(rutaBasura);
		log_info(mongoLogger, "se elimino el archivo basura");
		free(rutaBasura);
		break;
	}
}

//crea el archivo de generar oxigeno y escribe en el
void generar_oxigeno(int cantidad) {
	char* cadena = string_repeat('O', cantidad);
	char* rutaOxigeno = conseguir_ruta(OXIGENOO);
	int existeArchivo = access(rutaOxigeno, F_OK);
	t_bloque* bloque;
	sem_wait(&semaforo_para_file_oxigeno);
	if (!g_existe_file_oxigeno) {
		//inicializar_archivo(ruta_oxigeno,cantidad, 'O');
		inicializar_archivo(rutaOxigeno, 'O');
		g_existe_file_oxigeno = true;
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque);
		//Si no existe el archivo, crearlo y asignarle el carácter de llenado O
		g_en_uso_file_oxigeno = true;
		int archivo = open(rutaOxigeno, O_RDWR);
		struct stat statbuf;
		fstat(archivo, &statbuf);
		char *archivo_addr = mmap(NULL, statbuf.st_size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED, archivo, 0);
		archivoOxigeno = malloc(string_length(archivo_addr) + 1);
		memcpy(archivoOxigeno, archivo_addr, string_length(archivo_addr) + 1);
		g_abierto_file_oxigeno = true;
		munmap(archivo_addr, statbuf.st_size);
		close(archivo);
		escribir_el_archivo(OXIGENO, cadena, bloque);
		g_modificado_file_oxigeno = true;
	}
	//Agregar tantos caracteres de llenado del archivo como indique el parámetro CANTIDAD
	else {
		if (g_abierto_file_oxigeno) {
			g_en_uso_file_oxigeno = true;
			bloque = recuperar_ultimo_bloque(archivoOxigeno);
			escribir_el_archivo(OXIGENO, cadena, bloque);
			g_modificado_file_oxigeno = true;
		} else {
			g_en_uso_file_oxigeno = true;
			int archivo = open(rutaOxigeno, O_RDWR);
			struct stat statbuf;
			fstat(archivo, &statbuf);
			char *archivo_addr = mmap(NULL, statbuf.st_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, archivo, 0);
			archivoOxigeno = malloc(string_length(archivo_addr) + 1);
			memcpy(archivoOxigeno, archivo_addr,
					string_length(archivo_addr) + 1);
			g_abierto_file_oxigeno = true;
			munmap(archivo_addr, statbuf.st_size);
			close(archivo);
			bloque = recuperar_ultimo_bloque(archivoOxigeno);
			escribir_el_archivo(OXIGENO, cadena, bloque);
			g_modificado_file_oxigeno = true;
		}
	}
	free(rutaOxigeno);
	g_en_uso_file_oxigeno = false;
	close(existeArchivo);
	sem_post(&semaforo_para_file_oxigeno);
}
//comprueba el archivo de generar oxigeno y borra en el
void consumir_oxigeno(int cant_borrar) {
	char* rutaOxigeno = conseguir_ruta(OXIGENOO);
	sem_wait(&semaforo_para_file_oxigeno);
	if (g_existe_file_oxigeno) {
		g_en_uso_file_oxigeno = true;
		if (g_abierto_file_oxigeno) {
			eliminar_del_archivo(OXIGENO, cant_borrar, 'O');
			g_modificado_file_oxigeno = true;
		} else {
			int archivo = open(rutaOxigeno, O_RDWR);
			struct stat statbuf;
			fstat(archivo, &statbuf);
			char *archivo_addr = mmap(NULL, statbuf.st_size,
			PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);
			archivoOxigeno = malloc(string_length(archivo_addr) + 1);
			memcpy(archivoOxigeno, archivo_addr,
					string_length(archivo_addr) + 1);
			g_abierto_file_oxigeno = true;
			munmap(archivo_addr, statbuf.st_size);
			close(archivo);
			eliminar_del_archivo(OXIGENO, cant_borrar, 'O');
			g_modificado_file_oxigeno = true;
		}
		g_en_uso_file_oxigeno = false;
	} else {
		avisar_que_no_existe(rutaOxigeno);
	}
	sem_post(&semaforo_para_file_oxigeno);
	free(rutaOxigeno);
}

//crea el archivo de generar comida y escribe en el
void generar_comida(int cantidad) {
	char* cadena = string_repeat('C', cantidad);
	char* rutaComida = conseguir_ruta(COMIDAA);
	int existeArchivo = access(rutaComida, F_OK);
	t_bloque* bloque;
	sem_wait(&semaforo_para_file_comida);
	if (!g_existe_file_comida) {

		inicializar_archivo(rutaComida, 'C');
		g_existe_file_comida = true;
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque);

		g_en_uso_file_comida = true;
		int archivo = open(rutaComida, O_RDWR);
		struct stat statbuf;
		fstat(archivo, &statbuf);
		char *archivo_addr = mmap(NULL, statbuf.st_size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED, archivo, 0);

		archivoComida = malloc(string_length(archivo_addr) + 1);
		memcpy(archivoComida, archivo_addr, string_length(archivo_addr) + 1);
		g_abierto_file_comida = true;
		munmap(archivo_addr, statbuf.st_size);
		close(archivo);
		escribir_el_archivo(COMIDA, cadena, bloque);
		g_modificado_file_comida = true;
	}

	else {
		if (g_abierto_file_comida) {
			g_en_uso_file_comida = true;
			bloque = recuperar_ultimo_bloque(archivoComida);
			escribir_el_archivo(COMIDA, cadena, bloque);
			g_modificado_file_comida = true;
		} else {
			g_en_uso_file_comida = true;
			int archivo = open(rutaComida, O_RDWR);
			struct stat statbuf;
			fstat(archivo, &statbuf);
			char *archivo_addr = mmap(NULL, statbuf.st_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, archivo, 0);
			archivoComida = malloc(string_length(archivo_addr) + 1);
			memcpy(archivoComida, archivo_addr,
					string_length(archivo_addr) + 1);
			g_abierto_file_comida = true;
			munmap(archivo_addr, statbuf.st_size);
			close(archivo);
			bloque = recuperar_ultimo_bloque(archivoComida);
			escribir_el_archivo(COMIDA, cadena, bloque);
			g_modificado_file_comida = true;
		}
	}
	free(rutaComida);
	g_en_uso_file_comida = false;
	close(existeArchivo);
	sem_post(&semaforo_para_file_comida);
}

//comprueba el archivo de generar comida y borra en el
void consumir_comida(int cant_borrar) {
	char* rutaComida = conseguir_ruta(COMIDAA);
	sem_wait(&semaforo_para_file_comida);
	if (g_existe_file_comida) {
		g_en_uso_file_comida = true;
		if (g_abierto_file_comida) {
			eliminar_del_archivo(COMIDA, cant_borrar, 'C');
			g_modificado_file_comida = true;
		} else {
			int archivo = open(rutaComida, O_RDWR);
			struct stat statbuf;
			fstat(archivo, &statbuf);
			char *archivo_addr = mmap(NULL, statbuf.st_size,
			PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);
			archivoComida = malloc(string_length(archivo_addr) + 1);
			memcpy(archivoComida, archivo_addr,
					string_length(archivo_addr) + 1);
			g_abierto_file_comida = true;
			munmap(archivo_addr, statbuf.st_size);
			close(archivo);
			eliminar_del_archivo(COMIDA, cant_borrar, 'O');
			g_modificado_file_comida = true;
		}
		g_en_uso_file_comida = false;
	} else {
		avisar_que_no_existe(rutaComida);
	}
	sem_post(&semaforo_para_file_comida);
	free(rutaComida);
}

//crea el archivo de generar basura y escribe en el
void generar_basura(int cantidad) {
	char* cadena = string_repeat('B', cantidad);
	char* rutaBasura = conseguir_ruta(BASURAA);
	int existeArchivo = access(rutaBasura, F_OK);
	t_bloque* bloque;
	sem_wait(&semaforo_para_file_basura);
	if (!g_existe_file_basura) {

		inicializar_archivo(rutaBasura, 'B');
		g_existe_file_basura = true;
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque);

		g_en_uso_file_basura = true;
		int archivo = open(rutaBasura, O_RDWR);
		struct stat statbuf;
		fstat(archivo, &statbuf);
		char *archivo_addr = mmap(NULL, statbuf.st_size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED, archivo, 0);

		archivoBasura = malloc(string_length(archivo_addr) + 1);
		memcpy(archivoBasura, archivo_addr, string_length(archivo_addr) + 1);
		g_abierto_file_basura = true;
		munmap(archivo_addr, statbuf.st_size);
		close(archivo);
		escribir_el_archivo(BASURA, cadena, bloque);
		g_modificado_file_basura = true;
	} else {
		if (g_abierto_file_basura) {
			g_en_uso_file_basura = true;
			bloque = recuperar_ultimo_bloque(archivoBasura);
			escribir_el_archivo(BASURA, cadena, bloque);
			g_modificado_file_basura = true;
		} else {
			g_en_uso_file_basura = true;
			int archivo = open(rutaBasura, O_RDWR);
			struct stat statbuf;
			fstat(archivo, &statbuf);
			char *archivo_addr = mmap(NULL, statbuf.st_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, archivo, 0);
			archivoBasura = malloc(string_length(archivo_addr) + 1);
			memcpy(archivoBasura, archivo_addr,
					string_length(archivo_addr) + 1);
			g_abierto_file_basura = true;
			munmap(archivo_addr, statbuf.st_size);
			close(archivo);
			bloque = recuperar_ultimo_bloque(archivoBasura);
			escribir_el_archivo(BASURA, cadena, bloque);
			g_modificado_file_basura = true;
		}
	}
	free(rutaBasura);
	g_en_uso_file_basura = false;
	close(existeArchivo);
	sem_post(&semaforo_para_file_basura);
}

//comprueba el archivo de generar basura y borra en el
void descartar_basura(int cant_borrar) {
	char* rutaBasura = conseguir_ruta(BASURAA);
	sem_wait(&semaforo_para_file_basura);
	if (g_existe_file_basura) {
		g_en_uso_file_basura = true;
		if (g_abierto_file_basura) {
			eliminar_del_archivo(BASURA, cant_borrar, 'O');
			g_modificado_file_basura = true;
		} else {
			int archivo = open(rutaBasura, O_RDWR);
			struct stat statbuf;
			fstat(archivo, &statbuf);
			char *archivo_addr = mmap(NULL, statbuf.st_size,
			PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);
			archivoBasura = malloc(string_length(archivo_addr) + 1);
			memcpy(archivoBasura, archivo_addr,
					string_length(archivo_addr) + 1);
			g_abierto_file_basura = true;
			munmap(archivo_addr, statbuf.st_size);
			close(archivo);
			eliminar_del_archivo(BASURA, cant_borrar, 'O');
			g_modificado_file_basura = true;
		}
		g_en_uso_file_basura = false;
	} else {
		avisar_que_no_existe(rutaBasura);
	}
	sem_post(&semaforo_para_file_basura);
	free(rutaBasura);
}

/*arma la direccion de la bitacora dependiendo la accion que realiza.
 * porque por ejemplo las estructuras de tareas y movimiento son difernetes,
 * entonces los separo dependiendo de la accion q realiza*/
char *rutaBitacoraDelTripulante(tripulante_con_su_accion *tripulante) {
	char *rutaBitacora;
	char* dirBitacora = conseguir_ruta(BITACORA);
	switch (tripulante->accion) {

	case ACTUALIZAR_POSICION:
		;
		m_movimiento_tripulante *tripulante_mov =
				(m_movimiento_tripulante *) tripulante->tripulante;
		rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
				dirBitacora, tripulante_mov->idTripulante,
				tripulante_mov->idPatota);
		break;

	case INICIO_TAREA:
		;
		m_estado_tarea_tripulante *tripulante_inicio_tarea =
				(m_estado_tarea_tripulante*) tripulante->tripulante;
		rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
				dirBitacora, tripulante_inicio_tarea->idTripulante,
				tripulante_inicio_tarea->numPatota);
		break;

	case FIN_TAREA:
		;
		m_estado_tarea_tripulante *tripulante_fin_tarea =
				(m_estado_tarea_tripulante*) tripulante->tripulante;
		rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
				dirBitacora, tripulante_fin_tarea->idTripulante,
				tripulante_fin_tarea->numPatota);
		break;

	case INICIO_SABOTAJE:
		;
		m_movimiento_tripulante *tripulante_sab =
				(m_movimiento_tripulante *) tripulante->tripulante;
		rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
				dirBitacora, tripulante_sab->idTripulante,
				tripulante_sab->idPatota);
		break;

	case FIN_SABOTAJE:
		;
		m_movimiento_tripulante *tripulante_fin_sab =
				(m_movimiento_tripulante *) tripulante->tripulante;
		rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
				dirBitacora, tripulante_fin_sab->idTripulante,
				tripulante_fin_sab->idPatota);
		break;
	default:
		log_error(mongoLogger, "Error al buscar la bitacora del tripulante!");
		break;

	}
	free(dirBitacora);
	return rutaBitacora;
}

char *generarTextoAEscribir(tripulante_con_su_accion *tripulante) {
	char *lo_que_se_va_a_escribir;

	switch (tripulante->accion) {

	case ACTUALIZAR_POSICION:
		;
		m_movimiento_tripulante *tripulante_mov =
				(m_movimiento_tripulante *) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format(
				"Se mueve de %d | %d a %d | %d\n", tripulante_mov->origenX,
				tripulante_mov->origenY, tripulante_mov->destinoX,
				tripulante_mov->destinoY);
		break;

	case INICIO_TAREA:
		;
		m_estado_tarea_tripulante *tripulante_inicio_tarea =
				(m_estado_tarea_tripulante*) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format("Inicia la tarea %s\n",
				tripulante_inicio_tarea->nombreTarea);
		break;

	case FIN_TAREA:
		;
		m_estado_tarea_tripulante *tripulante_fin_tarea =
				(m_estado_tarea_tripulante*) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format("Finaliza la tarea %s\n",
				tripulante_fin_tarea->nombreTarea);
		break;

	case INICIO_SABOTAJE:
		;
		m_movimiento_tripulante *tripulante_sab =
				(m_movimiento_tripulante *) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format(
				"Se corre al sabotaje en ubicacion (%d, %d)\n",
				tripulante_sab->destinoX, tripulante_sab->destinoY);
		break;

	case FIN_SABOTAJE:
		;
		lo_que_se_va_a_escribir = string_from_format(
				"Se resuelve el sabotaje\n");
		break;
	default:
		log_error(mongoLogger, "Error al generar el tipo de texto a escribir!");
		lo_que_se_va_a_escribir = NULL;
		break;
	}
	return lo_que_se_va_a_escribir;
}

void actualizar_tamanio_bitacora(char* rutaBitacora, int tamanio_bitacora) {
	int archivo = open(rutaBitacora, O_RDWR);
	struct stat statbuf;
	//ftruncate(archivo, string_length(tamanio_mensaje));
	fstat(archivo, &statbuf);
	char *archivo_addr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);
	char** renglones = string_split(archivo_addr,"\n");
	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	char** size=string_split(renglones[0],"=");
	int tamanio=atoi(size[1])+tamanio_bitacora;
	char** bloques=string_split(renglones[1],"=");
	char* cadena = string_from_format("SIZE=%d\nBLOCKS=%s",tamanio,bloques[1]);
	archivo = open(rutaBitacora, O_RDWR);
	ftruncate(archivo, string_length(cadena));
	fstat(archivo, &statbuf);
	archivo_addr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
	MAP_SHARED, archivo, 0);

	for (int i = 0; i < string_length(cadena);i++){
		archivo_addr[i]=cadena[i];
	}
	munmap(archivo_addr, string_length(cadena));
	close(archivo);
}


void escribir_el_mensaje_del_tripulante(char* mensaje, t_bloque *bloque,
		char* rutaBitacora) {
	if (string_length(mensaje) <= bloque->espacio) {
		escribir_en_block(mensaje, bloque);
		actualizar_tamanio_bitacora(rutaBitacora,string_length(mensaje));
	} else {
		char* lo_que_puedo_escribir = string_substring_until(mensaje,
				bloque->espacio);
		char* lo_que_falta_escribir = string_substring_from(mensaje,
				string_length(lo_que_puedo_escribir));
		escribir_en_block(lo_que_puedo_escribir, bloque);

		actualizar_tamanio_bitacora(rutaBitacora,string_length(lo_que_puedo_escribir));
		int numero_nuevo_bloque = obtener_bloque_libre();
		t_bloque* nuevo_bloque = list_get(disco_logico->bloques,
				numero_nuevo_bloque);
		agregar_bloque_bitacora(rutaBitacora, nuevo_bloque->id_bloque);
		escribir_el_mensaje_del_tripulante(lo_que_falta_escribir, nuevo_bloque,
				rutaBitacora);

	}
}

void escribir_en_su_bitacora_la_accion(tripulante_con_su_accion *tripulante) {
	t_bloque *el_bloque;
	t_bloque *nuevo_bloque;
	char *bloque;
	int numero_del_nuevo_bloque;
	//armo toda la cadena que se va a escribir por movimiento
	char *rutaBitacora = rutaBitacoraDelTripulante(tripulante);
	char *lo_que_se_va_a_escribir = generarTextoAEscribir(tripulante);
	int existeArchivo = access(rutaBitacora, F_OK);
	//si la bitacora del tripulante existe, entonces recupero el ultimo bloque
	if (existeArchivo == 0) {
		log_info(mongoLogger,
				"El archivo del tripulante existe, buscando su ultimo bloque");

		//ultimo bloque en char
		el_bloque = buscar_ultimo_bloque_del_tripulante(rutaBitacora);
		log_info(mongoLogger, "ultimo bloque: %d", el_bloque->id_bloque);
		//si hay espacio en el bloque para escribir to do, entonces lo escribo
		escribir_el_mensaje_del_tripulante(lo_que_se_va_a_escribir,el_bloque, rutaBitacora);

		/*if (el_bloque->espacio >= string_length(lo_que_se_va_a_escribir)) {
			escribir_en_block(lo_que_se_va_a_escribir, el_bloque);
		}
		//sino escribo una parte y elijo otro bloque para lo restante
		else {
			char *lo_que_entra_en_el_bloque = string_substring_until(
					lo_que_se_va_a_escribir, el_bloque->espacio);
			if (string_length(lo_que_entra_en_el_bloque) > 0) {
				escribir_el_mensaje_del_tripulante(lo_que_entra_en_el_bloque,
						el_bloque, rutaBitacora);
				//escribir_en_block(lo_que_entra_en_el_bloque, el_bloque);
			}
			char *lo_que_falta_escribir = string_substring_from(
					lo_que_se_va_a_escribir,
					string_length(lo_que_entra_en_el_bloque));

			escribir_el_mensaje_del_tripulante(lo_que_falta_escribir, el_bloque,
					rutaBitacora);*/
		//}
	} else {
		//asigno el bloque nuevo
		numero_del_nuevo_bloque = obtener_bloque_libre();

		nuevo_bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque);
		log_info(mongoLogger, "Asigno el bloque numero:%d\n",
				nuevo_bloque->id_bloque);

		inicializar_bitacora(rutaBitacora,
				string_itoa(nuevo_bloque->id_bloque));

		//escribo lo_que_se_va_a_escribir en block
		escribir_el_mensaje_del_tripulante(lo_que_se_va_a_escribir,nuevo_bloque, rutaBitacora);
		/*if (string_length(lo_que_se_va_a_escribir) <= nuevo_bloque->espacio) {
			escribir_en_block(lo_que_se_va_a_escribir, nuevo_bloque);
		} else {
			escribir_el_mensaje_del_tripulante(lo_que_se_va_a_escribir,
					nuevo_bloque, rutaBitacora);
		}*/
	}

	free(tripulante->tripulante);
	free(tripulante);
	free(rutaBitacora);
	free(lo_que_se_va_a_escribir);
}

int cantidad_bloques_a_ocupar(char* texto) {
	//int cantidad = (double)ceil(string_length(texto)/block_size);
	double cantidad = (double) ceil(
			(double) string_length(texto) / (double) 256);
	return cantidad;
}

