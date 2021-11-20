#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "tlb.h"
#include "memoria-virtual.h"

int getPaginaByDireccion(uint32_t direccion);
int getPrimeraPaginaDisponible(int size);
t_heap_metadata *memRead(t_paquete* paquete);
t_contenidos_pagina *getLastContenidoByPagina(t_pagina* pagina);
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina* pagina);

//MARCOS
int getMarco();

int generarPaginaConMarco();

void mostrarPaginas();
#endif