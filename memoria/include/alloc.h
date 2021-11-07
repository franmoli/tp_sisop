#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>

t_heap_metadata *generarHeapVacio();
t_heap_metadata *getLastHeapByPagina(t_pagina *pagina, int numeroPagina);
t_heap_metadata *generarHeaderMetadataAlFinal(t_pagina *pagina,int numeroPagina);
t_heap_metadata *generarFooter(t_pagina *heapHeader, int numeropaginaHeapHeader);
int getPosicionEnLaPagina(int pagina);
t_heap_metadata* getFromMemoriaHeap(t_heap_metadata *heap, int numeroPagina);

t_heap_metadata* traerAllocDeMemoria(uint32_t);
void crearPrimerAlloc(int size);
void guardarAlloc(t_heap_metadata* data, uint32_t direccion);
void memAlloc(int size);


#endif