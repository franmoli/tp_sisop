#ifndef P_LARGO
#define P_LARGO

#include "kernel-global.h"

void iniciar_planificador_largo();
void atender_proceso (void* parametro);
void *iniciar_servidor_kernel(void *_);
void nuevo_carpincho(int socket_cliente);
void *planificador_largo_plazo(void *_);

#endif