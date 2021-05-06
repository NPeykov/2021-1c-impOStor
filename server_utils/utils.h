#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>
#include <errno.h>

#define PATH_MI_RAM_CONFIG "../config_files/mi_ram.config"
#define PATH_MONGO_STORE_CONFIG "../config_files/mongo_store.config"
#define PATH_CLIENTE_CONFIG "../config_files/cliente.config"

enum{MI_RAM_HQ, I_MONGO_STORE};

int errno;

int crear_servidor(char*, char*);
int levantar_servidor(int);
int esperar_cliente(int);

#endif
