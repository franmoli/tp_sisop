#ifndef MEMORIA_GLOBAL_H
#define MEMORIA_GLOBAL_H

#include "config_utils.h"
#include "server.h"

typedef enum
{
	PATOTA = 0,
    ALLOC = 1,
} t_contenido;

typedef struct {
    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;
} __attribute__((packed))
t_heap_metadata;

typedef struct{
    uint32_t numero_marco;
    uint32_t numero_pagina;
}t_tlb;

typedef struct{
    t_list *paginas;
    int paginas_totales_maximas;
}t_tabla_paginas;

typedef struct{
    uint32_t numero_pagina;
    uint32_t tamanio_ocupado;
    uint32_t cantidad_contenidos;
    t_list* contenidos_pagina;
}t_pagina;

typedef struct{
		uint32_t dir_comienzo; // NO DEBE SER UN PUNTERO
		uint32_t tamanio; 
		t_contenido contenido_pagina; //antes era un puntero
	} t_contenidos_pagina;

typedef struct{
    t_list *tlb;
}t_tabla_tlb;

t_config_memoria *config_memoria;
t_log *logger_memoria;
t_tabla_tlb* tabla_tlb;
t_tabla_paginas* tabla_paginas;

t_list *allocs;

void* tamanio_memoria;
int socket_server, socket_cliente_swap, socket_client;
#endif