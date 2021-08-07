#ifndef SABOTAJE_H_
#define SABOTAJE_H_

#include "utils_disc.h"
#include "tareas.h"



//prototipos
void esperar_sabotaje(void);
void atender_sabotaje(int, int);
void *mas_cercano_al_sabotaje(int, int);
bool llegue_al_sabotaje(Tripulante_Planificando*, int, int);
void moverse_al_sabotaje(Tripulante_Planificando*, int, int);
void resolver_sabotaje(Tripulante_Planificando *, int, int);
void avisar_estado_sabotaje_a_mongo(int , int , Tripulante*, op_code );

#endif
