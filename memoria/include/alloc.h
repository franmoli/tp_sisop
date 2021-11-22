#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "paginacion.h"
#include <math.h>

t_heap_metadata* traerAllocDeMemoria(uint32_t);
void guardarAlloc(t_heap_metadata* data, uint32_t direccion);

void memAlloc(t_paquete *paquete);
void freeAlloc(t_paquete *paquete);

bool direccionValida(uint32_t direccion, uint32_t carpincho_id);
t_heap_metadata* getLastHeapFromPagina(int pagina, int carpincho_id);
void mostrarAllocs(int carpincho_id);

void agregarPagina(t_pagina* pagina, t_heap_metadata* data, uint32_t nextAnterior, uint32_t size,bool ultimo);
void crearPrimerHeader(t_pagina* pagina);
#endif