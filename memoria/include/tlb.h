#ifndef TLB_H
#define TLB_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

int buscarEnTLB(int numero_pagina, int id);

//Reemplazo
void reordenarLRU(int numero_pagina_buscada, int id); 

void agregarTLB(int pagina, int marco, int id);

#endif