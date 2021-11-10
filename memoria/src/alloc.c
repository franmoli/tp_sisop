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

void crearPrimerAlloc(int size) {

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

void memAlloc(int size) {

    uint32_t inicio = tamanio_memoria;

    t_pagina* primeraPagina = list_get(tabla_paginas->paginas,0);
    if (primeraPagina->cantidad_contenidos == 0){

        crearPrimerAlloc(size);
        primeraPagina->cantidad_contenidos += 2;
        primeraPagina->tamanio_ocupado += size + sizeof(t_heap_metadata) * 2;

    }

    else {

        t_heap_metadata* data = traerAllocDeMemoria(inicio);
        log_info(logger_memoria, "Traje primer alloc de memoria");
        uint32_t sizeAlloc;
        uint32_t nextAnterior = tamanio_memoria;
        uint32_t primeraDir = tamanio_memoria;

        while(data->nextAlloc != NULL) { 
            if(data->isFree == true){

                //Estoy en un alloc libre y no es el ultimo, hacer si entra totalmente, sino que siga

                if(data->prevAlloc == NULL) {
                    log_info(logger_memoria, "first alloc");
                    sizeAlloc = data->nextAlloc - primeraDir - sizeof(t_heap_metadata);
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
                } else if(sizeAlloc > size + sizeof(t_heap_metadata)) {

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

        log_info(logger_memoria, "Voy a guardar un nuevo alloc");
        uint32_t a4 = data;
        if(config_memoria->TAMANIO + tamanio_memoria < nextAnterior + sizeof(t_heap_metadata) * 2 + size) {
            log_info(logger_memoria, "Memoria insuficiente");
            //Nose si tendria que mandar a swap aca
            return;
        }

        // Aca entonces tengo que guardar un nuevo alloc

        data->isFree = false;
        data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) + size;

        guardarAlloc(data, nextAnterior);

        data->isFree = true;
        data->prevAlloc = nextAnterior;
        data->nextAlloc = NULL;

        guardarAlloc(data,nextAnterior + size + sizeof(t_heap_metadata));

        log_info(logger_memoria, "Fin Memalloc, guarde alloc nuevo al final");

        return;

    }
}