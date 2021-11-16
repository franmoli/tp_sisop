#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "tlb.h"

int getPaginaByDireccion(uint32_t direccion);
int getPrimeraPaginaDisponible(int size);
t_heap_metadata *memRead(uint32_t direccion);
t_contenidos_pagina *getLastContenidoByPagina(t_pagina* pagina);
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina* pagina);

//MARCOS
int getMarco();
int generarPaginaConMarco();
#endif