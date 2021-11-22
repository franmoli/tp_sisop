#ifndef SERIALIZACIONES__H
#define SERIALIZACIONES__H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include "server.h"
/*int bytes_pagina(t_pagina pagina);
void *serializar_pagina(t_pagina pagina, void **stream_inicial, int offset);
t_pagina deserializar_pagina(void *stream, int offset);
*/

typedef struct
{
	uint32_t size_reservar;
	uint32_t carpincho_id;
} t_malloc_serializado;

typedef struct
{
	uint32_t carpincho_id;
} t_mateinit_serializado;

t_paquete *serializar_alloc(uint32_t size, uint32_t carpincho_id);
t_malloc_serializado* deserializar_alloc(t_paquete *paquete);

t_paquete *serializar_mate_init(uint32_t carpincho_id);
t_mateinit_serializado* deserializar_mate_init(t_paquete *paquete);
#endif