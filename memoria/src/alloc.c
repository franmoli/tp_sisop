#include "alloc.h"

void freeAlloc(t_paquete *paquete)
{
   uint32_t inicio = tamanio_memoria;

    t_malloc_serializado *mallocDeserializado = deserializar_alloc(paquete);

    uint32_t direccion = mallocDeserializado->size_reservar;
    uint32_t carpincho_id = mallocDeserializado->carpincho_id;

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);

    if (!direccionValida(direccion, carpincho_id))
    {
        //MATE FREE FAIÑT
        log_error(logger_memoria,"Direccion invalida");
        return;
    }
    int nropaginaAllocActual = getPaginaByDireccionLogica(direccion);
    t_pagina* pagina_alloc_actual = list_get(tabla_paginas->paginas, nropaginaAllocActual);
    //Traigo de memoria el alloc
    int direccion_fisica_alloc = inicio + direccion + config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado;
    
    t_heap_metadata *alloc = traerAllocDeMemoria(direccion_fisica_alloc );
    alloc->isFree = true;
    uint32_t next = alloc->nextAlloc;
    uint32_t back = alloc->prevAlloc;

    int direccion_fisica_back = 0;
    int direccion_fisica_next = 0;
    t_pagina *pagina_alloc_anterior = malloc(sizeof(t_pagina));
    t_pagina *pagina_alloc_siguiente = malloc(sizeof(t_pagina));

    t_heap_metadata *anterior;
    t_heap_metadata *posterior;
    bool anterior_free = false;
    bool posterior_free = false;

    if (back != direccion)
    { // SI EXISTE ANTERIOR TRAERLO
        int nropaginaAllocAnterior = getPaginaByDireccionLogica(back);
        pagina_alloc_anterior = list_get(tabla_paginas->paginas, nropaginaAllocAnterior);
        
        direccion_fisica_back = inicio +back + pagina_alloc_siguiente->marco_asignado * config_memoria->TAMANIO_PAGINA;
        anterior = traerAllocDeMemoria(direccion_fisica_back);
        if (anterior->isFree)
            anterior_free = true;
    }
    if (next != 0)
    { //SI EXISTE PROXIMO
        int nropaginaAllocSiguiente = getPaginaByDireccionLogica(next);
        pagina_alloc_siguiente = list_get(tabla_paginas->paginas, nropaginaAllocSiguiente);
        direccion_fisica_next = inicio +next + pagina_alloc_siguiente->marco_asignado * config_memoria->TAMANIO_PAGINA;
        posterior = traerAllocDeMemoria(direccion_fisica_next);
        if (posterior->isFree)
            posterior_free = true;
    }
    bool alloc_fue_liberado = false;
    if (anterior_free) //PROBADO
    {
        int tamanio_original_eliminar = next - sizeof(t_heap_metadata) - direccion;
         if(pagina_alloc_anterior->numero_pagina == pagina_alloc_actual->numero_pagina){

             if(next!=0){
                //HAY PROXIMO NO VACIO
                if(pagina_alloc_siguiente->numero_pagina == pagina_alloc_actual->numero_pagina){
                    //ESTA TODO EN EL MISMO ALLOC
                    int resta = next - sizeof(t_heap_metadata) - direccion;
                    pagina_alloc_actual->tamanio_ocupado -= resta;
                    pagina_alloc_actual->cantidad_contenidos-=1;
                    int direccion_fisica = inicio + direccion+ config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado;
                    guardarAlloc(alloc,direccion_fisica);
                    eliminarcontenidoBydireccion(direccion, pagina_alloc_actual);
                    eliminarcontenidoBydireccion(direccion + sizeof(t_heap_metadata), pagina_alloc_actual);
                }else{
                    //NO ESTA TODO EN LA MISMA PAGINA
                    int tamanio = config_memoria->TAMANIO_PAGINA - sizeof(t_heap_metadata) - direccion;
                    pagina_alloc_actual->tamanio_ocupado-=tamanio;
                    pagina_alloc_actual->cantidad_contenidos-=1;
                    eliminarcontenidoBydireccion(direccion + sizeof(t_heap_metadata), pagina_alloc_actual);
                    int resto = next - config_memoria->TAMANIO_PAGINA * pagina_alloc_siguiente->numero_pagina;
                    pagina_alloc_siguiente->tamanio_ocupado-= resto;
                    pagina_alloc_siguiente->cantidad_contenidos-=1;
                    eliminarcontenidoBydireccion(0, pagina_alloc_siguiente);
                }
                anterior->nextAlloc = alloc->nextAlloc;
                posterior->prevAlloc = alloc->prevAlloc;
                guardarAlloc(anterior,back + inicio + config_memoria->TAMANIO_PAGINA * pagina_alloc_anterior->marco_asignado);
                guardarAlloc(posterior,next + inicio + config_memoria->TAMANIO_PAGINA * pagina_alloc_siguiente->marco_asignado);
                pagina_alloc_actual->tamanio_ocupado-= sizeof(t_heap_metadata);
                pagina_alloc_actual->cantidad_contenidos-=1;
                eliminarcontenidoBydireccion(direccion, pagina_alloc_actual);
                free(alloc);
                return;
             }else{
                //ESTOY ULTIMO ALLOC
                pagina_alloc_actual->tamanio_ocupado-= tamanio_original_eliminar - sizeof(t_heap_metadata);
                pagina_alloc_actual->cantidad_contenidos-=2;
                anterior->nextAlloc = alloc->nextAlloc;

                guardarAlloc(anterior,inicio + back + config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado);
                free(alloc);
                eliminarcontenidoBydireccion(direccion, pagina_alloc_actual);
             }
            return;
        }
        
        anterior->nextAlloc = alloc->nextAlloc;
        if(next != NULL)
            posterior->prevAlloc =alloc->prevAlloc;

        guardarAlloc(anterior,direccion_fisica_back);
        guardarAlloc(posterior,direccion_fisica_next);

        pagina_alloc_actual->tamanio_ocupado-=tamanio_original_eliminar;
        pagina_alloc_actual->cantidad_contenidos-=1;
        eliminarcontenidoBydireccion(direccion +sizeof(t_heap_metadata), pagina_alloc_actual);
        free(alloc);
        alloc_fue_liberado = true;
    }

    if (posterior_free)
    {
        if(pagina_alloc_actual->numero_pagina == pagina_alloc_siguiente->numero_pagina){
            if(alloc_fue_liberado){
                anterior->nextAlloc = posterior->nextAlloc;
                guardarAlloc(anterior,direccion_fisica_back);
                eliminarcontenidoBydireccion(next,pagina_alloc_siguiente);
                pagina_alloc_siguiente->tamanio_ocupado-=sizeof(t_heap_metadata);
                pagina_alloc_siguiente->cantidad_contenidos-=1;
                free(posterior);
                return;
            }
            else{
                    if( back!= 0 && direccion!=0){ //HAY UNO ANTERIOR NO LIBRE
                    
                    alloc->isFree = true;
                    alloc->nextAlloc = posterior->nextAlloc;
        
                    int resta = next - sizeof(t_heap_metadata) - direccion;
                    pagina_alloc_actual->tamanio_ocupado -= resta;
                    pagina_alloc_actual->cantidad_contenidos-=1;
                    
                    guardarAlloc(alloc,direccion_fisica_alloc);
                    eliminarcontenidoBydireccion(posterior, pagina_alloc_actual);
                    eliminarcontenidoBydireccion(direccion + sizeof(t_heap_metadata), pagina_alloc_actual);
                    free(posterior);
                    pagina_alloc_siguiente->cantidad_contenidos-=1;
                    pagina_alloc_siguiente->tamanio_ocupado-=sizeof(t_heap_metadata);
                    return;
                }else{
                    //SOY EL PRIMER ALLOC
                    alloc->isFree = true;
                    alloc->nextAlloc = posterior->nextAlloc;

                    int resta = next - sizeof(t_heap_metadata) - direccion;
                    pagina_alloc_actual->tamanio_ocupado -= resta;
                    pagina_alloc_actual->cantidad_contenidos-=1;
                    int direccion_fisica = inicio + direccion+ config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado;
                    guardarAlloc(alloc,direccion_fisica);
                    eliminarcontenidoBydireccion(posterior, pagina_alloc_actual);
                    eliminarcontenidoBydireccion(direccion + sizeof(t_heap_metadata), pagina_alloc_actual);
                    free(posterior);
                    pagina_alloc_siguiente->cantidad_contenidos-=1;
                    pagina_alloc_siguiente->tamanio_ocupado-=sizeof(t_heap_metadata);
                }
            }
            return;
        }
        else{
            
            alloc->nextAlloc = posterior->nextAlloc;
            guardarAlloc(alloc,direccion_fisica_alloc);
            int tamanio = config_memoria->TAMANIO_PAGINA - sizeof(t_heap_metadata) - direccion;
            pagina_alloc_actual->tamanio_ocupado-=tamanio;
            pagina_alloc_actual->cantidad_contenidos-=1;
            eliminarcontenidoBydireccion(direccion+ sizeof(t_heap_metadata), pagina_alloc_actual);
            
            int resto = next - config_memoria->TAMANIO_PAGINA * pagina_alloc_siguiente->numero_pagina;
            pagina_alloc_siguiente->tamanio_ocupado-= resto;
            pagina_alloc_siguiente->cantidad_contenidos-=1;
            eliminarcontenidoBydireccion(0, pagina_alloc_siguiente);
            
            free(posterior);
            pagina_alloc_siguiente->cantidad_contenidos-=1;
            pagina_alloc_siguiente->tamanio_ocupado-=sizeof(t_heap_metadata);
            
            if(pagina_alloc_siguiente->cantidad_contenidos== 0)
                liberarPagina(pagina_alloc_actual,carpincho_id);
            
            return;
        }
       
    }

    int posiblepagina = next / config_memoria->TAMANIO_PAGINA;
    if(posiblepagina == pagina_alloc_actual->numero_pagina){
        //ESTA TODO EN EL MISMO ALLOC
        int resta = next - sizeof(t_heap_metadata) - direccion;
        pagina_alloc_actual->tamanio_ocupado -= resta;
        alloc->isFree;
        int direccion_fisica = inicio + direccion+ config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado;
        guardarAlloc(alloc,direccion_fisica);
        eliminarcontenidoBydireccion(direccion + sizeof(t_heap_metadata), pagina_alloc_actual);
    }else{
        //NO ESTA TODO EN LA MISMA PAGINA
        int tamanio = config_memoria->TAMANIO_PAGINA - sizeof(t_heap_metadata) - direccion;
        pagina_alloc_actual->tamanio_ocupado-=tamanio;

        pagina_alloc_actual->cantidad_contenidos-=1;

        eliminarcontenidoBydireccion(direccion, pagina_alloc_actual);
        int resto = next - config_memoria->TAMANIO_PAGINA * pagina_alloc_siguiente->numero_pagina;
        pagina_alloc_siguiente->tamanio_ocupado-= resto;
        pagina_alloc_siguiente->cantidad_contenidos-=1;
        eliminarcontenidoBydireccion(0, pagina_alloc_siguiente);

        guardarAlloc(alloc, inicio + direccion + config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado);
    }
}
void eliminarcontenidoBydireccion(uint32_t direccion,t_pagina* pagina){
    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    uint32_t inicio = tamanio_memoria;
    t_list* nuevos = list_create();
    t_contenidos_pagina *contenido_borrar;
    bool encontrado = false;
    while (list_iterator_has_next(list_iterator))
    {
        t_contenidos_pagina *contenido_actual = list_iterator_next(list_iterator);
        int direccion_fisica = inicio + direccion + config_memoria->TAMANIO_PAGINA * pagina->numero_pagina;
        if(contenido_actual->dir_comienzo == direccion_fisica){
            contenido_borrar = contenido_actual;
            encontrado = true;
        }else{
            list_add(nuevos,contenido_actual);
        }
    }
    if(list_size(nuevos) > 0 || pagina->cantidad_contenidos == 0){
        if(encontrado)
            free(contenido_borrar);
            
        pagina->listado_de_contenido = nuevos;
    }
    
     
     list_iterator_destroy(list_iterator);
}
bool direccionValida(uint32_t direccion, uint32_t carpincho_id)
{

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);

    bool esValida = true;
    int numero_pagina = getPaginaByDireccionLogica(direccion);
    log_info(logger_memoria, "Pagina:%d", numero_pagina);
    t_pagina *pagina = list_get(tabla_paginas->paginas, numero_pagina);
    if (!pagina->bit_presencia)
        esValida = false;

    return esValida;
}
t_heap_metadata *traerAllocDeMemoria(uint32_t direccion)
{

    t_heap_metadata *data = malloc(sizeof(t_heap_metadata));

    uint32_t offset = 0;
    memcpy(&data->prevAlloc, direccion + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->nextAlloc, direccion + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->isFree, direccion + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    return data;
}
void guardarAlloc(t_heap_metadata *data, uint32_t direccion)
{

    uint32_t offset = 0;
    memcpy(direccion + offset, &data->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(direccion + offset, &data->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(direccion + offset, &data->isFree, sizeof(uint8_t));
}

int memAlloc(t_paquete *paquete)
{

    t_malloc_serializado *mallocDeserializado = deserializar_alloc(paquete);

    int size = mallocDeserializado->size_reservar;
    int carpincho_id = socket_client;

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    uint32_t inicio = tamanio_memoria;
    uint32_t direccion_logica_enviar = 0;
    if (list_size(tabla_paginas->paginas) == 0)
    {
        int numero_pagina = solicitarPaginaNueva(carpincho_id);
        if(numero_pagina < 0){
            return 0;
        }
        t_pagina *pagina = list_get(tabla_paginas->paginas, numero_pagina);
        crearPrimerHeader(pagina, size);
        t_contenidos_pagina *contenido = list_get(pagina->listado_de_contenido, 0);
        t_heap_metadata *header = traerAllocDeMemoria(contenido->dir_comienzo);
        direccion_logica_enviar = contenido->dir_comienzo;
        direccion_logica_enviar = agregarPagina(pagina, header, contenido->dir_comienzo - inicio, sizeof(t_heap_metadata), true,0);
        return direccion_logica_enviar;
    }
    else
    {
        int paginaDisponible = getPrimeraPaginaDisponible(size, tabla_paginas);
        t_pagina *pagina_Disponible = list_get(tabla_paginas->paginas, paginaDisponible);
        t_pagina *primera_pagina;
        if(pagina_Disponible !=0){
            primera_pagina  = list_get(tabla_paginas->paginas,0);
        }else{
            primera_pagina  = pagina_Disponible;
        }


        t_contenidos_pagina *contenido = getContenidoPaginaByTipo(primera_pagina->listado_de_contenido, HEADER);
        t_heap_metadata *data = traerAllocDeMemoria(contenido->dir_comienzo);
        uint32_t nextAnterior = tamanio_memoria;
        int index = 0;
        while (data->nextAlloc != 0)
        {
            if (data->isFree)
            {

                //Estoy en un alloc libre y no es el ultimo, hacer si entra totalmente, sino que siga

                uint32_t sizeAlloc;
                if (data->prevAlloc == 0)
                {
                    log_info(logger_memoria, "Primer Alloc Posiblemente Libre");
                    sizeAlloc = data->nextAlloc - inicio - sizeof(t_heap_metadata);
                }
                else
                {
                    log_info(logger_memoria, "Alloc Del Medio Posiblemente Libre");
                    sizeAlloc = data->nextAlloc - nextAnterior - sizeof(t_heap_metadata);
                }

                if (size == sizeAlloc)
                {

                    log_info(logger_memoria, "Hay un Alloc Libre disponible paraa usar del mismo tamanio requerido");
                    //Uso este alloc para guardar

                    int pagina = getPaginaByDireccionLogica(nextAnterior);
                    t_pagina *paginaAlloc = list_get(tabla_paginas->paginas, pagina);
                    if (paginaAlloc->bit_presencia)
                    {
                    }
                    paginaAlloc->cantidad_contenidos += 1;
                    paginaAlloc->tamanio_ocupado += size;

                    data->isFree = false;
                    guardarAlloc(data, nextAnterior);
                    return;
                }
                else if (sizeAlloc > size + sizeof(t_heap_metadata))
                {

                    log_info(logger_memoria, "Encontre un Alloc libre que tiene un tamanio mayor la requerido. Hay que separarlo");

                    data->isFree = false;
                    data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) + size;

                    guardarAlloc(data, nextAnterior);

                    data->isFree = true;
                    data->prevAlloc = nextAnterior;
                    data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) * 2 + sizeAlloc;

                    guardarAlloc(data, nextAnterior + sizeof(t_heap_metadata) + size);
                }
            }

            if(nextAnterior == tamanio_memoria){
                nextAnterior = 0;
            }
            int nropagina_alloc =getPaginaByDireccionLogica(nextAnterior);
            t_pagina *pagina_alloc_anterior = list_get(tabla_paginas->paginas,nropagina_alloc);

            nextAnterior = data->nextAlloc;
            
            nropagina_alloc =getPaginaByDireccionLogica(nextAnterior);
            t_pagina *pagina_alloc_actual = list_get(tabla_paginas->paginas,nropagina_alloc);
            
            if(pagina_alloc_actual->bit_presencia == false){
                //SWAPP
            }
            int direccion = inicio + data->nextAlloc+ config_memoria->TAMANIO_PAGINA * pagina_alloc_anterior->marco_asignado;
            
            //t_contenidos_pagina *contenido_actual = getContenidoPaginaByTipoAndSize(pagina_Disponible->listado_de_contenido,CONTENIDO,data->nextAlloc);
            data = traerAllocDeMemoria(direccion);
            index++;
        }
        //Nuevo Alloc
        int paginaLastAlloc = getPaginaByDireccionLogica(nextAnterior);
        t_pagina *pagina = list_get(tabla_paginas->paginas, paginaLastAlloc);
        return agregarPagina(pagina, data, nextAnterior, size, false, index);
    }
}
void crearPrimerHeader(t_pagina *pagina, uint32_t size)
{
    uint32_t inicio = tamanio_memoria;
    t_heap_metadata *data = malloc(sizeof(t_heap_metadata));
    data->nextAlloc = (pagina->numero_pagina * config_memoria->TAMANIO_PAGINA) + sizeof(t_heap_metadata) + size;
    data->prevAlloc = NULL;
    data->isFree = false;

    pagina->cantidad_contenidos += 1;
    pagina->tamanio_ocupado += sizeof(t_heap_metadata) + size;

    t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
    contenido->carpincho_id = pagina->carpincho_id;
    contenido->dir_comienzo = inicio + (pagina->marco_asignado * config_memoria->TAMANIO_PAGINA);
    contenido->dir_fin = contenido->dir_comienzo + sizeof(t_heap_metadata);
    contenido->contenido_pagina = HEADER;
    contenido->tamanio = sizeof(t_heap_metadata);
    list_add(pagina->listado_de_contenido, contenido);

    t_contenidos_pagina *contenido_contenido = malloc(sizeof(t_contenidos_pagina));
    contenido_contenido->carpincho_id = pagina->carpincho_id;
    contenido_contenido->dir_comienzo = contenido->dir_fin;
    contenido_contenido->dir_fin = contenido_contenido->dir_comienzo + size;
    contenido_contenido->contenido_pagina = CONTENIDO;
    contenido_contenido->tamanio = size;
    list_add(pagina->listado_de_contenido, contenido_contenido);

    guardarAlloc(data, contenido->dir_comienzo);
    free(data);
}

int agregarPagina(t_pagina *pagina, t_heap_metadata *data, uint32_t nextAnterior, uint32_t size, bool ultimo, int index_alloc)
{
    bool agregado = false;
    uint32_t inicio = tamanio_memoria;
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(pagina->carpincho_id);
 
    if (pagina->tamanio_ocupado < config_memoria->TAMANIO_PAGINA)
    {
        if (pagina->tamanio_ocupado + size <= config_memoria->TAMANIO_PAGINA)
        {
            //entra completo
            t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
            if (ultimo)
            {
                data->prevAlloc = nextAnterior;
                nextAnterior = data->nextAlloc;
                data->nextAlloc = NULL;
                data->isFree = true;

                t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
                contenido->contenido_pagina = FOOTER;
                contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior;
                contenido->dir_fin = contenido->dir_comienzo + size;
                contenido->tamanio = size;
                contenido->carpincho_id = pagina->carpincho_id;
                list_add(pagina->listado_de_contenido, contenido);

                guardarAlloc(data, contenido->dir_comienzo);
                pagina->cantidad_contenidos += 1;
                pagina->tamanio_ocupado += size;
                agregado = true;

                int pagina_anterior = getPaginaByDireccionLogica(data->prevAlloc);
                pagina = list_get(tabla_paginas->paginas,pagina_anterior);
                return inicio + pagina->numero_pagina * config_memoria->TAMANIO_PAGINA + data->prevAlloc;
            }
            else
            {
                if (data->prevAlloc == NULL && index == 0)
                {
                    data->nextAlloc = nextAnterior + size;
                    t_contenidos_pagina *c = list_get(pagina->listado_de_contenido, 0);
                    guardarAlloc(data, c->dir_comienzo);

                    contenido->contenido_pagina = CONTENIDO;

                    contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior;
                    contenido->dir_fin = contenido->dir_comienzo + size;
                    contenido->tamanio = size;
                    contenido->carpincho_id = pagina->carpincho_id;

                    pagina->cantidad_contenidos += 1;
                    pagina->tamanio_ocupado += size;

                    list_add(pagina->listado_de_contenido, contenido);

                    data->isFree = false;
                    return agregarPagina(pagina, data, nextAnterior, sizeof(t_heap_metadata), true,index_alloc);
                }
                else
                {
                    data->nextAlloc = nextAnterior + size + sizeof(t_heap_metadata);
                    data->isFree = false;
                }
                
            }
            if(!agregado){
                contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior + sizeof(t_heap_metadata);
                contenido->contenido_pagina = CONTENIDO;
                contenido->dir_fin = contenido->dir_comienzo + size;
                contenido->tamanio = size;

                pagina->cantidad_contenidos += 1;
                pagina->tamanio_ocupado += size;
                list_add(pagina->listado_de_contenido, contenido);
                int index = list_size(pagina->listado_de_contenido) - 2;
                contenido = list_get(pagina->listado_de_contenido, index);
                guardarAlloc(data, contenido->dir_comienzo);
                data = traerAllocDeMemoria(contenido->dir_comienzo);
                return agregarPagina(pagina, data, nextAnterior, sizeof(t_heap_metadata), true,index_alloc);
            }
            
        }
        else
        {
            //ocupo el restante y pido otra
            int restante = size - (config_memoria->TAMANIO_PAGINA - pagina->tamanio_ocupado);
            if(ultimo){
                //ARREGLAR QUE ESTA MAL, FALTA RETURN
                asignarFooterSeparado(pagina,data,size,nextAnterior);
                return;
            }
            if (list_size(tabla_paginas->paginas) + 1 <= tabla_paginas->paginas_totales_maximas)
            {

                data->nextAlloc = nextAnterior + size + sizeof(t_heap_metadata);
                data->isFree = false;
                
                t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
                contenido->carpincho_id = pagina->carpincho_id;
                contenido->contenido_pagina = RESTO_CONTENIDO;
                contenido->tamanio = restante;
                contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior + sizeof(t_heap_metadata);
                contenido->dir_fin = contenido->dir_comienzo + (size - restante);
                list_add(pagina->listado_de_contenido,contenido);

                pagina->cantidad_contenidos += 1;
                pagina->tamanio_ocupado += (size - restante);
                
                guardarAlloc(data, inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA +nextAnterior);
                

                int numero_pagina = solicitarPaginaNueva(pagina->carpincho_id);
                if(numero_pagina < 0){
                    return 0;
                }
                pagina = list_get(tabla_paginas->paginas, numero_pagina);
                t_contenidos_pagina *contenido_nuevo = malloc(sizeof(t_contenidos_pagina));
                contenido_nuevo->carpincho_id = pagina->carpincho_id;
                contenido_nuevo->tamanio = restante;
                contenido_nuevo->contenido_pagina= RESTO_CONTENIDO;
                contenido_nuevo->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + data->nextAlloc - nextAnterior - size - sizeof(t_heap_metadata);
                contenido_nuevo->dir_fin = contenido_nuevo->dir_comienzo + restante;

                list_add(pagina->listado_de_contenido,contenido_nuevo);
                pagina->cantidad_contenidos += 1;
                return agregarPagina(pagina, data, restante, restante + sizeof(t_heap_metadata), true,index_alloc);
            }
        }
    }
    else
    {
        
        int numero_pagina = solicitarPaginaNueva(pagina->carpincho_id);
        if(numero_pagina < 0){
            return 0;
        }
        uint32_t dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA +nextAnterior;

        if (ultimo)
        {
            t_heap_metadata* data_anterior = data;
            data->prevAlloc = nextAnterior;
            nextAnterior = data->nextAlloc;
            data->nextAlloc = NULL;
            data->isFree = true;

            t_contenidos_pagina *contenido = malloc(sizeof(t_contenidos_pagina));
            contenido->contenido_pagina = FOOTER;
            contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior;
            contenido->dir_fin = contenido->dir_comienzo + size;
            contenido->tamanio = size;
            contenido->carpincho_id = pagina->carpincho_id;
            t_pagina* pagina_anterior = pagina;
            pagina = list_get(tabla_paginas->paginas, numero_pagina);
            list_add(pagina->listado_de_contenido, contenido);

            guardarAlloc(data, contenido->dir_comienzo);
            pagina->cantidad_contenidos += 1;
            pagina->tamanio_ocupado += size;
            agregado = true;

            int pagina_anterior_numero = getPaginaByDireccionLogica(data->prevAlloc);
            pagina = list_get(tabla_paginas->paginas,pagina_anterior_numero);
            return inicio + pagina->numero_pagina * config_memoria->TAMANIO_PAGINA + data->prevAlloc;
            //return inicio + pagina_anterior->numero_pagina * config_memoria->TAMANIO_PAGINA + nextAnterior - sizeof(t_heap_metadata);
        }
        pagina = list_get(tabla_paginas->paginas, numero_pagina);
        data->isFree = true;
        data->prevAlloc = nextAnterior;

        nextAnterior = data->nextAlloc;

        data->nextAlloc = NULL;
        guardarAlloc(data, dir_comienzo);
        pagina->cantidad_contenidos += 1;
        pagina->tamanio_ocupado += size;
    }
}
void asignarFooterSeparado(t_pagina* pagina,t_heap_metadata* data,uint32_t size, uint32_t nextAnterior){
    int restante = size - (config_memoria->TAMANIO_PAGINA - pagina->tamanio_ocupado);
    uint32_t inicio = tamanio_memoria;
    data->prevAlloc = nextAnterior;
    nextAnterior = data->nextAlloc;
    data->nextAlloc = NULL;
    data->isFree = true;
    //data = traerAllocDeMemoria(inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior);
    guardarAlloc(data, inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior);
    pagina = asignarFooterSeparadoSubContenido(PREV, pagina,nextAnterior);
    pagina = asignarFooterSeparadoSubContenido(NEXT, pagina,nextAnterior);
    pagina = asignarFooterSeparadoSubContenido(FREE, pagina,nextAnterior);
}

t_pagina* asignarFooterSeparadoSubContenido(t_contenido subcontenido, t_pagina* pagina, uint32_t nextAnterior){
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(pagina->carpincho_id);
    uint32_t inicio = tamanio_memoria;
    
    if(!pagina->tamanio_ocupado + sizeof(uint32_t) <= config_memoria->TAMANIO_PAGINA){
        int nro_pagina_nueva = solicitarPaginaNueva(pagina->carpincho_id);
        if(nro_pagina_nueva < 0){
            return 0;
        }
        pagina = list_get(tabla_paginas->paginas, nro_pagina_nueva);
    }
    pagina->tamanio_ocupado += sizeof(uint32_t);
    t_contenidos_pagina *contenido =malloc(sizeof(t_contenidos_pagina));

    t_subcontenido* subcontenido_agregar = malloc(sizeof(t_subcontenido));
    subcontenido_agregar->contenido_pagina = subcontenido;
    subcontenido_agregar->pagina_seguir = pagina->numero_pagina;
    subcontenido_agregar->desplazamiento=nextAnterior;

    contenido->carpincho_id = pagina->carpincho_id;
    contenido->tamanio = sizeof(uint32_t);
    contenido->subcontenido = subcontenido_agregar;
    contenido->contenido_pagina = RESTO_ALLOC;
    contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior;
    contenido->dir_fin = contenido->dir_comienzo + sizeof(uint32_t);
    list_add(pagina->listado_de_contenido, contenido);
    return pagina;
}
t_heap_metadata *getLastHeapFromPagina(int pagina, int carpincho_id)
{

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_pagina *paginaBuscada = list_get(tabla_paginas->paginas, pagina);
    t_contenidos_pagina *contenidoUltimo = getLastContenidoByPagina(paginaBuscada);
    t_heap_metadata *metadata = traerAllocDeMemoria(contenidoUltimo->dir_comienzo);
    return metadata;
}

void mostrarAllocs(int carpincho_id)
{
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_pagina *paginaLeida = list_get(tabla_paginas->paginas, 0);
    if (paginaLeida == NULL)
        return;

    uint32_t inicio = tamanio_memoria + config_memoria->TAMANIO_PAGINA * paginaLeida->numero_pagina;
    t_heap_metadata *data = traerAllocDeMemoria(inicio);
    while (data->nextAlloc != 0)
    {
        printf("Prev Alloc: %d  Next Alloc: %d.IsFree:%d \n", data->prevAlloc, data->nextAlloc, data->isFree);
        data = traerAllocDeMemoria(data->nextAlloc);
    }
}