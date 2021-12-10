#ifndef SERIALIZACIONES__H
#define SERIALIZACIONES__H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdarg.h>
#include "server.h"
#include "../../memoria/include/memoria-global.h"
#include "../../kernel/include/kernel-global.h"

typedef struct {
	uint32_t size_reservar;
	uint32_t carpincho_id;
} t_malloc_serializado;

typedef struct{
	uint32_t direccion_logica;
} t_kernel_dire_logica_serializado;

typedef struct {
	uint32_t carpincho_id;
} t_mateinit_serializado;

typedef struct {
	uint32_t carpincho_id;
	bool swap_free;
} t_swap_serializado;

typedef struct{
	char *resource;
	void *msg;
} t_mate_call_io;

typedef enum {
    INT = 1,
    CHAR_PTR = 2,
    UINT32 = 3,
    BOOL = 4,
    LIST = 5,
	U_INT = 6
}t_type;

/* Conexión de MEMORIA con SWAP */
typedef struct {
    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;
} __attribute__((packed))
t_heap_swap;

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
	t_list* contenido_carpincho_info;
} t_pagina_swap;

typedef struct {
	uint32_t inicio;
	uint32_t fin;
	t_heap_swap* contenido;
} t_info_heap_swap;

typedef struct {
	int size;
	uint32_t inicio;
	uint32_t fin;
	char* contenido;
} t_info_carpincho_swap;

t_paquete *serializar_mate_init(uint32_t carpincho_id);
t_mateinit_serializado* deserializar_mate_init(t_paquete *paquete);


t_paquete *serializar_alloc(uint32_t size, uint32_t carpincho_id);
t_malloc_serializado* deserializar_alloc(t_paquete *paquete);

t_paquete *serializar_consulta_swap(uint32_t carpincho_id);
t_swap_serializado* deserializar_swap(t_paquete *paquete);

t_paquete *serializar_direccion_logica(t_kernel_dire_logica_serializado* direccion_logica);
t_kernel_dire_logica_serializado *deserializar_direccion_logica(t_paquete* paquete);

//Utilizacion: serializar(Codigo de operacion , cant de argumentos (2 * cantidad de datos a serializar) , datos a serializar)
//             datos a serializar = (TIPO DE DATO (segun enum t_type) , DATO .....)
//             para enviar una lista será = (LIST, tipo de datos que contiene la lista , PUNTERO A LA LISTA) / las listas ponerlas siempre al final para usar el deserializar
t_paquete * serializar (int codigo_operacion, int arg_count, ...);
void serializar_single (void **stream, void *elem, int *stream_size, int elem_size, int *offset);


//Utilizacion: deserializar(paquete, TIPO DE DATO(segun el enum t_type), &DATO BUSCADO) para strings se pasa con & ej: char *string_objetivo; deserializar(paquete, CHAR_PTR, &string_objetivo)
void deserializar(t_paquete *paquete, int arg_count, ...);
void deserializar_single (void *stream, void *elem, int size, int *offset);

void* serializar_pagina(t_pagina_swap* pagina);
t_pagina_swap* deserializar_pagina(void *stream);

//Funciones para el calculo de bytes
int bytes_pagina(t_pagina_swap* pagina);
int bytes_info_heap(t_info_heap_swap info);
//int bytes_info_carpincho(t_carpincho info);

#endif