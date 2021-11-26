#include "../include/serializaciones.h"

/**************************** Memoria <---> Swap ****************************/
/* Mate sem init  */
void *serializar_mate_sem_init(void **stream,uint32_t valor, char *nombre_sem){
    int offset = 0;

    memcpy(stream + offset, &(valor), sizeof(uint32_t));
    offset+= sizeof(uint32_t);
    memcpy(stream + offset, &(nombre_sem), sizeof(char)*strlen(nombre_sem) +1);

    return *stream;
}

t_mate_sem *deserializar_mate_sem_init(t_paquete *paquete){

    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(sizeof(char)*15);
    void *stream = paquete->buffer->stream;

    int offset = 0;
    
    memcpy(&(datos_sem->value),stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(datos_sem->nombre),stream + offset, sizeof(char)*15);
    return datos_sem;
}

/* Mate Sem Resto */
t_paquete *serializar_mate_sem_resto(char *nombre_sem, op_code cod_op){
    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(sizeof(char)*15);
    datos_sem->nombre = nombre_sem;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(char)*15;

    int offset = 0;
    void *stream = malloc(buffer->size);

    memcpy(stream + offset,&(datos_sem->nombre),sizeof(char)*15);

    buffer->stream = stream;
    paquete->buffer = buffer;
    paquete->codigo_operacion = cod_op;

    return paquete;
}

t_mate_sem *deserializar_mate_sem_resto(t_paquete *paquete){

    t_mate_sem *datos_sem = malloc(sizeof(t_mate_sem));
    datos_sem->nombre = malloc(sizeof(char)*15);
    datos_sem->value = 0;
    void *stream = paquete->buffer->stream;

    int offset = 0;

    memcpy(&(datos_sem->nombre),stream + offset, sizeof(char)*15);
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