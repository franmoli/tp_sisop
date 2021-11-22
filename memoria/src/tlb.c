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


//Cuando hago un hit agrego el ttlb a la lista, si es fifo, agrego todo y saco el ultimo
//Si es lru tengo que fijarme de todos los que estan el que hace mas tiempo no haya sido buscado

void actualizarTLBFIFO(t_tlb *tlb){

    //Validar tamaño de la cola, si tiene menos elementos que los que deberia tener tengo que hacer push sin pop

    if(list_size(tabla_tlb->tlb) < entradas_tlb){
        list_add(tabla_tlb->tlb, tlb);
        queue_push(tlb_FIFO,tlb);
        return;
    }

    t_tlb *old = malloc(sizeof(t_tlb));

    old = queue_pop(tlb_FIFO);

    //Busco en mi tlb y elimino ese elemento para colocar el nuevo
    int index = 0;

    t_list_iterator *list_iterator = list_iterator_create(tabla_tlb->tlb);
    while (list_iterator_has_next(list_iterator))
    {
        t_tlb *tlbO = list_iterator_next(list_iterator);
        if (tlbO->numero_pagina == old->numero_pagina && tlbO->numero_marco == old->numero_marco)
        {
            tlbO = list_replace(tabla_tlb->tlb,index,tlb);
            free(tlbO);
        }

        index ++;
    }

    return;

}

int buscarEnTLB(int numero_pagina_buscada, int id){

    if(list_size(tabla_tlb->tlb)==0){
        //TLB vacio
        return 1000;
    }

    int marco = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla_tlb->tlb);
    bool marcoDisponible = false;
    while (list_iterator_has_next(list_iterator))
    {
        t_tlb *tlb = list_iterator_next(list_iterator);
        if (tlb->numero_pagina == numero_pagina_buscada && tlb->pid == id)
        {
            marco = tlb->numero_marco;
            sleep(config_memoria->RETARDO_ACIERTO_TLB);
            //Aca deberia hacer una entrada a la estructura de reemplazo
            list_iterator_destroy(list_iterator);
            return marco;

        }
    }

    //Hubo un miss
    //Busco en memoria la pagina y activo el algoritmo de reemplazo


}