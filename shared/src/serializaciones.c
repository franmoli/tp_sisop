#include "../include/serializaciones.h"

/**************************** Memoria <---> Swap ****************************/

t_paquete *serializar_mate_call_io(char *resource, void *msg){

    t_mate_call_io *aux = malloc(sizeof(t_mate_call_io));
    aux->resource = malloc(sizeof(char)*strlen(resource)+1);
    aux->resource = resource;
    aux->msg = malloc(sizeof(msg)+1);
    aux->msg = msg;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(char)*strlen(resource) + sizeof(msg) + 2;

    void *stream = malloc(buffer->size);

    int offset = 0;

    memcpy(stream + offset, &(aux->resource),sizeof(char)*strlen(resource)+1);
    offset += sizeof(char)*strlen(resource) + 1;
    memcpy(stream + offset, &(aux->msg),sizeof(msg) + 1);

    buffer->stream = stream;
    paquete->buffer = buffer;
    paquete->codigo_operacion = CALLIO;

    return paquete;
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

        memcpy(stream + offset, &(contenido->inicio), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->fin), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->contenido->prevAlloc), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->contenido->nextAlloc), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->contenido->isFree), sizeof(uint8_t));
	    offset += sizeof(uint8_t);
    }

    //Contenido Carpincho
    memcpy(stream + offset, &(pagina.contenido_carpincho_info->elements_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);
    
    for(int i=0; i<list_size(pagina.contenido_carpincho_info); i++) {
        t_info_carpincho_swap *contenido = list_get(pagina.contenido_carpincho_info, i);

        memcpy(stream + offset, &(contenido->size), sizeof(int));
	    offset += sizeof(int);
        memcpy(stream + offset, &(contenido->inicio), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->fin), sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(stream + offset, &(contenido->contenido), sizeof(contenido->size));
	    offset += sizeof(contenido->size);
    }

    return stream;
}

t_pagina_swap deserializar_pagina(void *stream) {
    t_pagina_swap pagina;
    int offset = 0;

    //Tipo de contenido
    memcpy(&pagina.tipo_contenido, stream + offset, sizeof(int));
	offset += sizeof(int);

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
        t_heap_metadata *heap_metadata = malloc(sizeof(t_heap_metadata));
        
        memcpy(&((*contenido_heap).inicio), stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&((*contenido_heap).fin), stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&((*heap_metadata).prevAlloc), stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&((*heap_metadata).nextAlloc), stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&((*heap_metadata).isFree), stream + offset, sizeof(uint8_t));
	    offset += sizeof(uint8_t);

        contenido_heap->contenido = heap_metadata;
        list_add(contenidos_heap, contenido_heap);
    }

    pagina.contenido_heap_info = contenidos_heap;

    //Contenido heap
    cantidad_elementos = 0;
    memcpy(&cantidad_elementos, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

    t_list *contenidos_carpincho = list_create();
    for(int i=0; i<cantidad_elementos; i++) {
        t_info_carpincho_swap *contenido_carpincho = malloc(sizeof(t_info_carpincho_swap));
        
        memcpy(&((*contenido_carpincho).size), stream + offset, sizeof(int));
	    offset += sizeof(int);
        memcpy(&((*contenido_carpincho).inicio), stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&((*contenido_carpincho).fin), stream + offset, sizeof(uint32_t));
	    offset += sizeof(uint32_t);
        memcpy(&((*contenido_carpincho).contenido), stream + offset, sizeof(contenido_carpincho->size));
	    offset += sizeof(contenido_carpincho->size);

        list_add(contenidos_carpincho, contenido_carpincho);
    }

    pagina.contenido_carpincho_info = contenidos_carpincho;
    return pagina;
}

t_paquete * serializar (int codigo_operacion, int arg_count, ...){

    //Declaraciones de variables
    void *stream = NULL;
    int size = 0;
    int offset = 0;
    int added_size = 0;

    //Declaracion de parametros posibles
    int param_i = 0;
    unsigned int param_un_i = 0; 
    char *param_s = NULL;
    uint32_t param_ui = 0;
    bool param_b = false;
    t_list *param_l = NULL; 
    t_paquete *paquete_aux = malloc(sizeof(t_paquete));
    paquete_aux->buffer = malloc(sizeof(t_buffer));
    t_type tipo_de_lista = INT;
    void *list_elem = NULL;

    va_list valist;
    va_start(valist, arg_count);

    for (int i = 0; i < arg_count; i += 2) {
        //Primer parametro es el tipo de dato que se quiere serializar
        t_type tipo = va_arg(valist, t_type);

        switch(tipo){
            case INT:
                param_i = va_arg(valist, int);
                added_size = sizeof(int);

                serializar_single(&stream, &param_i, &size, added_size, &offset);

                break;
            case CHAR_PTR:       
                param_s = va_arg(valist, char*);
                int string_length = strlen(param_s) +1;
                added_size = sizeof(char) * sizeof(char)*string_length;

                serializar_single(&stream, &string_length, &size, sizeof(int), &offset);   

                serializar_single(&stream, param_s, &size, added_size, &offset);                

                break;
            case UINT32:
                param_ui = va_arg(valist, uint32_t);
                added_size = sizeof(uint32_t);

                serializar_single(&stream, &param_ui, &size, added_size, &offset);

                break;
            case BOOL:
                param_b = va_arg(valist, bool);
                added_size = sizeof(bool);

                serializar_single(&stream, &param_b, &size, added_size, &offset);

                break;
            case LIST:
                param_l = va_arg(valist, t_list*);
                //Se trae el tipo de lista y se incrementa i por el va_arg extra
                tipo_de_lista = va_arg(valist, t_type); 
                i++;

                for(int j = 0; j < list_size(param_l); j++){

                    //Traigo un elemento de la lista y lo serializo recursivamente
                    list_elem = list_get(param_l, j);
                    paquete_aux = serializar(NUEVO_CARPINCHO, 2, tipo_de_lista, list_elem);
                    added_size = paquete_aux->buffer->size;


                    serializar_single(&stream, paquete_aux->buffer->stream, &size, added_size, &offset);
                }
                break;
            case U_INT:
                param_un_i = va_arg(valist, unsigned int);
                added_size = sizeof(unsigned int);

                serializar_single(&stream, &param_un_i, &size, added_size, &offset);

                break;
        }
    }
    
    //Se libera la lista de argumentos
    va_end(valist);

    //Se arma el paquete
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    paquete->buffer->size = size;
    paquete->buffer->stream = stream;
    paquete->codigo_operacion = codigo_operacion;



    //deserialize test
    /*printf("Intentando deserializar esto\n");

    int numero2 = 0;
    memcpy(&numero2, stream, sizeof(int));
    printf("Numero2: %d\n", numero2);

    char *string2 = malloc(sizeof(char)*7);
    memcpy(string2, stream + sizeof(int), sizeof(char)*7);
    printf("string2: %s\n", string2);*/

    return paquete;
}

void serializar_single (void **stream, void *elem, int *stream_size, int added_size, int *offset){

    //Expandir espacio en buffer
    *stream_size = *stream_size + added_size;
    *stream = realloc(*stream, *stream_size);
    

    //copiar contenido a buffer
    memcpy(*stream + *offset, elem, added_size);
    
    //correr offset
    *offset += added_size;

    return;
}

void deserializar_single (void *stream, void *elem, int size, int *offset){    

    //copiar contenido a variable
    memcpy(elem, stream + *offset, size);
    
    //correr offset
    *offset += size;

    return;
}

void deserializar(t_paquete *paquete, int arg_count, ...){

    int size = 0;
    void *param = NULL;
    void *stream = paquete->buffer->stream;
    int offset = 0;
    char **param_char = NULL;
    va_list valist;
    va_start(valist, arg_count);

    for (int i = 0; i < arg_count; i += 2){
        //Primer parametro es el tipo de dato que se quiere deserializar
        t_type tipo = va_arg(valist, t_type);
        switch(tipo){
            case INT:
                param = va_arg(valist, void*);
                size = sizeof(int);
                deserializar_single(stream, param, size, &offset);

                break;
            case CHAR_PTR:  
                param_char = va_arg(valist, char**);
                //Primero traigo el tamanio del string
                size = sizeof(int);
                deserializar_single(stream, &size, size, &offset);
                //Con el tamanio del string asigno memoria y traigo variable
                *param_char = realloc(*param_char, size);
                deserializar_single(stream, *param_char, size, &offset);              

                break;
            case UINT32:
                param = va_arg(valist, void*);
                size = sizeof(uint32_t);
                deserializar_single(stream, param, size, &offset);

                break;
            case BOOL:
                param = va_arg(valist, void*);
                size = sizeof(bool);
                deserializar_single(stream, param, size, &offset);

                break;
            case U_INT:
                param = va_arg(valist, void*);
                size = sizeof(unsigned int);
                deserializar_single(stream, param, size, &offset);

                break;
            case LIST:
                break;
        }
    }
}

t_paquete *serializar_direccion_logica(t_kernel_dire_logica_serializado* direccion_logica){
    t_paquete *paquete = malloc(sizeof(t_paquete));
    t_buffer *new_buffer = malloc(sizeof(t_buffer));
    new_buffer->size = sizeof(uint32_t);
    int offset = 0;
    void *stream = malloc(new_buffer->size);
    memcpy(stream + offset, &(direccion_logica->direccion_logica), sizeof(uint32_t));

    new_buffer->stream = stream;
    paquete->buffer = new_buffer;
    paquete->codigo_operacion = DIRECCION_LOGICA;
    return paquete;
}
t_kernel_dire_logica_serializado *deserializar_direccion_logica(t_paquete* paquete){
    t_kernel_dire_logica_serializado* direccion_logica = malloc(sizeof(t_kernel_dire_logica_serializado));
    void *stream = paquete->buffer->stream;
    int offset = 0;
    memcpy(&(direccion_logica->direccion_logica), stream + offset, sizeof(uint32_t));
    return direccion_logica;
}