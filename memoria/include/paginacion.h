#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "tlb.h"

void guardarMemoria(t_paquete *paquete);
int getPaginaByDireccion(uint32_t direccion);

t_heap_metadata *memRead(uint32_t direccion);


#endif