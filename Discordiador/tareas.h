#ifndef TAREAS_H_
#define TAREAS_H_

#include "utils_disc.h"
#include "comunicacion.h"

//prototipos
char *dar_proxima_tarea(Tripulante *);
Tarea *proxima_tarea(Tripulante *);
bool estoy_en_mismo_punto(int, int, int, int);
bool completo_tarea(Tripulante_Planificando *);
void moverse_una_unidad(Tripulante_Planificando *);
void realizar_tarea_IO(Tripulante_Planificando *);
void realizar_tarea_comun(Tripulante_Planificando *);
void hacer_una_unidad_de_tarea(Tripulante_Planificando *);

#endif
