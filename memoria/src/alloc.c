#include "alloc.h"


// Memfree -> Libero alloc (flag isFree en true), me fijo el anterior y posterior y los unifico
// TODO -> Meter paginacion (Mati gatooo)
void freeAlloc(uint32_t direccion) {

    //Traigo de memoria el alloc
    t_heap_metadata* alloc = traerAllocDeMemoria(direccion);

    alloc->isFree = true;

    t_heap_metadata* anterior = traerAllocDeMemoria(alloc->prevAlloc);
    t_heap_metadata* posterior = malloc(sizeof(t_heap_metadata));
    posterior->isFree = false;
    if(alloc->nextAlloc != NULL) {
        posterior = traerAllocDeMemoria(alloc->nextAlloc);
    }

    if(anterior->isFree) {
        //Juntar los 2 allocs
        anterior->nextAlloc = alloc->nextAlloc;
        guardarAlloc(anterior,alloc->prevAlloc);
        if(alloc->nextAlloc != NULL) {
            posterior->prevAlloc = alloc->prevAlloc;
            guardarAlloc(posterior,alloc->nextAlloc);
        }

    }

    if(alloc->nextAlloc != NULL) {

        if(posterior->isFree){
            //Juntar los 2 allocs
            alloc->nextAlloc = posterior->nextAlloc;
            t_heap_metadata* posteriorDelPosterior = traerAllocDeMemoria(posterior->nextAlloc);

            posteriorDelPosterior->prevAlloc = posterior->prevAlloc;

            guardarAlloc(alloc, direccion);
            guardarAlloc(posteriorDelPosterior,posterior->nextAlloc);

        }

    }

}


t_heap_metadata* traerAllocDeMemoria(uint32_t direccion) {

    t_heap_metadata* data = malloc(sizeof(t_heap_metadata));

    uint32_t offset = 0;
    memcpy(&data->prevAlloc,direccion + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->nextAlloc, direccion + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->isFree, direccion + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    return data;

}

void crearFooterAlloc(t_pagina *primeraPagina, int inicio,int size){
    t_heap_metadata* alloc = malloc(sizeof(t_heap_metadata));

    alloc->isFree = true;
    alloc->prevAlloc = inicio;
    alloc->nextAlloc = NULL;

    int offset = 0;
    t_contenidos_pagina *contenido = getLastContenidoByPagina(primeraPagina);
    memcpy(contenido->dir_fin + offset, &alloc->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(contenido->dir_fin + offset, &alloc->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(contenido->dir_fin + offset, &alloc->isFree, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    t_contenidos_pagina *contenidoAnterior = getLastContenidoByPagina(primeraPagina);
    t_contenidos_pagina *contenidoFooter = malloc(sizeof(t_contenidos_pagina));
    
    contenidoFooter->carpincho_id = socket_client;
    contenidoFooter->dir_comienzo = contenidoAnterior->dir_fin;
    contenidoFooter->tamanio =  sizeof(t_heap_metadata);
    contenidoFooter->dir_fin = contenidoFooter->dir_comienzo + contenidoFooter->tamanio;
    contenidoFooter->contenido_pagina = FOOTER;
    primeraPagina->cantidad_contenidos +=1;
    primeraPagina->tamanio_ocupado += size;
    list_add(primeraPagina->listado_de_contenido, contenidoFooter);

    free(alloc);
}
void crearHeaderAlloc(t_pagina *primeraPagina,int size){
     //HEADER
    t_heap_metadata* newAlloc = malloc(sizeof(t_heap_metadata));
    uint32_t inicio = tamanio_memoria;
    newAlloc->isFree = false;
    
    if(primeraPagina->numero_pagina == 0 && primeraPagina->cantidad_contenidos == 0){
        newAlloc->prevAlloc = NULL;
    }else{
        int prev = inicio + (config_memoria->TAMANIO_PAGINA * primeraPagina->numero_pagina);
        newAlloc->prevAlloc = prev;
    }
    newAlloc->nextAlloc = inicio + size;

    int offset = 0;
    //memcpy(tamanio_memoria, &newAlloc, sizeof(t_heap_metadata));
    memcpy(inicio + offset, &newAlloc->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(inicio + offset, &newAlloc->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(inicio + offset, &newAlloc->isFree, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    t_contenidos_pagina *contenidoNuevo = malloc(sizeof(t_contenidos_pagina));
    contenidoNuevo->carpincho_id = socket_client;
    contenidoNuevo->dir_comienzo = inicio;
    contenidoNuevo->dir_fin = newAlloc->nextAlloc;
    contenidoNuevo->tamanio = size;
    contenidoNuevo->contenido_pagina = HEADER;

    list_add(primeraPagina->listado_de_contenido, contenidoNuevo);
    primeraPagina->cantidad_contenidos +=1;
    primeraPagina->tamanio_ocupado += size;
    free(newAlloc);
}
void crearPrimerAlloc(t_pagina* primeraPagina,int size) {

    //HEADER + DATA
    t_heap_metadata* newAlloc = malloc(sizeof(t_heap_metadata));
    uint32_t inicio = tamanio_memoria;
    newAlloc->isFree = false;
    newAlloc->prevAlloc = NULL;
    newAlloc->nextAlloc = inicio + size + sizeof(t_heap_metadata);

    int offset = 0;
    //memcpy(tamanio_memoria, &newAlloc, sizeof(t_heap_metadata));
    memcpy(inicio + offset, &newAlloc->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(inicio + offset, &newAlloc->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(inicio + offset, &newAlloc->isFree, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    t_contenidos_pagina *contenidoNuevo = malloc(sizeof(t_contenidos_pagina));
    contenidoNuevo->carpincho_id = socket_client;
    contenidoNuevo->dir_comienzo = inicio;
    contenidoNuevo->dir_fin = newAlloc->nextAlloc;
    contenidoNuevo->tamanio = size + sizeof(t_heap_metadata);

    list_add(primeraPagina->listado_de_contenido, contenidoNuevo);

    free(newAlloc);

    //FOOTER
    t_heap_metadata* alloc = malloc(sizeof(t_heap_metadata));

    alloc->isFree = true;
    alloc->prevAlloc = inicio;
    alloc->nextAlloc = NULL;

    offset = 0;

    memcpy(inicio + size + sizeof(t_heap_metadata) + offset, &alloc->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(inicio + size + sizeof(t_heap_metadata) + offset, &alloc->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(inicio + size + sizeof(t_heap_metadata) + offset, &alloc->isFree, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    t_contenidos_pagina *contenidoFooter = malloc(sizeof(t_contenidos_pagina));
    contenidoFooter->carpincho_id = socket_client;
    contenidoFooter->dir_comienzo = contenidoNuevo->dir_fin + 1;
    contenidoFooter->tamanio =  sizeof(t_heap_metadata);
    contenidoFooter->dir_fin = contenidoFooter->dir_comienzo + contenidoFooter->tamanio;
    list_add(primeraPagina->listado_de_contenido, contenidoNuevo);

    free(alloc);

    log_info(logger_memoria, "Pude guardar primer alloc en memoria");
}

void guardarAlloc(t_heap_metadata* data, uint32_t direccion) {

    uint32_t offset = 0;
    //memcpy(tamanio_memoria, &newAlloc, sizeof(t_heap_metadata));
    memcpy(direccion + offset, &data->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(direccion + offset, &data->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(direccion + offset, &data->isFree, sizeof(uint8_t));
    offset += sizeof(uint8_t);

}


void memAlloc(t_paquete *paquete) {

    int size = deserializar_alloc(paquete);
    uint32_t inicio = tamanio_memoria;
    int paginaDisponible = getPrimeraPaginaDisponible(sizeof(t_heap_metadata));
    t_pagina* primeraPagina = list_get(tabla_paginas->paginas,paginaDisponible);
    int entraTotal = config_memoria->TAMANIO_PAGINA - primeraPagina->tamanio_ocupado - (sizeof(t_heap_metadata) * 2) - size;
    t_contenidos_pagina *contenidoHeader;
    if (entraTotal >=0){
        //HEADER
        crearHeaderAlloc(primeraPagina,sizeof(t_heap_metadata));
        contenidoHeader = getLastContenidoByPagina(primeraPagina);
        t_heap_metadata* heap = traerAllocDeMemoria(contenidoHeader->dir_comienzo);
        heap->nextAlloc +=size;
        guardarAlloc(heap, contenidoHeader->dir_comienzo);

        //CONTENIDO
        t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
        contenido->carpincho_id = socket_client;
        contenido->dir_comienzo = contenidoHeader->dir_fin;
        contenido->tamanio =  size;
        contenido->dir_fin = contenido->dir_comienzo + contenido->tamanio;
        contenido->contenido_pagina = CONTENIDO;
        list_add(primeraPagina->listado_de_contenido, contenido);
        primeraPagina->tamanio_ocupado+= size;
        

        //FOOTER
        primeraPagina->cantidad_contenidos += 1;
        crearFooterAlloc(primeraPagina,contenidoHeader->dir_comienzo,sizeof(t_heap_metadata));

    }
    else {
        uint32_t nextAnterior = tamanio_memoria;
        uint32_t primeraDir = tamanio_memoria;
        uint32_t sizeAlloc;
        t_heap_metadata* data = getLastHeapFromPagina(paginaDisponible);
        while(data->nextAlloc != NULL) { 
            nextAnterior = data->nextAlloc;
            data = traerAllocDeMemoria(data->nextAlloc);
        }
        entraTotal = config_memoria->TAMANIO_PAGINA - primeraPagina->tamanio_ocupado - sizeof(t_heap_metadata) - size;
        if(entraTotal > 0){
            //ENTRA EL HEADER Y CONTENIDO EN LA PAGINA

        }else{
            //ENTRA SOLO EL HEADER
            //EDITO ULTIMO HEAP
            contenidoHeader = getLastContenidoByPagina(primeraPagina);
            data->isFree = false;
            data->nextAlloc = contenidoHeader->dir_fin + size;
            guardarAlloc(data, contenidoHeader->dir_comienzo);

            //GUARDO NUEVO HEAP
            data = traerAllocDeMemoria(contenidoHeader->dir_comienzo);
            data->prevAlloc =contenidoHeader->dir_fin;
            data->nextAlloc = NULL;
            data->isFree = true;
            guardarAlloc(data, contenidoHeader->dir_fin);
        }
        return;
    }
}
t_heap_metadata* getLastHeapFromPagina(int pagina){
    t_pagina *paginaBuscada = list_get(tabla_paginas->paginas,pagina);
    t_contenidos_pagina* contenidoUltimo = getLastHeaderContenidoByPagina(paginaBuscada);
    t_heap_metadata *metadata = traerAllocDeMemoria(contenidoUltimo->dir_comienzo);
    return metadata;
}