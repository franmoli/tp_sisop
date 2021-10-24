#include "alloc.h"


void alloc_mem(int size){
    if(list_size(allocs) == 0){

        //me fijo si hay memoria disponible
        bool memoria_disponible = memoria_disponible(size);
        // reservo directamente el primer lugar
        if (memoria_disponible) {
            //genero alloc y reservo primer lugar
            t_heap_metadata* nuevo_alloc;
             nuevo_alloc = nuevo_alloc(size);

        }
    } else {

        t_heap_metadata* nuevo_alloc;
        nuevo_alloc = buscar_alloc_libre(size);

    }

}

//Crea un nuevo alloc y lo agrega a la lista
t_heap_metadata* nuevo_alloc(int size){

    t_heap_metadata* nuevo_alloc = malloc(sizeof(t_heap_metadata));

    nuevo_alloc->isFree = false;
    //nuevo_alloc->prevAlloc = if list no tiene elementos, seria primera posicion de memoria, sino que sea el nextAlloc del ultimo alloc
    nuevo_alloc->nextAlloc = nuevo_alloc->prevAlloc + size;

    list_add(nuevo_alloc,allocs);
    return nuevo_alloc;
}

//Dado el tamaño total de la memoria y el total reservado, me fijo si queda espacio disponible para reservar size
bool memoria_disponible(int size){

    int total_reservado = memoria_reservada();

    return config_memoria->TAMANIO - total_reservado >= size;
}


//Guardo primer y ultimo alloc y comparo para obtener el size total reservado
int memoria_reservada(){

    int cont = 0;

    t_list_iterator* list_iterator = list_iterator_create(allocs);
    while(list_iterator_has_next(list_iterator)) {
        if (cont == 0) {
            t_heap_metadata* first_alloc = allocs
        }
        t_heap_metadata* alloc = list_iterator_next(list_iterator);
        cont ++;
    }

    int memoria_reservada = alloc->nextAlloc - allocs->prevAlloc

    list_iterator_destroy(list_iterator);

    return memoria_reservada

}

//Busco el primer alloc libre en la lista (first fit)
t_heap_metadata* buscar_alloc_libre(int size) {

    t_list_iterator* list_iterator = list_iterator_create(allocs);
    while(list_iterator_has_next(list_iterator)) {
        t_heap_metadata* alloc = list_iterator_next(list_iterator);
        if (alloc->isFree && (alloc->nextAlloc - alloc->prevAlloc >= size)) {
            list_iterator_destroy(list_iterator);
            return alloc;
        }
    }

    list_iterator_destroy(list_iterator);

    bool memoria_disponible = memoria_disponible(size);
    // reservo directamente el primer lugar
        if (memoria_disponible) {
            //genero alloc y reservo primer lugar
            t_heap_metadata* nuevo_alloc;
            nuevo_alloc = nuevo_alloc(size);
            return nuevo_alloc;
        }

    //Si no puede encontrar alloc libre ni hay memoria disponible mando a swap

}