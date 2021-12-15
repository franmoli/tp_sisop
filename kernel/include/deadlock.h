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
t_list* quien_retiene_recurso(char *recurso, int id_solicitante);
bool proceso_en_deadlock(int solicitante, t_list *lista_recursos_en_deadlock, char *nombre_semaforo, t_list *lista_de_procesos_en_deadlock);
#endif