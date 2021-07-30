#ifndef SABOTAJE_MONGO_H
#define SABOTAJE_MONGO_H

#include "../utils/utils.h"
#include "mongo-store.h"

int socket_sabotaje_cliente;

char *s_superbloque;
char *s_blocks;
int s_tamanio_superbloque;
int s_tamanio_blocks;

bool fue_en_oxigeno;
bool fue_en_comida;
bool fue_en_basura;

int cantidad_bloques_file;

t_bitarray *bitarray_sb;

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
void reparar_MD5(char* , char);



//recuperacion
void iniciar_recuperacion(sabotaje_code);


#endif
