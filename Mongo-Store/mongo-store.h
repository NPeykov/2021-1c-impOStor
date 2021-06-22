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

typedef enum {
//	INICIAR_PATOTA,
//	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANT,
////	INICIAR_PLANIFICACION,
////	PAUSAR_PLANIFICACION,
	OBTENER_BITACOR,
	ACTUALIZAR_TRIPULANT,
//	EXIT
} op;
#endif
