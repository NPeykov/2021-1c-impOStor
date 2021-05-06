#include "mi-ram.h"

int main(){
  int socket_mi_ram;
  t_config *config;
  char* puerto;
  int socket_cliente;

  socket_mi_ram = levantar_servidor(MI_RAM_HQ);

  //-------------------------------------------------------//

  config = config_create(PATH_MI_RAM_CONFIG); //aca estarian todas las configs de este server

  puerto = config_get_string_value(config, "PUERTO");

  printf("MI_RAM escuchando en PUERTO:%s \n", puerto);

  socket_cliente = esperar_cliente(socket_mi_ram);









  //RECIBO MENSAJE DE CLIENTE
  char buffer[50]; //<-- guardo el msj que me manda aca

  int bytes = recv(socket_cliente, buffer, 50, 0);

  buffer[bytes] = '\0';

  printf("El tipo me mando %d bytes con el msj: %s\n", bytes, buffer);

  return EXIT_SUCCESS;
}
