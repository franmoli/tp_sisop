#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>

t_heap_metadata* traerAllocDeMemoria(uint32_t);
void crearPrimerAlloc(int size);
void guardarAlloc(t_heap_metadata* data, uint32_t direccion);
void memAlloc(int size);
void freeAlloc(uint32_t direccion);


#endif