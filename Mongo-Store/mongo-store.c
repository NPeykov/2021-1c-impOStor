#include "mongo-store.h"

int main() {

	int socket_mongo_store, socket_cliente;
//	t_config *config;
	char* puerto;
//	t_log* logger;

	//-------------------------------------------------------//

	mongoConfig = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(mongoConfig, "PUERTO");

	mongoLogger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_DEBUG);

	crearEstructuraFileSystem();

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);
	gestionarCliente(socket_mongo_store );
	signal(SIGUSR1,rutina);
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
			// Creo el archivo superBloque
			f = fopen(superBloqueRuta, "w");
			fputs("1", f);
			fclose(f);
			free(superBloqueRuta);
		//Crea Blocks
		char* blocksRuta = malloc(strlen(puntoMontaje) + strlen("/Blocks.ims") + 1);
		strcpy(blocksRuta, puntoMontaje);
		strcat(blocksRuta, "/Blocks.ims");
		// Creo el archivo Blocks
		f = fopen(blocksRuta, "w");
		fputs("1", f);
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
		/*
		//Si el fileSystem NO esta creado se toman los datos del archivo de configuracion.

		block_size=atoi(config_get_string_value(sindicatoConfig,"BLOCK_SIZE"));
		blocks=atoi(config_get_string_value(sindicatoConfig,"BLOCKS"));
		magic_number=malloc(20);
		strcpy(magic_number,(char*)config_get_string_value(sindicatoConfig,"MAGIC_NUMBER"));

		log_info(logger,"BLOCK_SIZE: %d. BLOCKS: %d. MAGIC_NUMBER: %s. ", block_size, blocks, magic_number);

		// Directorio Metadata
		if(mkdir(dirMetadata, 0777) == 0)
		{
			// Creo el archivo Metadata.bin
			f = fopen(metadataRuta, "w");

			char* tamanioBloque = malloc(10); sprintf(tamanioBloque, "%d",block_size);
			fputs("BLOCK_SIZE=", f); fputs(tamanioBloque,f); fputs("\n",f);

			char* bloques = malloc(10);
			sprintf(bloques, "%d",block_size);
			fputs("BLOCKS=", f);fputs(bloques,f); fputs("\n",f);

			fputs("MAGIC_NUMBER=", f); fputs(magic_number, f); fputs("\n",f);

			fclose(f);
			free(metadataRuta);
			free(tamanioBloque);
			free(bloques);
			bitmap = crear_bitmap(dirMetadata,blocks);

		}
		else
		{
			log_error(logger, "crearEstructuraFileSystem: No se pudo crear el directorio Metadata");
			free(metadataRuta);
			return;
		}

		// Directorio Files

		if(mkdir(dirFiles, 0777) == 0)
		{
			// Creo archivo Metadata.bin
			char* metadataRuta = malloc(strlen(dirFiles) + strlen("/Metadata.AFIP") + 1); // /Metadata.bin o Bitmap.bin
			strcpy(metadataRuta, dirFiles);
			strcat(metadataRuta, "/Metadata.AFIP"); // /Metadata.bin o Bitmap.bin
			// Creo el archivo Metadata.bin (para indicar que es un directorio)
			f = fopen(metadataRuta, "w");
			fputs("DIRECTORY=Y", f);
			fclose(f);
			free(metadataRuta);
		}
		else
		{
			log_error(logger, "Ha ocurrido un error al crear el directorio Files.");
			free(metadataRuta);
			return;
		}

		// Directorio Blocks

		if(mkdir(dirBlocks, 0777) == 0) {crearBloques(dirBlocks);}
		else {
			log_error(logger, "Ha ocurrido un error al crear el directorio Blocks.");
			return;
		}

		log_trace(logger, "Estructura creada.");
		*/
	}
}



//TODO: cambiar nombre funcion y completar
//NOTA: voy a buscar las funciones de el manejo de archivo q habia hecho

void funcion_para_llenar_con_tarea_IO(m_estado_tarea_tripulante* tripulanteConTareaFinalizada){

	switch (tripulanteConTareaFinalizada->codigo_tarea) {

	case GENERAR_OXIGENO:
		printf("ES GENERAR OXIGENO\n");
		printf("Cantidad a llenar: %d\n",
						tripulanteConTareaFinalizada->parametro);
		break;

	case CONSUMIR_OXIGENO:
		break;

	case GENERAR_COMIDA:
		break;

	case CONSUMIR_COMIDA:
		break;

	case GENERAR_BASURA:
		break;

	case DESCARTAR_BASURA:
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
		case ACTUALIZAR_POSICION:

			m_movimiento_tripulante *tripulanteEnMovimiento =
					(m_movimiento_tripulante *) malloc(sizeof(m_movimiento_tripulante));

			tripulanteEnMovimiento = recibirMovimientoTripulante(cliente);

			printf("Tripulante N: %d se movio de (%d, %d) a (%d, %d)",
					tripulanteEnMovimiento->idPatota,
					tripulanteEnMovimiento->origenX,
					tripulanteEnMovimiento->origenY,
					tripulanteEnMovimiento->destinoX,
					tripulanteEnMovimiento->destinoY);

			break;

		case INICIO_TAREA:
			m_estado_tarea_tripulante *tripulanteConTarea =
					(m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));
			tripulanteConTarea = recibirNuevoEstadoTareaTripulante(cliente);
			printf("Nombre tarea: %s\n", tripulanteConTarea->nombreTarea);
			printf("Duracion: %d\n", tripulanteConTarea->duracionTarea);

			break;

		case FIN_TAREA:
			m_estado_tarea_tripulante *tripulanteConTareaFinalizada =
								(m_estado_tarea_tripulante *) malloc(sizeof(m_estado_tarea_tripulante));

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
switch(n){
case SIGUSR1:
		printf("LLEGO SIGUSR1.\n”);
	break;
default:
	break;
}

}
#define PATH_OXIGENO "pruebas_tarea/Oxigeno.ims"
#define PATH_COMIDA "pruebas_tarea/Comida.ims"
#define PATH_BASURA "pruebas_tarea/Basura.ims"
void generar_oxigeno(int cantidad){
  int existeArchivo = access(PATH_OXIGENO, F_OK);
  FILE *archivo = fopen(PATH_OXIGENO, "a+");

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

  if(access(PATH_OXIGENO, F_OK) == 0){
    FILE *archivo = fopen(PATH_OXIGENO, "a+");
    long int pos_actual;
    long int cant_ox_disponible;

    fseek(archivo, 0, SEEK_END);
    cant_ox_disponible = ftell(archivo) / sizeof(char);

    printf("cantidadOxigenos = %ld\n", cant_ox_disponible);

    if(cant_borrar > cant_ox_disponible){
      fseek(archivo, 0, SEEK_SET);
      //hay que avisar que intento borrar mas de los disponible
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
    //hay que avisar
    //se podria avisar mediante un valor de retorno
    return;
  }
}


void generar_comida(int cantidad){
  int existeArchivo = access(PATH_COMIDA, F_OK);
  FILE *archivo = fopen(PATH_COMIDA, "a+");

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

  if(access(PATH_COMIDA, F_OK) == 0){
    FILE *archivo = fopen(PATH_COMIDA, "a+");
    long int pos_actual;
    long int cant_com_disponible;

    fseek(archivo, 0, SEEK_END);
    cant_com_disponible = ftell(archivo) / sizeof(char);

    printf("cantidadComida = %ld\n", cant_com_disponible);

    if(cant_borrar > cant_com_disponible){
      fseek(archivo, 0, SEEK_SET);
      //hay que avisar que intento borrar mas de los disponible
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
    //hay que avisar
    //se podria avisar mediante un valor de retorno
    return;
  }
}


void generar_basura(int cantidad){
  int existeArchivo = access(PATH_BASURA, F_OK);
  FILE *archivo = fopen(PATH_BASURA, "a+");

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

//void descartar_basura(int cant_borrar){
//
//  if(access(PATH_BASURA, F_OK) == 0){
//    FILE *archivo = fopen(PATH_BASURA, "a+");
//    long int pos_actual;
//    long int cant_bas_disponible;
//
//    fseek(archivo, 0, SEEK_END);
//    cant_bas_disponible = ftell(archivo) / sizeof(cvoid generar_oxigeno(int cantidad){
//    	  int existeArchivo = access(PATH_OXIGENO, F_OK);
//    	  FILE *archivo = fopen(PATH_OXIGENO, "a+");
//
//    	  if(existeArchivo == 0){
//    	    printf("El archivo EXISTE!\n");
//    	    fseek(archivo, -1, SEEK_END);
//    	    for(int i=0; i < cantidad; i++)
//    	      fputc('O', archivo);
//    	  }
//    	  else {
//    	    for(int i=0; i < cantidad; i++)
//    	      fputc('O', archivo);
//    	  }
//    	  fclose(archivo);
//    	}
//  }

    	void consumir_oxigeno(int cant_borrar){

    	  if(access(PATH_OXIGENO, F_OK) == 0){
    	    FILE *archivo = fopen(PATH_OXIGENO, "a+");
    	    long int pos_actual;
    	    long int cant_ox_disponible;

    	    fseek(archivo, 0, SEEK_END);
    	    cant_ox_disponible = ftell(archivo) / sizeof(char);

    	    printf("cantidadOxigenos = %ld\n", cant_ox_disponible);

    	    if(cant_borrar > cant_ox_disponible){
    	      fseek(archivo, 0, SEEK_SET);
    	      //hay que avisar que intento borrar mas de los disponible
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
    	    //hay que avisar
    	    //se podria avisar mediante un valor de retorno
    	    return;
    	  }
    	}


//    	void generar_comida(int cantidad){
//    	  int existeArchivo = access(PATH_COMIDA, F_OK);
//    	  FILE *archivo = fopen(PATH_COMIDA, "a+");
//
//    	  if(existeArchivo == 0){
//    	    printf("El archivo EXISTE!\n");
//    	    fseek(archivo, -1, SEEK_END);
//    	    for(int i=0; i < cantidad; i++)
//    	      fputc('C', archivo);
//    	  }
//    	  else {
//    	    for(int i=0; i < cantidad; i++)
//    	      fputc('C', archivo);
//    	  }
//    	  fclose(archivo);
//    	}
//
//
//    	void consumir_comida(int cant_borrar){
//
//    	  if(access(PATH_COMIDA, F_OK) == 0){
//    	    FILE *archivo = fopen(PATH_COMIDA, "a+");
//    	    long int pos_actual;
//    	    long int cant_com_disponible;
//
//    	    fseek(archivo, 0, SEEK_END);
//    	    cant_com_disponible = ftell(archivo) / sizeof(char);
//
//    	    printf("cantidadComida = %ld\n", cant_com_disponible);
//
//    	    if(cant_borrar > cant_com_disponible){
//    	      fseek(archivo, 0, SEEK_SET);
//    	      //hay que avisar que intento borrar mas de los disponible
//    	    }
//    	    else{
//    	      fseek(archivo, -cant_borrar * sizeof(char), SEEK_END);
//    	    }
//    	    pos_actual = ftell(archivo);
//    	    ftruncate(fileno(archivo), pos_actual);
//    	    fclose(archivo);
//    	    return;
//    	  }
//
//    	  else{
//    	    printf("SACAR-COM: no existe archivo!\n");
//    	    //no existe el archivo
//    	    //hay que avisar
//    	    //se podria avisar mediante un valor de retorno
//    	    return;
//    	  }
//    	}
//
//
//    	void generar_basura(int cantidad){
//    	  int existeArchivo = access(PATH_BASURA, F_OK);
//    	  FILE *archivo = fopen(PATH_BASURA, "a+");
//
//    	  if(existeArchivo == 0){
//    	    printf("El archivo EXISTE!\n");
//    	    fseek(archivo, -1, SEEK_END);
//    	    for(int i=0; i < cantidad; i++)
//    	      fputc('B', archivo);
//    	  }
//    	  else {
//    	    for(int i=0; i < cantidad; i++)
//    	      fputc('B', archivo);
//    	  }
//    	  fclose(archivo);
//    	}

//    	void descartar_basura(int cant_borrar){
//
//    	  if(access(PATH_BASURA, F_OK) == 0){
//    	    FILE *archivo = fopen(PATH_BASURA, "a+");
//    	    long int pos_actual;
//    	    long int cant_bas_disponible;
//
//    	    fseek(archivo, 0, SEEK_END);
//    	    cant_bas_disponible = ftell(archivo) / sizeof(char);
//
//    	    printf("cantidadBasura = %ld\n", cant_bas_disponible);
//
//    	    if(cant_borrar > cant_bas_disponible){
//    	      fseek(archivo, 0, SEEK_SET);
//    	      //hay que avisar que intento borrar mas de los disponible
//    	    }
//    	    else{
//    	      fseek(archivo, -cant_borrar * sizeof(char), SEEK_END);
//    	    }
//    	    pos_actual = ftell(archivo);
//    	    ftruncate(fileno(archivo), pos_actual);
//    	    fclose(archivo);
//    	    return;
//    	  }
//
//    	  else{
//    	    printf("SACAR-BASURA: no existe archivo!\n");
//    	    //no existe el archivo
//    	    //hay que avisar
//    	    //se podria avisar mediante un valor de retorno
//    	    return;
//    	  }
//    	}
//har);
//
//    printf("cantidadBasura = %ld\n", cant_bas_disponible);
//
//    if(cant_borrar > cant_bas_disponible){
//      fseek(archivo, 0, SEEK_SET);
//      //hay que avisar que intento borrar mas de los disponible
//    }
//    else{
//      fseek(archivo, -cant_borrar * sizeof(char), SEEK_END);
//    }void rutina(){}
//    pos_actual = ftell(archivo);
//    ftruncate(fileno(archivo), pos_actual);
//    fclose(archivo);
//    return;
//  }
//
//  else{
//    printf("SACAR-BASURA: no existe archivo!\n");
//    //no existe el archivo
//    //hay que avisar
//    //se podria avisar mediante un valor de retorno
//    return;
//  }
//}

