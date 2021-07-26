#include "bitmap.c"
#include "mongo-store.h"



int main() {

	/*mongoConfig = config_create(PATH_MONGO_STORE_CONFIG);
	struct stat statbuf;
	int archivo = open("/home/utnso/workspace/mnt/Blocks.ims", O_RDWR);
	fstat(archivo,&statbuf);
	block_mmap=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	crearEstructuraDiscoLogico();
	crearEstructurasBloques();


	int cantidad_de_bloques=list_size(disco_logico->bloques);
	for(int i=0;i<10;i++){
		t_bloque* bloque = malloc(sizeof(t_bloque));
		bloque=list_get(disco_logico->bloques,i);
		int a=ultima_posicion_escrita(bloque->inicio,bloque->fin);
		bloque->espacio=bloque->espacio-a;
		bloque->posicion_para_escribir=bloque->posicion_para_escribir+a;
		printf("el bloque %d tiene %d lugares y empieza a escribir en %d\n",bloque->id_bloque,bloque->espacio,bloque->posicion_para_escribir);
	}*/

	//////////////////////prueba/////////////////
	/*sem_init(&semaforo_bitmap, 0, 1);
	sem_init(&semaforo_bitacora, 0, 1);

	m_movimiento_tripulante *tripulante=malloc(sizeof(m_movimiento_tripulante));
	t_bloque* bloque1 = malloc(sizeof(t_bloque));
	t_bloque* bloque2 = malloc(sizeof(t_bloque));
	t_bloque* bloque3 = malloc(sizeof(t_bloque));
	t_bloque* bloque4 = malloc(sizeof(t_bloque));
	t_bloque* bloque5 = malloc(sizeof(t_bloque));
	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG);
	void* puntero_a_bits =malloc(1); //un byte de memoria, como por ejemplo malloc(1)
	sem_wait(&semaforo_bitmap);
	bitmap=bitarray_create(puntero_a_bits, 1);
	for(int i = 0; i<8;i++){
				bitarray_clean_bit(bitmap, i);
				printf("bitmap: %d\n",bitarray_test_bit(bitmap, i));
	}

	sem_post(&semaforo_bitmap);
	printf("cantidad de bloques: %d\n",bitarray_get_max_bit(bitmap));
	//bool a=bitarray_test_bit(bitmap, 0);
	//printf("valor: %d\n",a);


	sem_wait(&semaforo_bitmap);
	bitarray_set_bit(bitmap, 1);//2
	sem_post(&semaforo_bitmap);

	sem_wait(&semaforo_bitmap);
	bitarray_set_bit(bitmap, 3);//4
	sem_post(&semaforo_bitmap);

	sem_wait(&semaforo_bitmap);
	bitarray_set_bit(bitmap, 0);//1
	sem_post(&semaforo_bitmap);

	sem_wait(&semaforo_bitmap);
	bitarray_set_bit(bitmap, 2);//3
	sem_post(&semaforo_bitmap);
	//a=bitarray_test_bit(bitmap, 0);
	//printf("valor: %d\n",a);
	//int bloqueLibre= obtener_bloque_libre(bitmap);
	//printf("bloque libre: %d\n",bloqueLibre);
	for(int i = 0; i<8;i++){
			printf("bitmap: %d\n",bitarray_test_bit(bitmap, i));
		}
	tripulante->destinoX=0;
	tripulante->destinoY=2;
	tripulante->idPatota=1;
	tripulante->idTripulante=1;
	tripulante->origenX=0;
	tripulante->origenY=0;
	bloque1->id_bloque=1;
	bloque1->inicio=0;
	bloque1->fin=255;
	bloque1->espacio=256;
	bloque1->posicion_para_escribir=bloque1->inicio;
	bloque2->id_bloque=2;
	bloque2->inicio=256;
	bloque2->fin=511;
	bloque2->espacio=256;
	bloque2->posicion_para_escribir=bloque2->inicio;
	bloque3->id_bloque=3;
	bloque3->inicio=512;
	bloque3->fin=767;
	bloque3->espacio=256;
	bloque3->posicion_para_escribir=bloque3->inicio;
	bloque4->id_bloque=4;
	bloque4->inicio=768;
	bloque4->fin=1023;
	bloque4->espacio=256;
	bloque4->posicion_para_escribir=bloque4->inicio;
	bloque5->id_bloque=5;
	bloque5->inicio=1024;
	bloque5->fin=1279;
	bloque5->espacio=256;
	bloque5->posicion_para_escribir=bloque5->inicio;

	crearEstructuraDiscoLogico();
	list_add(disco_logico->bloques,bloque1);
	list_add(disco_logico->bloques,bloque2);
	list_add(disco_logico->bloques,bloque3);
	list_add(disco_logico->bloques,bloque4);
	list_add(disco_logico->bloques,bloque5);
	struct stat statbuf;
	int archivo = open("/home/utnso/workspace/mnt/Blocks.ims", O_RDWR);
	fstat(archivo,&statbuf);
	block_mmap=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);



	munmap(block_mmap,statbuf.st_size);
	close(archivo);*/
	/////////////////////fin prueba/////////////////////////////////////

	//crearEstructuraFileSystem();

	iniciar_recursos_mongo();

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);

	gestionarCliente(socket_mongo_store );

	printf("SOCKET DISCO %d\n", socket_mongo_store);
	return EXIT_SUCCESS;
}

void iniciar_recursos_mongo(void) {
	signal(SIGUSR1, rutina); //Recepcion mensaje de sabotaje

	sem_init(&dar_orden_sabotaje, 0, 0);
	sem_init(&contador_sabotaje, 0, 1);
//	sem_init(&semaforo_bitmap, 0, 1); lo cambie por un mutex
	sem_init(&semaforo_bitacora, 0, 1);



	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server
	puerto = config_get_string_value(mongoConfig, "PUERTO");
	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_DEBUG);

	log_info(mongoLogger, "INICIANDO RECURSOS");

	crear_estructura_filesystem();
}

void mostrar_estado_bitarray(void) {
	for (int i = 1; i <= *g_blocks; i++) {
		if (i % 5 == 0)
			printf("%d\n", bitarray_test_bit(bitmap, i));
		else
			printf("%d", bitarray_test_bit(bitmap, i));

	}

	printf("\n");
}

//genera el MD5 pasandole por parametro el contenido de un archivo de file
//por ejemplo el contenido puede ser 'OOOOOOOOOOO'
char *generarMD5(char *contenido){
  int fd;
  char command[string_length(contenido) + 150]; //por las dudas q el path sea largo
  int retorno;
  struct stat statbuf;
  void *data;
  char *md5;
  char ruta_md5[200];
  char *punto_montaje = config_get_string_value(mongoConfig,"PUNTO_MONTAJE");

  sprintf(ruta_md5, "%s/Files/md5.txt", punto_montaje);

  sprintf(command, "echo -n %s | md5sum > %s", contenido, ruta_md5);

  retorno = system(command);

  if(retorno != 0)
    {
      printf("ERROR AL GENERAR EL MD5\n");
      exit(1);
    }

  fd = open("../MD5/md5.txt", O_RDWR);

  if (fd == -1){
    printf("ERROR AL ABRIR ARCHIVO\n");
    exit(1);
  }

  fstat(fd, &statbuf);

  data = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
  md5 = strdup(data);

  munmap(data, statbuf.st_size); //cierro el archivo

  return md5;
}

void crearEstructuraDiscoLogico(){
	//TODO poner semaforo para disco logico
	disco_logico=(t_disco_logico *)malloc(sizeof(t_disco_logico));
	disco_logico->bloques=list_create();
}

void crearEstructurasBloques(){
	//TODO poner semaforo para disco logico
	//tamanio_de_bloque=atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
	//cantidad_de_bloques=atoi(config_get_string_value(mongoConfig,"BLOCKS"));
	int _cantidad_de_bloques = (int)*g_blocks;

	for(int contador=1;contador <=_cantidad_de_bloques;contador++){
		t_bloque *bloque = (t_bloque *)malloc(sizeof(t_bloque));
		bloque->id_bloque=contador;
		bloque->inicio= (contador-1) * *g_block_size;
		bloque->fin = bloque->inicio+ (*g_block_size-1);
		bloque->espacio=*g_block_size;
		bloque->posicion_para_escribir=bloque->inicio;
		list_add(disco_logico->bloques,bloque);
		//disco_logico->bloques = bloque; //TODO CAMBIE ACA

	}

	log_info(mongoLogger, "Se creo estructura bloques!");

	log_info(mongoLogger, "Cantidad de bloques generados: %d", list_size(disco_logico->bloques));
}


t_bloque* buscar_ultimo_bloque_del_tripulante(char* rutaBitacora){

	t_bloque* el_bloque;
	int cantidad_de_bloques=0;
	struct stat statbuf;
	int archivo = open(rutaBitacora, O_RDWR);
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char **renglones_bitacora= string_split(archivo_addr, "=");

	char **bloques_bitacora= string_split(renglones_bitacora[2], ",");
	while(bloques_bitacora[cantidad_de_bloques] != NULL){
		cantidad_de_bloques++;
	}

	int indexUltimoBloque = atoi(bloques_bitacora[cantidad_de_bloques - 1]) - 1;

	el_bloque=(t_bloque *)list_get(disco_logico->bloques, indexUltimoBloque);

	log_info(mongoLogger, "El ultimo bloque asignado en la bitacora %s es: %d y esta en indice: %d",
				rutaBitacora, el_bloque->id_bloque, indexUltimoBloque);

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	return el_bloque;
}

void crear_archivo(char* ruta){
	FILE *archivo =fopen(ruta, "w");
	fclose(archivo);
}


void inicializar_bitacora(char *rutaBitacora, char *numeroDeBloque){
	char *block_size= config_get_string_value(mongoConfig,"BLOCK_SIZE");
	crear_archivo(rutaBitacora);
	int archivo = open(rutaBitacora, O_RDWR);
	struct stat statbuf;
	ftruncate(archivo, (off_t)14+string_length(block_size));
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char *cadena=string_new();
	string_append(&cadena,"SIZE=");
	string_append(&cadena,block_size);
	string_append(&cadena,"\nBLOCKS=");
	string_append(&cadena,numeroDeBloque);

	for(int i=0; i<string_length(cadena);i++){
		archivo_addr[i]=cadena[i];
	}
	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
}


void agregar_bloque_bitacora(char *rutaBitacora,int bloque){
	int archivo = open(rutaBitacora, O_RDWR);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	ftruncate(archivo,(off_t) statbuf.st_size + 2);

	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	archivo_addr[statbuf.st_size]=',';
	archivo_addr[statbuf.st_size+1]=bloque+'0';
	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
}

void crearBitMapLogico(){
	int cantBloques = atoi(config_get_string_value(mongoConfig,"BLOCKS"));

	int cantBytes = (double)ceil((double)cantBloques/8);

	void* puntero_a_bits = malloc(cantBytes);

	bitmap = bitarray_create(puntero_a_bits, cantBytes);
	for(int i = 1; i <= cantBloques; i++){
		bitarray_clean_bit(bitmap, i);
	}

	log_info(mongoLogger, "Se creo el Bitmap logico con %d Bytes", cantBytes);
}


void crearSuperbloque(char * dirSuperbloque){
	char *block_size= config_get_string_value(mongoConfig,"BLOCK_SIZE");
	char *blocks= config_get_string_value(mongoConfig,"BLOCKS");
		crear_archivo(dirSuperbloque);
		int archivo = open(dirSuperbloque, O_RDWR);
		struct stat statbuf;
		ftruncate(archivo, (off_t)19+string_length(block_size)+string_length(blocks));
		fstat(archivo,&statbuf);
		char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
		char *cadena=string_new();
		string_append(&cadena,"Block_size=");
		string_append(&cadena,block_size);
		string_append(&cadena,"\nBLOCKS=");
		string_append(&cadena,blocks);

		for(int i=0; i<string_length(cadena);i++){
			archivo_addr[i]=cadena[i];
		}
		munmap(archivo_addr,statbuf.st_size);
		close(archivo);
}

void crearblocks(char* dirBlocks){

	struct stat statbuf;
	//int block_size= atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
	int block_size  = (int) *g_block_size;
	//int blocks= atoi(config_get_string_value(mongoConfig,"BLOCKS"));
	int blocks      = (int) *g_blocks;
	crear_archivo(dirBlocks);
	int peso=(int)block_size*blocks;

	printf("PESO ES: %d\n", peso);

	FILE *fd;

	fd = fopen(dirBlocks, "w+");

	ftruncate(fileno(fd), peso);

	for(int i=0; i < peso; i++) {
		fputc(' ', fd);
	}

	fclose(fd);
/*
	int archivo = open(dirBlocks, O_RDWR);

	fstat(archivo, &statbuf);

	ftruncate(archivo, (off_t)peso);

	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);

	for (int i = 0; i < statbuf.st_size; i++) {
		archivo_addr[i] = ' ';
	}


	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
*/
	log_info(mongoLogger, "Se crearon los bloques!");
}

void crearCarpetaFile(char * dirFiles) {
	if (mkdir(dirFiles, 0777) == 0) {
		log_info(mongoLogger, "se creo el directorio file correctamente\n");
	} else
		log_error(mongoLogger,
				"Ha ocurrido un error al crear el directorio Files.");

}

void crearCarpetaBitacora(char * dirBitacora) {
	if (mkdir(dirBitacora, 0777) == 0) {
		log_info(mongoLogger, "se creo el directorio bitacora correctamente\n");
	} else
		log_error(mongoLogger,
				"Ha ocurrido un error al crear el directorio bitacora.");
}

int comprobar_que_todos_los_datos_existen(char* puntoMontaje){

	dirSuperbloque = string_new();
	string_append(&dirSuperbloque, puntoMontaje);
	string_append(&dirSuperbloque,"/SuperBloque.ims");
	int existeArchivoSuperBloque = access(dirSuperbloque, F_OK);

	dirFiles= string_new();
	string_append(&dirFiles, puntoMontaje);
	string_append(&dirFiles, "/Files");
	int existeArchivoFiles = access(dirFiles, F_OK);

	dirBitacora= string_new();
	string_append(&dirBitacora, puntoMontaje);
	string_append(&dirBitacora, "/Bitacora");
	int existeArchivoBitacora = access(dirBitacora, F_OK);

	dirBlocks= string_new();
	string_append(&dirBlocks, puntoMontaje);
	string_append(&dirBlocks, "/Blocks.ims");
	int existeArchivoBlocks = access(dirBlocks, F_OK);

	if(!existeArchivoSuperBloque && !existeArchivoBitacora && !existeArchivoFiles && !existeArchivoBlocks){
		printf("se recupero todo perfecto\n");
		return 1;
	}
	else {
		printf("no se recupero todo perfecto\n");

		return 0;
	}

}

void copiar_bitmap_de_disco(char* dirSuperbloque){
	struct stat statbuf;

	int fdm = open(dirSuperbloque, O_RDWR);

	if (fdm == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR ARCHIVO");
		exit(1);
	}

	fstat(fdm, &statbuf);
	superbloque = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fdm, 0);
	g_block_size = superbloque;
	g_blocks     = superbloque + sizeof(uint32_t);
	bitarrayEnChar = superbloque + 2 * sizeof(uint32_t);

	int cantLeer2 = (int) ceil((double) *g_blocks / 8);

	bitmap = bitarray_create(bitarrayEnChar, cantLeer2);

	//reviso las cosas

	log_info(mongoLogger, "SIZE GUARDADO EN SUPERBLOQUE: %d\n", (int)*g_block_size);
	log_info(mongoLogger, "CANT GUARDADO EN SUPERBLOQUE: %d\n", (int)*g_blocks);


	mostrar_estado_bitarray(); //usarlo cuando hay pocos bits

	log_info(mongoLogger, "Se bajaron los datos del Superbloque existente!");

}

void crearSuperbloqueNuevo(char *path){
	FILE *fd;
	fd = fopen(path, "wb");

	if (fd == NULL) {
		printf("ERROR AL ABRIR ARCHIVO");
		exit(1);
	}

	uint32_t prueba = (unsigned)atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));

	int cantBytes = (int)ceil((double)(*g_blocks)/8);

	printf("%d", cantBytes);

	fwrite(g_block_size, sizeof(uint32_t), 1, fd);
	fwrite(g_blocks, sizeof(uint32_t), 1, fd);
	fwrite(bitmap->bitarray, cantBytes, 1, fd);

	log_info(mongoLogger, "Se creo el superbloque nuevo!");

	//mostrar_estado_bitarray();


	//mostrar_estado_bitarray();

	//msync(superbloque, 2*sizeof(uint32_t) + cantBytes, MS_SYNC);
	//munmap(superbloque, 2*sizeof(uint32_t) + cantBytes);

	fclose(fd);
}

int ultima_posicion_escrita(int inicio,int fin){
	int ultima_posicion=inicio;
	int offset=0;
	int primera_posicion=inicio;
	while(primera_posicion<=fin){
		if(block_mmap[primera_posicion]!=' '){
			ultima_posicion=primera_posicion;
		}
		primera_posicion++;
	}
	offset=ultima_posicion-inicio;
	return offset;
}

void copiar_datos_de_bloques(t_list* bloques){
	int inicio, fin, offset;
	t_bloque *bloque;

	for(int i=0;i<list_size(bloques);i++){
		//agarro un bloque
		bloque=list_get(bloques,i);
		//veo donde empieza
		inicio = bloque->inicio;
		//veo donde termina
		fin = bloque->fin;
		//leo desde empieza hasta termina en bloc los caracteres escritos
		offset = ultima_posicion_escrita(inicio,fin);
		//resto espacio en bloque
		bloque->espacio=bloque->espacio-offset;
		//acomodo el puntero en bloque
		//bloque->posicion_para_escribir=bloque->posicion_para_escribir+offset+1;
		printf("Bloque %d inicio %d fin %d offset %d tiene espacio %d\n",
				bloque->id_bloque,
				bloque->inicio,
				bloque->fin,
				offset,
				bloque->espacio);
	}
}



void crear_estructura_filesystem(){
	puntoMontaje = config_get_string_value(mongoConfig, "PUNTO_MONTAJE");
	dirMetadata = string_new();
	string_append(&dirMetadata, puntoMontaje);
	string_append(&dirMetadata,"/Metadata");

	dirSuperbloque = string_new();
	string_append(&dirSuperbloque, puntoMontaje);
	string_append(&dirSuperbloque,"/SuperBloque.ims");

	dirFiles= string_new();
	string_append(&dirFiles, puntoMontaje);
	string_append(&dirFiles, "/Files");

	dirBitacora= string_new();
	string_append(&dirBitacora, puntoMontaje);
	string_append(&dirBitacora, "/Bitacora");

	dirBlocks= string_new();
	string_append(&dirBlocks, puntoMontaje);
	string_append(&dirBlocks, "/Blocks.ims");



	if (mkdir(puntoMontaje, 0777) != 0) {
		int todoBien=comprobar_que_todos_los_datos_existen(puntoMontaje);

		if(todoBien){
			log_info(mongoLogger, "Es un FS existente.");

			copiar_bitmap_de_disco(dirSuperbloque); //busco la info superB

			crearEstructuraDiscoLogico();
			crearEstructurasBloques();

			//crearBitMapLogico();
			struct stat statbuf;
			int archivo = open(dirBlocks, O_RDWR);
			fstat(archivo,&statbuf);
			block_mmap=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);

			copiar_datos_de_bloques(disco_logico->bloques);
		}
	}
	else{
		log_info(mongoLogger, "Generando estructura del FS");

		g_nuevo_block_size = (unsigned) atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
		g_nuevo_blocks	 = (unsigned) atoi(config_get_string_value(mongoConfig,"BLOCKS"));

		g_block_size = &g_nuevo_block_size;
		g_blocks	 = &g_nuevo_blocks;

		printf("BLOCKS SIZE: %u\n", *g_block_size);
		printf("BLOCKS: %u\n", *g_blocks);

		crearEstructuraDiscoLogico();
		crearEstructurasBloques();
		crearBitMapLogico();

		crearSuperbloqueNuevo(dirSuperbloque);//archivo nuevo
		copiar_bitmap_de_disco(dirSuperbloque);
		//crearSuperbloque(dirSuperbloque);

		crearblocks(dirBlocks);//archivo
		crearCarpetaFile(dirFiles);//carpeta
		crearCarpetaBitacora(dirBitacora);//carpeta
		struct stat statbuf;
		int archivo = open(dirBlocks, O_RDWR);
		fstat(archivo,&statbuf);
		block_mmap=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);

		//msync(superbloque, 2*sizeof(uint32_t) + 41, MS_SYNC);
		//munmap(superbloque, 2*sizeof(uint32_t) + 41);
	}

}
/*
void crearEstructuraFileSystem() {
	FILE *f;



	puntoMontaje = config_get_string_value(mongoConfig, "PUNTO_MONTAJE");
	dirMetadata = malloc(strlen(puntoMontaje) + strlen("/Metadata") + 1);
	strcpy(dirMetadata, puntoMontaje);
	strcat(dirMetadata, "/Metadata");

	dirFiles = malloc(strlen(puntoMontaje) + strlen("/Files") + 1);
	strcpy(dirFiles, puntoMontaje);
	strcat(dirFiles, "/Files");

	dirBitacora = malloc(strlen(dirFiles) + strlen("/Bitacora") + 1);
	strcpy(dirBitacora, dirFiles);
	strcat(dirBitacora, "/Bitacora");

	dirBlocks = malloc(strlen(puntoMontaje) + strlen("/Blocks") + 1);
	strcpy(dirBlocks, puntoMontaje);
	strcat(dirBlocks, "/Blocks");

	if (mkdir(puntoMontaje, 0777) != 0) {
		//Directorio de montaje existente, se verifica que exista superbloque y blocks
		char* superBloqueRuta = malloc(
				strlen(puntoMontaje) + strlen("/SuperBloque.ims") + 1);
		strcpy(superBloqueRuta, puntoMontaje);
		strcat(superBloqueRuta, "/SuperBloque.ims");
		char* blocksRuta = malloc(
				strlen(puntoMontaje) + strlen("/Blocks.ims") + 1);
		strcpy(blocksRuta, puntoMontaje);
		strcat(blocksRuta, "/Blocks.ims");
		int existeArchivo = access(superBloqueRuta, F_OK);
		if (existeArchivo == 0) {
			existeArchivo = access(blocksRuta, F_OK);
			if (existeArchivo == 0) {
				//Si el fileSystem esta creado se toman los datos de la metadata existente.
				log_info(mongoLogger, "El directorio %s ya existe. ",
						puntoMontaje);
				block_size = atoi(
						config_get_string_value(mongoConfig, "BLOCK_SIZE"));
				blocks = atoi(config_get_string_value(mongoConfig, "BLOCKS"));
				//bitmap = crear_bitmap(superBloqueRuta, blocks);
				return;
			} else {
				log_error(mongoLogger, "No se encuentra archivo Blocks.ims");
			}
			return;
		} else {
			log_error(mongoLogger, "No se encuentra archivo SuperBloque.ims");
		}
		free(superBloqueRuta);
		free(blocksRuta);
		return;

	} else {
		printf("Se creo el directorio de montaje =) \n");

		crearEstructuraDiscoLogico();

		crearEstructurasBloques();

		crearBitMapLogico();

		//Crea superBloque
		char* superBloqueRuta = malloc(
				strlen(puntoMontaje) + strlen("/SuperBloque.ims") + 1);
		strcpy(superBloqueRuta, puntoMontaje);
		strcat(superBloqueRuta, "/SuperBloque.ims");
		block_size = atoi(config_get_string_value(mongoConfig, "BLOCK_SIZE"));
		blocks = atoi(config_get_string_value(mongoConfig, "BLOCKS"));
		// Creo el archivo superBloque
		f = fopen(superBloqueRuta, "w");
		char* tamanioBloque = malloc(10);
		sprintf(tamanioBloque, "%d", block_size);

		char* bloques = malloc(10);
		sprintf(bloques, "%d", blocks);
		int bitmapsize = blocks / 8;
		int sizesuperbloque = string_length(tamanioBloque)
				+ string_length(bloques) + bitmapsize;
		fputs(tamanioBloque, f);
		fputs(bloques, f);

		fseek(f, sizesuperbloque, SEEK_SET);
		putc('\0', f);
		fclose(f);
		//Creo el archivo bitmap.bin
		crearBitMapLogico();
		//bitmap = crear_bitmap(superBloqueRuta, blocks);
		free(superBloqueRuta);
		free(tamanioBloque);
		free(bloques);

		//Crea Blocks
		char* blocksRuta = malloc(
				strlen(puntoMontaje) + strlen("/Blocks.ims") + 1);
		strcpy(blocksRuta, puntoMontaje);
		strcat(blocksRuta, "/Blocks.ims");
		//			log_trace(mongoLogger, "Estructura creada.");
		int X = block_size * blocks;
		f = fopen(blocksRuta, "w");
		fputs("1", f);
		fseek(f, X, SEEK_SET);
		putc('\0', f);
		fclose(f);
		//free(blocksRuta);
		struct stat statbuf;
		int archivo = open(blocksRuta, O_RDWR);
		fstat(archivo,&statbuf);
		block_mmap_size=statbuf.st_size;
		block_mmap=mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
		printf("se levanto el archivo block\n");
		//Creo carpeta Files
		if (mkdir(dirFiles, 0777) == 0) {
			printf("Se creo carpeta Files  =) \n");
			//Creo carpeta Bitacora
			if (mkdir(dirBitacora, 0777) == 0) {
				printf("Se creo carpeta Bitacora  =) \n");
			}

		} else {
			log_error(mongoLogger,
					"Ha ocurrido un error al crear el directorio Files.");
			return;
		}
	}
}*/


void escribir_en_block(char* lo_que_se_va_a_escribir,t_bloque* el_bloque){
	pthread_mutex_lock(&mutex_disco_logico);
	int longitud_texto = string_length(lo_que_se_va_a_escribir);
	for(int i=0;i<longitud_texto;i++){
		block_mmap[el_bloque->posicion_para_escribir]=lo_que_se_va_a_escribir[i];
		el_bloque->posicion_para_escribir++;
	}
	el_bloque->espacio=el_bloque->espacio-string_length(lo_que_se_va_a_escribir);

	log_info(mongoLogger, "Se escribieron %d bytes en el bloque %d, "
			"le quedan %d bytes disponibles.",
			longitud_texto,
			el_bloque->id_bloque,
			el_bloque->espacio);
	pthread_mutex_unlock(&mutex_disco_logico);
}



void funcion_para_llenar_con_tarea_IO(m_estado_tarea_tripulante* tripulanteConTareaFinalizada){

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
//	int respuesta;

	while (1) {
		int cliente = esperar_cliente(socket, mongoLogger);
//		printf("Cliente: %d\n", cliente);
		operacion = recibir_operacion(cliente);
		lista = NULL;

//		printf("\nLA OPERACION ES: %d\n", operacion);

//		switch(operacion) {
//			case OBTENGO_BITACORA:
//				lista = recibir_paquete(cliente);
//				uint32_t idTripulante = (uint32_t) atoi(list_get(lista,0));
//				uint32_t idPatota = (uint32_t) atoi(list_get(lista,1));
//				printf("Tripulante recibido %d\n", idTripulante);
//				printf("Patota recibida %d\n", idPatota);
//				break;
//			case ELIMINAR_TRIPULANTE:

		switch (operacion) {
		case OBTENGO_BITACORA:
			lista = recibir_paquete(cliente);
			uint32_t idTripulante = (uint32_t) atoi(list_get(lista, 0));
			uint32_t idPatota = (uint32_t) atoi(list_get(lista, 1));
			printf("Tripulante recibido %d\n", idTripulante);
			paquete = crear_paquete(OBTENGO_BITACORA);
			printf("Patota recibida %d\n", idPatota);
			//agregar los elementos encontrados para ese ID al paquete "paquete"
			enviar_paquete(paquete, cliente);
//             	int idTripulante = atoi((char *) list_get(lista,0));
//            	printf("Tripulante recibido %d\n", idTripulante);
			liberar_cliente(cliente);
			break;
		case ELIMINAR_TRIPULANTE:

//				lista = recibir_paquete(cliente);
//				int idTripulante = atoi((char *) list_get(lista,0));
//				eliminarTripulante(idTripulante);
//				printf("Tripulante eliminado de la nave %d\n", idTripulante);
			liberar_cliente(cliente);
			break;
		case ESPERANDO_SABOTAJE:
			;
			pthread_create(&hilo_sabotaje, NULL,
					(void*) enviar_aviso_sabotaje_a_discordiador,
					(void*) cliente);
			pthread_detach(hilo_sabotaje);
			break;
		case ACTUALIZAR_POSICION:
			;
			pthread_t hilo_actualizar_posicion;
			tripulanteEnMovimiento = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));
			tripulanteEnMovimiento = recibirMovimientoTripulante(cliente);

			tripulante_con_su_accion *tripulanteAP = (tripulante_con_su_accion*) malloc(sizeof(tripulante_con_su_accion));
			tripulanteAP->tripulante = tripulanteEnMovimiento;
			tripulanteAP->accion	 = ACTUALIZAR_POSICION;


			pthread_create(&hilo_actualizar_posicion, NULL, (void*) escribir_en_su_bitacora_la_accion, (void*) tripulanteAP);
			pthread_detach(hilo_actualizar_posicion);
			liberar_cliente(cliente);
			break;

		case INICIO_TAREA:
			;
			pthread_t hilo_escribir_inicio_tarea;
			m_estado_tarea_tripulante *tripulanteConTarea = (m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));
			tripulanteConTarea = recibirNuevoEstadoTareaTripulante(cliente);

			tripulante_con_su_accion *tripulanteIT = (tripulante_con_su_accion*) malloc(sizeof(tripulante_con_su_accion));
			tripulanteIT->tripulante = tripulanteConTarea;
			tripulanteIT->accion	 = INICIO_TAREA;

			pthread_create(&hilo_escribir_inicio_tarea, NULL, (void*) escribir_en_su_bitacora_la_accion, (void*) tripulanteIT);
			pthread_detach(hilo_escribir_inicio_tarea);

			liberar_cliente(cliente);
			break;

		case FIN_TAREA:
			;
			pthread_t hilo_escribir_fin_tarea;
			m_estado_tarea_tripulante *tripulanteConTareaFinalizada = (m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));
			tripulanteConTareaFinalizada = recibirNuevoEstadoTareaTripulante(cliente);

			tripulante_con_su_accion *tripulanteFT = (tripulante_con_su_accion*) malloc(sizeof(tripulante_con_su_accion));
			tripulanteFT->tripulante = tripulanteConTareaFinalizada;
			tripulanteFT->accion	 = INICIO_TAREA;
			//aca avisaria A BITACORA que termino tarea independientemente si es IO/COMUN
			pthread_create(&hilo_escribir_fin_tarea, NULL, (void*) escribir_en_su_bitacora_la_accion, (void*) tripulanteFT);
			pthread_detach(hilo_escribir_fin_tarea);

			if (tripulanteConTareaFinalizada->tipo_tarea == TAREA_IO) {

				//aca llenaria el archivo tantas veces como el 'parametro'
				funcion_para_llenar_con_tarea_IO(tripulanteConTareaFinalizada);
			}
			liberar_cliente(cliente);
			break;

		case INICIO_SABOTAJE:
			;
			m_movimiento_tripulante *trip_inicio_sabotaje = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));
			trip_inicio_sabotaje = recibirMovimientoTripulante(cliente);

			//TODO: GENERAR STRUCT, HILO DE INICIO

			liberar_cliente(cliente);
			break;

		case FIN_SABOTAJE:
			;
			m_movimiento_tripulante *trip_fin_sabotaje = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));
			trip_fin_sabotaje = recibirMovimientoTripulante(cliente);

			//TODO: GENERAR STRUCT, HILO DE FIN

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
		//liberar_cliente(cliente);

	}
}

void gestionarSabotaje() {
	int operacion;
	switch (operacion) {
	case SUPERBLOQUE:
		//si cambia valor Blocks constatar con tamaño archivo blocks.ims
		//si cambia valor Bitmap recorrer FILES y obtener bloques usados
		break;
	case FILES:
		//si cambia el SIZE recorrer todos los bloques y asumir correcto el tamaño de los mismos
		//si son inconsistentes block_count y blocks actualizo block_count en base a la lista de blocks
		//si se altera la lista BLOCKS y no estan en orden(nos damos cuenta porque cambia el valor de md5)
		//se debe reescribir la lista hasta que se obtenga el mismo tamaño
	default:
		printf("Operacion desconocida.\n");
		break;

	}
}

void rutina(int n) {
	printf("El SABOTAJE EXISTE!\n");
	sem_post(&dar_orden_sabotaje);
}

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
void enviar_aviso_sabotaje_a_discordiador(void *data) {
	int socket_mongo_store = (int) data;
	char** sabotaje_pos_aux;
	char* sabotaje_posY;
	//char** pos_dividida;
	char* sabotaje_posX;
	char* pos_sabotaje;
	//int socket_para_sabotaje = esperar_cliente(socket_mongo_store);

	sem_wait(&dar_orden_sabotaje);

	printf("SOCKET DISCO %d\n", socket_mongo_store);

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
	enviar_paquete(paquete_sabotaje, socket_mongo_store);
	eliminar_paquete(paquete_sabotaje);
	liberar_cliente(socket_mongo_store);

	pthread_exit(NULL);
}


void generar_oxigeno(int cantidad){
	char* oxigenoRuta = malloc(strlen(dirFiles) + strlen("/Oxigeno.ims") + 1);
	strcpy(oxigenoRuta, dirFiles);
	strcat(oxigenoRuta, "/Oxigeno.ims");
	int existeArchivo = access(oxigenoRuta, F_OK);
	FILE *archivo = fopen(oxigenoRuta, "a+");

  if(existeArchivo == 0){
    printf("El archivo EXISTE!\n");
    fseek(archivo, -1, SEEK_END);
    for(int i=0; i < cantidad; i++)
      fputc('O', archivo);
  }
  else {
    for(int i=0; i < cantidad; i++)
      fputc('O', archivo);
  }
  fclose(archivo);
}

void consumir_oxigeno(int cant_borrar){
	char* oxigenoRuta = malloc(strlen(dirFiles) + strlen("/Oxigeno.ims") + 1);
	strcpy(oxigenoRuta, dirFiles);
	strcat(oxigenoRuta, "/Oxigeno.ims");

  if(access(oxigenoRuta, F_OK) == 0){
    FILE *archivo = fopen(oxigenoRuta, "a+");
    long int pos_actual;
    long int cant_ox_disponible;

    fseek(archivo, 0, SEEK_END);
    cant_ox_disponible = ftell(archivo) / sizeof(char);

    printf("cantidadOxigenos = %ld\n", cant_ox_disponible);

    if(cant_borrar > cant_ox_disponible){
      fseek(archivo, 0, SEEK_SET);
      //hay que avisar que intento borrar mas de los disponible
      log_error(mongoLogger, "Se intento borrar más oxigenos de los disponibles");
    }
    else{
      fseek(archivo, -cant_borrar * sizeof(char), SEEK_END);
    }
    pos_actual = ftell(archivo);
    ftruncate(fileno(archivo), pos_actual);
    fclose(archivo);
    return;
  }

  else{
    printf("SACAR-OX: no existe archivo!\n");
    //no existe el archivo
    log_error(mongoLogger, "No existe el archivo Oxigeno.ims");
    return;
  }
}


void generar_comida(int cantidad){
	char* comidaRuta = malloc(strlen(dirFiles) + strlen("/Comida.ims") + 1);
	strcpy(comidaRuta, dirFiles);
	strcat(comidaRuta, "/Comida.ims");
	int existeArchivo = access(comidaRuta, F_OK);
	FILE *archivo = fopen(comidaRuta, "a+");

  if(existeArchivo == 0){
    printf("El archivo EXISTE!\n");
    fseek(archivo, -1, SEEK_END);
    for(int i=0; i < cantidad; i++)
      fputc('C', archivo);
  }
  else {
    for(int i=0; i < cantidad; i++)
      fputc('C', archivo);
  }
  fclose(archivo);
}


void consumir_comida(int cant_borrar){
	char* comidaRuta = malloc(strlen(dirFiles) + strlen("/Comida.ims") + 1);
  if(access(comidaRuta, F_OK) == 0){
    FILE *archivo = fopen(comidaRuta, "a+");
    long int pos_actual;
    long int cant_com_disponible;

    fseek(archivo, 0, SEEK_END);
    cant_com_disponible = ftell(archivo) / sizeof(char);

    printf("cantidadComida = %ld\n", cant_com_disponible);

    if(cant_borrar > cant_com_disponible){
      fseek(archivo, 0, SEEK_SET);
      //hay que avisar que intento borrar mas de los disponible
      log_error(mongoLogger, "Se intento borrar más comida de la disponible");
    }
    else{
      fseek(archivo, -cant_borrar * sizeof(char), SEEK_END);
    }
    pos_actual = ftell(archivo);
    ftruncate(fileno(archivo), pos_actual);
    fclose(archivo);
    return;
  }

  else{
    printf("SACAR-COM: no existe archivo!\n");
    //no existe el archivo
    log_error(mongoLogger, "No existe el archivo Comida.ims");
    return;
  }
}


void generar_basura(int cantidad){
	char* basuraRuta = malloc(strlen(dirFiles) + strlen("/Basura.ims") + 1);
	strcpy(basuraRuta, dirFiles);
	strcat(basuraRuta, "/Basura.ims");
	int existeArchivo = access(basuraRuta, F_OK);

  FILE *archivo = fopen(basuraRuta, "a+");

  if(existeArchivo == 0){
    printf("El archivo EXISTE!\n");
    fseek(archivo, -1, SEEK_END);
    for(int i=0; i < cantidad; i++)
      fputc('B', archivo);
  }
  else {
    for(int i=0; i < cantidad; i++)
      fputc('B', archivo);
  }
  fclose(archivo);
}

void descartar_basura(int cant_borrar) {
	char* basuraRuta = malloc(strlen(dirFiles) + strlen("/Basura.ims") + 1);
	strcpy(basuraRuta, dirFiles);
	strcat(basuraRuta, "/Basura.ims");
	if (access(basuraRuta, F_OK) == 0) {
		remove(basuraRuta);
	}

	else {
		printf("SACAR-BASURA: no existe archivo!\n");
		//no existe el archivo
		log_error(mongoLogger, "No existe el archivo Basura.ims");
		return;
	}
}

/*arma la direccion de la bitacora dependiendo la accion que realiza.
 * porque por ejemplo las estructuras de tareas y movimiento son difernetes,
 * entonces los separo dependiendo de la accion q realiza*/
char *rutaBitacoraDelTripulante(tripulante_con_su_accion *tripulante) {
	char *rutaBitacora = string_new();



	switch (tripulante->accion) {

		case ACTUALIZAR_POSICION: ;
			m_movimiento_tripulante *tripulante_mov = (m_movimiento_tripulante *) tripulante->tripulante;
			rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
					dirBitacora, tripulante_mov->idTripulante, tripulante_mov->idPatota);
			break;

		case INICIO_TAREA: ;
			m_estado_tarea_tripulante *tripulante_inicio_tarea = (m_estado_tarea_tripulante*) tripulante->tripulante;
			rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
								dirBitacora, tripulante_inicio_tarea->idTripulante, tripulante_inicio_tarea->numPatota);
			break;

		case FIN_TAREA: ;
			m_estado_tarea_tripulante *tripulante_fin_tarea = (m_estado_tarea_tripulante*) tripulante->tripulante;
			rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
								dirBitacora, tripulante_fin_tarea->idTripulante, tripulante_fin_tarea->numPatota);
			break;

		case INICIO_SABOTAJE: ;
			m_movimiento_tripulante *tripulante_sab = (m_movimiento_tripulante *) tripulante->tripulante;
			rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
								dirBitacora, tripulante_sab->idTripulante, tripulante_sab->idPatota);
			break;

		case FIN_SABOTAJE: ;
			m_movimiento_tripulante *tripulante_fin_sab = (m_movimiento_tripulante *) tripulante->tripulante;
			rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",
								dirBitacora, tripulante_fin_sab->idTripulante, tripulante_fin_sab->idPatota);
			break;
		default:
			log_error(mongoLogger, "Error al buscar la bitacora del tripulante!");
			break;

	}

	return rutaBitacora;
}

char *generarTextoAEscribir(tripulante_con_su_accion *tripulante) {
	char *lo_que_se_va_a_escribir = string_new();


	switch (tripulante->accion) {

	case ACTUALIZAR_POSICION: ;
		m_movimiento_tripulante *tripulante_mov = (m_movimiento_tripulante *) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format("Se mueve de %d | %d a %d | %d\n",
				tripulante_mov->origenX, tripulante_mov->origenY,
				tripulante_mov->destinoX, tripulante_mov->destinoY);
		break;

	case INICIO_TAREA: ;
		m_estado_tarea_tripulante *tripulante_inicio_tarea = (m_estado_tarea_tripulante*) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format("Inicia la tarea %s\n",
				tripulante_inicio_tarea->nombreTarea);
		break;

	case FIN_TAREA: ;
		m_estado_tarea_tripulante *tripulante_fin_tarea = (m_estado_tarea_tripulante*) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format("Finaliza la tarea %s\n",
				tripulante_fin_tarea->nombreTarea);
		break;

	case INICIO_SABOTAJE: ;
		m_movimiento_tripulante *tripulante_sab = (m_movimiento_tripulante *) tripulante->tripulante;
		lo_que_se_va_a_escribir = string_from_format(
				"Se corre al sabotaje en ubicacion (%d, %d)\n",
				tripulante_sab->destinoX, tripulante_sab->destinoY);
		break;

	case FIN_SABOTAJE: ;
		lo_que_se_va_a_escribir = string_from_format("Se resuelve el sabotaje\n");
		break;
	default:
		log_error(mongoLogger, "Error al generar el tipo de texto a escribir!");
		lo_que_se_va_a_escribir = NULL;
		break;
	}

	return lo_que_se_va_a_escribir;
}

void escribir_en_su_bitacora_la_accion(tripulante_con_su_accion *tripulante){
	t_bloque *el_bloque;
	t_bloque *nuevo_bloque;
	char *bloque;
	int numero_del_nuevo_bloque;
	//armo toda la cadena que se va a escribir por movimiento
	char *rutaBitacora		      = rutaBitacoraDelTripulante(tripulante);
	char *lo_que_se_va_a_escribir = generarTextoAEscribir(tripulante);


	int existeArchivo = access(rutaBitacora, F_OK);
	//si la bitacora del tripulante existe, entonces recupero el ultimo bloque
	if (existeArchivo == 0) {
		log_info(mongoLogger,
				"El archivo del tripulante existe, buscando su ultimo bloque");

		//ultimo bloque en char
		el_bloque = buscar_ultimo_bloque_del_tripulante(rutaBitacora);
		//si hay espacio en el bloque para escribir to do, entonces lo escribo
		if (el_bloque->espacio > string_length(lo_que_se_va_a_escribir)) {

			escribir_en_block(lo_que_se_va_a_escribir, el_bloque);

		}
		//sino escribo una parte y elijo otro bloque para lo restante
		else {
			char *lo_que_entra_en_el_bloque = string_substring_until(
					lo_que_se_va_a_escribir, el_bloque->espacio);
			escribir_en_block(lo_que_entra_en_el_bloque, el_bloque);
			char *lo_que_falta_escribir = string_substring_from(
					lo_que_se_va_a_escribir,
					string_length(lo_que_entra_en_el_bloque));
			//asigno el bloque nuevo
			numero_del_nuevo_bloque = obtener_bloque_libre();
			log_info(mongoLogger,
					"Asigno el bloque numero:%d\n",
					numero_del_nuevo_bloque);
			pthread_mutex_lock(&mutex_disco_logico);
			nuevo_bloque = (t_bloque *) list_get(disco_logico->bloques,
					numero_del_nuevo_bloque - 1);
			pthread_mutex_unlock(&mutex_disco_logico);
			//escribir lo_que_falta_escribir
			escribir_en_block(lo_que_falta_escribir, nuevo_bloque);
			//actualizar el bitmap
			ocupar_bloque(numero_del_nuevo_bloque);
			// actualizar bitacora con el bloque nuevo del tripulante
			agregar_bloque_bitacora(rutaBitacora, nuevo_bloque->id_bloque);
		}
	} else {
		//asigno el bloque nuevo
		numero_del_nuevo_bloque = obtener_bloque_libre();

		log_info(mongoLogger,
				"Asigno el bloque numero:%d\n",
				numero_del_nuevo_bloque);

		//creo el archivo bitacora
		nuevo_bloque = (t_bloque *) list_get(disco_logico->bloques,
				numero_del_nuevo_bloque - 1);
		sem_wait(&semaforo_bitacora);
		inicializar_bitacora(rutaBitacora,
				string_itoa(nuevo_bloque->id_bloque));
		sem_post(&semaforo_bitacora);
		//modifico bitmap
		ocupar_bloque(numero_del_nuevo_bloque);
		//escribo lo_que_se_va_a_escribir en block
		escribir_en_block(lo_que_se_va_a_escribir, nuevo_bloque);
	}

	free(tripulante->tripulante);
	free(tripulante);
}

int cantidad_bloques_a_ocupar(char* texto)
{
	int cantidad = string_length(texto)/block_size;
	return cantidad;
}

