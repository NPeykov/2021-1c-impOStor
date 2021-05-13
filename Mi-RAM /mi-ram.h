#ifndef MiRAM_H_
#define MiRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"

int servidor;

void *gestionarCliente(int cliente);

int socket_mi_ram;
t_config *config;
char* puerto;
int socket_cliente;

#endif
