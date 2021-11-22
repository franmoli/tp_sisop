#include "tlb.h"

int getTLBPaginaByMarco(int marco){
    if(list_size(tabla_tlb->tlb) == 0)
        return 0;

    t_list_iterator* list_iterator = list_iterator_create(tabla_tlb->tlb);
    while(list_iterator_has_next(list_iterator)) {
        t_tlb* tlb = list_iterator_next(list_iterator);
        if(tlb->numero_marco == marco)
            return tlb->numero_pagina;
    }
    
    list_iterator_destroy(list_iterator);
    return 0;
}
int getFromTLB(int numero_pagina_buscado, t_tabla_paginas *tabla_paginas){

    int numeroPagina = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla_tlb->tlb);
    bool marcoDisponible = false;
    while (list_iterator_has_next(list_iterator) && !marcoDisponible)
    {
        t_tlb *tlb = list_iterator_next(list_iterator);
        if (tlb->numero_pagina == numero_pagina_buscado)
        {
            marcoDisponible = true;
            numeroPagina = tlb->numero_pagina;

            sleep(config_memoria->RETARDO_ACIERTO_TLB);
        }
    }

    list_iterator = list_iterator_create(tabla_paginas->paginas);
    while (list_iterator_has_next(list_iterator) && !marcoDisponible)
    {
        t_pagina *pagina = list_iterator_next(list_iterator);
        if (pagina->numero_pagina == numero_pagina_buscado)
        {
            marcoDisponible = true;
            numeroPagina = pagina->numero_pagina;

            sleep(config_memoria->RETARDO_FALLO_TLB);
        }
    }

    list_iterator_destroy(list_iterator);

    return numeroPagina;
}
int asignarTlb(int pagina, int marco){
     if(list_size(tabla_tlb->tlb) <config_memoria->CANTIDAD_ENTRADAS_TLB){
         t_tlb *tlb = malloc(sizeof(t_tlb));
         tlb->numero_marco = marco;
         tlb->numero_pagina = pagina;
         list_add(tabla_tlb->tlb,tlb);
     }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int buscarEnTLB(int numero_pagina_buscada, int id){

    int marco = -1;

    if(list_size(tabla_tlb->tlb) == 0){
            
        t_tlb* primerElem = malloc(sizeof(primerElem));
        //Ver si logueo un miss
        sleep(config_memoria->RETARDO_FALLO_TLB);
        marco = buscarMarcoEnMemoria(numero_pagina_buscada, id);
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
            sleep(config_memoria->RETARDO_ACIERTO_TLB);

            if(config_memoria->ALGORITMO_REEMPLAZO_TLB == "LRU"){
                reordenarLRU(tlb->numero_pagina, id);
            }

            list_iterator_destroy(list_iterator);
            return marco;
        }
    }

    //MISS
    marco = buscarMarcoEnMemoria(numero_pagina_buscada, id);

    sleep(config_memoria->RETARDO_FALLO_TLB);

    t_tlb* newElem = malloc(sizeof(t_tlb));

    newElem->pid = id;
    newElem->numero_pagina = numero_pagina_buscada;
    newElem->numero_marco = marco;

    if(list_size(tabla_tlb->tlb) < config_memoria->CANTIDAD_ENTRADAS_TLB){

        list_add(tabla_tlb->tlb,newElem);
        return newElem->numero_marco;
    }

    //TLB completo tengo que reemplazar
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
    }

    //Aca deberia haber un error porq si no lo encuentra paso algo
    list_iterator_destroy(list_iterator);
    return;
}