#include "../include/serializaciones.h"

/**************************** Memoria <---> Swap ****************************/
/* Mate sem init  */
t_paquete *serializar_mate_sem_init(uint32_t valor, char *nombre_sem){
    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(sizeof(char)*strlen(nombre_sem)+1);
    datos_sem->nombre = nombre_sem;
    datos_sem->value = valor;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    u_int32_t stream_size = sizeof(uint32_t) + sizeof(char)*strlen(nombre_sem) + 1;
    buffer->size = stream_size;

    void *stream = malloc(buffer->size);

    printf("estoy en el serializar, el nombre del semaforo es: %s\n",datos_sem->nombre);
    
    int offset = 0;

    memcpy(stream + offset, &(datos_sem->value), sizeof(uint32_t));
    offset+= sizeof(uint32_t);
    memcpy(stream + offset, &(datos_sem->nombre), sizeof(char)*strlen(nombre_sem)+1);

    buffer->stream = stream;
    paquete->buffer = buffer;
    paquete->codigo_operacion = INIT_SEM;

    return paquete;
}

t_mate_sem *deserializar_mate_sem_init(t_paquete *paquete){

    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(paquete->buffer->size - sizeof(u_int32_t));
    void *stream = paquete->buffer->stream;
    printf("el tamanio del buffer del paquete recibido es: %d\n",paquete->buffer->size);

    int offset = 0;
    
    memcpy(&(datos_sem->value),stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(datos_sem->nombre),stream + offset, paquete->buffer->size - sizeof(u_int32_t));
    
    return datos_sem;
}

/* Mate Sem Resto */
t_paquete *serializar_mate_sem_resto(char *nombre_sem, op_code cod_op){
    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(sizeof(char)*strlen(nombre_sem)+1);
    datos_sem->nombre = nombre_sem;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(char)*strlen(nombre_sem)+1;

    int offset = 0;
    void *stream = malloc(buffer->size);

    memcpy(stream + offset,&(datos_sem->nombre),buffer->size);

    buffer->stream = stream;
    paquete->buffer = buffer;
    paquete->codigo_operacion = cod_op;

    return paquete;
}

t_mate_sem *deserializar_mate_sem_resto(t_paquete *paquete){

    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(paquete->buffer->size);
    datos_sem->value = 0;
    void *stream = paquete->buffer->stream;

    int offset = 0;

    memcpy(&(datos_sem->nombre),stream + offset, paquete->buffer->size);
    return datos_sem;
}

/* Allocs */
t_paquete *serializar_alloc(uint32_t size, uint32_t carpincho_id){
    t_malloc_serializado* malloc_serializado = malloc(sizeof(t_malloc_serializado));
    malloc_serializado->size_reservar = size;
    malloc_serializado->carpincho_id = carpincho_id;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *new_buffer = malloc(sizeof(t_buffer));
    new_buffer->size = sizeof(uint32_t)*2;
    int offset = 0;
    void *stream = malloc(new_buffer->size);
    memcpy(stream + offset, &(malloc_serializado->size_reservar), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(malloc_serializado->carpincho_id), sizeof(uint32_t));

    new_buffer->stream = stream;
    paquete->buffer = new_buffer;
    paquete->codigo_operacion = MEMALLOC;
    return paquete;
}

t_malloc_serializado* deserializar_alloc(t_paquete *paquete){
    t_malloc_serializado* malloc_serializado = malloc(sizeof(t_malloc_serializado));
    void *stream = paquete->buffer->stream;
    int offset = 0;
    memcpy(&(malloc_serializado->size_reservar), stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(malloc_serializado->carpincho_id), stream + offset, sizeof(uint32_t));
    return malloc_serializado;
}

/* Mateinit */
t_paquete *serializar_mate_init(uint32_t carpincho_id){
    t_mateinit_serializado* malloc_serializado = malloc(sizeof(t_mateinit_serializado));
    malloc_serializado->carpincho_id = carpincho_id;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *new_buffer = malloc(sizeof(t_buffer));
    new_buffer->size = sizeof(uint32_t)*2;
    int offset = 0;
    void *stream = malloc(new_buffer->size);
    memcpy(stream + offset, &(malloc_serializado->carpincho_id), sizeof(uint32_t));

    new_buffer->stream = stream;
    paquete->buffer = new_buffer;
    paquete->codigo_operacion = MATEINIT;
    return paquete;
}

t_mateinit_serializado* deserializar_mate_init(t_paquete *paquete){
    t_mateinit_serializado* malloc_serializado = malloc(sizeof(t_mateinit_serializado));
    void *stream = paquete->buffer->stream;
    int offset = 0;
    memcpy(&(malloc_serializado->carpincho_id), stream + offset, sizeof(uint32_t));
    return malloc_serializado;
}

/* SWAP <-> MEMORIA */
t_paquete *serializar_consulta_swap(uint32_t carpincho_id){
    t_swap_serializado* swap_serializado = malloc(sizeof(t_swap_serializado));
    swap_serializado->carpincho_id = carpincho_id;
    swap_serializado->swap_free = false;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *new_buffer = malloc(sizeof(t_buffer));
    new_buffer->size = sizeof(uint32_t)*2 + sizeof(bool);
    int offset = 0;
    void *stream = malloc(new_buffer->size);
    memcpy(stream + offset, &(swap_serializado->carpincho_id), sizeof(uint32_t));
    offset+=(sizeof(uint32_t));
    memcpy(stream + offset, &(swap_serializado->swap_free), sizeof(bool));

    new_buffer->stream = stream;
    paquete->buffer = new_buffer;
    paquete->codigo_operacion = SWAPFREE;
    return paquete;
}

t_swap_serializado* deserializar_swap(t_paquete *paquete){
    t_swap_serializado* swap_serializado = malloc(sizeof(t_swap_serializado));
    void *stream = paquete->buffer->stream;
    int offset = 0;
    memcpy(&(swap_serializado->carpincho_id), stream + offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(&(swap_serializado->swap_free), stream + offset, sizeof(bool));
    return swap_serializado;
}

//--------------------------------------------------------------------------------------------
int bytes_pagina(t_pagina_swap pagina) {
    int size = 0;

    size += sizeof(pagina.tipo_contenido);
    size += sizeof(pagina.pid);
    size += sizeof(pagina.numero_pagina);

    //Contenidos heap
    size += sizeof(pagina.contenido_heap_info->elements_count);
    for(int i=0; i<list_size(pagina.contenido_heap_info); i++) {
		t_info_heap_swap *contenido = list_get(pagina.contenido_heap_info, i);
		size += bytes_info_heap(*contenido);
	}

    return size;
}

int bytes_info_heap(t_info_heap_swap info) {
    int size = 0;

    //Heap metadata
    size += sizeof(info.contenido->prevAlloc);
    size += sizeof(info.contenido->nextAlloc);
    size += sizeof(info.contenido->isFree);

    return size;
}
//--------------------------------------------------------------------------------------------
void* serializar_pagina(t_pagina_swap pagina) {
    int bytes = bytes_pagina(pagina);
    void *stream = malloc(bytes);
    int offset = 0;

    //Tipo de contenido
    memcpy(stream + offset, &(pagina.tipo_contenido), sizeof(int));
	offset += sizeof(int);

    //PID
    memcpy(stream + offset, &(pagina.pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    //Número de pagina
    memcpy(stream + offset, &(pagina.numero_pagina), sizeof(uint32_t));
	offset += sizeof(uint32_t);

    //Contenido heap
    memcpy(stream + offset, &(pagina.contenido_heap_info->elements_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);
    
    for(int i=0; i<list_size(pagina.contenido_heap_info); i++) {
        t_info_heap_swap *contenido = list_get(pagina.contenido_heap_info, i);

        memcpy(stream + offset, &(contenido->contenido->prevAlloc), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->contenido->nextAlloc), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->contenido->isFree), sizeof(uint8_t));
	    offset += sizeof(uint8_t);
    }

    return stream;
}

t_pagina_swap deserializar_pagina(void *stream) {
    t_pagina_swap pagina;
    int offset = 0;

    //Tipo de contenido
    memcpy(&pagina.tipo_contenido, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    //PID
    memcpy(&pagina.pid, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    //Número de página
    memcpy(&pagina.numero_pagina, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    //Contenido heap
    int cantidad_elementos = 0;
    memcpy(&cantidad_elementos, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    t_list *contenidos_heap = list_create();
    for(int i=0; i<cantidad_elementos; i++) {
        t_info_heap_swap *contenido_heap = malloc(sizeof(t_info_heap_swap));
        
        t_heap_metadata heap_metadata;
        memcpy(&heap_metadata.prevAlloc, stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&heap_metadata.nextAlloc, stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&heap_metadata.isFree, stream + offset, sizeof(uint8_t));
	    offset += sizeof(uint8_t);

        contenido_heap->contenido = &heap_metadata;
        list_add(contenidos_heap, contenido_heap);
    }

    pagina.contenido_heap_info = contenidos_heap;
    return pagina;
}