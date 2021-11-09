#ifndef P_LARGO
#define P_LARGO

#include "kernel-global.h"
#include "server.h"
#include "proceso.h"

void iniciar_planificador_largo();
void atender_proceso (void* parametro);
void *iniciar_servidor_kernel(void *_);
t_proceso *nuevo_carpincho(int socket_cliente);
void *planificador_largo_plazo(void *_);
void *hilo_salida_a_exit(void *multiprogramacion_disponible_p);

#endif