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


// Podria tener una lista de allocs que contenga por cada alloc la posicion y el tamaño
// Tengo el size, voy a la lista y la recorro, buscando un alloc que vaya con mi size, cuando lo encuentro, con la posicion voy a memoria y busco ese alloc

// Caso 1: encuentro alloc libre
// Cuando obtengo ese alloc, veo el isFree -> Si esta en true, lo pongo en false, y vuelvo a guardar en memoria, y genero un alloc nuevo y lo mando a memoria y guardo en lista
// Si esta en false, simplemente sigo iterando la lista de allocs

// Caso 2: lista vacia
// Si la lista esta vacia o termino de recorrerla y no encontre alloc, tengo que generar uno nuevo
// Primero valido si hay espacio, esto es... pregunto config tamaño memoria y me fijo tamaño en la lista de allocs, eso con el size de cada alloc guardado
// Si no tengo disponible, mando a swap
// Si tengo disponible creo el nuevo heap metadata, y lo mando a paginacion para que mati lo guarde



// Si quiero hacer memwrite

/**
t_heap_metadata* allocMem(int size) {


    t_list_iterator* list_iterator = list_iterator_create(allocs);
    while(list_iterator_has_next(list_iterator)) {
        t_alloc* alloc = list_iterator_next(list_iterator);
        if (alloc->size >= size) {
            t_heap_metadata* data = traerData(alloc->direccion);
            if(data->isFree == 1 && data->nextAlloc + 9 - alloc->direccion == 0){
                return data;
            }
            else if(data->isFree){
                t_heap_metadata* nuevoAlloc = newAllocs(size,alloc);
            }
            if(espacioDisponible(size)){
            t_heap_metadata* newAlloc = allocNew()
            }
        }
    }


}

t_heap_metadata* traerData(uint32_t direccion) {

    t_heap_metadata* data = malloc(sizeof(t_heap_metadata));
    int offset = posicion;

    memcpy(&data->prevAlloc,tamanio_memoria + offset,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->nextAlloc,tamanio_memoria + offset,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->isFree,tamanio_memoria + offset,sizeof(uint8_t));
    offset += sizeof(uint8_t);

    return data;

}

bool espacioDisponible(int size) {

    return config_memoria->TAMANIO - memoriaReservada() - 9 >= size; 

}

int memoriaReservada() {

    t_alloc* alloc = list_get(allocs,list_size(allocs) - 1);
    return alloc->posicion + 9 + alloc->size;

}

t_heap_metadata* allocNew(int size) {

    t_alloc* alloc = list_get(allocs,list_size(allocs) - 1);
    t_alloc* newAlloc = malloc(sizeof(t_alloc));

    newAlloc->size = size;
    newAlloc->direccion = alloc->direccion + 9 + alloc->size;

    list_add(allocs,newAlloc);
    free(alloc);

    //Cambiar ultimo alloc que tiene nextAlloc en null
    t_heap_metadata* data = allocWrite(newAlloc);

    return data;

}

t_heap_metadata* allocWrite(t_alloc newAlloc) {

    t_heap_metadata* data = malloc(sizeof(t_heap_metadata));
    data->prevAlloc = //alloc->direccion del ultimo de la lista - el agregado
    data->nextAlloc = NULL;
    data->isFree = 0;

    int offset = newAlloc->direccion;

    memcpy(tamanio_memoria + offset, &data->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(tamanio_memoria + offset, &data->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(tamanio_memoria + offset, &data->isFree, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    return data;

}

t_heap_metadata* newAllocs(int size, t_alloc* alloc) {

    if(alloc->size > size + 9) {
 
        t_alloc* newAlloc = malloc(sizeof(t_alloc));
        newAlloc->direccion = alloc->direccion + size;
        newAlloc->size = alloc->size - size - 9;
        alloc->size = size;

        //Funcion que busque y cambie los metadata, agrega el nuevo y cambia el viejo

    }






}

*/