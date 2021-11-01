#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "tlb.h"


void guardarMemoria(t_paquete* paquete);
int getPaginaAGuardar(int size);
int getPosicionPagina(t_pagina *pagina);
void findAndSaveEnPagina(int pagina);
t_heap_metadata* guardarHeader(t_pagina *paginaHeader, int paginaHeapHeader);

void memRead(uint32_t direccion);
void leerContenidoEnMemoria(uint32_t direccion, t_contenido tipoContenido);
#endif