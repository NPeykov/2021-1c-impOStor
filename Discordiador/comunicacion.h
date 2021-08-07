#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include "utils_disc.h"

void crear_y_enviar_inicio_patota(char*, char*, int);
void serializar_y_enviar_tripulante(Tripulante*, op_code, int);
void avisar_a_mongo_estado_tarea(Tarea *, Tripulante*, op_code);
void avisar_movimiento_a_mongo(int, int, Tripulante*);
void enviar_mensaje_mongo(op_code);

/*id, patota*/
void avisar_a_ram_expulsion_tripulante(int, int);


#endif
