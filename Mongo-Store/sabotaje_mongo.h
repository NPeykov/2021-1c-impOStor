#ifndef SABOTAJE_MONGO_H
#define SABOTAJE_MONGO_H

#include "../utils/utils.h"
#include "mongo-store.h"

int socket_sabotaje_cliente;

typedef enum{
	SB_BLOCKS,
	SB_BITMAP,
	FILES_SIZE,
	FILES_BLOCK_COUNT,
	FILES_MD5,
	NO_HAY_SABOTAJE
}sabotaje_code;

void rutina(int);
sabotaje_code obtener_tipo_sabotaje();
void gestionarSabotaje(void);
char* siguiente_posicion_sabotaje(void);
void enviar_aviso_sabotaje_a_discordiador(void);


#endif
