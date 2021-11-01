#include "paginacion.h"

void guardarMemoria(t_paquete *paquete)
{

    int espacioAguardar = 9;

    //Reservo Header
    //Reservo Contenido
    //Reservo Footer
    //Agarro primera pagina disponible para los 9bits del heap_metadata
    int numeropaginaHeapHeader = getPrimeraPaginaDisponible(sizeof(t_heap_metadata));
    if (numeropaginaHeapHeader >= 0)
    {
        t_pagina *paginaHeader = list_get(tabla_paginas->paginas, numeropaginaHeapHeader);
        if (config_memoria->TAMANIO_PAGINA - paginaHeader->tamanio_ocupado - (sizeof(t_heap_metadata) * 2) >= espacioAguardar)
        {
            //en la misma pagina me entra el 100% del contenido.
            t_heap_metadata *heapHeader = guardarHeader(paginaHeader, numeropaginaHeapHeader);
            heapHeader->nextAlloc = tamanio_memoria + numeropaginaHeapHeader + (config_memoria->TAMANIO_PAGINA) * numeropaginaHeapHeader + espacioAguardar;
            t_heap_metadata *heapFooter = generarFooter(paginaHeader, numeropaginaHeapHeader);
            memcpy(heapHeader->nextAlloc , &heapFooter, sizeof(t_heap_metadata));

            return;
        }
        else
        {
            if (config_memoria->TAMANIO_PAGINA - paginaHeader->tamanio_ocupado - sizeof(t_heap_metadata) == 0)
            {
                //La pagina esta llena tengo que buscar otra.
                guardarHeader(paginaHeader, numeropaginaHeapHeader);
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
    t_heap_metadata *heapHeader = generarHeaderMetadataAlFinal(paginaHeader);
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



// Mem Read: Dada una direccion de memoria busco el contenido que se encuentra alli

void memRead(uint32_t direccion) {

    t_pagina* pagina = buscarPagina(direccion);


    t_list_iterator *list_iterator = list_iterator_create(pagina->contenidos_pagina);

    while(list_iterator_has_next(list_iterator)) {
        t_contenidos_pagina* contenido = list_iterator_next(list_iterator);

        if(contenido->dir_comienzo <= direccion && (contenido->dir_comienzo + tamanio) >= direccion) {

            free(pagina);
            list_iterator_destroy(list_iterator);

            leerContenidoEnMemoria(contenido->dir_comienzo, contenido->contenido_pagina);
        }

    }

}

// Dada la direccion del contenido y su tipo, lo busco en memoria
void leerContenidoEnMemoria(uint32_t direccion, t_contenido tipoContenido) {

    if(tipoContenido == ALLOC) {
        t_heap_metadata* alloc = traerAllocDeMemoria(direccion);
        //Hasta aca llegue, fijate que queres hacer con esto
    }

    // To be continued... cuando sepamos que otros contenidos guardamos

}

//Dada una direccion logica devuelvo la pagina que la contiene
t_pagina* buscarPagina(uint32_t direccion) {

    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);

    while(list_iterator_has_next(list_iterator)) {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        t_contenidos_pagina* primerContenido = list_get(paginaLeida->contenidos_pagina,0);
        if(primerContenido->dir_comienzo <= direccion && (primerContenido->dir_comienzo + config_memoria->TAMANIO_PAGINA) >= direccion) {
            free(primerContenido);
            list_iterator_destroy(list_iterator);
            return paginaLeida;
        }
    }

    //Si no encontro la pagina es porque no existe nada en la direccion que busca
    return -1;

}*/