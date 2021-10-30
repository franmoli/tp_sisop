#ifndef P_CORTO
#define P_CORTO

#include <unistd.h>
#include "kernel-global.h"

void iniciar_planificador_corto();
void *planificador_corto_plazo_sjf(void *_);
void *planificador_corto_plazo_hrrn(void *_);
void *esperar_salida_exec(void *_);
void *esperar_salida_block(void *multiprocesamiento_p);
void *hilo_liberar_multiprocesamiento(void *multiprocesamiento_p);

#endif