#include "memoria-virtual.h"

/*
t_pagina* reemplazarPagina(t_pagina* paginaAgregar, int carpincho_id){

    t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
        //SWAP CLOCK
    }
    else{
        //SWAP LRU
        log_info(logger_memoria,"AGREGADO A TABLA LRU PAGINA: %d", paginaAgregar->numero_pagina);
        int marco = eliminarPrimerElementoLista(carpincho_id);
        paginaAgregar->marco_asignado = marco;
        list_add(tabla_paginas->Lru,paginaAgregar);
    }
    return NULL;
}*/

int reemplazarPagina(int nro_pagina, int carpincho_id){
    t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
        //SWAP CLOCK
    }
    else{
        //SWAP LRU
        int marco = reemplazarLRU(nro_pagina,carpincho_id);
        return marco;
    }
}

//LRU saca la pagina que hace mas tiempo no se hace referencia
//Cuando hago memread o memwrite significa una referencia

int eliminarPrimerElementoLista(int carpincho_id){
    t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->Lru);
    bool primero = true;
    int marco = 0;
    t_list *paginas = list_create();
    while (list_iterator_has_next(list_iterator))
    {
        t_pagina *paginaList = list_iterator_next(list_iterator);
        if(!primero){
            list_add(paginas,paginaList);
        }
        marco = paginaList->marco_asignado;
        primero = false;
    }
    list_iterator_destroy(list_iterator);
    
    free(tabla_paginas->Lru);
    tabla_paginas->Lru = paginas;
    return marco;
}

int reemplazarLRU(int nro_pagina, int carpincho_id){

    t_reemplazo* paginaNueva = malloc(sizeof(t_reemplazo));
    paginaNueva->numero_pagina = nro_pagina;
    paginaNueva->pid = carpincho_id;
    int numeroMarco = -1;

    if(config_memoria->TIPO_ASIGNACION == "FIJA"){
        //LRU CON FIJA
        
    }else
    {
        //LRU CON GLOBAL
        t_reemplazo* paginaOld = malloc(sizeof(t_pagina));
        paginaOld = list_get(reemplazo_LRU,0);
        t_pagina* paginaVieja = getPaginaByNumero(paginaOld->numero_pagina,paginaOld->pid);

        numeroMarco = paginaVieja->marco_asignado;

        list_remove(reemplazo_LRU,0);
        list_add(reemplazo_LRU,paginaNueva);
        
    }
    return numeroMarco;

}


void consultaSwap(int carpincho_id) {
    t_paquete *paquete = serializar_consulta_swap(carpincho_id);
    //Enviar a swap y ver como esperar respuesta
    return;
}
