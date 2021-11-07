#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "tlb.h"

void guardarMemoria(t_paquete *paquete);
int getPrimeraPaginaDisponible(int size);
int getPosicionPagina(t_pagina *pagina);
void findAndSaveEnPagina(int pagina);
t_heap_metadata *guardarHeader(t_pagina *paginaHeader, int paginaHeapHeader);

t_heap_metadata *memRead(uint32_t direccion);
t_heap_metadata* leerContenidoEnMemoria(uint32_t direccion, t_contenido tipoContenido);
t_pagina* buscarPagina(uint32_t direccion); 

#endif