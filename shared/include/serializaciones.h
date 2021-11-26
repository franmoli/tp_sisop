#ifndef SERIALIZACIONES__H
#define SERIALIZACIONES__H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include "../../memoria/include/memoria-global.h"
#include "../../kernel/include/kernel-global.h"
#include "server.h"

typedef struct
{
	uint32_t size_reservar;
	uint32_t carpincho_id;
} t_malloc_serializado;

typedef struct
{
	uint32_t carpincho_id;
} t_mateinit_serializado;

typedef struct
{
	uint32_t carpincho_id;
	bool swap_free;
} t_swap_serializado;

int bytes_marco(t_marco marco);
void *serializar_marco(t_marco marco, void **stream_inicial, int offset);

int bytes_pagina(t_pagina pagina);
void *serializar_pagina(t_pagina pagina, void **stream_inicial, int offset);
t_pagina deserializar_pagina(void *stream, int offset);

t_paquete *serializar_mate_init(uint32_t carpincho_id);
t_mateinit_serializado* deserializar_mate_init(t_paquete *paquete);

void *serializar_mate_sem_init(void **stream,uint32_t valor, char *nombre_sem);
t_mate_sem *deserializar_mate_sem_init(t_paquete *paquete);
t_paquete *serializar_mate_sem_resto(char *nombre_sem,op_code cod_op);
t_mate_sem *deserializar_mate_sem_resto(t_paquete *paquete);

t_paquete *serializar_alloc(uint32_t size, uint32_t carpincho_id);
t_malloc_serializado* deserializar_alloc(t_paquete *paquete);

t_paquete *serializar_consulta_swap(uint32_t carpincho_id);
t_swap_serializado* deserializar_swap(t_paquete *paquete);

#endif