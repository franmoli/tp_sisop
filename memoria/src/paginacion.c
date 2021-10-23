#include "paginacion.h"


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