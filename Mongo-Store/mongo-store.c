#include "mongo-store.h"


int main() {

	iniciar_recursos_mongo();
	pthread_t hilo_bajada_a_disco;

	pthread_create(&hilo_bajada_a_disco, NULL, (void *)gestionar_bajadas_a_disco, NULL);
	pthread_detach(hilo_bajada_a_disco);

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);

	gestionarCliente(socket_mongo_store );

	printf("SOCKET DISCO %d\n", socket_mongo_store);
	return EXIT_SUCCESS;
}


void iniciar_recursos_mongo(void) {
	signal(SIGUSR1, rutina); //Recepcion mensaje de sabotaje

	numero_sabotaje = 0;

	sem_init(&dar_orden_sabotaje, 0, 0);
	sem_init(&contador_sabotaje, 0, 1);
//	sem_init(&semaforo_bitmap, 0, 1); lo cambie por un mutex
	sem_init(&semaforo_bitacora, 0, 1);
	sem_init(&semaforo_para_file_oxigeno,0,1);
	sem_init(&semaforo_para_file_comida,0,1);
	sem_init(&semaforo_para_file_basura,0,1);
	sem_init(&inicio_fsck, 0, 0);


	//inicializo los booleanos de existen archivos
	g_existe_file_oxigeno = false;
	g_existe_file_comida  = false;
	g_existe_file_basura  = false;


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

	archivo_blocks = open(dirBlocks, O_RDWR);

	if(archivo_blocks == -1)
	{
		log_error(mongoLogger, "Error al abrir el archivo para bajar los blocks");
		exit(1);
	}

	fstat(archivo_blocks, &statbuf);

	contenido_blocks = (char*) mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, archivo_blocks, 0);

	memcpy(contenido_blocks, block_mmap, statbuf.st_size);

	msync(contenido_blocks, statbuf.st_size, MS_SYNC);

	munmap(contenido_blocks, statbuf.st_size);

	close(archivo_blocks);
}

void bajar_datos_superbloque(void) {
	int archivo_superbloque;
	void *contenido_superbloque;
	struct stat statbuf;

	archivo_superbloque = open(dirSuperbloque, O_RDWR);

	if (archivo_superbloque == -1) {
		log_error(mongoLogger, "Error al abrir el archivo para bajar el superbloque");
		exit(1);
	}

	fstat(archivo_superbloque, &statbuf);

	contenido_superbloque = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, archivo_superbloque, 0);

	memcpy(contenido_superbloque, superbloque, statbuf.st_size);

	int retorno = msync(contenido_superbloque, statbuf.st_size, MS_SYNC);

	munmap(contenido_superbloque, statbuf.st_size);

	if(retorno == -1)
	{
		log_error(mongoLogger, "Error al sincronizar el superbloque");
	}

	close(archivo_superbloque);
}

void gestionar_bajadas_a_disco(void){
	int tiempo_bajada = atoi(config_get_string_value(mongoConfig, "TIEMPO_SINCRONIZACION"));

	while(1)
	{
		sleep(tiempo_bajada);
		log_info(mongoLogger, "Realizando bajada a disco de blocks y superbloque..");

		//TODO poner un mutex de blocks
		bajar_datos_blocks();


		bajar_datos_superbloque();

		log_info(mongoLogger, "Finalizo la bajada a disco");
	}
}



void mostrar_estado_bitarray(void) {
	//TODO: cambiarlo por un hexdump

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
  char *command = string_new(); //por las dudas q el path sea largo
  int retorno;
  struct stat statbuf;
  void *data;
  char *md5;
  char *ruta_md5 = string_new();
  char *punto_montaje = config_get_string_value(mongoConfig,"PUNTO_MONTAJE");


  ruta_md5 = string_from_format("%s/Files/md5.txt", punto_montaje);
  //sprintf(ruta_md5, "%s/Files/md5.txt", punto_montaje);

  command = string_from_format("echo -n %s | md5sum > %s", contenido, ruta_md5);
  //sprintf(command, "echo -n %s | md5sum > %s", contenido, ruta_md5);

  retorno = system(command);

  if(retorno != 0)
    {
      printf("ERROR AL GENERAR EL MD5\n");
      exit(1);
    }

  fd = open(ruta_md5, O_RDWR);

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

//creamos el disco logico
void crearEstructuraDiscoLogico(){
	disco_logico=(t_disco_logico *)malloc(sizeof(t_disco_logico));
	disco_logico->bloques=list_create();
}

//creamos todos los bloques del filesystem
void crearEstructurasBloques(){
	int _cantidad_de_bloques = (int)*g_blocks;

	for(int contador=1;contador <=_cantidad_de_bloques;contador++){
		t_bloque *bloque = (t_bloque *)malloc(sizeof(t_bloque));
		bloque->id_bloque=contador;
		bloque->inicio= (contador-1) * *g_block_size;
		bloque->fin = bloque->inicio+ (*g_block_size-1);
		bloque->espacio=*g_block_size;
		bloque->posicion_para_escribir=bloque->inicio;
		list_add(disco_logico->bloques,bloque);

	}

	log_info(mongoLogger, "Se creo estructura bloques!");

	log_info(mongoLogger, "Cantidad de bloques generados: %d", list_size(disco_logico->bloques));
}

//devuelve el ultimo bloque que usa el tripulante para sus datos
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

//crea un archivo en una ruta
void crear_archivo(char* ruta){
	FILE *archivo =fopen(ruta, "w");
	fclose(archivo);
}

//crea el archivo bitacora cuando no existe
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

//agrega un bloque a la bitacora cuando se queda sin lugar un bloque
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

//crea el bitmap logico para operar los bloques
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

//crea el archivo superbloque
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

//crea el archivo blocks
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

//crea la carpeta file
void crearCarpetaFile(char * dirFiles){

	if (mkdir(dirFiles, 0777) == 0) {
		log_info(mongoLogger, "se creo el directorio file correctamente\n");
	} else
		log_error(mongoLogger,
				"Ha ocurrido un error al crear el directorio Files.");

}

//crea la carpeta bitacora
void crearCarpetaBitacora(char * dirBitacora){
	if (mkdir(dirBitacora, 0777) == 0) {
		log_info(mongoLogger, "se creo el directorio bitacora correctamente\n");
	} else
		log_error(mongoLogger,
				"Ha ocurrido un error al crear el directorio bitacora.");
}

//comprueba si existen todos los datos al levantar de nuevo el filesystem
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
	void *contenido_superbloque;
	int fdm = open(dirSuperbloque, O_RDWR);

	if (fdm == -1) {
		log_error(mongoLogger, "ERROR AL ABRIR ARCHIVO");
		exit(1);
	}

	fstat(fdm, &statbuf);
	contenido_superbloque = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fdm, 0);

	superbloque = malloc(statbuf.st_size);

	memcpy(superbloque, contenido_superbloque, statbuf.st_size);

	g_block_size = superbloque;
	g_blocks     = superbloque + sizeof(uint32_t);
	bitarrayEnChar = superbloque + 2 * sizeof(uint32_t);

	int cantLeer2 = (int) ceil((double) *g_blocks / 8);

	bitmap = bitarray_create(bitarrayEnChar, cantLeer2);

	g_tamanio_superbloque = statbuf.st_size;

	//reviso las cosas
	log_info(mongoLogger, "SIZE GUARDADO EN SUPERBLOQUE: %d\n", (int)*g_block_size);
	log_info(mongoLogger, "CANT GUARDADO EN SUPERBLOQUE: %d\n", (int)*g_blocks);

	mostrar_estado_bitarray(); //usarlo cuando hay pocos bits

	log_info(mongoLogger, "Se bajaron los datos del Superbloque existente!");

	munmap(contenido_superbloque, g_tamanio_superbloque);
	close(fdm);

}

void crearSuperbloqueNuevo(char *path){
	FILE *fd;
	fd = fopen(path, "wb+");

	if (fd == NULL) {
		log_error(mongoLogger, "ERROR AL ABRIR ARCHIVO");
		exit(1);
	}

	int cantBytes = (int)ceil((double)(*g_blocks)/8);

	printf("%d", cantBytes);

	fwrite(g_block_size, sizeof(uint32_t), 1, fd);
	fwrite(g_blocks, sizeof(uint32_t), 1, fd);
	fwrite(bitmap->bitarray, cantBytes, 1, fd);

	log_info(mongoLogger, "Se creo el superbloque nuevo!");

	fclose(fd);
}

//devuelve la cantidad de caracteres ocupados
int caracteres_ocupados(int inicio,int fin){
	int posicion=inicio;
	int caracterOcupado=0;
	int ultimoCaracter=0;
	bool tiene_caracteres_escritos=false;
	while(posicion<=fin){
		if(block_mmap[posicion]!=' '){
			ultimoCaracter=posicion;
			tiene_caracteres_escritos=true;
		}
		posicion++;
	}

	if(tiene_caracteres_escritos){
		caracterOcupado=ultimoCaracter+1-inicio;
	}

	return caracterOcupado;
}

//copia los datos de cada bloque a los bloques logicos para recuperar la info cuando
//se levanta el filesystem
void copiar_datos_de_bloques(t_list* bloques){
	int inicio, fin;
	t_bloque *bloque;

	for(int i=0;i<list_size(bloques);i++){
		t_bloque *bloque=malloc(sizeof(t_bloque));
		//agarro un bloque
		bloque=list_get(bloques,i);
		//veo donde empieza
		int inicio = bloque->inicio;

		//veo donde termina
		int fin = bloque->fin;

		//leo desde empieza hasta termina en bloc los caracteres escritos
		int caracteresOcupados = caracteres_ocupados(inicio,fin);
		//resto espacio en bloque
		bloque->espacio=bloque->espacio-caracteresOcupados;

		//acomodo el puntero en bloque
		bloque->posicion_para_escribir=caracteresOcupados+bloque->inicio;
	}
}

void crearCopiaBlocks(char *dirBlocks) {
	struct stat statbuf;
	int archivo;
	char *contenidoArchivo;

	archivo = open(dirBlocks, O_RDWR);

	if (archivo == -1)
	{
		log_error(mongoLogger, "Error al abrir el archivo para copiar los blocks!");
	}

	fstat(archivo,&statbuf);

	contenidoArchivo = (char *) mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);

	block_mmap = (char *) malloc(statbuf.st_size);

	memcpy(block_mmap, contenidoArchivo, statbuf.st_size);

	munmap(contenidoArchivo, statbuf.st_size);
}


//crea el filesystem completo si no existe o levanta el que existe
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
			crearCopiaBlocks(dirBlocks);
			copiar_datos_de_bloques(disco_logico->bloques);
		}
	}

	else{
		log_info(mongoLogger, "Generando estructura del FS");

		g_nuevo_block_size = (unsigned) atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
		g_nuevo_blocks	   = (unsigned) atoi(config_get_string_value(mongoConfig,"BLOCKS"));

		g_block_size = &g_nuevo_block_size;
		g_blocks	 = &g_nuevo_blocks;

		log_info(mongoLogger, "BLOCKS SIZE: %u", *g_block_size);
		log_info(mongoLogger, "BLOCKS: %u", *g_blocks);

		crearEstructuraDiscoLogico();
		crearEstructurasBloques();
		crearBitMapLogico();
		crearSuperbloqueNuevo(dirSuperbloque);//archivo nuevo
		copiar_bitmap_de_disco(dirSuperbloque);
		crearblocks(dirBlocks);//archivo
		crearCarpetaFile(dirFiles);//carpeta
		crearCarpetaBitacora(dirBitacora);//carpeta
		crearCopiaBlocks(dirBlocks);
	}

}

//escribe lo que quieras en el archivo block
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

char* obtener_bloques_bitacora(char* ruta){
	int archivo = open(ruta, O_RDWR);
		struct stat statbuf;
		fstat(archivo,&statbuf);
		char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
		char** auxiliar = string_split(archivo_addr,"\n");
		char** auxiliar2= string_split(auxiliar[1],"=");
		char* blocks = auxiliar2[1];

		munmap(archivo_addr,statbuf.st_size);
		close(archivo);
		return blocks;
}

void obtener_bitacora_tripulante(int cliente){
	t_list* lista = recibir_paquete(cliente);
	uint32_t idTripulante = (uint32_t) atoi(list_get(lista, 0));
	uint32_t idPatota = (uint32_t) atoi(list_get(lista, 1));
	char* rutaBitacora = string_from_format("%s/Tripulante%dPatota%d.ims",dirBitacora,idTripulante,idPatota);
	int archivo = open(rutaBitacora, O_RDONLY);
	if(archivo!=-1){
		char* bloques_bitacora=obtener_bloques_bitacora(rutaBitacora);
		char* texto=contenido_de_bloques(bloques_bitacora);
		t_paquete* paquete = crear_paquete(OBTENGO_BITACORA);
		agregar_a_paquete(paquete, texto, string_length(texto) + 1);
		enviar_paquete(paquete, cliente);
		eliminar_paquete(paquete);

	}else{
		printf("la bitacora todavia no existe\n");
	}
	liberar_cliente(cliente);
	close(archivo);

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
			;
			pthread_t obtener_bitacora;
			pthread_create(&obtener_bitacora, NULL,	(void*) obtener_bitacora_tripulante,(void*) cliente);
			pthread_detach(obtener_bitacora);
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
			socket_sabotaje_cliente = cliente;

			log_info(mongoLogger, "El socket para sabotajes es: %d\n", socket_sabotaje_cliente);

			//pthread_create(&hilo_sabotaje, NULL,
			//		(void*) enviar_aviso_sabotaje_a_discordiador,
			//		NULL);
			//pthread_detach(hilo_sabotaje);
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
			tripulanteFT->accion	 = FIN_TAREA;
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
			pthread_t hilo_inicio_sabotaje;
			m_movimiento_tripulante *trip_inicio_sabotaje = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));
			trip_inicio_sabotaje = recibirMovimientoTripulante(cliente);

			tripulante_con_su_accion *tripulanteIS = (tripulante_con_su_accion*) malloc(sizeof(tripulante_con_su_accion));
			tripulanteIS->tripulante = trip_inicio_sabotaje;
			tripulanteIS->accion     = INICIO_SABOTAJE;

			pthread_create(&hilo_inicio_sabotaje, NULL, (void*) escribir_en_su_bitacora_la_accion, (void*) tripulanteIS);
			pthread_detach(hilo_inicio_sabotaje);

			liberar_cliente(cliente);
			break;

		case FIN_SABOTAJE:
			;
			pthread_t hilo_fin_sabotaje;
			m_movimiento_tripulante *trip_fin_sabotaje = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));
			trip_fin_sabotaje = recibirMovimientoTripulante(cliente);

			tripulante_con_su_accion *tripulanteFS = (tripulante_con_su_accion*) malloc(sizeof(tripulante_con_su_accion));
			tripulanteFS->tripulante = trip_fin_sabotaje;
			tripulanteFS->accion     = FIN_SABOTAJE;

			sem_post(&inicio_fsck);

			pthread_create(&hilo_fin_sabotaje, NULL, (void*) escribir_en_su_bitacora_la_accion, (void*) tripulanteFS);
			pthread_detach(hilo_fin_sabotaje);

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


//crea y escribe el archivo necesario con el texto necesario
void inicializar_archivo(char* ruta_archivo, int cantidad_caracter, char caracter){

	crear_archivo(ruta_archivo);
	char * cadena_a_grabar = string_repeat(caracter, cantidad_caracter);
	int cantidad_de_bloques=cantidad_bloques_a_ocupar(cadena_a_grabar);
	char* caracter_a_string=string_itoa(cantidad_caracter);
	char* cantidad_de_bloques_a_string=string_itoa(cantidad_de_bloques);
	char* el_caracter=string_from_format("%c", caracter);

	int archivo = open(ruta_archivo, O_RDWR);
	struct stat statbuf;
	ftruncate(archivo, (off_t)59/*+string_length(caracter_a_string)+string_length(cantidad_de_bloques_a_string)*/);
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);

	char* cadena = string_new();
	string_append(&cadena,"SIZE=0");
	//string_append(&cadena,caracter_a_string);
	string_append(&cadena,"\nBLOCK_COUNT=");
	//string_append(&cadena,cantidad_de_bloques_a_string);
	string_append(&cadena,"\nBLOCKS=");
	string_append(&cadena,"\nCARACTER_LLENADO=");
	string_append(&cadena,el_caracter);
	string_append(&cadena,"\nMD5_ARCHIVO=");

	for(int i=0; i<=string_length(cadena);i++){
			archivo_addr[i]=cadena[i];
		}

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
}

//devuelte el tamaño del archivo en char*
char *size_de_archivo(char* ruta){
	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);

	char** auxiliar = string_split(archivo_addr,"\n");
	char** auxiliar2= string_split(auxiliar[0],"=");
	char* size = auxiliar2[1];

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	return size;
}
//devuelve la cantidad de bloques de un archivo en char*
char* cantidad_de_bloques_de_archivo(char* ruta){

	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char** auxiliar = string_split(archivo_addr,"\n");
	char** auxiliar2= string_split(auxiliar[1],"=");
	char* block_count = auxiliar2[1];

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	return block_count;

}
//devuelve los bloques de un archivo en char*
char* bloques_de_archivo(char* ruta){
	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char** auxiliar = string_split(archivo_addr,"\n");
	char** auxiliar2= string_split(auxiliar[2],"=");
	char* blocks = auxiliar2[1];

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	return blocks;
}
//lee un bloque y devuelve to do su texto
char* leo_el_bloque(t_bloque* bloque){
	char* texto=string_new();

	int inicio=bloque->inicio;
	int fin = bloque-> posicion_para_escribir;

	while(inicio<fin){
		char *aux=string_from_format("%c", block_mmap[inicio]);


		string_append(&texto,aux);
		inicio++;
	}
	return texto;
}

char* leo_el_bloque_incluyendo_espacios(t_bloque* bloque){
	char* texto=string_new();

	int inicio=bloque->inicio;
	int fin = bloque-> fin;

	while(inicio<=fin){
		char *aux=string_from_format("%c", block_mmap[inicio]);


		string_append(&texto,aux);
		inicio++;
	}
	return texto;
}
//devuelve el texto de TODOS los bloques
char* contenido_de_bloques(char* bloques){


	t_bloque* bloque= malloc(sizeof(t_bloque));
	char** auxiliar=string_split(bloques,",");
	char* texto_todos_los_bloques=string_new();

	int i = 0;


	while(auxiliar[i]){
		bloque=list_get(disco_logico->bloques,atoi(auxiliar[i])-1);
		char* texto= leo_el_bloque(bloque);

		string_append(&texto_todos_los_bloques,texto);
		i++;
	}


	return texto_todos_los_bloques;
}

char* leer_md5file(char* ruta){
	int archivo=open(ruta,O_RDWR);
	char* md5=string_new();
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char** archivo_dividido=string_split(archivo_addr,"\n");
	string_append(&md5,archivo_dividido[4]);
	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	return md5;
}


//para comprobar si un array de bloques en char* contiene a un bloque en char*
bool contiene(char* bloques,char* bloque){
	bool flag=false;
	char** bloques_separados=string_split(bloques,",");
	int i = 0;
	while(bloques_separados[i]){

		if(string_equals_ignore_case(bloques_separados[i],bloque)){
			flag=true;
		}
		i++;
	}

	return flag;
}

//actualiza un archivo si se agregan bloques o cambia su tamaño
void actualizar_el_archivo(char *ruta,char* cadena,t_bloque* bloque){

	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char* cantidad_bloques;
	char* bloques=bloques_de_archivo(ruta);
	bool contiene_bloque=contiene(bloques,string_itoa(bloque->id_bloque));
	if(!contiene_bloque){
		cantidad_bloques=string_itoa(atoi(cantidad_de_bloques_de_archivo(ruta))+1);
	}
	else{
		cantidad_bloques=string_itoa(atoi(cantidad_de_bloques_de_archivo(ruta)));
	}
	char* size=string_itoa(atoi(size_de_archivo(ruta))+string_length(cadena));

	if(!contiene_bloque){
		string_append(&bloques,string_itoa(bloque->id_bloque));
	}
	else{
		bloques=string_substring_until(bloques,string_length(bloques)-1);
	}

	char* caracter_llenado=string_substring_until(cadena, 1);
	char* contenido_de_los_bloques=contenido_de_bloques(bloques);


	char* md5=generarMD5(contenido_de_los_bloques);
	string_append(&bloques,",");


	char* texto = string_from_format("SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
			size,cantidad_bloques,bloques,caracter_llenado,md5);
	int tamanio_texto=string_length(texto);
	if(tamanio_texto>statbuf.st_size){
		int lo_que_hay_que_agrandar=tamanio_texto-statbuf.st_size;
		ftruncate(archivo, (off_t)statbuf.st_size+lo_que_hay_que_agrandar);
	}
	for(int i=0;i<tamanio_texto;i++){
		archivo_addr[i]=texto[i];
	}

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
}

//borra la cantidad de caracteres que se desean de un archivo
void actualizar_archivo_borrado(char *ruta,int cadena,bool flag,char caracter ){
	int archivo = open(ruta, O_RDWR);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	char* cantidad_bloques;
	char* bloques=bloques_de_archivo(ruta);
	char* bloquesaux=string_new();
	char* contenido_de_los_bloques=string_new();
	char* md5=string_new();
	//bool contiene_bloque=contiene(bloques,string_itoa(bloque->id_bloque));
	if(!flag){
		cantidad_bloques=cantidad_de_bloques_de_archivo(ruta);
	}
	else{
		cantidad_bloques=string_itoa(atoi(cantidad_de_bloques_de_archivo(ruta))-1);
		char** bloques_divididos=string_split(bloques,",");
		int i = 0;
		bloques=string_new();
		while(i<atoi(cantidad_bloques)){
			string_append(&bloques,bloques_divididos[i]);
			string_append(&bloques,",");
			i++;
		}
	}
	char* size=string_itoa(atoi(size_de_archivo(ruta))-cadena);

	char* caracter_llenado=string_from_format("%c", caracter);
	if(!string_is_empty(bloques)){
		bloquesaux=string_substring_until(bloques,string_length(bloques)-1);
		contenido_de_los_bloques=contenido_de_bloques(bloquesaux);
		md5=generarMD5(contenido_de_los_bloques);
	}


	char* texto = string_from_format("SIZE=%s\nBLOCK_COUNT=%s\nBLOCKS=%s\nCARACTER_LLENADO=%s\nMD5_ARCHIVO=%s",
			size,cantidad_bloques,bloques,caracter_llenado,md5);
	int tamanio_texto=string_length(texto);
	if(tamanio_texto>statbuf.st_size){
		int lo_que_hay_que_agrandar=tamanio_texto-statbuf.st_size;
		ftruncate(archivo, (off_t)statbuf.st_size+lo_que_hay_que_agrandar);
	}
	else if((tamanio_texto<statbuf.st_size)){
		int lo_que_hay_que_achicar=statbuf.st_size-tamanio_texto;
				ftruncate(archivo, (off_t)statbuf.st_size-lo_que_hay_que_achicar);
	}
	for(int i=0;i<tamanio_texto;i++){
		archivo_addr[i]=texto[i];
	}

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
}

//escribe lo que se desea en un archivo
void escribir_el_archivo(char* ruta,char* cadena, t_bloque* bloque){


	if(bloque->espacio>=string_length(cadena)){
		escribir_en_block(cadena,bloque);
		actualizar_el_archivo(ruta,cadena,bloque);

	}
	else{
		char *lo_que_entra_en_el_bloque=string_substring_until(cadena,bloque->espacio);
		char *lo_que_falta_escribir=string_substring_from(cadena,string_length(lo_que_entra_en_el_bloque));
		t_bloque* nuevo_bloque=malloc(sizeof(t_bloque));
		escribir_en_block(lo_que_entra_en_el_bloque,bloque);
		actualizar_el_archivo(ruta,lo_que_entra_en_el_bloque,bloque);
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		nuevo_bloque=(t_bloque *)list_get(disco_logico->bloques,numero_del_nuevo_bloque - 1);
		escribir_el_archivo(ruta,lo_que_falta_escribir,nuevo_bloque);
	}


}

//recupera el bloque en forma de bloque de un archivo
t_bloque* recuperar_ultimo_bloque_file(char* ruta){

	t_bloque* el_bloque;
	int cantidad_de_bloques=0;
	char* sizeArchivo=size_de_archivo(ruta);
	struct stat statbuf;
	int archivo = open(ruta, O_RDWR);
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE, MAP_SHARED, archivo, 0);
	if(atoi(sizeArchivo)==0){
		int numero_del_nuevo_bloque = obtener_bloque_libre();
		el_bloque=(t_bloque *)list_get(disco_logico->bloques,numero_del_nuevo_bloque - 1);
	}
	else{
		char **renglones_file= string_split(archivo_addr, "\n");
		char **linea_bloques_file= string_split(renglones_file[2], "=");
		char **bloques_file= string_split(linea_bloques_file[1], ",");
		while(bloques_file[cantidad_de_bloques]){
			cantidad_de_bloques++;
		}
		el_bloque=(t_bloque *)list_get(disco_logico->bloques, atoi(bloques_file[cantidad_de_bloques-2])-1);
	}

	munmap(archivo_addr,statbuf.st_size);
	close(archivo);

	return el_bloque;
}

//borra los caracteres de un archivo y ajusta los bloques
void borrar_del_archivo(char *ruta,int cant_borrar, t_bloque* bloque,char caracter){
	int caracteresOcupados=caracteres_ocupados(bloque->inicio,bloque->fin);

	if(cant_borrar<caracteresOcupados){
		for(int i = 0;i<cant_borrar;i++){
			block_mmap[bloque->posicion_para_escribir-1]=' ';
			bloque->espacio++;
			bloque->posicion_para_escribir--;
		}
		actualizar_archivo_borrado(ruta,cant_borrar,false, caracter);

	}
	else{
	//borrar_todo y actualizar el archivo
		for(int i = bloque->posicion_para_escribir-1;i>=bloque->inicio;i--){
			block_mmap[i]=' ';
		}
		bloque->espacio=bloque->fin-bloque->inicio+1;
		bloque->posicion_para_escribir=bloque->inicio;
		actualizar_archivo_borrado(ruta,cant_borrar,true,caracter);
		pthread_mutex_lock(&mutex_bitmap);
		bitarray_clean_bit(bitmap,bloque->id_bloque-1);
		pthread_mutex_unlock(&mutex_bitmap);
	}
}

//avisa que el archivo no existe por log
void avisar_que_no_existe(char* ruta){
	log_info(mongoLogger, "El archivo %s al que intenta borrar no existe",ruta);
}

//vacia to do un bloque
void vaciar_bloque(t_bloque* bloque){
	for(int i=bloque->posicion_para_escribir;i>=bloque->inicio;i--)
	block_mmap[i]=' ';
}

//comprueba si el archivo existe y si existe elimina los caracteres deseados
void eliminar_del_archivo(char* ruta,int cant_borrar,char caracter){
	char* contenido_total=string_new();
	char* bloques;
	if(atoi(size_de_archivo(ruta))>0){
		bloques=bloques_de_archivo(ruta);
		char* bloquesaux=string_substring_until(bloques,string_length(bloques)-1);
		contenido_total=contenido_de_bloques(bloquesaux);
	}


	t_bloque* bloque=malloc(sizeof(t_bloque*));

	if(string_length(contenido_total)>=cant_borrar){
		//recupero el ultimo bloque
		bloque= recuperar_ultimo_bloque_file(ruta);

		//int cantidad_ocupada=bloque->posicion_para_escribir-bloque->inicio-1;

		int cantidadOcupada=caracteres_ocupados(bloque->inicio,bloque->fin);
		int lo_que_puedo_borrar;
		int lo_que_falta;
		//me fijo si la cantidad ocupada es mayor que la cantidad a borrar
		if(cantidadOcupada>=cant_borrar){
			borrar_del_archivo(ruta,cant_borrar,bloque,caracter);
		}
		else{
			lo_que_puedo_borrar=cantidadOcupada;
			//sino borro lo que puedo
			borrar_del_archivo(ruta,lo_que_puedo_borrar,bloque,caracter);
			lo_que_falta=cant_borrar-lo_que_puedo_borrar;
			//elimino del archivo lo que falta
			eliminar_del_archivo(ruta,lo_que_falta,caracter);
		}
	}
	else{
		printf("vas a borrar mas de la cuenta \n");
		char** bloques_divididos=string_split(bloques,",");
		int cantidad_de_bloques= atoi(cantidad_de_bloques_de_archivo(ruta));
		inicializar_archivo(ruta,0,caracter);
		for(int i = 0 ; i < cantidad_de_bloques;i++){
			bloque=list_get(disco_logico->bloques,atoi(bloques_divididos[i])-1);
			vaciar_bloque(bloque);
			bloque->espacio=bloque->fin-bloque->inicio+1;
			bloque->posicion_para_escribir=bloque->inicio;
			pthread_mutex_lock(&mutex_bitmap);
			bitarray_clean_bit(bitmap,atoi(bloques_divididos[i])-1);
			pthread_mutex_unlock(&mutex_bitmap);

		}
		log_info(mongoLogger, "se borro mas de la cuenta");
	}

}

//crea el archivo de generar oxigeno y escribe en el
void generar_oxigeno(int cantidad){
	char* cadena=string_repeat('O', cantidad);
	char* ruta_oxigeno =string_new();
	string_append(&ruta_oxigeno,puntoMontaje);
	string_append(&ruta_oxigeno,"/Files/oxigeno.ims");

	//Verificar que exista un archivo llamado Oxigeno.ims en el i-Mongo-Store
	sem_wait(&semaforo_para_file_oxigeno);
	int existeArchivo = access(ruta_oxigeno, F_OK);

	t_bloque* bloque=malloc(sizeof(bloque));

	if(existeArchivo==-1){
		inicializar_archivo(ruta_oxigeno,cantidad, 'O');

		int numero_del_nuevo_bloque = obtener_bloque_libre();
		bloque=(t_bloque *)list_get(disco_logico->bloques,numero_del_nuevo_bloque - 1);
		//Si no existe el archivo, crearlo y asignarle el carácter de llenado O
		escribir_el_archivo(ruta_oxigeno,cadena,bloque);

	}
	//Agregar tantos caracteres de llenado del archivo como indique el parámetro CANTIDAD
	else{
		bloque=recuperar_ultimo_bloque_file(ruta_oxigeno);
		escribir_el_archivo(ruta_oxigeno,cadena,bloque);
	}

	sem_post(&semaforo_para_file_oxigeno);


}
//comprueba el archivo de generar oxigeno y borra en el
void consumir_oxigeno(int cant_borrar){
	char* ruta_oxigeno =string_new();
	string_append(&ruta_oxigeno,puntoMontaje);
	string_append(&ruta_oxigeno,"/Files/oxigeno.ims");
	int existeArchivo = access(ruta_oxigeno, F_OK);

	if(existeArchivo!=-1){
		eliminar_del_archivo(ruta_oxigeno,cant_borrar,'O');
	}
	else{
		avisar_que_no_existe(ruta_oxigeno);
	}

}

//crea el archivo de generar comida y escribe en el
void generar_comida(int cantidad){
	char* cadena=string_repeat('C', cantidad);
	char* ruta_comida =string_new();
	string_append(&ruta_comida,puntoMontaje);
	string_append(&ruta_comida,"/Files/comida.ims");

	//Verificar que exista un archivo llamado Oxigeno.ims en el i-Mongo-Store
	sem_wait(&semaforo_para_file_comida);
	int existeArchivo = access(ruta_comida, F_OK);

	t_bloque* bloque=malloc(sizeof(bloque));

	if(existeArchivo==-1){
		inicializar_archivo(ruta_comida,cantidad, 'C');

		int numero_del_nuevo_bloque = obtener_bloque_libre();
		bloque=(t_bloque *)list_get(disco_logico->bloques,numero_del_nuevo_bloque - 1);
		//Si no existe el archivo, crearlo y asignarle el carácter de llenado O
		escribir_el_archivo(ruta_comida,cadena,bloque);

	}
	//Agregar tantos caracteres de llenado del archivo como indique el parámetro CANTIDAD
	else{
		bloque=recuperar_ultimo_bloque_file(ruta_comida);
		escribir_el_archivo(ruta_comida,cadena,bloque);
	}

	sem_post(&semaforo_para_file_comida);

}

//comprueba el archivo de generar comida y borra en el
void consumir_comida(int cant_borrar){
	char* ruta_comida =string_new();
		string_append(&ruta_comida,puntoMontaje);
		string_append(&ruta_comida,"/Files/comida.ims");
		int existeArchivo = access(ruta_comida, F_OK);

		if(existeArchivo!=-1){
			eliminar_del_archivo(ruta_comida,cant_borrar,'C');
		}
		else{
			avisar_que_no_existe(ruta_comida);
		}
}

//crea el archivo de generar basura y escribe en el
void generar_basura(int cantidad){
	char* cadena=string_repeat('B', cantidad);
	char* ruta_basura =string_new();
	string_append(&ruta_basura,puntoMontaje);
	string_append(&ruta_basura,"/Files/basura.ims");

	//Verificar que exista un archivo llamado Oxigeno.ims en el i-Mongo-Store
	sem_wait(&semaforo_para_file_basura);
	int existeArchivo = access(ruta_basura, F_OK);

	t_bloque* bloque=malloc(sizeof(bloque));

	if(existeArchivo==-1){
		inicializar_archivo(ruta_basura,cantidad, 'B');

		int numero_del_nuevo_bloque = obtener_bloque_libre();
		bloque=(t_bloque *)list_get(disco_logico->bloques,numero_del_nuevo_bloque - 1);
		//Si no existe el archivo, crearlo y asignarle el carácter de llenado O
		escribir_el_archivo(ruta_basura,cadena,bloque);

	}
	//Agregar tantos caracteres de llenado del archivo como indique el parámetro CANTIDAD
	else{
		bloque=recuperar_ultimo_bloque_file(ruta_basura);
		escribir_el_archivo(ruta_basura,cadena,bloque);
	}
	sem_post(&semaforo_para_file_basura);


}

//comprueba el archivo de generar basura y borra en el
void descartar_basura(int cant_borrar){

	char* ruta_basura =string_new();
		string_append(&ruta_basura,puntoMontaje);
		string_append(&ruta_basura,"/Files/basura.ims");
		int existeArchivo = access(ruta_basura, F_OK);

		if(existeArchivo!=-1){
			eliminar_del_archivo(ruta_basura,cant_borrar,'B');
			remove(ruta_basura);
		}
		else{
			avisar_que_no_existe(ruta_basura);
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

		nuevo_bloque=(t_bloque *)list_get(disco_logico->bloques,numero_del_nuevo_bloque-1 );

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
	//int cantidad = (double)ceil(string_length(texto)/block_size);
	double cantidad =(double) ceil((double)string_length(texto)/(double)256);
	return cantidad;
}

