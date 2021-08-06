#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils_disc.h"
#include "tareas.h"
#include "sabotaje.h"
#include "comunicacion.h"

//movimientos
void moverse_a_ready(Tripulante_Planificando *);
void moverse_a_bloq(Tripulante_Planificando *);
void mover_tripulante_a_exit(Tripulante_Planificando *);

//
void realizar_trabajo(Tripulante_Planificando *);
void planificar(void);
void atender_comandos_consola(void);
void iniciar_patota(char**);
void tripulante(void*);
void expulsar_tripulante(int, int);
void inicializar_recursos_necesarios(void);
void liberar_memoria_discordiador(void);
void esperar_tripulantes_hermanos(Tripulante_Planificando*);
void avisar_a_tripulantes_hermanos(Tripulante_Planificando*);
void sacarlo_de_finalizado(Tripulante_Planificando*);



#endif

