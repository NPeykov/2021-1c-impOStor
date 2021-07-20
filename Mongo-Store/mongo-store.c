#include "bitmap.c"
#include "mongo-store.h"



int main() {
	signal(SIGUSR1,rutina); //Recepcion mensaje de sabotaje

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
	return EXIT_SUCCESS;
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
		int cliente = esperar_cliente(socket);
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
	char *ruta=string_new();
	string_append(&ruta, dirBitacora);
	string_append(&ruta, "/");
	char *idTripulante=string_itoa(tripulante->idTripulante);
	char *idPatota=string_itoa(tripulante->idPatota);
	string_append(&ruta, "Tripulante");
	string_append(&ruta, idTripulante);
	string_append(&ruta, "Patota");
	string_append(&ruta, idPatota);
	string_append(&ruta, ".ims");

	int existeArchivo = access(ruta, F_OK);
	FILE *archivo = fopen(ruta, "a+");

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
		//					tripulanteEnMovimiento->destinoY);
	  return;*/
}




