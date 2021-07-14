#include "mongo-store.h"

void enviar_mensaje_a_discordiador();

int main() {

	int socket_mongo_store, socket_cliente;
//	t_config *config;
	char* puerto;
//	t_log* logger;

	pthread_t hilo_sabotaje;


	//-------------------------------------------------------//



	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(mongoConfig, "PUERTO");

	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_DEBUG);

	crearEstructuraFileSystem();

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);


	pthread_create(&hilo_sabotaje, NULL, (void*)enviar_mensaje_a_discordiador, (void*)socket_mongo_store);
	pthread_detach(hilo_sabotaje);

	gestionarCliente(socket_mongo_store );

	//signal(SIGUSR1,rutina);


	return EXIT_SUCCESS;
}

//para probar el aviso de inicio de sabotaje
void enviar_mensaje_a_discordiador(void *data){
	int socket_mongo_store = (int) data;
	int socket_para_sabotaje = esperar_cliente(socket_mongo_store);

	sleep(50);

	printf("ESTOY POR ENVIAR SABOTAJE\n");

	enviar_mensaje(INICIO_SABOTAJE, "Ks", socket_para_sabotaje);
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

	char *metadataRuta = malloc(strlen(dirMetadata) + strlen("/Metadata.AFIP") + 1); // /Metadata.bin  /Bitmap.bin
	strcpy(metadataRuta, dirMetadata);
	strcat(metadataRuta, "/Metadata.AFIP"); ///Metadata.bin  /Bitmap.bin


	if(mkdir(puntoMontaje, 0777) != 0)
	{
		printf("El directorio de montaje ya existe!! =( \n");
		/*
		//Si el fileSystem esta creado se toman los datos de la metadata existente.
		log_info(logger, "El directorio %s ya existe. ", puntoMontaje);
		t_config* metadataConfig=config_create(metadataRuta);

		block_size=atoi(config_get_string_value(metadataConfig,"BLOCK_SIZE"));
		blocks=atoi(config_get_string_value(metadataConfig,"BLOCKS"));
		magic_number=malloc(20);
		strcpy(magic_number,(char*)config_get_string_value(metadataConfig,"MAGIC_NUMBER"));

		config_destroy(metadataConfig);

		log_info(logger,"BLOCK_SIZE: %d. BLOCKS: %d. MAGIC_NUMBER: %s. ", block_size, blocks, magic_number);
		bitmap = crear_bitmap(dirMetadata, blocks);
		free(metadataRuta);
		*/
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
			 fputs("BLOCK_SIZE=", f); fputs(tamanioBloque,f); fputs("\n",f);
			 char* bloques = malloc(10);
			 sprintf(bloques, "%d",block_size);
			 fputs("BLOCKS=", f);fputs(bloques,f); fputs("\n",f);
			 fclose(f);
			 free(superBloqueRuta);
			 free(tamanioBloque);
			 free(bloques);
			//Crep el archivo bitmap.bin
			 bitmap = crear_bitmap(puntoMontaje,blocks);


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
					// Creo archivo Metadata.bin
					char* metadataRuta = malloc(strlen(dirFiles) + strlen("/Metadata.ims") + 1); // /Metadata.bin o Bitmap.bin
					strcpy(metadataRuta, dirFiles);
					strcat(metadataRuta, "/Metadata.ims"); // /Metadata.bin o Bitmap.bin
					// Creo el archivo Metadata.bin (para indicar que es un directorio)
					f = fopen(metadataRuta, "w");
					fputs("SIZE=132", f);
					fclose(f);
					free(metadataRuta);
					//Creo carpeta Bitacora
					if(mkdir(dirBitacora, 0777) == 0)
					{
						printf("Se creo carpeta Bitacora  =) \n");
					}
				}
				else
				{
					log_error(mongoLogger, "Ha ocurrido un error al crear el directorio Files.");
					free(metadataRuta);
					return;
				}
	}
}


t_bitarray* crear_bitmap(char *ubicacion, int cant_bloques){
	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_TRACE);


	size_t size = (size_t) cant_bloques / 8;
	//printf("\nSize = %d\n", size);
	char *rutaBitmap = malloc(strlen(ubicacion) + 20);
	strcpy(rutaBitmap, ubicacion);
	strcat(rutaBitmap, "/Bitmap.bin");

	int fd = open(rutaBitmap, O_CREAT | O_RDWR, 0777);

	if (fd == -1) {
		log_error(mongoLogger, "Error al abrir el archivo Bitmap.bin");
		exit(1);
	}
	ftruncate(fd, size);

	void* bmap = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (bmap == MAP_FAILED) {
		close(fd);
		exit(1);
	}

	t_bitarray* bitmap = bitarray_create_with_mode((char*) bmap, size, MSB_FIRST);


	msync(bitmap, size, MS_SYNC);
	free(rutaBitmap);
	return bitmap;
}


//TODO: cambiar nombre funcion y completar
//NOTA: voy a buscar las funciones de el manejo de archivo q habia hecho

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
	int cliente;

	while (1) {
		cliente = esperar_cliente(socket);
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
			break;
		case ELIMINAR_TRIPULANTE:

//				lista = recibir_paquete(cliente);
//				int idTripulante = atoi((char *) list_get(lista,0));
//				eliminarTripulante(idTripulante);
//				printf("Tripulante eliminado de la nave %d\n", idTripulante);
			//liberar_cliente(cliente);
			break;
		case ACTUALIZAR_POSICION:;

			m_movimiento_tripulante *tripulanteEnMovimiento = (m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));

			tripulanteEnMovimiento = recibirMovimientoTripulante(cliente);

			printf("Tripulante N: %d se movio de (%d, %d) a (%d, %d)",
					tripulanteEnMovimiento->idPatota,
					tripulanteEnMovimiento->origenX,
					tripulanteEnMovimiento->origenY,
					tripulanteEnMovimiento->destinoX,
					tripulanteEnMovimiento->destinoY);

			break;

		case INICIO_TAREA:;
			m_estado_tarea_tripulante *tripulanteConTarea = (m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));
			tripulanteConTarea = recibirNuevoEstadoTareaTripulante(cliente);
			printf("Nombre tarea: %s\n", tripulanteConTarea->nombreTarea);
			printf("Duracion: %d\n", tripulanteConTarea->duracionTarea);

			break;

		case FIN_TAREA:;
			m_estado_tarea_tripulante *tripulanteConTareaFinalizada = (m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));

			tripulanteConTareaFinalizada = recibirNuevoEstadoTareaTripulante(cliente);

			//aca avisaria A BITACORA que termino tarea independientemente si es IO/COMUN

			if(tripulanteConTareaFinalizada -> tipo_tarea == TAREA_IO){

				//aca llenaria el archivo tantas veces como el 'parametro'
				funcion_para_llenar_con_tarea_IO(tripulanteConTareaFinalizada);
			}

			break;

		case -1:
			printf("El cliente %d se desconecto.\n", cliente);
			//liberar_cliente(cliente);
			break;
		default:
			printf("Operacion desconocida.\n");
			break;

		}
		liberar_cliente(cliente);

	}
//	 Se mueve de X|Y a X’|Y’
//	 Comienza ejecución de tarea X
//	 Se finaliza la tarea X
//	 Se corre en pánico hacia la ubicación del sabotaje
//	 Se resuelve el sabotaje
}

void gestionarSabotaje(){
int operacion;
	switch(operacion) {
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
void rutina(int n){
switch(n) {
		case SIGUSR1:;
			printf("llego sigusr1");
			break;
		case 2:
			break;
    default:
	printf("Operacion desconocida.\n");
	break;
}
}

int obtener_bloque_libre(t_bitarray* bitmap){
	size_t tamanio = bitarray_get_max_bit(bitmap);
	int i;
	for(i=0; i<tamanio; i++){
		if(bitarray_test_bit(bitmap, i)== 0){
			return i;
		}
	}
	return -1;
}
void ocupar_bloque(t_bitarray* bitmap, int bloque){
	bitarray_set_bit(bitmap,bloque);
	return;
}
void liberar_bloque(t_bitarray* bitmap, int bloque){
	bitarray_clean_bit(bitmap,bloque);
	return;
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
