#ifndef ALLOC_H
#define ALLOC_H

#include "memoria-global.h"
#include <commons/collections/list.h>

void allocMem(int size);
bool memoriaDisponible(int size);
t_heap_metadata *nuevoAlloc(int size);
bool memoriaDisponible(int size);
int memoriaReservada();
t_heap_metadata *buscarAllocLibre(int size);

t_heap_metadata *generarHeapVacio();
t_heap_metadata *getLastHeapByPagina(t_pagina *pagina, int numeroPagina);
t_heap_metadata *generarHeaderMetadataAlFinal(t_pagina *pagina,int numeroPagina);
t_heap_metadata *generarFooter(t_pagina *heapHeader, int numeropaginaHeapHeader);
int getPosicionEnLaPagina(int pagina);
t_heap_metadata* getFromMemoriaHeap(t_heap_metadata *heap, int numeroPagina);
#endif