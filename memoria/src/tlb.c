#include "tlb.h"

int buscarEnTLB(int numero_pagina_buscada, int id){

    log_info(logger_memoria, "Entre a tlb");

    int marco = -1;

    if(list_size(tabla_tlb->tlb) == 0){
            
        t_tlb* primerElem = malloc(sizeof(primerElem));
        //Ver si logueo un miss
        log_info(logger_memoria,"TLB MISS: PID: %d PAGINA: %d",numero_pagina_buscada,id);
        sleep(config_memoria->RETARDO_FALLO_TLB);
        tabla_tlb->miss_totales ++;
        t_tabla_paginas *tabla = buscarTablaPorPID(id);
        tabla->miss ++;

        marco = buscarMarcoEnMemoria(numero_pagina_buscada, id);
        log_info(logger_memoria, "Encontre el marco %d",marco);
        primerElem->pid = id;
        primerElem->numero_marco = marco;
        primerElem->numero_pagina = numero_pagina_buscada;
        list_add(tabla_tlb->tlb,primerElem);
        return primerElem->numero_marco;
    }

    t_list_iterator *list_iterator = list_iterator_create(tabla_tlb->tlb);
    while (list_iterator_has_next(list_iterator))
    {
        t_tlb *tlb = list_iterator_next(list_iterator);
        if (tlb->numero_pagina == numero_pagina_buscada && tlb->pid == id)
        {
            marco = tlb->numero_marco;
            log_info(logger_memoria,"TLB HIT: PID: %d PAGINA: %d MARCO: %d",tlb->pid,tlb->numero_pagina,tlb->numero_marco);
            sleep(config_memoria->RETARDO_ACIERTO_TLB);
            tabla_tlb->hit_totales ++;
            t_tabla_paginas *tabla = buscarTablaPorPID(id);
            tabla->hit ++;
            if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_TLB, "LRU") == 0){
                reordenarLRU(tlb->numero_pagina, id);
            }

            list_iterator_destroy(list_iterator);
            return marco;
        }
    }

    //MISS
    marco = buscarMarcoEnMemoria(numero_pagina_buscada, id);

    log_info(logger_memoria,"TLB MISS: PID: %d PAGINA: %d",numero_pagina_buscada,id);
    sleep(config_memoria->RETARDO_FALLO_TLB);
    tabla_tlb->miss_totales ++;
    t_tabla_paginas *tabla = buscarTablaPorPID(id);
    tabla->miss ++;

    t_tlb* newElem = malloc(sizeof(t_tlb));

    newElem->pid = id;
    newElem->numero_pagina = numero_pagina_buscada;
    newElem->numero_marco = marco;

    if(list_size(tabla_tlb->tlb) < config_memoria->CANTIDAD_ENTRADAS_TLB){

        list_add(tabla_tlb->tlb,newElem);
        return newElem->numero_marco;
    }

    //TLB completo tengo que reemplazar
    t_tlb* old = list_get(tabla_tlb->tlb,0);
    log_info(logger_memoria,"Reemplazo TLB - VICTIMA - PID: %d PAGINA: %d MARCO: %d NUEVO - PID: %d PAGINA: %d MARCO: %d ",old->pid,old->numero_pagina,old->numero_marco,newElem->pid,newElem->numero_pagina,newElem->numero_marco);
    list_remove(tabla_tlb->tlb,0);
    list_add(tabla_tlb->tlb,newElem);

    return marco;

}

void reordenarLRU(int numero_pagina_buscada, int id) {

    //Busco y guardo el indice, hago un remove y agrego al final de la lista el mismo elemento
    int index = 0;

    t_list_iterator *list_iterator = list_iterator_create(tabla_tlb->tlb);
    while (list_iterator_has_next(list_iterator))
    {
        t_tlb *tlb = list_iterator_next(list_iterator);
        if (tlb->numero_pagina == numero_pagina_buscada && tlb->pid == id)
        {
            list_remove(tabla_tlb->tlb,index);
            list_add(tabla_tlb->tlb,tlb);
            list_iterator_destroy(list_iterator);
            return;
        }
        index++;
    }

    //Aca deberia haber un error porq si no lo encuentra paso algo
    list_iterator_destroy(list_iterator);
    return;
}