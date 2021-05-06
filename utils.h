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

enum{SERVER_I_MONGO_STORE, SERVER_MI_RAM_HQ};

int crear_conexion(char* ip, char* puerto);
int iniciar_conexion(int, t_config *); //es para uno de los svs en especifico

#endif
