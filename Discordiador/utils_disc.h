#ifndef UTILS_DISC_H_
#define UTILS_DISC_H_

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
int g_numero_patota;
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

//sockets
//int socket_ram;
//int socket_store;

bool g_hay_pausa;
bool g_hay_sabotaje;
int lugares_en_exec;

//DATOS PARA MANEJO DE CONSOLA

char* comandos_validos[CANT_COMANDOS];/* = {
  "INICIAR_PATOTA",
  "LISTAR_TRIPULANTES",
  "EXPULSAR_TRIPULANTE",
  "INICIAR_PLANIFICACION",
  "PAUSAR_PLANIFICACION",
  "OBTENER_BITACORA",
  "EXIT"
};*/

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
	int cantidad_tripulantes;
	t_list *semaforos;
}argumentos_creacion_tripulantes;

struct tripulantes_iniciados{
	Tripulante *tripulante;
	struct tripulantes_iniciados *proximo_tripulante;
};


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
	/*tarea_code tarea_code;*/
}Tarea;

typedef struct Tripulante_Planificando{
	Tripulante *tripulante;
	int quantum_disponible;
	Tarea *tarea;
	sem_t ir_exec;
	sem_t salir_pausa;
	sem_t termino_sabotaje;
	bool sigo_planificando;
	bool fui_expulsado;
	t_list *semaforos;
	int cant_trip;
	bool fue_sacado_de_fin;
}Tripulante_Planificando;


typedef struct tripulantes_iniciados tripulantes_iniciados;




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
pthread_mutex_t mutex_lista_semaforos;
pthread_mutex_t mutex_log; //sino me imprime mal

pthread_cond_t sabotaje_resuelto;
pthread_mutex_t sabotaje_lock;
pthread_mutex_t pausa_lock;
pthread_mutex_t mutex;

//semaforos
sem_t bloq_disponible; //iniciar en 1
sem_t moverse_a_em;
sem_t se_movio_a_em;
sem_t primer_inicio;
sem_t otros_inicios;
sem_t termino_sabotaje_planificador;
sem_t resolvi_sabotaje; //capaz no sea necesario
sem_t ya_sali_de_exec; //por si fue expulsado
sem_t voy_a_ready;

sem_t semaforo_tarea;

void imprimir_respuesta_log(t_list*);
void reanudar_hilos_lista(Estado);
void listar_discordiador(void);
void listar_cola_planificacion(Estado);
void fijarse_si_hay_pausa_hilo(Tripulante_Planificando *);
void fijarse_si_hay_pausa_planificador(void);

#endif
