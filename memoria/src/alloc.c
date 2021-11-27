#include "alloc.h"

// Memfree -> Libero alloc (flag isFree en true), me fijo el anterior y posterior y los unifico
// TODO -> Meter paginacion (Mati gatooo)
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
    }
    int nropaginaAllocActual = getPaginaByDireccionLogica(direccion);
    t_pagina* paginaAllocActual = list_get(tabla_paginas->paginas, nropaginaAllocActual);
    //Traigo de memoria el alloc
    t_heap_metadata *alloc = traerAllocDeMemoria(inicio + direccion + config_memoria->TAMANIO_PAGINA * paginaAllocActual->marco_asignado );
    alloc->isFree = true;
    uint32_t next = alloc->nextAlloc;
    uint32_t back = alloc->prevAlloc;

    
    t_pagina *pagina_alloc_anterior;
    t_pagina *pagina_alloc_siguiente;

    t_heap_metadata *anterior;
    t_heap_metadata *posterior;
    bool hayAnterior = false;
    bool hayPosterior = false;

    if (alloc->prevAlloc != NULL)
    { // SI EXISTE ANTERIOR TRAERLO
        int nropaginaAllocAnterior = getPaginaByDireccionLogica(back);
        pagina_alloc_anterior = list_get(tabla_paginas->paginas, nropaginaAllocAnterior);
        pagina_alloc_anterior->cantidad_contenidos -= 1;
        
        anterior = traerAllocDeMemoria(alloc->prevAlloc);
        if (anterior->isFree)
            hayAnterior = true;
    }
    if (next != NULL)
    { //SI EXISTE PROXIMO
        int nropaginaAllocSiguiente = getPaginaByDireccionLogica(next);
        pagina_alloc_siguiente = list_get(tabla_paginas->paginas, nropaginaAllocSiguiente);
        int direccion = inicio +next + pagina_alloc_siguiente->marco_asignado * config_memoria->TAMANIO_PAGINA;
        posterior = traerAllocDeMemoria(direccion);
        if (posterior->isFree)
            hayPosterior = true;
    }
    if (hayAnterior && hayPosterior)
    {
        int paginaNextAlloc = getPaginaByDireccionLogica(next);
        if (pagina_alloc_anterior->numero_pagina == pagina_alloc_siguiente->numero_pagina)
        {
            //TODO EL CONTENIDO ESTA EN UNA MISMA PAGINA
            /*t_pagina *pagina_alloc = list_get(tabla_paginas->paginas, nropaginaAllocAnterior);
            pagina_alloc->tamanio_ocupado -= next - back - sizeof(t_heap_metadata);
            pagina_alloc->cantidad_contenidos -= 1;*/
        }
        int paginaAlloc = getPaginaByDireccionLogica(next);

        anterior->nextAlloc = posterior->nextAlloc;

        paginaNextAlloc = getPaginaByDireccionLogica(next);
        t_pagina *pagina_allocNext = list_get(tabla_paginas->paginas, paginaNextAlloc);
        pagina_allocNext->tamanio_ocupado -= posterior->nextAlloc - next - sizeof(t_heap_metadata);
        pagina_allocNext->cantidad_contenidos -= 1;

        if (list_size(tabla_paginas->paginas) == pagina_allocNext->numero_pagina && pagina_allocNext->cantidad_contenidos == 1)
        {
            list_remove(tabla_paginas->paginas, pagina_allocNext);
            free(pagina_allocNext);
            tabla_paginas->paginas_en_memoria -= 1;
        }

        free(alloc);
        free(posterior);
        guardarAlloc(anterior, back);
        return;
    }
    if (hayAnterior)
    {
        anterior->nextAlloc = alloc->nextAlloc;
        int paginaAlloc = getPaginaByDireccionLogica(back);
        t_pagina *pagina_alloc = list_get(tabla_paginas->paginas, paginaAlloc);
        pagina_alloc->tamanio_ocupado -= next - back - sizeof(t_heap_metadata);
        pagina_alloc->cantidad_contenidos -= 1;
        free(alloc);
        guardarAlloc(anterior, back);
        return;
    }

    if (hayPosterior)
    {
        alloc->nextAlloc = posterior->nextAlloc;
        int paginaNext = getPaginaByDireccionLogica(next);
        t_pagina *pagina_Next = list_get(tabla_paginas->paginas, paginaNext);
        if (list_size(tabla_paginas->paginas) == pagina_Next->numero_pagina && pagina_Next->cantidad_contenidos == 1)
        {
            list_remove(tabla_paginas->paginas, paginaNext);
            free(pagina_Next);
            tabla_paginas->paginas_en_memoria -= 1;
        }
        else
        {
            pagina_Next->cantidad_contenidos -= 1;
            //restar tamanio ocupado
            //pagina del alloc actual restar tamanio
        }
        free(posterior);
        guardarAlloc(alloc, direccion);
        return;
    }
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

void memAlloc(t_paquete *paquete)
{

    t_malloc_serializado *mallocDeserializado = deserializar_alloc(paquete);

    int size = mallocDeserializado->size_reservar;
    int carpincho_id = mallocDeserializado->carpincho_id;

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    uint32_t inicio = tamanio_memoria;

    if (list_size(tabla_paginas->paginas) == 0)
    {
        int numero_pagina = solicitarPaginaNueva(carpincho_id);
        t_pagina *pagina = list_get(tabla_paginas->paginas, numero_pagina);
        crearPrimerHeader(pagina, size);
        t_contenidos_pagina *contenido = list_get(pagina->listado_de_contenido, 0);
        t_heap_metadata *header = traerAllocDeMemoria(contenido->dir_comienzo);
        agregarPagina(pagina, header, contenido->dir_comienzo - inicio, sizeof(t_heap_metadata), true);
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
        while (data->nextAlloc != NULL)
        {
            if (data->isFree)
            {

                //Estoy en un alloc libre y no es el ultimo, hacer si entra totalmente, sino que siga

                uint32_t sizeAlloc;
                if (data->prevAlloc == NULL)
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
            nextAnterior = data->nextAlloc;
            
            int nropagina_alloc =getPaginaByDireccionLogica(nextAnterior);
            t_pagina *pagina_alloc_actual = list_get(tabla_paginas->paginas,nropagina_alloc);
            int direccion = inicio + data->nextAlloc+ config_memoria->TAMANIO_PAGINA * pagina_alloc_actual->marco_asignado;
            
            //t_contenidos_pagina *contenido_actual = getContenidoPaginaByTipoAndSize(pagina_Disponible->listado_de_contenido,CONTENIDO,data->nextAlloc);
            data = traerAllocDeMemoria(direccion);
        }
        //Nuevo Alloc
        int paginaLastAlloc = getPaginaByDireccionLogica(nextAnterior);
        t_pagina *pagina = list_get(tabla_paginas->paginas, paginaLastAlloc);
        agregarPagina(pagina, data, nextAnterior, size, false);
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
    list_add(pagina->listado_de_contenido, contenido);

    t_contenidos_pagina *contenido_contenido = malloc(sizeof(t_contenidos_pagina));
    contenido_contenido->carpincho_id = pagina->carpincho_id;
    contenido_contenido->dir_comienzo = contenido->dir_fin;
    contenido_contenido->dir_fin = contenido_contenido->dir_comienzo + size;
    contenido_contenido->contenido_pagina = CONTENIDO;
    list_add(pagina->listado_de_contenido, contenido_contenido);

    guardarAlloc(data, contenido->dir_comienzo);
    free(data);
}

void agregarPagina(t_pagina *pagina, t_heap_metadata *data, uint32_t nextAnterior, uint32_t size, bool ultimo)
{

    uint32_t inicio = tamanio_memoria;
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(pagina->carpincho_id);
    t_heap_metadata* data2 = malloc(sizeof(t_heap_metadata));
 
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
                return;
            }
            else
            {
                if (data->prevAlloc == NULL)
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

                    agregarPagina(pagina, data, nextAnterior, sizeof(t_heap_metadata), true);
                }
                else
                {
                    data->nextAlloc = nextAnterior + size + sizeof(t_heap_metadata);
                }
                data->isFree = false;
            }

            contenido->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior;
            contenido->contenido_pagina = CONTENIDO;
            contenido->dir_fin = contenido->dir_comienzo + size;

            pagina->cantidad_contenidos += 1;
            pagina->tamanio_ocupado += size;
            list_add(pagina->listado_de_contenido, contenido);
            contenido = list_get(pagina->listado_de_contenido, list_size(pagina->listado_de_contenido) - 1);
            guardarAlloc(data, contenido->dir_comienzo);
            data = traerAllocDeMemoria(contenido->dir_comienzo);
            agregarPagina(pagina, data, nextAnterior, sizeof(t_heap_metadata), true);
        }
        else
        {
            //ocupo el restante y pido otra
            int restante = size - (config_memoria->TAMANIO_PAGINA - pagina->tamanio_ocupado);
            if(ultimo){
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
                pagina = list_get(tabla_paginas->paginas, numero_pagina);
                t_contenidos_pagina *contenido_nuevo = malloc(sizeof(t_contenidos_pagina));
                contenido_nuevo->carpincho_id = pagina->carpincho_id;
                contenido_nuevo->tamanio = restante;
                contenido_nuevo->contenido_pagina= RESTO_CONTENIDO;
                contenido_nuevo->dir_comienzo = inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + data->nextAlloc - nextAnterior - size - sizeof(t_heap_metadata);
                contenido_nuevo->dir_fin = contenido_nuevo->dir_comienzo + restante;

                list_add(pagina->listado_de_contenido,contenido_nuevo);
                pagina->cantidad_contenidos += 1;
                agregarPagina(pagina, data, restante, restante + sizeof(t_heap_metadata), true);
            }
        }
    }
    else
    {
        int numero_pagina = solicitarPaginaNueva(pagina->carpincho_id);
        pagina = list_get(tabla_paginas->paginas, numero_pagina);
        data->isFree = true;
        data->prevAlloc = nextAnterior;

        nextAnterior = data->nextAlloc;

        data->nextAlloc = NULL;
        guardarAlloc(data, nextAnterior);
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
    guardarAlloc(data, inicio + pagina->marco_asignado * config_memoria->TAMANIO_PAGINA + nextAnterior);
    data = traerAllocDeMemoria(134550526);
    pagina = asignarFooterSeparadoSubContenido(PREV, pagina,nextAnterior);
    data = traerAllocDeMemoria(134550526);
    pagina = asignarFooterSeparadoSubContenido(NEXT, pagina,nextAnterior);
    data = traerAllocDeMemoria(134550526);
    pagina = asignarFooterSeparadoSubContenido(FREE, pagina,nextAnterior);
    data = traerAllocDeMemoria(134550526);
}

t_pagina* asignarFooterSeparadoSubContenido(t_contenido subcontenido, t_pagina* pagina, uint32_t nextAnterior){
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(pagina->carpincho_id);
    uint32_t inicio = tamanio_memoria;
    
    if(!pagina->tamanio_ocupado + sizeof(uint32_t) <= config_memoria->TAMANIO_PAGINA){
        int nro_pagina_nueva = solicitarPaginaNueva(pagina->carpincho_id);
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
    while (data->nextAlloc != NULL)
    {
        printf("Prev Alloc: %d  Next Alloc: %d.IsFree:%d \n", data->prevAlloc, data->nextAlloc, data->isFree);
        data = traerAllocDeMemoria(data->nextAlloc);
    }
}