#ifndef DEADLOCK_H
#define DEADLOCK_H

#include "kernel-global.h"
#include "planificador_corto.h"
#include "planificador_mediano.h"
#include "planificador_largo.h"

void iniciar_deadlock();
void* algoritmo_deteccion(void *_);
bool recurso_retenido(int solicitante, char *semaforo_solicitado, t_list *lista_procesos_en_deadlock);
bool proceso_bloqueado(int id);
char *proceso_bloqueado_por_sem(int id);

#endif