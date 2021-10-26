#include "alloc.h"


void allocMem(int size){
    if(list_size(allocs) == 0){

        //me fijo si hay memoria disponible
        bool memoria_disponible = memoriaDisponible(size);
        // reservo directamente el primer lugar
        if (memoria_disponible) {
            //genero alloc y reservo primer lugar
            t_heap_metadata* nuevo_alloc;
            nuevo_alloc = nuevoAlloc(size);

        }
    } else {

        t_heap_metadata* nuevo_alloc;
        nuevo_alloc = buscarAllocLibre(size);

    }

}

//Crea un nuevo alloc y lo agrega a la lista
t_heap_metadata* nuevoAlloc(int size){

    t_heap_metadata* nuevo_alloc = malloc(sizeof(t_heap_metadata));

    nuevo_alloc->isFree = false;
    //nuevo_alloc->prevAlloc = if list no tiene elementos, seria primera posicion de memoria, sino que sea el nextAlloc del ultimo alloc
    nuevo_alloc->nextAlloc = nuevo_alloc->prevAlloc + size;

    list_add(nuevo_alloc,allocs);
    return nuevo_alloc;
}

//Dado el tamaño total de la memoria y el total reservado, me fijo si queda espacio disponible para reservar size
bool memoriaDisponible(int size){

    int total_reservado = memoriaReservada();

    return config_memoria->TAMANIO - (total_reservado + 9) >= size;
}


//Guardo primer y ultimo alloc y comparo para obtener el size total reservado
int memoriaReservada(){

    t_heap_metadata* anterior = malloc(sizeof(t_heap_metadata));
    t_heap_metadata* alloc = malloc(sizeof(t_heap_metadata));
    alloc = list_get(allocs,0);

    t_list_iterator* list_iterator = list_iterator_create(allocs);

    while(alloc->nextAlloc != NULL) {
        anterior = alloc;
        alloc = list_iterator_next(list_iterator);
    }

    int memoria_reservada = malloc(sizeof(int));
    memoria_reservada = anterior->nextAlloc;

    list_iterator_destroy(list_iterator);
    free(anterior);
    free(alloc);

    return memoria_reservada;

}

//Busco el primer alloc libre en la lista (first fit)
t_heap_metadata* buscarAllocLibre(int size) {

    t_list_iterator* list_iterator = list_iterator_create(allocs);
    while(list_iterator_has_next(list_iterator)) {
        t_heap_metadata* alloc = list_iterator_next(list_iterator);
        if (alloc->isFree && (alloc->nextAlloc - alloc->prevAlloc >= size)) {
            list_iterator_destroy(list_iterator);
            return alloc;
        }
    }

    list_iterator_destroy(list_iterator);

    bool memoria_disponible = memoriaDisponible(size);
    // reservo directamente el primer lugar
        if (memoria_disponible) {
            //genero alloc y reservo primer lugar
            t_heap_metadata* nuevo_alloc;
            nuevo_alloc = nuevoAlloc(size);
            return nuevo_alloc;
        }

    //Si no puede encontrar alloc libre ni hay memoria disponible mando a swap

}