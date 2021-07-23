#include "bitmap.c"
#include "mongo-store.h"



int main() {
	m_movimiento_tripulante *tripulante=malloc(sizeof(m_movimiento_tripulante));
	t_bloque* bloque1 = malloc(sizeof(t_bloque));
	t_bloque* bloque2 = malloc(sizeof(t_bloque));
	t_bloque* bloque3 = malloc(sizeof(t_bloque));
	t_bloque* bloque4 = malloc(sizeof(t_bloque));
	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG);
	void* puntero_a_bits =malloc(1); //un byte de memoria, como por ejemplo malloc(1)
	bitmap=bitarray_create(puntero_a_bits, 1);
	//bool a=bitarray_test_bit(bitmap, 0);
	//printf("valor: %d\n",a);
	bitarray_set_bit(bitmap, 0);
	bitarray_set_bit(bitmap, 1);
	//a=bitarray_test_bit(bitmap, 0);
	//printf("valor: %d\n",a);
	//int bloqueLibre= obtener_bloque_libre(bitmap);
	//printf("bloque libre: %d\n",bloqueLibre);

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
	crearEstructuraDiscoLogico();
	list_add(disco_logico->bloques,bloque1);
	list_add(disco_logico->bloques,bloque2);
	list_add(disco_logico->bloques,bloque3);
	list_add(disco_logico->bloques,bloque4);
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
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);
	actualizar_posicion(tripulante);

	munmap(block_mmap,statbuf.st_size);
	close(archivo);
	/*signal(SIGUSR1,rutina); //Recepcion mensaje de sabotaje

	sem_init(&dar_orden_sabotaje,0 , 0);
	sem_init(&contador_sabotaje, 0, 1);


	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(mongoConfig, "PUERTO");

	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_DEBUG);

	crearEstructuraFileSystem();

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);

	gestionarCliente(socket_mongo_store );

	printf("SOCKET DISCO %d\n", socket_mongo_store);
	return EXIT_SUCCESS;*/
}

t_disco_logico* crearEstructuraDiscoLogico(){
	//TODO poner semaforo para disco logico
	disco_logico=(t_disco_logico *)malloc(sizeof(t_disco_logico));
	disco_logico->bloques=list_create();
	return disco_logico;
}

void crearEstructurasBloques(){
	//TODO poner semaforo para disco logico
	int tamanio_de_bloque=atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
	int cantidad_de_bloques=atoi(config_get_string_value(mongoConfig,"BLOCKS"));
	t_bloque *bloque = (t_bloque *)malloc(sizeof(t_bloque));

	for(int contador=1;contador<=cantidad_de_bloques;contador++){
		bloque->id_bloque=contador;
		bloque->inicio= (contador-1) * tamanio_de_bloque;
		bloque->fin = bloque->inicio+ (tamanio_de_bloque-1);
		bloque->espacio=tamanio_de_bloque;
		bloque->posicion_para_escribir=bloque->inicio;
		list_add(disco_logico->bloques,bloque);

	}
	free(bloque);
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

	while(bloques_bitacora[cantidad_de_bloques]!=NULL){
		cantidad_de_bloques++;
	}
	el_bloque=(t_bloque *)list_get(disco_logico->bloques, atoi(bloques_bitacora[cantidad_de_bloques-1])-1);
	munmap(archivo_addr,statbuf.st_size);
	close(archivo);
	return el_bloque;
}

void inicializar_bitacora(char *rutaBitacora, char *numeroDeBloque){
	int archivo = open(rutaBitacora, O_WRONLY|O_CREAT);
	struct stat statbuf;
	fstat(archivo,&statbuf);
	char *archivo_addr =mmap(NULL,statbuf.st_size,PROT_WRITE, MAP_SHARED, archivo, 0);
	char *cadena=string_new();
	string_append(&cadena,"SIZE=");
	char *block_size= config_get_string_value(mongoConfig,"BLOCK_SIZE");
	string_append(&cadena,block_size);
	string_append(&cadena,"/nBLOCKS=");
	string_append(&cadena,numeroDeBloque);
	archivo_addr=cadena;
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


void crearEstructuraFileSystem()
{
	FILE *f;

	puntoMontaje = config_get_string_value(mongoConfig,"PUNTO_MONTAJE");
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



	if(mkdir(puntoMontaje, 0777) != 0)
	{
	//Directorio de montaje existente, se verifica que exista superbloque y blocks
	  char* superBloqueRuta = malloc(strlen(puntoMontaje) + strlen("/SuperBloque.ims") + 1);
	  strcpy(superBloqueRuta, puntoMontaje);
	  strcat(superBloqueRuta, "/SuperBloque.ims");
	  char* blocksRuta = malloc(strlen(puntoMontaje) + strlen("/Blocks.ims") + 1);
	  strcpy(blocksRuta, puntoMontaje);
	  strcat(blocksRuta, "/Blocks.ims");
	  int existeArchivo = access(superBloqueRuta, F_OK);
	  if(existeArchivo == 0)
	  {
		  existeArchivo = access(blocksRuta, F_OK);
	      if(existeArchivo == 0)
	      {
	    	  //Si el fileSystem esta creado se toman los datos de la metadata existente.
	    	  log_info(mongoLogger, "El directorio %s ya existe. ", puntoMontaje);
	    	  block_size=atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
	    	  blocks=atoi(config_get_string_value(mongoConfig,"BLOCKS"));
	    	  bitmap = crear_bitmap(superBloqueRuta, blocks);
	    	  return;
		  }
	      else{
	    	  log_error(mongoLogger,"No se encuentra archivo Blocks.ims");
	      }
	      return;
	  }
	  else
	  {
		  log_error(mongoLogger,"No se encuentra archivo SuperBloque.ims");
	  }
	  free(superBloqueRuta);
	  free(blocksRuta);
	  return;

	}
	else
	{
	printf("Se creo el directorio de montaje =) \n");
	//Crea superBloque
	char* superBloqueRuta = malloc(strlen(puntoMontaje) + strlen("/SuperBloque.ims") + 1);
	strcpy(superBloqueRuta, puntoMontaje);
	strcat(superBloqueRuta, "/SuperBloque.ims");
	block_size=atoi(config_get_string_value(mongoConfig,"BLOCK_SIZE"));
	blocks=atoi(config_get_string_value(mongoConfig,"BLOCKS"));
	// Creo el archivo superBloque
	f = fopen(superBloqueRuta, "w");
	char* tamanioBloque = malloc(10); sprintf(tamanioBloque, "%d",block_size);

	char* bloques = malloc(10);
	sprintf(bloques, "%d",blocks);
	int bitmapsize = blocks / 8;
	int sizesuperbloque = string_length(tamanioBloque)+ string_length(bloques)+ bitmapsize;
	fputs(tamanioBloque,f);
	fputs(bloques,f);

	fseek(f, sizesuperbloque , SEEK_SET);
	putc('\0', f);
	fclose(f);
	//Creo el archivo bitmap.bin
	bitmap = crear_bitmap(superBloqueRuta,blocks);
	free(superBloqueRuta);
	free(tamanioBloque);
	free(bloques);

	//Crea Blocks
	char* blocksRuta = malloc(strlen(puntoMontaje) + strlen("/Blocks.ims") + 1);
	strcpy(blocksRuta, puntoMontaje);
	strcat(blocksRuta, "/Blocks.ims");
	//			log_trace(mongoLogger, "Estructura creada.");
	int X = block_size * blocks;
	f = fopen(blocksRuta, "w");
	fputs("1", f);
	fseek(f, X , SEEK_SET);
	putc('\0', f);
	fclose(f);
	free(blocksRuta);
	//Creo carpeta Files
	if(mkdir(dirFiles, 0777) == 0)
	{
	 printf("Se creo carpeta Files  =) \n");
	 //Creo carpeta Bitacora
	 if(mkdir(dirBitacora, 0777) == 0)
	 {
	  printf("Se creo carpeta Bitacora  =) \n");
	 }

	}
    else
    {
    log_error(mongoLogger, "Ha ocurrido un error al crear el directorio Files.");
	return;
    }
}
}


void escribir_en_block(char* lo_que_se_va_a_escribir,t_bloque* el_bloque){
	for(int i=0;i<string_length(lo_que_se_va_a_escribir);i++){
		block_mmap[el_bloque->posicion_para_escribir]=lo_que_se_va_a_escribir[i];
		el_bloque->posicion_para_escribir++;
	}
	el_bloque->espacio=el_bloque->espacio-string_length(lo_que_se_va_a_escribir);

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
//	socket_cliente = esperar_cliente(socket);
	int conexionCliente;
	t_list* lista;
	int operacion;
	t_paquete *paquete;
	int respuesta;


	while (1) {
		int cliente = esperar_cliente(socket, mongoLogger);
		printf("Cliente: %d\n", cliente);
		operacion = recibir_operacion(cliente);
		lista = NULL;

		printf("\nLA OPERACION ES: %d\n", operacion);



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
			uint32_t idPatota = (uint32_t) atoi(list_get(lista,1));
			printf("Tripulante recibido %d\n", idTripulante);
			paquete=crear_paquete(OBTENGO_BITACORA);
			printf("Patota recibida %d\n", idPatota);
			//agregar los elementos encontrados para ese ID al paquete "paquete"
			enviar_paquete(paquete,cliente);
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
		case ESPERANDO_SABOTAJE:;
		      pthread_create(&hilo_sabotaje, NULL, (void*)enviar_aviso_sabotaje_a_discordiador, (void*)cliente);
		      pthread_detach(hilo_sabotaje);
		      break;
		case ACTUALIZAR_POSICION:;

			tripulanteEnMovimiento = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));

			tripulanteEnMovimiento = recibirMovimientoTripulante(cliente);
			//Se escribe en blocks.ims
			actualizar_posicion(tripulanteEnMovimiento);

			printf("Tripulante N: %d de la patota %d se movio de (%d, %d) a (%d, %d)",
					tripulanteEnMovimiento->idTripulante,
		     		tripulanteEnMovimiento->idPatota,
					tripulanteEnMovimiento->origenX,
					tripulanteEnMovimiento->origenY,
					tripulanteEnMovimiento->destinoX,
					tripulanteEnMovimiento->destinoY);
			liberar_cliente(cliente);
			break;

		case INICIO_TAREA:;
			m_estado_tarea_tripulante *tripulanteConTarea = (m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));
			tripulanteConTarea = recibirNuevoEstadoTareaTripulante(cliente);
			printf("Nombre tarea: %s\n", tripulanteConTarea->nombreTarea);
			printf("Duracion: %d\n", tripulanteConTarea->duracionTarea);
			liberar_cliente(cliente);
			break;

		case FIN_TAREA:;
			m_estado_tarea_tripulante *tripulanteConTareaFinalizada = (m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));

			tripulanteConTareaFinalizada = recibirNuevoEstadoTareaTripulante(cliente);

			//aca avisaria A BITACORA que termino tarea independientemente si es IO/COMUN

			if(tripulanteConTareaFinalizada -> tipo_tarea == TAREA_IO){

				//aca llenaria el archivo tantas veces como el 'parametro'
				funcion_para_llenar_con_tarea_IO(tripulanteConTareaFinalizada);
			}
			liberar_cliente(cliente);
			break;

		case -1:
			printf("El cliente %d se desconecto.\n", cliente);
			liberar_cliente(cliente);
			break;
		default:
			printf("Operacion desconocida.\n");
			liberar_cliente(cliente);
			break;

		}
		//liberar_cliente(cliente);

	}
//	 Se mueve de X|Y a X’|Y’
//	 Comienza ejecución de tarea X
//	 Se finaliza la tarea X
//	 Se corre en pánico hacia la ubicación del sabotaje
//	 Se resuelve el sabotaje
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

void descartar_basura(int cant_borrar){
  		char* basuraRuta = malloc(strlen(dirFiles) + strlen("/Basura.ims") + 1);
		strcpy(basuraRuta, dirFiles);
		strcat(basuraRuta, "/Basura.ims");
    	  if(access(basuraRuta, F_OK) == 0){
    		  remove(basuraRuta);
    	  }

    	  else{
    	    printf("SACAR-BASURA: no existe archivo!\n");
    	    //no existe el archivo
    	    log_error(mongoLogger, "No existe el archivo Basura.ims");
    	    return;
    	  }
    	}
void actualizar_posicion(m_movimiento_tripulante *tripulante){
	t_bloque *el_bloque = (t_bloque*)malloc(sizeof(t_bloque*));
	t_bloque *nuevo_bloque = (t_bloque*)malloc(sizeof(t_bloque*));
	char *bloque;
	char *lo_que_se_va_a_escribir=string_new();
	char *rutaBitacora=string_new();
	char *idTripulante=string_itoa(tripulante->idTripulante);
	char *idPatota=string_itoa(tripulante->idPatota);
	char *xinicio=string_itoa(tripulante->origenX);
	char *yinicio=string_itoa(tripulante->origenY);
	char *xfinal=string_itoa(tripulante->destinoX);
	char *yfinal=string_itoa(tripulante->destinoY);
	int numero_del_nuevo_bloque;


	//1)buscar bitacora del tripulante
	//2)Sino existe crearla sino pasar al paso 3
	//3)leer la bitacora blocks y size
	//4)ir a block.ims
//	Ej: Si mi tripulante se tiene que mover de 0|0 a 3|3
//
//	Lo que se va a escribir en mi archivo Blocks.ims va a ser:

//	Se mueve de 0|0 a 0|1
//	Se mueve de 0|1 a 0|2
//	Se mueve de 0|2 a 0|3
//	Se mueve de 0|3 a 1|3
//	Se mueve de 1|3 a 2|3
//	Se mueve de 2|3 a 3|3
	//armo toda la cadena que se va a escribir por movimiento
	string_append(&lo_que_se_va_a_escribir, "Se mueve de ");
	string_append(&lo_que_se_va_a_escribir, xinicio);
	string_append(&lo_que_se_va_a_escribir, "|");
	string_append(&lo_que_se_va_a_escribir, yinicio);
	string_append(&lo_que_se_va_a_escribir," a ");
	string_append(&lo_que_se_va_a_escribir,xfinal);
	string_append(&lo_que_se_va_a_escribir,"|");
	string_append(&lo_que_se_va_a_escribir,yfinal);
	string_append(&lo_que_se_va_a_escribir,"\n");

	//armo la ruta de la bitacora correspondiente al tripulante
	char *dirBitacora="/home/utnso/workspace/mnt/Files/Bitacora";
	string_append(&rutaBitacora, dirBitacora);
	string_append(&rutaBitacora, "/");
	string_append(&rutaBitacora, "Tripulante");
	string_append(&rutaBitacora, idTripulante);
	string_append(&rutaBitacora, "Patota");
	string_append(&rutaBitacora, idPatota);
	string_append(&rutaBitacora, ".ims");

	int existeArchivo = access(rutaBitacora, F_OK);
	//si la bitacora del tripulante existe, entonces recupero el ultimo bloque
	if(existeArchivo==0){
		//ultimo bloque en char
		el_bloque=buscar_ultimo_bloque_del_tripulante(rutaBitacora);
		//si hay espacio en el bloque para escribir to do, entonces lo escribo
		if(el_bloque->espacio>string_length(lo_que_se_va_a_escribir)){
			escribir_en_block(lo_que_se_va_a_escribir,el_bloque);
			}
		//sino escribo una parte y elijo otro bloque para lo restante
			else{
				char *lo_que_entra_en_el_bloque=string_substring_until(lo_que_se_va_a_escribir,el_bloque->espacio);
				escribir_en_block(lo_que_entra_en_el_bloque,el_bloque);
				char *lo_que_falta_escribir=string_substring_from(lo_que_se_va_a_escribir,string_length(lo_que_entra_en_el_bloque));
				//asigno el bloque nuevo
				numero_del_nuevo_bloque = obtener_bloque_libre(bitmap);
				nuevo_bloque=(t_bloque *)list_get(disco_logico->bloques, numero_del_nuevo_bloque);
				//escribir lo_que_falta_escribir
				escribir_en_block(lo_que_falta_escribir,nuevo_bloque);
				//actualizar el bitmap
				ocupar_bloque(bitmap, numero_del_nuevo_bloque);
				// actualizar bitacora con el bloque nuevo del tripulante
				agregar_bloque_bitacora(rutaBitacora,nuevo_bloque->id_bloque);
			}
	}
	else{
		//asigno el bloque nuevo
		numero_del_nuevo_bloque = obtener_bloque_libre(bitmap);
		//creo el archivo bitacora
		inicializar_bitacora(rutaBitacora,string_itoa(numero_del_nuevo_bloque));
		nuevo_bloque=(t_bloque *)list_get(disco_logico->bloques, numero_del_nuevo_bloque);
		//modifico bitmap
		ocupar_bloque(bitmap, numero_del_nuevo_bloque);
		//escribo lo_que_se_va_a_escribir en block
		escribir_en_block(lo_que_se_va_a_escribir,nuevo_bloque);
		//escribo tamaño y bloque en archivo bitacora
		agregar_bloque_bitacora(rutaBitacora,nuevo_bloque->id_bloque);

	}


/*

	FILE *archivo = fopen(rutaBitacora, "a+");

	  if(existeArchivo == 0){
//Bitacora existente
	    fseek(archivo, 0 , SEEK_END);
//		memcpy(nuevo_bloques_config,bloques_config,tamanio_bloques_config-strlen(bloques_array[viejaCantidadDeBloques-1])-2);
//	    memcpy(atrapar,&menosUno,sizeof(int));
//	    for(int i=0; i < cantidad; i++)
	      fputc('C', archivo);
	  }
	  else {
//Se crea archivo para el tripulante de la patota
	      fputc('C', archivo);
	  }
	  fclose(archivo);
		//					tripulanteEnMovimiento->idPatota,
		//					tripulanteEnMovimiento->idTripulante,
		//					tripulanteEnMovimiento->origenX,
		//					tripulanteEnMovimiento->origenY,
		//					tripulanteEnMovimiento->destinoX,
		//					tripulanteEnMovimiento->destinoY);*/
	  return;
}
int cantidad_bloques_a_ocupar(char* texto)
{
	int cantidad = string_length(texto)/block_size;
	return cantidad;
}



