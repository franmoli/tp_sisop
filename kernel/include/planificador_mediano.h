#ifndef P_MEDIANO
#define P_MEDIANO

#include "kernel-global.h"

void iniciar_planificador_mediano();
void *planificador_mediano_plazo(void *_);
void *salida_de_block(void *_);

#endif