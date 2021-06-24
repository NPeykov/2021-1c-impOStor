#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"

char *puntoMontaje;
char *dirMetadata;
char *dirFiles;
char *dirBitacora;
char *dirBlocks;
t_config* mongoConfig;
t_log* mongoLogger;
int socket_cliente;
void crearEstructuraFileSystem();
void *gestionarCliente(int cliente);


#endif
