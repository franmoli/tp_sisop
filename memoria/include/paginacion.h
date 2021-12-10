#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "tlb.h"
#include "memoria-virtual.h"

int getPaginaByDireccionLogica(uint32_t direccion);
int getPaginaByDireccionFisica(uint32_t direccion);
int getPrimeraPaginaDisponible(int size, t_tabla_paginas *tabla_paginas);
t_contenidos_pagina *getLastContenidoByPagina(t_pagina* pagina);
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina* pagina);
t_tabla_paginas* buscarTablaPorPID(int id);
t_pagina *getPaginaByNumero(int nro_pagina, int carpincho_id);

int getIndexByPid(int pid);

//MARCOS
int getMarco(t_tabla_paginas* tabla_paginas);
int buscarMarcoEnMemoria(int numero_pagina_buscada, int id);

int getMarcoParaPagina(t_tabla_paginas* tabla_paginas);

void mostrarPaginas(t_tabla_paginas* tabla_paginas);
int solicitarPaginaNueva(uint32_t carpincho_id);

t_contenidos_pagina* getContenidoPaginaByTipo(t_contenidos_pagina* contenidos, t_contenido tipo);
t_contenidos_pagina *getContenidoPaginaByTipoAndSize(t_contenidos_pagina *contenidos, t_contenido tipo, uint32_t nextAlloc);

void liberarPagina(t_pagina* pagina, uint32_t carpincho_id);

//MEMREAD
void *memRead(t_paquete *paquete);
char* traerDeMemoria(int marco, int desplazamiento, int size);
//MEMWRITE
void memWrite(t_paquete *paquete);
void escribirEnMemoria(int marco, int desplazamiento, int size, void* contenido);

void* traerMarcoDeMemoria(t_pagina* pagina);
t_heap_metadata* traerAllocIncompleto(int marco,uint32_t dir_comienzo, uint32_t dir_final);
void escribirMarcoEnMemoria(t_pagina* pagina, void* stream);
void escribirAllocIncompleto(int marco,uint32_t dir_comienzo,uint32_t dir_fin,void *stream);

#endif