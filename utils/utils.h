#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>
#include<errno.h>

#define PATH_MI_RAM_CONFIG "../config_files/mi_ram.config"
#define PATH_MONGO_STORE_CONFIG "../config_files/mongo_store.config"
#define PATH_DISCORDIADOR_CONFIG "../config_files/discordiador.config"
#define PATH_MONGO_STORE_LOG "../logs_files/mongo.log"

typedef enum{
	MENSAJE,
	PAQUETE,
	INICIO_PATOTA,
	NUEVO_TRIPULANTE,
	ACTUALIZAR_POSICION,
	ACTUALIZAR_ESTADO,
	ELIMINAR_TRIPULANTE, //id, patota
	OBTENGO_BITACORA,
	PEDIDO_TAREA, //para ram
	INICIO_TAREA, //para store
	FIN_TAREA, //para store
	INICIO_SABOTAJE
}op_code;

typedef enum{
	GENERAR_OXIGENO,
	CONSUMIR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA
}tarea_code;

typedef enum{
	I_MONGO_STORE, MI_RAM_HQ
}cod_servidor;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct{
	uint32_t numPatota;
	uint32_t tid;
	uint32_t size_status;
	char status;
	uint32_t posX;
	uint32_t posY;
} t_tripulante_iniciado;

typedef enum{
	TAREA_COMUN,
	TAREA_IO
} Tipo_Tarea;

typedef struct{
	int idTripulante;
	int numPatota;
	char *nombreTarea;
	int duracionTarea;
	int tipo_tarea;
	int parametro;
	tarea_code codigo_tarea;
} m_estado_tarea_tripulante;

typedef struct {
	int origenX;
	int origenY;
	int destinoX;
	int destinoY;
	int idTripulante;
	int idPatota;
}m_movimiento_tripulante;

int errno;

int crear_servidor(char*, char*);
int levantar_servidor(int);
int esperar_cliente(int);
void liberar_cliente(int socket_cliente);
int crear_conexion(char* ip, char* puerto);
int iniciar_conexion(int, t_config *); //es para uno de los svs en especifico

t_paquete* crear_paquete(op_code operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void enviar_mensaje(op_code, char*, int);
char *recibir_mensaje(int);
void enviar_mensaje_simple(char *, int);

t_list* recibir_paquete(int);
int recibir_operacion(int);
void* recibir_buffer(int*, int);
t_tripulante_iniciado *recibir_tripulante_iniciado(int);
m_estado_tarea_tripulante *recibirNuevoEstadoTareaTripulante(int);
m_movimiento_tripulante *recibirMovimientoTripulante(int);


//Sockets de todos los servidores
int mongo_socket;
int mi_ram_hq_socket;


#endif
