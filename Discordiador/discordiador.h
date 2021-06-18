#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include "../utils/utils.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/collections/node.h>
#include <semaphore.h>

#define CANT_COMANDOS 7

//DECLARACION DE ESTRUCTURAS

typedef enum Estado{
    LLEGADA, LISTO, TRABAJANDO, BLOQUEADO_IO, BLOQUEADO_EMERGENCIA, FINALIZADO
}Estado;

t_queue *lista_llegada;
t_queue *lista_listo;
t_list *lista_trabajando;
t_list *lista_bloqueado_IO;
t_list *lista_finalizado;
t_list *lista_bloqueado_EM;


typedef struct Tripulante{
	int id;
	int patota;
	int posicionX;
	int posicionY;
	Estado estado;
} Tripulante;


//DECLARACION DE ATRIBUTOS
int g_numero_patota = 1;
t_log *logs_discordiador;
char *ip_mi_ram;
char *ip_mongo_store;
t_config* config;
char *algoritmo_planificacion;
char *puerto_mi_ram;
char *puerto_mongo_store;
int grado_multitarea;
int quantum;
int duracion_sabotaje;
int retardo_ciclo_cpu;

bool g_hay_pausa    = true;
bool g_hay_sabotaje = false;
int lugares_en_exec;

//DATOS PARA MANEJO DE CONSOLA

const char* comandos_validos[CANT_COMANDOS] = {
  "INICIAR_PATOTA",
  "LISTAR_TRIPULANTES",
  "EXPULSAR_TRIPULANTE",
  "INICIAR_PLANIFICACION",
  "PAUSAR_PLANIFICACION",
  "OBTENER_BITACORA",
  "EXIT"
};

typedef enum {
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXIT
} tipo_comando;

typedef struct argumentos_creacion_tripulantes {
	int numero_tripulante;
	int posicionX;
	int posicionY;
	int patota_actual;
}argumentos_creacion_tripulantes;

struct tripulantes_iniciados{
	Tripulante *tripulante;
	struct tripulantes_iniciados *proximo_tripulante;
};

typedef enum{
	TAREA_COMUN,
	TAREA_IO
} Tipo_Tarea;

typedef enum{
	RR,
	FIFO
} Algoritmo;

typedef struct Tarea{
	char* nombre;
	int parametro;
	int posX;
	int posY;
	int duracion;
	Tipo_Tarea tipo;
}Tarea;

typedef struct Tripulante_Planificando{
	Tripulante *tripulante;
	int quantum_disponible;
	Tarea *tarea;
	sem_t ir_exec;
}Tripulante_Planificando;


typedef struct tripulantes_iniciados tripulantes_iniciados;


//PROTOTIPO DE FUNCIONES
void  inicializarTripulantes();
void crear_tripulante(void*);
void atender_comandos_consola(void);
void inicializar_recursos_necesarios(void);
tripulantes_iniciados *crear_lista_tripulantes(char **);
void iniciar_patota(char**);
void tripulante(void*);
void liberar_memoria_discordiador(void);
void listar_cola_planificacion(Estado);

//mutexs
pthread_mutex_t lockear_creacion_tripulante; //me creaban tripulantes con mismo id
pthread_mutex_t lock_lista_listo;
pthread_mutex_t lock_lista_llegada;
pthread_mutex_t lock_lista_exec;
pthread_mutex_t lock_lista_bloq_io;
pthread_mutex_t lock_lista_bloq_em;
pthread_mutex_t lock_lista_exit;
pthread_mutex_t lock_grado_multitarea;
pthread_mutex_t mutex_tarea;

pthread_cond_t sabotaje_resuelto;
pthread_mutex_t sabotaje_lock; //por ahora no lo uso
pthread_mutex_t pausa_lock;

//semaforos
sem_t bloq_disponible; //iniciar en 1
sem_t tripulantes_hermanos; //todavia no implementado, lo uso para q un proceso se quede esperando en exit




#endif

