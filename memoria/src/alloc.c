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
    contenidoNuevo->tamanio = sizeof(t_heap_metadata);
    contenidoNuevo->contenido_pagina = HEADER;

    list_add(primeraPagina->listado_de_contenido, contenidoNuevo);

    t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
    contenido->carpincho_id = socket_client;
    contenido->dir_comienzo = contenidoNuevo->dir_fin;
    contenido->tamanio =  size;
    contenido->dir_fin = contenido->dir_comienzo + contenido->tamanio;
    contenido->contenido_pagina = CONTENIDO;
    list_add(primeraPagina->listado_de_contenido, contenido);
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
    contenidoFooter->dir_comienzo = contenidoNuevo->dir_fin;
    contenidoFooter->tamanio =  sizeof(t_heap_metadata);
    contenidoFooter->dir_fin = contenidoFooter->dir_comienzo + contenidoFooter->tamanio;
    list_add(primeraPagina->listado_de_contenido, contenidoFooter);

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
    if(list_size(tabla_paginas->paginas)==0){
        t_pagina* pagina = malloc(sizeof(t_pagina));
        pagina->listado_de_contenido = list_create();
        pagina->numero_pagina = 0;
        pagina->tamanio_ocupado = size + sizeof(t_heap_metadata) * 2;
        pagina->cantidad_contenidos = 3;
        crearPrimerAlloc(pagina, size);
        list_add(tabla_paginas->paginas,pagina);
    }else{
        int paginaDisponible = getPrimeraPaginaDisponible(size);
        t_heap_metadata* data = traerAllocDeMemoria(inicio);
        uint32_t nextAnterior = tamanio_memoria;
        while(data->nextAlloc != NULL) {
            if(data->isFree){

                //Estoy en un alloc libre y no es el ultimo, hacer si entra totalmente, sino que siga

                 uint32_t sizeAlloc;
                if(data->prevAlloc == NULL) {
                    log_info(logger_memoria, "first alloc");
                    sizeAlloc = data->nextAlloc - inicio - sizeof(t_heap_metadata);
                } else {
                    log_info(logger_memoria, "not first alloc");
                    sizeAlloc = data->nextAlloc - nextAnterior - sizeof(t_heap_metadata);
                }

                if(size == sizeAlloc) {

                    log_info(logger_memoria, "encontre un alloc con el mismo size");

                    //Uso este alloc para guardar
                    data->isFree = false;
                    guardarAlloc(data, nextAnterior);
                    return;
                } 
                else if(sizeAlloc > size + sizeof(t_heap_metadata)) {

                    log_info(logger_memoria, "encontre un alloc con mayor size");

                    data->isFree = false;
                    data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) + size;

                    guardarAlloc(data, nextAnterior);

                    data->isFree = true;
                    data->prevAlloc = nextAnterior;
                    data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) * 2 + sizeAlloc;

                    guardarAlloc(data,nextAnterior + sizeof(t_heap_metadata) + size);
                }
            }
              nextAnterior = data->nextAlloc;
              data = traerAllocDeMemoria(data->nextAlloc);
        }
        //Nuevo Alloc
        int paginaLastAlloc = getPaginaByDireccion(nextAnterior);
         t_pagina* pagina = list_get(tabla_paginas->paginas,paginaLastAlloc);
         if(pagina->tamanio_ocupado < config_memoria->TAMANIO_PAGINA){
             if(pagina->tamanio_ocupado + size < config_memoria->TAMANIO_PAGINA){
                 //entra completo
                data->nextAlloc = nextAnterior + size;
                data->isFree = false;
                guardarAlloc(data,nextAnterior);
                pagina->cantidad_contenidos+=1;
                pagina->tamanio_ocupado += size;

                t_contenidos_pagina *contenidoNuevo = malloc(sizeof(t_contenidos_pagina));
                contenidoNuevo->carpincho_id = socket_client;
                contenidoNuevo->dir_comienzo = data->nextAlloc;

                data->isFree = true;
                data->prevAlloc = nextAnterior;
                data->nextAlloc = NULL;
                guardarAlloc(data,nextAnterior + size + sizeof(t_heap_metadata));
                
                contenidoNuevo->tamanio =  (size);
                contenidoNuevo->dir_fin = contenidoNuevo->dir_comienzo + contenidoNuevo->tamanio;
                contenidoNuevo->contenido_pagina = FOOTER;
                list_add(pagina->listado_de_contenido, contenidoNuevo);

             }
             else{
                 //ocupo el restante y pido otra
                 if(list_size(tabla_paginas->paginas) + 1 <= tabla_paginas->paginas_totales_maximas){
                    int restante = size  - (config_memoria->TAMANIO_PAGINA - pagina->tamanio_ocupado);

                    data->nextAlloc = nextAnterior + size;
                    data->isFree = false;
                    guardarAlloc(data,nextAnterior);
                    pagina->cantidad_contenidos+=1;
                    pagina->tamanio_ocupado += (size - restante);
                    
                    t_contenidos_pagina *contenidoNuevo = malloc(sizeof(t_contenidos_pagina));
                    contenidoNuevo->carpincho_id = socket_client;
                    contenidoNuevo->dir_comienzo = nextAnterior;
                    contenidoNuevo->tamanio =  (size - restante);
                    contenidoNuevo->dir_fin = contenidoNuevo->dir_comienzo + contenidoNuevo->tamanio;
                    contenidoNuevo->contenido_pagina = FOOTER;
                    list_add(pagina->listado_de_contenido, contenidoNuevo);

                    t_pagina* paginaNueva = malloc(sizeof(t_pagina));
                    paginaNueva->listado_de_contenido = list_create();

                    data->isFree = true;
                    data->prevAlloc = nextAnterior;
                    data->nextAlloc = NULL;
                    guardarAlloc(data,nextAnterior + size + sizeof(t_heap_metadata));

                    paginaNueva->numero_pagina = list_size(tabla_paginas->paginas)+1;
                    paginaNueva->tamanio_ocupado = size + sizeof(t_heap_metadata);  
                    paginaNueva->cantidad_contenidos = 2;
                    list_add(tabla_paginas->paginas,paginaNueva);
                 }
                 else{
                     //NO PUEDO PEDIR MAS PAGINAS
                 }
             }
         }
        data = traerAllocDeMemoria(inicio);
        while(data->nextAlloc != NULL) { 
            nextAnterior = data->nextAlloc;
            data = traerAllocDeMemoria(data->nextAlloc);
        }
    }
}
t_heap_metadata* getLastHeapFromPagina(int pagina){
    t_pagina *paginaBuscada = list_get(tabla_paginas->paginas,pagina);
    t_contenidos_pagina* contenidoUltimo = getLastContenidoByPagina(paginaBuscada);
    t_heap_metadata *metadata = traerAllocDeMemoria(contenidoUltimo->dir_comienzo);
    return metadata;
}