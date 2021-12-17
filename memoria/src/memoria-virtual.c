#include "memoria-virtual.h"

void agregarAsignacion(t_pagina* pagina){
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){

        if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
            list_add(tabla_paginas->Clock,pagina);
        }else 
        {
            list_add(reemplazo_CLOCK,pagina);
        }
    }else
    {
        if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
            list_add(tabla_paginas->Lru,pagina);
        }else
        {
            list_add(reemplazo_LRU,pagina);
        }
    }
    return;
}

int reemplazarPagina(){
    int marco = -1;
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
        //SWAP CLOCK 
        marco = reemplazarClockM();
        return marco;
    }
    else{
        //SWAP LRU
        marco = reemplazarLRU();
        return marco;
    }
}

//LRU saca la pagina que hace mas tiempo no se hace referencia
//Cuando hago memread o memwrite significa una referencia

int eliminarPrimerElementoLista(int carpincho_id){
    t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->Lru);
    bool primero = true;
    int marco = 0;
    t_list *paginas = list_create();
    while (list_iterator_has_next(list_iterator))
    {
        t_pagina *paginaList = list_iterator_next(list_iterator);
        if(!primero){
            list_add(paginas,paginaList);
        }
        marco = paginaList->marco_asignado;
        primero = false;
    }
    list_iterator_destroy(list_iterator);
    
    free(tabla_paginas->Lru);
    tabla_paginas->Lru = paginas;
    return marco;
}

int recibirPaginaSwap(t_pagina* pagina){

    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete = serializar(SWAPFREE,4,INT,pagina->carpincho_id,INT,pagina->numero_pagina);

    //Enviarlo el paquete y checkear que ande bien
    enviar_paquete(paquete, socket_cliente_swap);
    
    paquete = recibir_paquete(socket_cliente_swap);
    if(paquete->codigo_operacion != RECEPCION_PAGINA){
        //NO PUDO DEVOLVERLA
        free(paquete);
        return -1;
    }

    t_pagina_enviada_swap *pagina_swap = deserializar_pagina(paquete->buffer->stream);

    //uint32_t inicio = tamanio_memoria;
    //Escribir pagina en memoria
    escribirPaginaEnMemoria(pagina, pagina_swap);
    free(paquete);
    return 0;
}


int enviarPaginaSwap(t_pagina* pagina){

    //Borro la pagina de la tlb, no deberia hacerlo aca pero no queria ponerlo en todos lados
    eliminarDeTLB(pagina->numero_pagina, pagina->carpincho_id);

    //t_contenidos_pagina* contenido = list_get(pagina->listado_de_contenido,0);
    uint32_t inicio = tamanio_memoria;
    t_pagina_enviada_swap *pagina_swap = malloc(sizeof(t_pagina_enviada_swap));
    pagina_swap->heap_contenidos = list_create();
    pagina_swap->pid = pagina->carpincho_id;
    pagina_swap->numero_pagina = pagina->numero_pagina;

    if(pagina->numero_pagina == 4){
        int q = 0;
        q++;
    }
    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);
    while(list_iterator_has_next(list_iterator)){

        t_contenidos_pagina* contenido = list_iterator_next(list_iterator);

        if(contenido->contenido_pagina == RESTO_CONTENIDO || contenido->contenido_pagina == CONTENIDO){
            int offset = (contenido->dir_comienzo - inicio) % config_memoria->TAMANIO_PAGINA;
            if(list_size(pagina_swap->heap_contenidos) == 0){
                t_heap_contenido_enviado* heap_swap = malloc(sizeof(t_heap_contenido_enviado));
                heap_swap->prevAlloc = 1;
                heap_swap->nextAlloc = 1;
                heap_swap->isFree = false;
                list_add(pagina_swap->heap_contenidos,heap_swap);
            }else{
                t_heap_contenido_enviado *contenido_heap_enviado = list_get(pagina_swap->heap_contenidos,list_size(pagina_swap->heap_contenidos)-1);
                contenido_heap_enviado->size_contenido = contenido->tamanio;
                contenido_heap_enviado->contenido = traerDeMemoria(pagina->marco_asignado, offset, contenido->tamanio);
            }
        }else
        {
            t_heap_metadata* heap = traerAllocIncompleto(pagina->marco_asignado,contenido->dir_comienzo,contenido->dir_fin);
            t_heap_contenido_enviado* heap_swap = malloc(sizeof(t_heap_contenido_enviado));
            heap_swap->prevAlloc = heap->prevAlloc;
            heap_swap->nextAlloc = heap->nextAlloc;
            heap_swap->isFree = heap->isFree;
            if(heap->nextAlloc == 0){
                heap_swap->contenido = malloc(sizeof(7));
                heap_swap->contenido= "basura";
                heap_swap->size_contenido= strlen(heap_swap->contenido)+1;
                int a = 0;
                a++;
            }
            list_add(pagina_swap->heap_contenidos,heap_swap);
        }
    }
    list_iterator_destroy(list_iterator);
    void *pagina_serial = serializar_pagina(pagina_swap);

    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = bytes_pagina(pagina_swap);
    buffer->stream = pagina_serial;

    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = SWAPSAVE;
    paquete->buffer = buffer;

    if(socket_cliente_swap < 0)
        return -1;
        
    enviar_paquete(paquete, socket_cliente_swap);
    //free(cont);
    //free(pagina_serial);
    //free(buffer);

    list_iterator = list_iterator_create(pagina_swap->heap_contenidos);
    while(list_iterator_has_next(list_iterator))
    {
         t_heap_contenido_enviado* heap_swap  = list_iterator_next(list_iterator);
         if(heap_swap->nextAlloc != 0)
            free(heap_swap->contenido);
            
         free(heap_swap);
    }
    list_iterator_destroy(list_iterator);
    list_destroy(pagina_swap->heap_contenidos);
    
    free(pagina_swap);

    t_paquete* paquete_recibir = recibir_paquete(socket_cliente_swap);
    if(paquete_recibir->codigo_operacion== PAGINA_GUARDADA){
        //PUDO GUARDAR
        free(paquete_recibir->buffer->stream);
        free(paquete_recibir->buffer);
        free(paquete_recibir);
        return 1;
    }
    else{
        free(paquete_recibir->buffer->stream);
        free(paquete_recibir->buffer);
        free(paquete_recibir);
        return -1;
    }
    return 0;

}

int reemplazarLRU(){
    int marco = -1;
    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
        t_pagina* old = list_get(tabla_paginas->Lru,0);
        marco = enviarPaginaSwap(old);

        if(marco <0){
            return -1;
        }else
        {
            old->bit_presencia = 0;
            tabla_paginas->paginas_en_memoria -= 1;
            list_remove(tabla_paginas->Lru,0);
            return old->marco_asignado;
        }
        
    }else
    {   
        t_pagina* old = list_get(reemplazo_LRU,0);
        marco = enviarPaginaSwap(old);

        if(marco <0){
            return -1;
        }else
        {
            old->bit_presencia = 0;
            t_tabla_paginas* tabla_old = buscarTablaPorPID(old->carpincho_id);
            tabla_old->paginas_en_memoria -= 1;
            list_remove(reemplazo_LRU,0);
            return old->marco_asignado;
        }
    }
}

void actualizarLRU(t_pagina* pagina){

    pagina->bit_uso = 1;
    int index = 0;

    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){

        t_tabla_paginas *tabla = buscarTablaPorPID(pagina->carpincho_id);
        t_list_iterator *list_iterator = list_iterator_create(tabla->Lru);
        t_pagina *pag;

        while (list_iterator_has_next(list_iterator)){

            pag = list_iterator_next(list_iterator);
            if(pag->numero_pagina == pagina->numero_pagina){
                list_remove(tabla->Lru,index);
                list_add(tabla->Lru, pagina);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }

    }else
    {
        t_list_iterator *list_iterator = list_iterator_create(reemplazo_LRU);
        t_pagina *pag = malloc(sizeof(t_pagina));

        while (list_iterator_has_next(list_iterator)){

            pag = list_iterator_next(list_iterator);
            if(pag->numero_pagina == pagina->numero_pagina){
                list_remove(reemplazo_LRU,index);
                list_add(reemplazo_LRU, pagina);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }

    }
}

void replaceClock(t_pagina *pagina){

    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){

        t_tabla_paginas *tabla = buscarTablaPorPID(pagina->carpincho_id);
        int posicion = getPosicionEnTablaDeProcesos(tabla);
        int posClock = pagina->marco_asignado - (posicion * config_memoria->MARCOS_POR_CARPINCHO);
        list_replace(tabla->Clock,pagina,posClock);

    }else
    {
        list_replace(reemplazo_CLOCK,pagina,pagina->marco_asignado);
    }

}

int reemplazarClockM(){

    int index = 0;
    int marco = -1;

    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){

        while(marco == -1){

            //CLOCK M con fija -> tabla de paginas tiene el clock, las paginas tienen uso y modificado

            t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->Clock);
            t_pagina* page = malloc(sizeof(t_pagina));

            while(list_iterator_has_next(list_iterator)){

                page = list_iterator_next(list_iterator);
            
                while(index < tabla_paginas->ultimo_Clock){
                    page = list_iterator_next(list_iterator);
                    index ++;
                }

                if(page->bit_presencia == 1){

                    if(page->bit_modificado == 0 && page->bit_uso == 0){

                        //Ver de cambiar esto despues de swapear
                        marco = enviarPaginaSwap(page);
                        if(marco <0){
                            return -1;
                        }else
                        {
                            page->bit_presencia = 0;
                            tabla_paginas->paginas_en_memoria -= 1;
                            tabla_paginas->ultimo_Clock = index + 1;
                            list_iterator_destroy(list_iterator);
                            return page->marco_asignado;
                        }

                    }

                }

                index++;

            }

            // Termine la primera media vuelta, ahora hago lo mismo hasta ultimo

            index = 0;
            list_iterator_destroy(list_iterator);

            list_iterator = list_iterator_create(tabla_paginas->Clock);

            while(list_iterator_has_next(list_iterator)){

                page = list_iterator_next(list_iterator);

                while(index < tabla_paginas->ultimo_Clock){
                    if(page->bit_presencia == 1){

                        if(page->bit_modificado == 0 && page->bit_uso == 0){

                            marco = enviarPaginaSwap(page);
                            if(marco <0){
                                return -1;
                            }else
                            {
                                page->bit_presencia = 0;
                                tabla_paginas->paginas_en_memoria -= 1;
                                tabla_paginas->ultimo_Clock = index + 1;
                                list_iterator_destroy(list_iterator);
                                return page->marco_asignado;
                            }
                        }
                    }
                    page = list_iterator_next(list_iterator);
                    index ++;
                }


                //Arranco segunda vuelta

                if(page->bit_presencia == 1){

                    if(page->bit_modificado == 1 && page->bit_uso == 0){

                        //Ver de cambiar esto despues de swapear
                        marco = enviarPaginaSwap(page);
                        if(marco <0){
                            return -1;
                        }else
                        {
                            page->bit_presencia = 0;
                            tabla_paginas->paginas_en_memoria -= 1;
                            tabla_paginas->ultimo_Clock = index + 1;
                            list_iterator_destroy(list_iterator);
                            return page->marco_asignado;
                        } 
                    }
                }

                page->bit_uso = 0;

                index ++;

            }

            index = 0;
            list_iterator_destroy(list_iterator);

            list_iterator = list_iterator_create(tabla_paginas->Clock);

            while(list_iterator_has_next(list_iterator)){

                page = list_iterator_next(list_iterator);

                while(index < tabla_paginas->ultimo_Clock){
                    if(page->bit_presencia == 1){

                        if(page->bit_modificado == 1 && page->bit_uso == 0){

                            //Ver de cambiar esto despues de swapear
                            marco = enviarPaginaSwap(page);
                            if(marco <0){
                                return -1;
                            }else
                            {
                                page->bit_presencia = 0;
                                tabla_paginas->paginas_en_memoria -= 1;
                                tabla_paginas->ultimo_Clock = index + 1;
                                list_iterator_destroy(list_iterator);
                                return page->marco_asignado;
                            }
                        }
                    }
                    page->bit_uso = 0;
                    page = list_iterator_next(list_iterator);
                    index ++;
                }

            }

            index = 0;
            list_iterator_destroy(list_iterator);

        }

    }else 
    {

        while(marco == -1){

            //CLOCK M con fija -> tabla de paginas tiene el clock, las paginas tienen uso y modificado

            t_list_iterator *list_iterator = list_iterator_create(reemplazo_CLOCK->Clock);
            t_pagina* page = malloc(sizeof(t_pagina));

            while(list_iterator_has_next(list_iterator)){

                page = list_iterator_next(list_iterator);
            
                while(index < reemplazo_CLOCK->ultimo){
                    page = list_iterator_next(list_iterator);
                    index ++;
                }

                if(page->bit_presencia == 1){

                    if(page->bit_modificado == 0 && page->bit_uso == 0){

                        //Ver de cambiar esto despues de swapear
                        marco = enviarPaginaSwap(page);
                        if(marco <0){
                            return -1;
                        }else
                        {
                            page->bit_presencia = 0;
                            tabla_paginas->paginas_en_memoria -= 1;
                            reemplazo_CLOCK->ultimo = index + 1;
                            list_iterator_destroy(list_iterator);
                            return page->marco_asignado;
                        }
                    }

                }

                index++;

            }

            // Termine la primera media vuelta, ahora hago lo mismo hasta ultimo

            index = 0;
            list_iterator_destroy(list_iterator);

            list_iterator = list_iterator_create(reemplazo_CLOCK->Clock);

            while(list_iterator_has_next(list_iterator)){

                page = list_iterator_next(list_iterator);

                while(index < reemplazo_CLOCK->ultimo){
                    if(page->bit_presencia == 1){

                        if(page->bit_modificado == 0 && page->bit_uso == 0){

                            //Ver de cambiar esto despues de swapear
                            marco = enviarPaginaSwap(page);
                            if(marco <0){
                                return -1;
                            }else
                            {
                                page->bit_presencia = 0;
                                tabla_paginas->paginas_en_memoria -= 1;
                                reemplazo_CLOCK->ultimo = index + 1;
                                list_iterator_destroy(list_iterator);
                                return page->marco_asignado;
                            }
                        }
                    }
                    page = list_iterator_next(list_iterator);
                    index ++;
                }


                //Arranco segunda vuelta

                if(page->bit_presencia == 1){

                    if(page->bit_modificado == 1 && page->bit_uso == 0){

                        //Ver de cambiar esto despues de swapear
                        marco = enviarPaginaSwap(page);
                        if(marco <0){
                            return -1;
                        }else
                        {
                            page->bit_presencia = 0;
                            tabla_paginas->paginas_en_memoria -= 1;
                            reemplazo_CLOCK->ultimo = index + 1;
                            list_iterator_destroy(list_iterator);
                            return page->marco_asignado;
                        }
                    }
                }

                page->bit_uso = 0;

                index ++;

            }

            index = 0;
            list_iterator_destroy(list_iterator);

            list_iterator = list_iterator_create(reemplazo_CLOCK->Clock);

            while(list_iterator_has_next(list_iterator)){

                page = list_iterator_next(list_iterator);

                while(index < reemplazo_CLOCK->ultimo){
                    if(page->bit_presencia == 1){

                        if(page->bit_modificado == 1 && page->bit_uso == 0){

                            //Ver de cambiar esto despues de swapear
                            marco = enviarPaginaSwap(page);
                            if(marco <0){
                                return -1;
                            }else
                            {
                                page->bit_presencia = 0;
                                tabla_paginas->paginas_en_memoria -= 1;
                                reemplazo_CLOCK->ultimo = index + 1;
                                list_iterator_destroy(list_iterator);
                                return page->marco_asignado;
                            }
                        }
                    }
                    page->bit_uso = 0;
                    page = list_iterator_next(list_iterator);
                    index ++;
                }

            }

            index = 0;
            list_iterator_destroy(list_iterator);
        }

    }

}

void actualizarModificado(uint32_t nro_pagina, uint32_t carpincho_id){

    t_clock *clockM = malloc(sizeof(t_clock));
    int index = 0;
    
    
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "FIJA") == 0){

        t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
        t_list_iterator* list_iterator = list_iterator_create(tabla_paginas->Clock);

        while(list_iterator_has_next(list_iterator)){
            clockM = list_iterator_next(list_iterator);
            if(clockM->pid == carpincho_id && clockM->numero_pagina == nro_pagina){
                clockM->bit_modificado = 1;
                clockM = list_replace(tabla_paginas->Clock,index,clockM);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }

        list_iterator_destroy(list_iterator);

        //No se encontro la pagina en clock
        clockM->pid = carpincho_id;
        clockM->numero_pagina = nro_pagina;
        clockM->bit_modificado = 1;
        clockM->bit_referido = 0;
        list_add(tabla_paginas->Clock,clockM);
    }else
    {
        t_list_iterator* list_iterator = list_iterator_create(reemplazo_CLOCK);

        while(list_iterator_has_next(list_iterator)){
            clockM = list_iterator_next(list_iterator);
            if(clockM->pid == carpincho_id && clockM->numero_pagina == nro_pagina){
                clockM->bit_modificado = 1;
                clockM = list_replace(reemplazo_CLOCK,index,clockM);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }

        list_iterator_destroy(list_iterator);

        //No se encontro la pagina en clock
        clockM->pid = carpincho_id;
        clockM->numero_pagina = nro_pagina;
        clockM->bit_modificado = 1;
        clockM->bit_referido = 0;
        list_add(reemplazo_CLOCK,clockM);
        
    }
        
}

void actualizarReferido(uint32_t nro_pagina, uint32_t carpincho_id){

    t_clock *clockM = malloc(sizeof(t_clock));
    int index = 0;
    
    
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "FIJA") == 0){

        t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
        t_list_iterator* list_iterator = list_iterator_create(tabla_paginas->Clock);

        while(list_iterator_has_next(list_iterator)){
            clockM = list_iterator_next(list_iterator);
            if(clockM->pid == carpincho_id && clockM->numero_pagina == nro_pagina){
                clockM->bit_referido = 1;
                clockM = list_replace(tabla_paginas->Clock,index,clockM);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }

        list_iterator_destroy(list_iterator);

        //No se encontro la pagina en clock
        clockM->pid = carpincho_id;
        clockM->numero_pagina = nro_pagina;
        clockM->bit_modificado = 0;
        clockM->bit_referido = 1;
        list_add(tabla_paginas->Clock,clockM);
    }else
    {
        t_list_iterator* list_iterator = list_iterator_create(reemplazo_CLOCK);

        while(list_iterator_has_next(list_iterator)){
            clockM = list_iterator_next(list_iterator);
            if(clockM->pid == carpincho_id && clockM->numero_pagina == nro_pagina){
                clockM->bit_referido = 1;
                clockM = list_replace(reemplazo_CLOCK,index,clockM);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }

        list_iterator_destroy(list_iterator);

        //No se encontro la pagina en clock
        clockM->pid = carpincho_id;
        clockM->numero_pagina = nro_pagina;
        clockM->bit_modificado = 0;
        clockM->bit_referido = 1;
        list_add(reemplazo_CLOCK,clockM);
        
    }
        
}



void consultaSwap(int carpincho_id) {
    t_paquete *paquete = serializar_consulta_swap(carpincho_id);
    //Enviar a swap y ver como esperar respuesta
    return;
}
