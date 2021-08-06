#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../utils/utils.h"
#include <signal.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "bitmap.h"
#include <math.h>
#include "sabotaje_mongo.h"
#include <commons/memory.h>



//Semaforos
sem_t contador_sabotaje; //iniciar en 1
sem_t dar_orden_sabotaje;
sem_t semaforo_generar_md5;
sem_t semaforo_bitacora;
sem_t semaforo_para_file_oxigeno;
sem_t semaforo_para_file_comida;
sem_t semaforo_para_file_basura;
sem_t inicio_fsck; //para fin de sabotaje
sem_t semaforo_modificacion_de_datos;
//MUTEXS
pthread_mutex_t mutex_disco_logico;
pthread_mutex_t mutex_bitmap;

typedef struct {
	void *tripulante;
	op_code accion;
} tripulante_con_su_accion;

typedef struct {
	t_list *bloques;
} t_disco_logico;

typedef struct {
	int inicio;
	int fin;
	int id_bloque;
	int espacio;
	int posicion_para_escribir;
} t_bloque;

typedef enum{
	OXIGENO,
	COMIDA,
	BASURA
}files;

typedef enum{
	OXIGENOO,
	COMIDAA,
	BASURAA,
	BITACORA,
	SUPERBLOQUE,
	BLOCKS,
	FILES
}rutas;

//booleanos para saber si los archivos fueron creados
bool g_existe_file_oxigeno;
bool g_existe_file_comida;
bool g_existe_file_basura;
bool g_modificado_file_oxigeno;
bool g_modificado_file_comida;
bool g_modificado_file_basura;
bool g_en_uso_file_oxigeno;
bool g_en_uso_file_comida;
bool g_en_uso_file_basura;
bool g_abierto_file_oxigeno;
bool g_abierto_file_comida;
bool g_abierto_file_basura;


char *puntoMontaje;

t_disco_logico *disco_logico;
t_config* mongoConfig;
t_log* mongoLogger;
//int socket_cliente;
int numero_sabotaje;
m_movimiento_tripulante * tripulanteEnMovimiento;
pthread_t hilo_sabotaje;
t_list* archAbiertos;
//int socket_mongo_store, socket_cliente;
char* puerto;
char *block_mmap;
char * archivoBasura;
char * archivoComida;
char * archivoOxigeno;
size_t block_mmap_size;


//relacionadas a escribir las acciones del tripulante
char *generarTextoAEscribir(tripulante_con_su_accion *);
void escribir_en_su_bitacora_la_accion(tripulante_con_su_accion*);
char *rutaBitacoraDelTripulante(tripulante_con_su_accion*);

void esperar_senial();

//bajadas a disco
void mostrar_estado_bitarray(void);
void gestionar_bajadas_a_disco(void);
void bajar_datos_superbloque(void);

void iniciar_recursos_mongo(void);
void crearEstructuraFileSystem();
void crearblocks(char*);
void crear_estructura_filesystem();
void crearEstructurasBloques();
void crearEstructuraDiscoLogico();
void crearBitMapLogico();
void escribir_en_block(char*,t_bloque*);
void borrar_en_block(int,t_bloque*);
void inicializar_archivo(char* , char);
t_bloque* buscar_ultimo_bloque_del_tripulante(char*);
int cantidad_bloques_a_ocupar(char* texto);
void copiar_datos_de_bloques(t_list*);
int ultima_posicion_escrita(int,int);
void main2();
//sobre files
void *gestionarCliente(int cliente);
void obtener_bitacora_tripulante(int);
char* contenido_de_bloques(char*);
void gestionarSabotaje();
void generar_oxigeno(int);
void consumir_oxigeno(int);
void generar_comida(int);
void consumir_comida(int);
void generar_basura(int);
void descartar_basura(int);
char *generarMD5(char *);
void (*signal(int sig, void (*func)(int)))(int) ;
char *size_de_archivo(char*);
char* bloques_de_archivo(char*);
char* contenido_de_bloques(char*);
char* cantidad_de_bloques_de_archivo(char*);
t_bloque* recuperar_ultimo_bloque(char*);
char* leo_el_bloque_incluyendo_espacios(t_bloque*);
char* leer_md5file(char*);
char* cantidad_de_bloques_de_archivo_fisico(char *);
char* bloques_de_archivo_fisico(char*);
char *size_de_archivo_fisico(char* );
char* contenido_de_bloques_fisico(char*);
char* leo_el_bloque_fisico(t_bloque* );
char* conseguir_ruta(rutas);







#endif
