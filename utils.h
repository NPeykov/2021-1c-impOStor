#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/config.h>

#define PATH_CONFIG "config_files/cliente.config"

typedef enum
{
	SERVER_I_MONGO_STORE, SERVER_MI_RAM_HQ,
	ELIMINAR_TRIPULANTE,
}op_code;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


int crear_conexion(char* ip, char* puerto);
int iniciar_conexion(int, t_config *); //es para uno de los svs en especifico
//
t_paquete* crear_paquete(op_code operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);


#endif
