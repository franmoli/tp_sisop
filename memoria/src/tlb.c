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
int getFromTLB(int numero_pagina_buscado){

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