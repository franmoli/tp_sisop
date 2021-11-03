#include "alloc.h"

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

//Tampoco funciona la estructura administrativa, hay que traer pagina por pagina desde la primera y acceder a memoria para buscar los allocs

// Itero por la lista de paginas dentro de la tabla de pagina, veo como puedo ir a buscar los elementos de la pagina dentro de memoria, me traigo pagina por pagina y veo los allocs en orden
// onda se que los primeros 9 bytes de la pagina 0 son el primer heap metadata, me traigo eso y veo que onda, con tener el primer alloc ya se el nextalloc y puedo ir al siguiente

//Si el heap disponible tiene un size mayor al size que quiero meter, ese espacio sobrante tiene que ser mayor a 9 para poder armar otro heap, sino no sirve ese heap, se sigue buscando

/*
iterarPaginas(int size) {

    t_list_iterator* list_iterator = list_iterator_create(tabla_paginas->paginas);
    while(list_iterator_has_next(list_iterator)) {
        t_pagina* paginaLeida = list_iterator_next(list_iterator);

    }

}


t_heap_metadata* traerAllocDeMemoria(uint32_t) {

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

t_heap_metadata* obtenerPrimerAlloc() {

    t_pagina* primeraPagina = list_get(tabla_paginas->paginas,0);
    t_contenidos_pagina* primerContenido = list_get(primeraPagina->contenidos_pagina,0);
    if(primerContenido->tamanio != 9) {
        // Pense que era alloc
        return -1;
    }

    return traerAllocDeMemoria(primerContenido->dir_comienzo);

}

t_heap_metadata* crearPrimerAlloc(int size) {

    t_heap_metadata* newAlloc = malloc(sizeof(t_heap_metadata));
    newAlloc->isFree = 0;
    newAlloc->prevAlloc = NULL;
    newAlloc->nextAlloc = size + 9;

    return newAlloc;

}

t_heap_metadata* crearUltimoAlloc(uint32_t dirUltimoAlloc, int sizeUltimoAlloc) {

    // Cuando creo un alloc al final, tengo que cambiar el ultimo que tiene nextalloc en null,

    return 0;
}*/
t_heap_metadata *getLastHeapByPagina(t_pagina *pagina, int numeroPagina){
    t_heap_metadata *heap = generarHeapVacio();
    
    if(pagina->cantidad_contenidos == 0){
        return heap;
    }
    //Traigo primera paginita
    memcpy(&heap,tamanio_memoria + (config_memoria->TAMANIO_PAGINA) * numeroPagina, sizeof(t_heap_metadata));
    t_contenidos_pagina *contenidos = list_get(pagina->contenidos_pagina,0);
    int i = 0;
    while(contenidos->recorrido < pagina->cantidad_contenidos){
         if(getFromMemoriaHeap(heap,i, numeroPagina)){
              memcpy(&heap,heap->nextAlloc, sizeof(t_heap_metadata));
              return heap;
         }
          i++;
    }
}
bool getFromMemoriaHeap(t_heap_metadata *heap,int i, int numeroPagina){
    memcpy(&heap,heap->nextAlloc, sizeof(t_heap_metadata));
    if(heap->isFree == true){
        return true;
    }  
    return false;
}
t_heap_metadata *generarHeapVacio(){
    t_heap_metadata *heapHeader = malloc(sizeof(t_heap_metadata));
    heapHeader->isFree = true;
    heapHeader->nextAlloc = NULL;
    heapHeader->prevAlloc = NULL;
    return heapHeader;
}
t_heap_metadata *generarHeaderMetadataAlFinal(t_pagina *pagina,int numeroPagina)
{
    //FALTA RECORRER LA PAGINA Y QUEDARSE CON EL ULTIMO HEAP
    t_heap_metadata *heapHeader = getLastHeapByPagina(pagina,numeroPagina);
    heapHeader->isFree = false;
    heapHeader->nextAlloc = NULL;
    heapHeader->prevAlloc = NULL;
    return heapHeader;
}
t_heap_metadata *generarFooter(t_pagina *paginaHeader, int numeropaginaHeapHeader)
{
    t_heap_metadata *heapFooter = malloc(sizeof(t_heap_metadata));
    heapFooter->isFree = true;
    heapFooter->nextAlloc = NULL;
    heapFooter->prevAlloc = tamanio_memoria + paginaHeader->cantidad_contenidos + (config_memoria->TAMANIO_PAGINA) * numeropaginaHeapHeader;
    return heapFooter;
}
int getPosicionEnLaPagina(int pagina)
{
    return 0;
}
// El ultimo alloc que tiene nextalloc Null, es un alloc que nunca se reserva, debido a que tiene el tamaño de lo que falta para terminar la ultima pagina
// Cuando creo un alloc, tengo que achicar este para que existan los 2, y cuando el alloc que creo es mas grande que este, tengo que arrancar una nueva pagina
// y crear este alloc con lo que me sobre de la pagina.
