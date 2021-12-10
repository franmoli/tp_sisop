#include "memoria-virtual.h"

void agregarAsignacion(t_pagina* pagina){
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){

        if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
            t_tabla_paginas* tabla = buscarTablaPorPID(pagina->carpincho_id);
            list_add(tabla->Clock,pagina);
        }else 
        {
            list_add(reemplazo_CLOCK,pagina);
        }
    }else
    {
        if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){
            t_tabla_paginas* tabla = buscarTablaPorPID(pagina->carpincho_id);
            list_add(tabla->Lru,pagina);
        }else
        {
            list_add(reemplazo_LRU,pagina);
        }
    }
    return;
}

int reemplazarPagina(t_tabla_paginas* tabla){
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
        //SWAP CLOCK
    }
    else{
        //SWAP LRU
        int marco = reemplazarLRU(tabla);
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

int enviarPaginaSwap(t_pagina* pagina){

    //t_contenidos_pagina* contenido = list_get(pagina->listado_de_contenido,0);
    uint32_t inicio = tamanio_memoria;
    t_pagina_swap *pagina_swap = malloc(sizeof(t_pagina_swap));
    pagina_swap->tipo_contenido = AMBOS;
    pagina_swap->contenido_heap_info = list_create();
    pagina_swap->contenido_carpincho_info = list_create();
    pagina_swap->pid = pagina->carpincho_id;
    pagina_swap->numero_pagina = pagina->numero_pagina;

    t_list_iterator *list_iterator = list_iterator_create(pagina->listado_de_contenido);

    while(list_iterator_has_next(list_iterator)){

        t_contenidos_pagina* contenido = list_iterator_next(list_iterator);

        if(contenido->contenido_pagina == RESTO_CONTENIDO || contenido->contenido_pagina == CONTENIDO){
            int offset = (contenido->dir_comienzo - inicio) % config_memoria->TAMANIO_PAGINA;
            char* cont = traerDeMemoria(pagina->marco_asignado, offset, contenido->tamanio);
            t_info_carpincho_swap* contenido_swap = malloc(sizeof(t_info_carpincho_swap));
            contenido_swap->size = contenido->tamanio;
            contenido_swap->inicio = contenido->dir_comienzo;
            contenido_swap->fin = contenido->dir_fin;
            contenido_swap->contenido = cont;
            list_add(pagina_swap->contenido_carpincho_info,contenido_swap);
        }else
        {
            t_heap_metadata* heap = traerAllocIncompleto(pagina->marco_asignado,contenido->dir_comienzo,contenido->dir_fin);
            t_info_heap_swap* heap_swap = malloc(sizeof(t_info_heap_swap));
            heap_swap->inicio = contenido->dir_comienzo;
            heap_swap->fin = contenido->dir_fin;
            heap_swap->contenido = heap;
            list_add(pagina_swap->contenido_heap_info,heap_swap);
        }

    }

    t_paquete *paquete;
    //t_buffer *new_buffer = malloc(sizeof(t_buffer));
    paquete = serializar(SWAPSAVE,12,INT,pagina_swap->tipo_contenido,INT,pagina_swap->pid,INT,pagina_swap->numero_pagina
                ,LIST,SWAP_PAGINA_HEAP,(pagina_swap->contenido_heap_info),LIST,SWAP_PAGINA_CONTENIDO,(pagina_swap->contenido_carpincho_info));
    //void *pagina_serial = serializar_pagina(pagina_swap);
    

    deserializar(paquete,10,INT,&(pagina_swap->tipo_contenido),INT,&(pagina_swap->pid),INT,&(pagina_swap->numero_pagina)
                ,LIST,&(pagina_swap->contenido_heap_info),LIST,&(pagina_swap->contenido_carpincho_info));
    
    enviar_paquete(paquete, socket_cliente_swap);
    
    paquete = recibir_paquete(socket_cliente_swap);
    if(paquete->codigo_operacion== OP_CONFIRMADA){
        //PUDO GUARDAR
    }
    else{
        //NO PUDO GUARDAR
    }
    return 1;

}

int reemplazarLRU(t_tabla_paginas* tabla){
    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){

        t_pagina* old = list_get(tabla->Lru,0);
        enviarPaginaSwap(old);

        /*
        t_tabla_paginas* tabla = buscarTablaPorPID(pagina->carpincho_id);
        t_pagina* old = list_get(tabla->Lru,0);
        //Serializar pagina
        //Enviar a swap
        pagina->marco_asignado = old->marco_asignado;
        list_remove(tabla->Lru,0);
        list_add(tabla->Lru,pagina);
        return pagina->marco_asignado;*/
    }else
    {   /*
        t_pagina* old = list_get(reemplazo_LRU,0);
        pagina->marco_asignado = old->marco_asignado;
        list_remove(reemplazo_LRU,0);
        list_add(reemplazo_LRU,pagina);
        return pagina->marco_asignado;*/
        return -1;
    }
}

void actualizarLRU(t_pagina* pagina){

    pagina->bit_uso = 1;
    int index = 0;

    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){

        t_tabla_paginas *tabla = buscarTablaPorPID(pagina->carpincho_id);
        t_list_iterator *list_iterator = list_iterator_create(tabla->Lru);
        t_pagina *pag = malloc(sizeof(t_pagina));

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

int reemplazarClockM(t_pagina* pagina){

    int index = 0;
    int numeroMarco = -1;


    if(strcmp(config_memoria->TIPO_ASIGNACION, "FIJA") == 0){

        while(numeroMarco == -1){

            //CLOCK M con fija -> tabla de paginas tiene el clock, las paginas tienen uso y modificado

            t_tabla_paginas* tabla_paginas = buscarTablaPorPID(pagina->carpincho_id);

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
                    page->bit_presencia = 0;
                    pagina->bit_presencia = 1;
                    pagina->marco_asignado = page->marco_asignado;
                    list_replace(tabla_paginas->Clock,index,pagina);
                    tabla_paginas->ultimo_Clock = index + 1;
                    numeroMarco = pagina->marco_asignado;
                    list_iterator_destroy(list_iterator);
                    //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                    return pagina->marco_asignado;
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

                        //Ver de cambiar esto despues de swapear
                        page->bit_presencia = 0;
                        pagina->bit_presencia = 1;
                        pagina->marco_asignado = page->marco_asignado;
                        list_replace(tabla_paginas->Clock,index,pagina);
                        tabla_paginas->ultimo_Clock = index + 1;
                        numeroMarco = pagina->marco_asignado;
                        list_iterator_destroy(list_iterator);
                        //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                        return pagina->marco_asignado; 
                        }
                    }
                    page = list_iterator_next(list_iterator);
                    index ++;
                }


                //Arranco segunda vuelta

                if(page->bit_presencia == 1){

                    if(page->bit_modificado == 1 && page->bit_uso == 0){

                    //Ver de cambiar esto despues de swapear
                    page->bit_presencia = 0;
                    pagina->bit_presencia = 1;
                    pagina->marco_asignado = page->marco_asignado;
                    list_replace(tabla_paginas->Clock,index,pagina);
                    tabla_paginas->ultimo_Clock = index + 1;
                    numeroMarco = pagina->marco_asignado;
                    list_iterator_destroy(list_iterator);
                    //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                    return pagina->marco_asignado; 
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
                        page->bit_presencia = 0;
                        pagina->bit_presencia = 1;
                        pagina->marco_asignado = page->marco_asignado;
                        list_replace(tabla_paginas->Clock,index,pagina);
                        tabla_paginas->ultimo_Clock = index + 1;
                        numeroMarco = pagina->marco_asignado;
                        list_iterator_destroy(list_iterator);
                        //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                        return pagina->marco_asignado; 
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

        while(numeroMarco == -1){

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
                    page->bit_presencia = 0;
                    pagina->bit_presencia = 1;
                    pagina->marco_asignado = page->marco_asignado;
                    list_replace(reemplazo_CLOCK->Clock,index,pagina);
                    reemplazo_CLOCK->ultimo = index + 1;
                    numeroMarco = pagina->marco_asignado;
                    list_iterator_destroy(list_iterator);
                    //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                    return pagina->marco_asignado;
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
                        page->bit_presencia = 0;
                        pagina->bit_presencia = 1;
                        pagina->marco_asignado = page->marco_asignado;
                        list_replace(reemplazo_CLOCK->Clock,index,pagina);
                        reemplazo_CLOCK->ultimo = index + 1;
                        numeroMarco = pagina->marco_asignado;
                        list_iterator_destroy(list_iterator);
                        //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                        return pagina->marco_asignado; 
                        }
                    }
                    page = list_iterator_next(list_iterator);
                    index ++;
                }


                //Arranco segunda vuelta

                if(page->bit_presencia == 1){

                    if(page->bit_modificado == 1 && page->bit_uso == 0){

                    //Ver de cambiar esto despues de swapear
                    page->bit_presencia = 0;
                    pagina->bit_presencia = 1;
                    pagina->marco_asignado = page->marco_asignado;
                    list_replace(reemplazo_CLOCK->Clock,index,pagina);
                    reemplazo_CLOCK->ultimo = index + 1;
                    numeroMarco = pagina->marco_asignado;
                    list_iterator_destroy(list_iterator);
                    //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                    return pagina->marco_asignado; 
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
                        page->bit_presencia = 0;
                        pagina->bit_presencia = 1;
                        pagina->marco_asignado = page->marco_asignado;
                        list_replace(reemplazo_CLOCK->Clock,index,pagina);
                        reemplazo_CLOCK->ultimo = index + 1;
                        numeroMarco = pagina->marco_asignado;
                        list_iterator_destroy(list_iterator);
                        //Devuelvo el marco debido a que con eso despues de llamar a swap hago el cambio en la lista
                        return pagina->marco_asignado; 
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
