#include "paginacion.h"

int getPaginaByDireccion(uint32_t direccion){
    uint32_t inicio = tamanio_memoria;
    uint32_t resta =  direccion - inicio;
    return resta/config_memoria->TAMANIO_PAGINA;
}
int getPrimeraPaginaDisponible(int size)
{
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int numeroPagina = -1;
    bool pagainaFueEncontrada = false;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada)
    {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        int a = (config_memoria->TAMANIO_PAGINA - paginaLeida->tamanio_ocupado);
        if (a > 0)
        {
            pagainaFueEncontrada = true;
            numeroPagina = paginaLeida->numero_pagina;
        }
    }
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}
t_contenidos_pagina *getLastContenidoByPagina(t_pagina* pagina){
     t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
     t_contenidos_pagina *contenido;
     while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
    }
    return contenido;
}
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina* pagina){
     t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
     t_contenidos_pagina *contenido;
     t_contenidos_pagina *contenidoFinal;
     while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
        if(contenido->contenido_pagina == HEADER){
            contenidoFinal = contenido;
        }
        
    }
    return contenidoFinal;
}
//////////////////////////////////////////////////////////////////////////////

// Mem Read: Dada una direccion de memoria busco el contenido que se encuentra alli
t_heap_metadata* memRead(uint32_t direccion) {

    //TODO: Tener en cuenta que la direccion pertenezca a una pagina de la tabla de este proceso

    t_heap_metadata* alloc = traerAllocDeMemoria(direccion);

    log_info(logger_memoria, "Traje alloc de memoria");
    return alloc;

}
int getMarco(){

    t_list_iterator *list_iterator = list_iterator_create(tabla_marcos->marcos);
    int numeroPagina = -1;
    bool marcoDisponible = false;
    while (list_iterator_has_next(list_iterator) && !marcoDisponible)
    {
        t_marco *marco = list_iterator_next(list_iterator);
        if (marco->isFree)
        {
            marcoDisponible = true;
            numeroPagina = marco->numero_marco;
        }
    }
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}
int generarPaginaConMarco(){
    
    if(tabla_paginas->paginas_en_memoria <=config_memoria->MARCOS_POR_CARPINCHO){
        return getMarco();
    }
    else{
        log_error(logger_memoria,"ERROR EL CARPINCHO NO PUEDE ASIGNAR MAS MARCOS EN MEMORIA");
        return -1;
    }
    
}