#ifndef PAGINACION_H
#define PAGINACION_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include "tlb.h"


void guardarMemoria(t_paquete* paquete);
int getPaginaAGuardar(int size);
int getPosicionPagina(int pagina);
void findAndSaveEnPagina(int pagina);
#endif