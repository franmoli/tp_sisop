#ifndef TLB_H
#define TLB_H

#include "memoria-global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

int getTLBPaginaByMarco(int marco);
int getFromTLB(int numero_pagina_buscado, t_tabla_paginas *tabla_paginas);

int buscarEnTLB(int numero_pagina, int id);

//Reemplazo
void reordenarLRU(int numero_pagina_buscada, int id); 

#endif