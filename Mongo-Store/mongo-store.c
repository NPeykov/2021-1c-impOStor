#include "mongo-store.h"

int main() {

	int socket_mongo_store, socket_cliente;
	t_config *config;
	char* puerto;
	t_log* logger;

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);

	//-------------------------------------------------------//

	config = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	logger = log_create(PATH_MONGO_STORE_LOG, "Mongo", 1, LOG_LEVEL_DEBUG);

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_cliente = esperar_cliente(socket_mongo_store);
	crearEstructuraFileSystem();
	return EXIT_SUCCESS;
}

void crearEstructuraFileSystem()
{
	FILE *f;
//esto va comentado hasta que se configuren
//	puntoMontaje = config_get_string_value(sindicatoConfig,"PUNTO_MONTAJE");
//	dirMetadata = malloc(strlen(puntoMontaje) + strlen("/Metadata") + 1);
//	strcpy(dirMetadata, puntoMontaje);
//	strcat(dirMetadata, "/Metadata");
//
//	dirFiles = malloc(strlen(puntoMontaje) + strlen("/Files") + 1);
//	strcpy(dirFiles, puntoMontaje);
//	strcat(dirFiles, "/Files");
//
//	dirBlocks = malloc(strlen(puntoMontaje) + strlen("/Blocks") + 1);
//	strcpy(dirBlocks, puntoMontaje);
//	strcat(dirBlocks, "/Blocks");
//
//	char *metadataRuta = malloc(strlen(dirMetadata) + strlen("/Metadata.AFIP") + 1); // /Metadata.bin  /Bitmap.bin
//	strcpy(metadataRuta, dirMetadata);
//	strcat(metadataRuta, "/Metadata.AFIP"); ///Metadata.bin  /Bitmap.bin
//
//
//	if(mkdir(puntoMontaje, 0777) != 0)
//	{
//		printf("El directorio de montaje ya existe!! =( \n");
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
