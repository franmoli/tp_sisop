#include "paginacion.h"

int getPaginaByDireccionLogica(uint32_t direccion)
{
    if(direccion < 0)
        return 0;
        
    return direccion / config_memoria->TAMANIO_PAGINA;
}
int getPaginaByDireccionFisica(uint32_t direccion)
{
    uint32_t inicio = tamanio_memoria;
    uint32_t resta = direccion - inicio;
    return resta / config_memoria->TAMANIO_PAGINA;
}
int getPrimeraPaginaDisponible(int size, t_tabla_paginas *tabla_paginas)
{
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int numeroPagina = -1;
    bool pagainaFueEncontrada = false;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada)
    {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        int a = (config_memoria->TAMANIO_PAGINA - paginaLeida->tamanio_ocupado);
        if (a > 0)
        {
            pagainaFueEncontrada = true;
            numeroPagina = paginaLeida->numero_pagina;
        }
    }
    list_iterator_destroy(list_iterator);
    return numeroPagina;
}
t_contenidos_pagina *getLastContenidoByPagina(t_pagina *pagina)
{
    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    t_contenidos_pagina *contenido;
    while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
    }
    return contenido;
}
t_contenidos_pagina *getLastHeaderContenidoByPagina(t_pagina *pagina)
{
    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    t_contenidos_pagina *contenido;
    t_contenidos_pagina *contenidoFinal;
    while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
        if (contenido->contenido_pagina == HEADER)
        {
            contenidoFinal = contenido;
        }
    }
    return contenidoFinal;
}
//////////////////////////////////////////////////////////////////////////////

// Mem Read: Dada una direccion de memoria busco el contenido que se encuentra alli
char *memRead(t_paquete *paquete)
{

    /*1- Deserializo el paquete -> carpincho_id, direccion_logica
      2- Con la dir logica calculo la pagina y el desplazamiento -> pagina = dir logica / tamaño_pagina ... deplazamiento = resto
      3- Con la pagina y el pid busco en la tlb
        3b- Si ocurre un miss sumo metrica y voy a memoria
        3c- Si no esta en memoria voy a swap y cambio bits (referido)
        3d- Si estaba en tlb sumo metrica HIT
      4- Obtenido el marco, junto con el desplazamiento voy a memoria y busco la info

      typedef struct {
	uint32_t size_reservar; -> direccion logica
	uint32_t carpincho_id;
} t_malloc_serializado;
    */

   char*  contenido_escribir= NULL;
   int size = 0;
   int32_t direccion_logica ; 

   deserializar(paquete,6,CHAR_PTR,&contenido_escribir,INT,&direccion_logica,INT,&size);
   
   int inicio = tamanio_memoria;
   int numero_pagina = (direccion_logica - inicio) / config_memoria->TAMANIO_PAGINA;
   t_tabla_paginas* tabla_paginas = buscarTablaPorPID(socket_client);
   t_pagina* pagina = malloc(sizeof(t_pagina));

   if(numero_pagina > list_size(tabla_paginas)){
       return -1;
   }

   int marco = buscarEnTLB(numero_pagina,tabla_paginas->pid);
   if (marco == -1){
       pagina = list_get(tabla_paginas->paginas,numero_pagina);
       if(pagina->bit_presencia != 0){
            marco = buscarMarcoEnMemoria(numero_pagina,tabla_paginas->pid);
       }else
       {
           //TRAER DE SWAP
            marco = traerPaginaAMemoria(pagina);
            if(marco == -1){
                //No pude traer a memoria
                return -1;
            }
       }
   }

   agregarTLB(numero_pagina,marco,tabla_paginas->pid);
    
   int desplazamiento = (direccion_logica-inicio) % config_memoria->TAMANIO_PAGINA;

   char * contenido = traerDeMemoria(marco,desplazamiento, size);

    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
        pagina = list_get(tabla_paginas->paginas,numero_pagina);
        actualizarLRU(pagina);
    }else
    {
        pagina->bit_uso = 1;
    }

   return contenido;

}

char* traerDeMemoria(int marco, int desplazamiento, int size) {
    char* contenido = malloc(sizeof(char) * (size + 1));
    uint32_t dir_fisica = tamanio_memoria + marco * config_memoria->TAMANIO_PAGINA + desplazamiento;

    memcpy(contenido, dir_fisica, sizeof(char) * (size + 1));
    contenido = string_substring_until(contenido, size);

    return contenido;
}

int memWrite(t_paquete *paquete)
{
   char*  contenido_escribir= NULL;
   int size = 0;
   int32_t direccion_logica ; 

   deserializar(paquete,6,CHAR_PTR,&contenido_escribir,INT,&direccion_logica,INT,&size);
   
   int inicio = tamanio_memoria;
   int numero_pagina = (direccion_logica - inicio) / config_memoria->TAMANIO_PAGINA;
   t_tabla_paginas* tabla_paginas = buscarTablaPorPID(socket_client);

   if(numero_pagina > list_size(tabla_paginas)){
       return -1;
   }

   t_pagina* pagina = list_get(tabla_paginas->paginas,numero_pagina);
   if(pagina->bit_presencia == 0){
       //TRAER DE SWAP
       int marco = traerPaginaAMemoria(pagina);
       if(marco == -1){
           //No pude traer a memoria
           return -1;
       }
   }
    
   int desplazamiento = (direccion_logica-inicio) % config_memoria->TAMANIO_PAGINA;

   escribirEnMemoria(pagina->marco_asignado,desplazamiento, size, contenido_escribir);

   if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
        actualizarLRU(pagina);
    }else
    {
        pagina->bit_modificado = 1;
    }

   return 1;
}

void escribirEnMemoria(int marco, int desplazamiento, int size, char* contenido)
{
    uint32_t inicio = tamanio_memoria;
    uint32_t dir_fisica = inicio + marco * config_memoria->TAMANIO_PAGINA + desplazamiento;
    uint32_t offset = 0;
    log_info(logger_memoria,"Escribiendo en la direccion %d el contenido: %s.\n",dir_fisica,contenido);
    memcpy(dir_fisica,&contenido, size);
    log_info(logger_memoria,"Se termino de escribir en memoria.");
}

void* traerMarcoDeMemoria(t_pagina* pagina){

    int offset = 0;
    int size = 0;
    void* stream = malloc(config_memoria->TAMANIO_PAGINA);

    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    t_contenidos_pagina *contenido;
    while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
        size = contenido->dir_fin - contenido->dir_comienzo;

        if(contenido->contenido_pagina == HEAP || contenido->contenido_pagina == FOOTER){

            t_heap_metadata* heap = traerAllocIncompleto(pagina->marco_asignado,contenido->dir_comienzo,contenido->dir_fin);
            if(heap->prevAlloc != -1){
                memcpy(stream + offset, &(heap->prevAlloc),sizeof(uint32_t));
                offset += sizeof(uint32_t);
            }
            if(heap->nextAlloc != -1){
                memcpy(stream + offset, &(heap->nextAlloc),sizeof(uint32_t));
                offset += sizeof(uint32_t);
            }
            if(heap->isFree != -1){
                memcpy(stream + offset, &(heap->isFree),sizeof(bool));
                offset += sizeof(bool);
            }

        }else
        {
            void* contenido = traerDeMemoria(pagina->marco_asignado,offset,size);
            memcpy(stream + offset, contenido,size);
            offset += size;
        }
    }
    list_iterator_destroy(list_iterator);
    return stream;

}

t_heap_metadata* traerAllocIncompleto(int marco,uint32_t dir_comienzo, uint32_t dir_final){

    t_heap_metadata* heap = malloc(sizeof(t_heap_metadata));
    uint32_t inicio = tamanio_memoria;
    uint32_t direccion = marco * config_memoria->TAMANIO_PAGINA + inicio;
    int offset = (dir_comienzo - inicio) % config_memoria->TAMANIO_PAGINA;
    int size = dir_final - dir_comienzo;

    switch(size)
    {
        case 9:
            memcpy(&heap->prevAlloc, direccion + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&heap->nextAlloc, direccion + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&heap->isFree, direccion + offset, sizeof(bool));
            offset += sizeof(bool);
            break;
        case 5:
            heap->prevAlloc = -1;
            memcpy(&heap->nextAlloc, direccion + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&heap->isFree, direccion + offset, sizeof(bool));
            offset += sizeof(bool);
            break;
        case 1:
            heap->prevAlloc = -1;
            heap->nextAlloc = -1;
            memcpy(&heap->isFree, direccion + offset, sizeof(bool));
            offset += sizeof(bool);
            break;
        case 8:
            memcpy(&heap->prevAlloc, direccion + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&heap->nextAlloc, direccion + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            heap->isFree = -1;
            break;
        case 4:
            memcpy(&heap->prevAlloc, direccion + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            heap->nextAlloc = -1;
            heap->isFree = -1;
            break;
    }

    return heap;

}

void escribirMarcoEnMemoria(t_pagina* pagina, void* stream){

    int offset = 0;
    int size = 0;
    void* cont;

    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    t_contenidos_pagina *contenido;
    while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
        size = contenido->dir_fin - contenido->dir_comienzo;

        if(contenido->contenido_pagina == HEAP || contenido->contenido_pagina == FOOTER){
            escribirAllocIncompleto(pagina->marco_asignado,contenido->dir_comienzo,contenido->dir_fin,stream);
            offset += size;
        }else
        {
            //Ver si va bien
            cont = malloc(size);
            memcpy(cont,stream + offset, size + 1);
            escribirEnMemoria(pagina->marco_asignado,offset,size,cont);
            offset += size;
        }
    }
    list_iterator_destroy(list_iterator);
}

void escribirPaginaEnMemoria(t_pagina* pagina,t_pagina_swap* pagina_swap){

    int marco = pagina->marco_asignado;
    int offset = 0;
    int size = 0;

    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    t_contenidos_pagina *contenido;

    while (list_iterator_has_next(list_iterator))
    {
        contenido = list_iterator_next(list_iterator);
        size = contenido->dir_fin - contenido->dir_comienzo;

        if(contenido->contenido_pagina == HEAP || contenido->contenido_pagina == FOOTER){
            escribirAllocEnMarco(pagina->marco_asignado,contenido->dir_comienzo,contenido->dir_fin,list_get(pagina_swap->contenido_heap_info,0));
            list_remove(pagina_swap->contenido_heap_info,0);
            offset += size;
        }else
        {
            //Ver si va bien
            escribirEnMemoria(pagina->marco_asignado,offset,size,list_get(pagina_swap->contenido_carpincho_info,0));
            list_remove(pagina_swap->contenido_carpincho_info,0);
            offset += size;
        }
    }
    list_iterator_destroy(list_iterator);
    return;
}

void escribirAllocEnMarco(int marco,uint32_t dir_comienzo,uint32_t dir_fin,t_heap_metadata* heap){

    int size = dir_fin - dir_comienzo;
    uint32_t inicio = tamanio_memoria;
    uint32_t direccion = marco * config_memoria->TAMANIO_PAGINA + inicio;
    int offset = (dir_comienzo - inicio) % config_memoria->TAMANIO_PAGINA;

    switch(size)
    {
        case 9:
            memcpy(direccion + offset, &(heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(direccion + offset, &(heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(direccion + offset, &(heap->isFree), sizeof(bool));
            offset += sizeof(bool);
            break;
        case 5:
            memcpy(direccion + offset, &(heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(direccion + offset, &(heap->isFree), sizeof(bool));
            offset += sizeof(bool);
            break;
        case 1:
            memcpy(direccion + offset, &(heap->isFree), sizeof(bool));
            offset += sizeof(bool);
            break;
        case 8:
            memcpy(direccion + offset, &(heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(direccion + offset, &(heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            break;
        case 4:
            memcpy(direccion + offset, &(heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            break;
    }

}

void escribirAllocIncompleto(int marco,uint32_t dir_comienzo,uint32_t dir_fin,void *stream){

    int size = dir_fin - dir_comienzo;
    uint32_t inicio = tamanio_memoria;
    uint32_t direccion = marco * config_memoria->TAMANIO_PAGINA + inicio;
    int offset = (dir_comienzo - inicio) % config_memoria->TAMANIO_PAGINA;
    t_heap_metadata* heap = malloc(sizeof(t_heap_metadata));

    switch(size)
    {
        case 9:
            memcpy(&(heap->prevAlloc),stream + offset, sizeof(uint32_t));
            memcpy(direccion + offset, &(heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&(heap->nextAlloc), stream + offset, sizeof(uint32_t));
            memcpy(direccion + offset, &(heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&(heap->isFree), stream + offset, sizeof(bool));
            memcpy(direccion + offset, &(heap->isFree), sizeof(bool));
            offset += sizeof(bool);
            break;
        case 5:
            memcpy(&(heap->nextAlloc), stream + offset, sizeof(uint32_t));
            memcpy(direccion + offset, &(heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&(heap->isFree), stream + offset, sizeof(bool));
            memcpy(direccion + offset, &(heap->isFree), sizeof(bool));
            offset += sizeof(bool);
            break;
        case 1:
            memcpy(&(heap->isFree), stream + offset, sizeof(bool));
            memcpy(direccion + offset, &(heap->isFree), sizeof(bool));
            offset += sizeof(bool);
            break;
        case 8:
            memcpy(&(heap->prevAlloc),stream + offset, sizeof(uint32_t));
            memcpy(direccion + offset, &(heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&(heap->nextAlloc), stream + offset, sizeof(uint32_t));
            memcpy(direccion + offset, &(heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            break;
        case 4:
            memcpy(&(heap->prevAlloc),stream + offset, sizeof(uint32_t));
            memcpy(direccion + offset, &(heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            break;
    }

}

int getMarco(t_tabla_paginas* tabla_paginas){
    int numeroMarco = -1;
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "DINAMICA") == 0){
        t_list_iterator *list_iterator = list_iterator_create(tabla_marcos_memoria->marcos);
        bool marcoDisponible = false;
        while (list_iterator_has_next(list_iterator) && !marcoDisponible)
        {
            t_marco *marco = list_iterator_next(list_iterator);
            if (marco->isFree)
            {
                marcoDisponible = true;
                numeroMarco = marco->numero_marco;
                list_iterator_destroy(list_iterator);
                return numeroMarco;
            }
        }
        
    }else
    {
        if(tabla_paginas->paginas_en_memoria <=config_memoria->MARCOS_POR_CARPINCHO){
            //Asigno una nueva pagina al marco correspondiente
            int numero = getIndexByPid(tabla_paginas->pid);
            numeroMarco =  numero * config_memoria->MARCOS_POR_CARPINCHO + tabla_paginas->paginas_en_memoria;
            return numeroMarco;
        }

    }

    return numeroMarco;

}
int getIndexByPid(int pid){
    t_list_iterator *list_iterator = list_iterator_create(tabla_procesos);
    int i = 0;
    int indice = 0;
     while (list_iterator_has_next(list_iterator))
        {
            t_tabla_paginas* tabla = list_iterator_next(list_iterator);
            if(tabla->pid == pid){
                indice = i;
            }
            i++;
        }
    list_iterator_destroy(list_iterator);
    return indice;
}
int getMarcoParaPagina(t_tabla_paginas* tabla_paginas){
    
    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
        if(tabla_paginas->paginas_en_memoria <=config_memoria->MARCOS_POR_CARPINCHO){
            return getMarco(tabla_paginas);
        }else{

        }
    }
    else
    {
        log_error(logger_memoria, "ERROR EL CARPINCHO NO PUEDE ASIGNAR MAS MARCOS EN MEMORIA");
        //reemplazarPagina();
        return -1;
    }
}

void mostrarPaginas(t_tabla_paginas *tabla_paginas)
{
    system("clear");
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->paginas);
    int i = 0;
    while (list_iterator_has_next(list_iterator))
    {
        t_pagina *paginaLeida = list_iterator_next(list_iterator);
        printf("Entrada: %d PAGINA nro: %d. Marco Asignado:%d Tam Ocupado:%d \n", i, paginaLeida->numero_pagina, paginaLeida->marco_asignado, paginaLeida->tamanio_ocupado);
        i++;
    }
    list_iterator_destroy(list_iterator);
}

t_tabla_paginas *buscarTablaPorPID(int id)
{

    if (list_size(tabla_procesos) == 0)
    {
        //No existen procesos
        return 1000;
    }

    int numeroTabla = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla_procesos);

    while (list_iterator_has_next(list_iterator))
    {

        t_tabla_paginas *tabla = list_iterator_next(list_iterator);
        if (tabla->pid == id)
        {
            list_iterator_destroy(list_iterator);
            return tabla;
        }
    }

    //No se encontro la tabla --> me pego un tiro
    log_info(logger_memoria, "No se encontro la tabla perteneciente al proceso");
    list_iterator_destroy(list_iterator);
    return 1000;
}

int buscarMarcoEnMemoria(int numero_pagina_buscada, int id)
{

    //Manejar error si no existe tabla
    t_tabla_paginas *tabla = buscarTablaPorPID(id);

    int numeroPagina = -1;
    t_list_iterator *list_iterator = list_iterator_create(tabla->paginas);

    while (list_iterator_has_next(list_iterator))
    {

        t_pagina *pagina = list_iterator_next(list_iterator);
        if (pagina->numero_pagina == numero_pagina_buscada)
        {
            list_iterator_destroy(list_iterator);
            return pagina->marco_asignado;
        }
    }

    return -1;
}

int solicitarPaginaNueva(uint32_t carpincho_id)
{

    bool trajeDeMemoria = false;

    //Para agregar la pagina nueva primero tengo que ver si puedo agregarla si es asignacion fija o dinamica
    int marco = getMarco(tabla_paginas);   
    if(marco < -1){
        log_error(logger_memoria,"No se pudo asignar un marco");
        return;
    }if (marco > -1){
        trajeDeMemoria = true;
    }
    if(marco == -1){
        log_info(logger_memoria, "Tengo que ir a swap");
        marco = reemplazarPagina(tabla_paginas);
        if(marco < 0)
            return -1;
    }
    int numero_pagina = 0;
    if (list_size(tabla_paginas->paginas) > 0)
        numero_pagina = list_size(tabla_paginas->paginas);

    t_pagina *pagina = malloc(sizeof(t_pagina));
    pagina->listado_de_contenido = list_create();
    pagina->marco_asignado = marco;
    pagina->numero_pagina = numero_pagina;
    pagina->tamanio_ocupado = 0;
    pagina->carpincho_id = carpincho_id;
    pagina->cantidad_contenidos = 0;
    pagina->bit_presencia = true;
    if (strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0) //SOLO CLOCK USA ESTE CAMPO
        pagina->bit_uso = true;

    list_add(tabla_paginas->paginas, pagina);

    t_marco *marcoAsignado = list_get(tabla_marcos_memoria->marcos, marco);
    marcoAsignado->isFree = false;
    tabla_paginas->paginas_en_memoria += 1;

    if(trajeDeMemoria == true){
        agregarAsignacion(pagina);
    }else{
        if (strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
            agregarAsignacion(pagina); 
        }else{
            //Resolver reemplazo Clock
            replaceClock(pagina);
        }
           
    }
    
    return pagina->numero_pagina;
}
t_contenidos_pagina *getContenidoPaginaByTipo(t_contenidos_pagina *contenidos, t_contenido tipo)
{
    t_list_iterator *list_iterator = list_iterator_create(contenidos);
    t_contenidos_pagina *contenidoBuscado;
    bool pagainaFueEncontrada = false;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada)
    {
        t_contenidos_pagina *contenido = list_iterator_next(list_iterator);
        if (contenido->contenido_pagina == tipo)
        {
            pagainaFueEncontrada = true;
            contenidoBuscado = contenido;
        }
    }
    list_iterator_destroy(list_iterator);
    return contenidoBuscado;
}
t_contenidos_pagina *getContenidoPaginaByTipoAndSize(t_contenidos_pagina *contenidos, t_contenido tipo, uint32_t nextAlloc)
{
    t_list_iterator *list_iterator = list_iterator_create(contenidos);
    t_contenidos_pagina *contenidoBuscado;
    bool pagainaFueEncontrada = false;
    uint32_t inicio = tamanio_memoria;
    while (list_iterator_has_next(list_iterator) && !pagainaFueEncontrada)
    {
        t_contenidos_pagina *contenido = list_iterator_next(list_iterator);
        if (contenido->contenido_pagina == tipo && (contenido->dir_fin - inicio)==nextAlloc )
        {
            pagainaFueEncontrada = true;
            contenidoBuscado = contenido;
        }
    }
    list_iterator_destroy(list_iterator);
    return contenidoBuscado;
}
t_pagina *getPaginaByNumero(int nro_pagina, int carpincho_id){
    t_pagina *pagina = malloc(sizeof(t_pagina));
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);

    pagina = list_get(tabla_paginas->paginas,nro_pagina);
    return pagina;

}
void liberarPagina(t_pagina* pagina, uint32_t carpincho_id){
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_marco *marcoAsignado = list_get(tabla_marcos_memoria->marcos, pagina->marco_asignado);
    marcoAsignado->isFree = true;

    list_remove(tabla_paginas->paginas,pagina->numero_pagina);
    tabla_paginas->paginas_en_memoria-=1;
}

int traerPaginaAMemoria(t_pagina* pagina_alloc_actual){

    t_tabla_paginas* tabla = buscarTablaPorPID(pagina_alloc_actual->carpincho_id);

    // Busco pagina que esta en swap, puede ser que haya liberado espacio y que ahora tenga un marco libre en memoria
    int marco = -1;
    int trajeDeMemoria = false;
    marco = getMarco(tabla);
    if(marco < -1){
        log_error(logger_memoria,"No se pudo asignar un marco");
        return;
    }if (marco > -1){
        trajeDeMemoria = true;
    }
    if(marco == -1){
        log_info(logger_memoria, "Tengo que ir a swap");
        marco = reemplazarPagina(tabla_paginas);
        if(marco < 0)
            return -1;
        int resultado = recibirPaginaSwap(pagina_alloc_actual);
        if(resultado == -1){
            //Swap no trajo nada
            //Capaz tengo que mandar devuelta la pagina anterior
            return -1;
        }
    }

    pagina_alloc_actual->marco_asignado = marco;
    pagina_alloc_actual->bit_presencia = 1;
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0 || trajeDeMemoria){
        agregarAsignacion(pagina_alloc_actual);
    }else
    {
        replaceClock(pagina_alloc_actual);
    }

    return pagina_alloc_actual->marco_asignado;
}

int getPosicionEnTablaDeProcesos(t_tabla_paginas* tabla){

    int index = 0;

    t_list_iterator* list_iterator = list_iterator_create(tabla_procesos);

    while(list_iterator_has_next(list_iterator)){

        t_tabla_paginas* tablaIterada = list_iterator_next(list_iterator);
        if(tabla->pid == tablaIterada->pid){
            list_iterator_destroy(list_iterator);
            return index;
        }
        index++;
    }

    list_iterator_destroy(list_iterator);
    //La tabla no esta cargada en memoria -> No va a pasar nunca creo
    return -1;

}