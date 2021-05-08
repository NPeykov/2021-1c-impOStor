#include "mongo-store.h"

int main() {

	int socket_mongo_store, socket_cliente;
	t_config *config;
	char* puerto;

	socket_mongo_store = levantar_servidor(I_MONGO_STORE);

	//-------------------------------------------------------//

	config = config_create(PATH_MONGO_STORE_CONFIG); //aca estarian todas las configs de este server

	puerto = config_get_string_value(config, "PUERTO");

	printf("MONGO_STORE escuchando en PUERTO:%s \n", puerto);

	socket_cliente = esperar_cliente(socket_mongo_store);

	return EXIT_SUCCESS;
}
