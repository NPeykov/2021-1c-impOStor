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

/*
 Debera recibir conexion de i Mongo Store para administrar los sabotajes.
 Debera conectarse a Mi Ram HQ

 iniciar_patota(int cantTripulantes, string path_to_file_tasks, Posicion posiciones[])

  si no se especifica la posicion del tripulante se lo inicializa en (0,0)


 listar_tripulantes()


 expulsar_tripulante(int idTripulante)

  Conecta con Mi Ram HQ y le manda el tripulante para eyectarlo (eliminar segmento de tareas)


 iniciar_planificacion()

 pausar_planificacion()

 obtener_bitacora(int idTripulante)

 Conecta con i Mongo Store y le pide la bitacora del tripulante.


 */


//DECLARACION DE ESTRUCTURAS
/*
typedef struct Posicion{ //cuesta un poco mas manejarlo asi
   int x;
   int y;
} Posicion;
*/

typedef enum Estado{
    LLEGADA, LISTO, TRABAJANDO, BLOQUEADO, FINALIZADO
}Estado;

t_queue *lista_llegada;
t_queue *lista_listo;
t_queue *lista_trabajando;
t_queue *lista_bloqueado;
t_queue *lista_finalizado;


typedef struct Tripulante{
	int id;
	int patota;
	int posicionX;
	int posicionY;
	Estado estado;
//    char* nombre;
//    t_list* tareasPendientes;
//    int cantCiclosCPUTotales;
} Tripulante;


typedef struct Patota{
    t_list* tripulantes;
    int idPatota;
} Patota;

typedef struct Discordiador{
    t_list* tripulantes;
    t_list* procesos;
    t_list* procesosDeIntercambio;
} Discordiador;

typedef struct Proceso{
	Tripulante* tripulante;
//	int rafagaAnterior;
}Proceso;

typedef struct ProcesoIntercambio{
	Tripulante* tripulante1;
	Tripulante* tripulante2;
//	int rafagaAnterior;
//	float estimadoAnterior;
//	float estimadoActual;
//	bool favorableParaUnLado;
}ProcesoIntercambio;

//DECLARACION DE ATRIBUTOS
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


Discordiador* discordiador; //por ahora no lo uso
t_list* tripulantesBloqueados; //por ahora no lo uso


//PROTOTIPO DE FUNCIONES
void  inicializarTripulantes();
void crear_tripulante(void*);
void atender_comandos_consola(void);
void inicializar_recursos_necesarios(void);

//DATOS PARA MANEJO DE CONSOLA
#define CANT_COMANDOS 7

int g_numero_patota = 1;

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
}Tripulante_Planificando;



typedef struct tripulantes_iniciados tripulantes_iniciados;

tripulantes_iniciados *crear_lista_tripulantes(char **);
void iniciar_patota(char**);
void tripulante(void*);
void liberar_memoria_discordiador(void);
void listar_cola_planificacion(Estado);

pthread_mutex_t lockear_creacion_tripulante;
pthread_mutex_t lockear_cambio_new_rdy;
pthread_mutex_t lockear_cambio_rdy_exec;
pthread_mutex_t lockear_cambio_exec_bloq;
pthread_mutex_t lockear_exit;

sem_t cambio_new_rdy;
sem_t proceso_nuevo;
sem_t quiero_rdy;
sem_t quiero_exec;
sem_t quiero_bloq;
sem_t anda_rdy;
sem_t anda_exec;
sem_t anda_bloq;
sem_t bloq_disponible;
sem_t libere_bloq;


bool g_pausa = false; //posible
bool programa_activo = true;   //posible

#endif

