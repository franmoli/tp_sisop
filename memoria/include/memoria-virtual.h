#ifndef MEMORIA_VIRTUAL_H
#define MEMORIA_VIRTUAL_H

#include "memoria-global.h"


//t_pagina* reemplazarPagina(t_pagina* paginaAgregar, int carpincho_id);
int reemplazarPagina(int nro_pagina, int carpincho_id);
int eliminarPrimerElementoLista(int carpincho_id);
void consultaSwap(int carpincho_id);

//Reemplazo
int reemplazarLRU(int nro_pagina, int carpincho_id);
int reemplazarClockM(int nro_pagina, int carpincho_id, int referido, int modificado);
void actualizarModificado(uint32_t nro_pagina, uint32_t carpincho_id);
void actualizarReferido(uint32_t nro_pagina, uint32_t carpincho_id);
#endif