#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "tlb.h"
#include "memoria-virtual.h"

int getPaginaByDireccion(uint32_t direccion);
int getPrimeraPaginaDisponible(int size, t_tabla_paginas *tabla_paginas);
t_heap_metadata *memRead(t_paquete* paquete);
t_contenidos_pagina *getLastContenidoByPagina(t_pagina* pagina);
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina* pagina);
t_tabla_paginas* buscarTablaPorPID(int id);


//MARCOS
int getMarco();
int buscarMarcoEnMemoria(int numero_pagina_buscada, int id);

int getMarcoParaPagina(t_tabla_paginas* tabla_paginas);

void mostrarPaginas(t_tabla_paginas* tabla_paginas);
int solicitarPaginaNueva(uint32_t carpincho_id);
#endif