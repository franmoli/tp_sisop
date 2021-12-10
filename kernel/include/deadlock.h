#ifndef DEADLOCK_H
#define DEADLOCK_H

#include "kernel-global.h"
#include "planificador_corto.h"
#include "planificador_mediano.h"
#include "planificador_largo.h"

void iniciar_deadlock();
void* algoritmo_deteccion(void *_);

#endif