#include "paginacion.h"

void guardarMemoria(t_paquete *paquete)
{
    int espacioAguardar = deserializar_alloc(paquete);
    //Reservo Header
    //Reservo Contenido
    //Reservo Footer
    //Agarro primera pagina disponible para los 9bits del heap_metadata
    int numeropaginaHeapHeader = getPrimeraPaginaDisponible(sizeof(t_heap_metadata));
    if (numeropaginaHeapHeader >= 0)
    {
        t_pagina *paginaHeader = list_get(tabla_paginas->paginas, numeropaginaHeapHeader);

        int entraTotal = config_memoria->TAMANIO_PAGINA - paginaHeader->tamanio_ocupado - (sizeof(t_heap_metadata) * 2) - espacioAguardar;
        if (entraTotal >= 0)
        {
            //en la misma pagina me entra el 100% del contenido.
            t_heap_metadata *heapHeader = guardarHeader(paginaHeader, numeropaginaHeapHeader);
            heapHeader->nextAlloc = tamanio_memoria + numeropaginaHeapHeader + (config_memoria->TAMANIO_PAGINA) * numeropaginaHeapHeader + espacioAguardar;
            paginaHeader->cantidad_contenidos = paginaHeader->cantidad_contenidos + 1;
            t_heap_metadata *heapFooter = generarFooter(paginaHeader, numeropaginaHeapHeader);
            memcpy(heapHeader->nextAlloc, &heapFooter, sizeof(t_heap_metadata));
            paginaHeader->cantidad_contenidos = paginaHeader->cantidad_contenidos + 2;
            paginaHeader->tamanio_ocupado = paginaHeader->tamanio_ocupado + sizeof(t_heap_metadata) * 2 + espacioAguardar;
            return;
        }
        else
        {
            int entraTotal = config_memoria->TAMANIO_PAGINA - paginaHeader->tamanio_ocupado - sizeof(t_heap_metadata);
            if (entraTotal == 0)
            {
                //La pagina se llenacon el header nada mas. tengo que buscar otra.
                t_heap_metadata *heapHeader = guardarHeader(paginaHeader, numeropaginaHeapHeader);
                int paginaAGuardar = getPrimeraPaginaDisponible(sizeof(t_heap_metadata));
                if (paginaAGuardar >= 0)
                {
                }
                else
                { // No encontro memoria para meter el contenido
                    //Swap Time
                }
            }
            else
            {
                //Completo esta pagina y tengo que pedir otra mas.
            }
        }
    }
}
t_heap_metadata *guardarHeader(t_pagina *paginaHeader, int paginaHeapHeader)
{
    t_heap_metadata *heapHeader = generarHeaderMetadataAlFinal(paginaHeader,paginaHeapHeader);
    log_info(logger_memoria, "primera direccion %d",tamanio_memoria);
    log_info(logger_memoria, "Voy a guardar data en %d",tamanio_memoria + paginaHeader->cantidad_contenidos + (config_memoria->TAMANIO_PAGINA) * paginaHeapHeader, &heapHeader);
    memcpy(tamanio_memoria + paginaHeader->cantidad_contenidos + (config_memoria->TAMANIO_PAGINA) * paginaHeapHeader, &heapHeader, sizeof(t_heap_metadata));
    return heapHeader;
}
int getPosicionPagina(t_pagina *pagina)
{
    return getPosicionEnLaPagina(pagina);
}
int getPrimeraPaginaDisponible(int size)
{
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int numeroPagina = -1;
    bool pagainaFueEncontrada = false;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada)
    {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        int a = (config_memoria->TAMANIO_PAGINA - paginaLeida->tamanio_ocupado - size);
        if (a >= 0)
        {
            pagainaFueEncontrada = true;
            numeroPagina = paginaLeida->numero_pagina;
        }
    }
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}

/*
void findAndSaveEnPagina(int pagina){
    //Busca en la tabla de paginas dicha pagina si esta mete ahi el contenido
    //Sino esta crea la pagina y guarda
}

*/

// Dada la direccion del contenido y su tipo, lo busco en memoria
t_heap_metadata* leerContenidoEnMemoria(uint32_t direccion, t_contenido tipoContenido) {

        t_heap_metadata* alloc = traerAllocDeMemoria(direccion);
            log_info(logger_memoria, "Traje alloc de memoria");
        //Hasta aca llegue, fijate que queres hacer con esto
        return alloc;

    // To be continued... cuando sepamos que otros contenidos guardamos

}

//Dada una direccion logica devuelvo la pagina que la contiene
t_pagina* buscarPagina(uint32_t direccion) {

    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);

    while(list_iterator_has_next(list_iterator)) {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        log_info(logger_memoria, "Encontre la pagina: %d",paginaLeida->numero_pagina);
        t_contenidos_pagina* primerContenido = list_get(paginaLeida->contenidos_pagina,0);
        log_info(logger_memoria, "Su primer contenido esta en: %d",primerContenido->dir_comienzo);
        
        if(primerContenido->dir_comienzo <= direccion && (primerContenido->dir_comienzo + config_memoria->TAMANIO_PAGINA) >= direccion) {
            free(primerContenido);
            list_iterator_destroy(list_iterator);
            return paginaLeida;
        }
    }

    //Si no encontro la pagina es porque no existe nada en la direccion que busca
    return -1;

}



//////////////////////////////////////////////////////////////////////////////

// Mem Read: Dada una direccion de memoria busco el contenido que se encuentra alli
t_heap_metadata* memRead(uint32_t direccion) {

    //TODO: Tener en cuenta que la direccion pertenezca a una pagina de la tabla de este proceso

    t_heap_metadata* alloc = traerAllocDeMemoria(direccion);

    log_info(logger_memoria, "Traje alloc de memoria");
    return alloc;

}