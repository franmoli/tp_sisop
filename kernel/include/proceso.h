#ifndef PROCESO_H
#define PROCESO_H

#include <unistd.h>
#include <time.h>

#include "kernel-global.h"

void *proceso(void *_);
void new();
void ready();
void exec(t_proceso *self);
void blocked();
void bloquear(t_proceso *self);
void desbloquear(t_proceso *self);
void iniciar_semaforo(t_semaforo *semaforo);
void enviar_sem_disponible(int id);
t_semaforo *traer_semaforo(char *nombre_solicitado);
#endif