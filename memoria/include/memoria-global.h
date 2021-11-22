#ifndef MEMORIA_GLOBAL_H
#define MEMORIA_GLOBAL_H

#include "config_utils.h"
#include "server.h"
#include "serializaciones.h"
#include <commons/collections/queue.h>
typedef enum
{
	CONTENIDO = 0,
    ALLOC = 1,
    HEADER = 2,
    FOOTER = 3,
} t_contenido;

typedef struct {
    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;
} __attribute__((packed))
t_heap_metadata;

typedef struct{
    uint32_t pid;
    uint32_t numero_marco;
    uint32_t numero_pagina;
}t_tlb;

typedef struct{
    uint32_t numero_marco;
    bool isFree;
}t_marco;

typedef struct{
    int pid;
    t_list *paginas;
    t_list *Lru;
    t_list *Clock;
    int paginas_totales_maximas;
    int paginas_en_memoria;
}t_tabla_paginas;

typedef struct{
    uint32_t numero_pagina;
    uint32_t tamanio_ocupado;
    uint32_t cantidad_contenidos;
    uint32_t marco_asignado;
    bool bit_presencia;
    bool bit_modificado;
    int  usado;
    t_list*  listado_de_contenido;
}t_pagina;

typedef struct{
		uint32_t dir_comienzo; // NO DEBE SER UN PUNTERO
		uint32_t dir_fin;
        uint32_t tamanio; 
        uint32_t carpincho_id;
        uint32_t recorrido;
		t_contenido contenido_pagina; //antes era un puntero
	} t_contenidos_pagina;

typedef struct{
    t_list *tlb;
}t_tabla_tlb;

typedef struct{
    t_list *marcos;
}t_tabla_marcos;

t_config_memoria *config_memoria;
t_log *logger_memoria;
t_tabla_tlb* tabla_tlb;
t_tabla_marcos* tabla_marcos;

t_list* tabla_procesos;

//Reemplazo tlb
t_list *tlb_LRU;
t_queue *tlb_FIFO;
int entradas_tlb;


void* tamanio_memoria;
int socket_server, socket_cliente_swap, socket_client;
#endif