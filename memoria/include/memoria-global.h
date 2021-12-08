#ifndef MEMORIA_GLOBAL_H
#define MEMORIA_GLOBAL_H

#include "config_utils.h"
#include "server.h"
#include "serializaciones.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

typedef enum {
	CONTENIDO = 0,
    ALLOC = 1,
    HEADER = 2,
    FOOTER = 3,
    RESTO_CONTENIDO = 4,
    PREV = 5,
    NEXT = 6,
    FREE = 7,
    RESTO_ALLOC = 8,
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
    uint32_t pid;
    uint32_t numero_pagina;
}t_reemplazo;

typedef struct{
    uint32_t pid;
    uint32_t numero_pagina;
    int bit_referido;
    int bit_modificado;
}t_clock;

typedef struct{
    t_list* paginas;
    int ultimo;
}t_lista_clock;

typedef struct{
    uint32_t numero_marco;
    bool isFree;
}t_marco;

typedef struct{
    int pid;
    t_list *paginas;
    t_list *Lru;
    t_list *Clock;
    int ultimo_Clock;
    int paginas_totales_maximas;
    int paginas_en_memoria;
}t_tabla_paginas;

typedef struct{
    uint32_t numero_pagina;
    uint32_t tamanio_ocupado;
    uint32_t cantidad_contenidos;
    uint32_t marco_asignado;
    uint32_t carpincho_id;
    bool bit_uso;
    bool bit_presencia;
    bool bit_modificado;
    int  usado;
    t_list*  listado_de_contenido;
}t_pagina;

typedef struct{
    t_contenido contenido_pagina;
    uint32_t pagina_seguir;
    uint32_t desplazamiento;
}
t_subcontenido;

typedef struct{
		uint32_t dir_comienzo; // NO DEBE SER UN PUNTERO
		uint32_t dir_fin;
        uint32_t tamanio; 
        uint32_t carpincho_id;
        uint32_t recorrido;
		t_contenido contenido_pagina; //antes era un puntero
        t_subcontenido *subcontenido; //PREV-NEXT-FREE
	} t_contenidos_pagina;

typedef struct{
    t_list *tlb;
    uint32_t hit_totales;
    uint32_t miss_totales;
}t_tabla_tlb;

typedef struct{
    t_list *marcos;
}t_tabla_marcos;

typedef struct{
    t_pagina *pagina;
    void *contenido;
}t_envio_swap;

t_config_memoria *config_memoria;
t_log *logger_memoria;
t_tabla_tlb* tabla_tlb;
t_tabla_marcos* tabla_marcos_memoria;

t_list* tabla_procesos;

//Reemplazo tlb
t_list *tlb_LRU;
t_queue *tlb_FIFO;
int entradas_tlb;

//Reemplazo LRU y CLOCK
t_list *reemplazo_LRU;
t_lista_clock *reemplazo_CLOCK;

void* tamanio_memoria;
int socket_server, socket_cliente_swap, socket_client;

/* Conexión de MEMORIA con SWAP */
enum info_contenido {
	AMBOS = 0, 
	CARPINCHO = 1,
	HEAP = 2
}; 

typedef struct {
    enum info_contenido tipo_contenido;
	uint32_t pid;
	uint32_t numero_pagina;
	t_list* contenido_heap_info;
	//t_list* contenido_carpincho_info;
} t_pagina_swap;

typedef struct {
	//uint32_t inicio;
	//uint32_t fin;
	t_heap_metadata* contenido;
} t_info_heap_swap;

typedef struct {
	uint32_t size_proximo_contenido;
	uint32_t inicio;
	uint32_t fin;
	//t_carpincho* carpincho;
} t_info_carpincho_swap;

//Funciones para el calculo de bytes
int bytes_pagina(t_pagina_swap pagina);
int bytes_info_heap(t_info_heap_swap info);
//int bytes_info_carpincho(t_carpincho info);

//Funciones para serializar y deserializar
void* serializar_pagina(t_pagina_swap pagina);
t_pagina_swap deserializar_pagina(void *stream);

pthread_mutex_t mutex_envio_pagina;
#endif