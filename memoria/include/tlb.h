#ifndef TLB_H
#define TLB_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

int getTLBPaginaByMarco(int marco);
int getFromTLB(int numero_pagina_buscado);

//Reemplazo
void actualizarTLBFIFO(t_tlb *tlb);

#endif