#include "alloc.h"


void alloc_mem(int size){
        if(list_size(allocs) == 0){

            //me fijo si hay memoria disponible
            bool memoria_disponible = memoria_disponible(size);
            // reservo directamente el primer lugar
            if (memoria_disponible) {
                //genero alloc y reservo primer lugar

            }
        }
        

    //buscar en la lista si hay un espacio disponible
        //si hay reservarlo y si sobra hacer otro alloc
        //si no hay cuando llego al final de la lista veo de agregarlo
            //si tengo memoria disponible lo agrego
            //si no tengo memoria hago un return -1

}

bool memoria_disponible(int size){

    //return total memoria - total reservado >= size
    //total memoria = archivo config
    //total reservado = ultimo alloc - primer alloc creo

    int total_reservado = memoria_reservada();

    //ver que valor tiene tamanio_memoria y si puedo restarlo
    return config_memoria->TAMANIO - total_reservado >= size;
}


int memoria_reservada(){

    t_list_iterator* list_iterator = list_iterator_create(allocs);
    while(list_iterator_has_next(list_iterator)) {
        //aca tendria que saltar o ir guardando en un contador para llegar al ultimo elemento y encontrar el alloc
        t_heap_metadata* alloc = list_iterator_next(list_iterator);
    }

    list_iterator_destroy(list_iterator);

    //comparo alloc (ultimo) con el primero y calculo la memoria reservada

    //return alloc->prevAlloc - primer_alloc->prevAlloc


}

    /*t_list_iterator* list_iterator = list_iterator_create(tabla_tlb->tlb);
    while(list_iterator_has_next(list_iterator)) {
        t_tlb* tlb = list_iterator_next(list_iterator);
        if(tlb->numero_marco == marco)
            return tlb->numero_pagina;
    }
    
    list_iterator_destroy(list_iterator);
    return 0;
*/



t_heap_metadata* buscar_alloc_libre(int size) {

    //itero la lista de allocs
        //por cada elemento me fijo primero si isFree == true
            //si es true me fijo si el tamaño es suficiente
            //return ese alloc, sino aca mismo puedo seguir con todo

        //si no se cumple sigo con la iteracion hasta el final


}