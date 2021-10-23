#ifndef MEMORIA_GLOBAL_H
#define MEMORIA_GLOBAL_H

#include "config_utils.h"
#include "server.h"

typedef struct {
    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;
}t_heap_metadata;

typedef struct{
    uint32_t numero_marco;
    uint32_t numero_pagina;
}t_tlb;

typedef struct{
    t_list *paginas;
    int paginas_totales_maximas;
}t_tabla_paginas;

typedef struct{
    uint32_t numero_marco;
}t_marco;

typedef struct{
    uint32_t numero_pagina;
    t_marco* marco;
    t_heap_metadata metadata;
}t_pagina;
typedef struct{
    t_list *tlb;
}t_tabla_tlb;

t_config_memoria *config_memoria;
t_log *logger_memoria;
t_tabla_tlb* tabla_tlb;
t_tabla_paginas* tabla_paginas;

void* tamanio_memoria;
int socket_server, socket_cliente_swap, socket_client;
#endif