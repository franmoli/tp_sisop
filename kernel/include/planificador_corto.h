#ifndef P_CORTO
#define P_CORTO

#include "kernel-global.h"

void iniciar_planificador_corto();
void *planificador_corto_plazo_sjf(void *_);
void *planificador_corto_plazo_hrrn(void *_);

#endif