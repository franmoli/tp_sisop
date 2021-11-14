#include "alloc.h"

// Memfree -> Libero alloc (flag isFree en true), me fijo el anterior y posterior y los unifico
// TODO -> Meter paginacion (Mati gatooo)
void freeAlloc(t_paquete *paquete) {

    uint32_t inicio = tamanio_memoria;
    uint32_t direccion = deserializar_alloc(paquete);
    if(!direccionValida(direccion)){
        //MATE FREE FAIÑT
    }
    //Traigo de memoria el alloc
    t_heap_metadata* alloc = traerAllocDeMemoria(direccion);

    alloc->isFree = true;
    uint32_t next = alloc->nextAlloc;
    uint32_t back = alloc->prevAlloc;
    t_heap_metadata* anterior;
    t_heap_metadata* posterior;
    bool hayAnterior = false;
    bool hayPosterior = false;

    if(alloc->prevAlloc!=0){// SI EXISTE ANTERIOR TRAERLO
        hayAnterior = true;
        anterior = traerAllocDeMemoria(alloc->prevAlloc);
        
        int pagina = getPaginaByDireccion(direccion);
        t_pagina* paginaBuscada = list_get(tabla_paginas->paginas,pagina);
        paginaBuscada->tamanio_ocupado -= (alloc->nextAlloc- inicio -sizeof(t_heap_metadata));

        free(alloc);
        anterior->nextAlloc = next;
        guardarAlloc(anterior,back);
    }
    if(next != NULL){ //SI EXISTE PROXIMO
        hayPosterior=true;
        posterior = traerAllocDeMemoria(next);
        posterior->prevAlloc = back;
        guardarAlloc(posterior,next);
    }
    if(hayAnterior && hayPosterior){
        if(anterior->isFree && posterior->isFree){ //BASICAMENTE, SI AMBOS ESTAN LIBRES -> UNIRLOS
            anterior->nextAlloc = posterior->nextAlloc;
            free(posterior);
            guardarAlloc(anterior,back);
        }
    }
}

bool direccionValida(uint32_t direccion){
    bool esValida = true;
    return esValida;
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
                  int restante = size  - (config_memoria->TAMANIO_PAGINA - pagina->tamanio_ocupado);
                 if(list_size(tabla_paginas->paginas) + round(restante) <= tabla_paginas->paginas_totales_maximas){
                   
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