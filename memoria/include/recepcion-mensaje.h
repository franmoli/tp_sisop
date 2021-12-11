#ifndef RECEPCION_MENSAJE_H
#define RECEPCION_MENSAJE_H

#include "memoria-global.h"
#include "serializaciones.h"
#include <semaphore.h>

void receptor();
void analizar_paquete(t_paquete *paquete);
#endif