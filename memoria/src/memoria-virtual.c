#include "memoria-virtual.h"

/*
t_pagina* reemplazarPagina(t_pagina* paginaAgregar, int carpincho_id){

    t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
        //SWAP CLOCK
    }
    else{
        //SWAP LRU
        log_info(logger_memoria,"AGREGADO A TABLA LRU PAGINA: %d", paginaAgregar->numero_pagina);
        int marco = eliminarPrimerElementoLista(carpincho_id);
        paginaAgregar->marco_asignado = marco;
        list_add(tabla_paginas->Lru,paginaAgregar);
    }
    return NULL;
}*/

int reemplazarPagina(int nro_pagina, int carpincho_id){
    t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
        //SWAP CLOCK
    }
    else{
        //SWAP LRU
        int marco = reemplazarLRU(nro_pagina,carpincho_id);
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

int reemplazarLRU(int nro_pagina, int carpincho_id){

    t_reemplazo* paginaNueva = malloc(sizeof(t_reemplazo));
    paginaNueva->numero_pagina = nro_pagina;
    paginaNueva->pid = carpincho_id;
    int numeroMarco = -1;

    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "FIJA") == 0){

        //LRU CON FIJA
        t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
        t_reemplazo* old = malloc(sizeof(t_reemplazo));
        old = list_get(tabla_paginas->Lru,0);
        t_pagina* oldPage = getPaginaByNumero(old->numero_pagina,old->pid);

        numeroMarco = oldPage->marco_asignado;

        list_remove(tabla_paginas->Lru,0);
        list_add(tabla_paginas->Lru,paginaNueva);
        
    } else
    {
        //LRU CON GLOBAL
        t_reemplazo* paginaOld = malloc(sizeof(t_pagina));
        paginaOld = list_get(reemplazo_LRU,0);
        t_pagina* paginaVieja = getPaginaByNumero(paginaOld->numero_pagina,paginaOld->pid);

        numeroMarco = paginaVieja->marco_asignado;

        list_remove(reemplazo_LRU,0);
        list_add(reemplazo_LRU,paginaNueva);
        
    }
    return numeroMarco;

}

void actualizarLRU(int nro_pagina, int carpincho_id){

    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "FIJA") == 0){
        //LRU CON FIJA
        t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
        t_reemplazo* pagina = malloc(sizeof(t_reemplazo));
        t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->Lru);
        int index = 0;

        while (list_iterator_has_next(list_iterator))
        {
            pagina = list_iterator_next(list_iterator);
            if(pagina->numero_pagina == nro_pagina){
                list_remove(tabla_paginas->Lru,index);
                list_add(tabla_paginas->Lru,pagina);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }
        return;

        
    }else
    {
        //LRU CON GLOBAL
        t_reemplazo* pagina = malloc(sizeof(t_reemplazo));
        t_list_iterator *list_iterator = list_iterator_create(reemplazo_LRU);
        int index = 0;

        while (list_iterator_has_next(list_iterator))
        {
            pagina = list_iterator_next(list_iterator);
            if(pagina->pid == carpincho_id && pagina->numero_pagina == nro_pagina){
                list_remove(reemplazo_LRU,index);
                list_add(reemplazo_LRU,pagina);
                list_iterator_destroy(list_iterator);
                return;
            }
            index++;
        }
        return;
    }

}

int reemplazarClockM(int nro_pagina, int carpincho_id, int referido, int modificado) {

    t_clock* paginaNueva = malloc(sizeof(t_clock));
    t_clock* pagina = malloc(sizeof(t_clock));
    t_clock* modificar = malloc(sizeof(t_clock));
    t_clock *ultimoClock = malloc(sizeof(t_clock));
    paginaNueva->numero_pagina = nro_pagina;
    paginaNueva->pid = carpincho_id;
    paginaNueva->bit_referido = referido;       //Definir referido
    paginaNueva->bit_modificado = modificado;   //Definir modificado
    int numeroMarco = -1;
    int inicioLista = reemplazo_CLOCK->ultimo;
    int index = 0;


    if(strcmp(config_memoria->ALGORITMO_REEMPLAZO_MMU, "FIJA") == 0){
        //CLock para asignacion fija
        t_tabla_paginas* tabla_paginas = buscarTablaPorPID(carpincho_id);
        t_list_iterator *list_iterator = list_iterator_create(tabla_paginas->Clock);

        while(numeroMarco = -1){

            while(list_iterator_has_next(list_iterator))
            {
                pagina = list_iterator_next(list_iterator);
                ultimoClock = list_get(tabla_paginas->Clock,tabla_paginas->ultimo_Clock);
                while(pagina->pid != ultimoClock->pid && pagina->numero_pagina != ultimoClock->numero_pagina){
                    pagina = list_iterator_next(list_iterator);
                    index = index + 1;
                }
                //Me ubico en el ultimo clock para empezar a recorrer
                if(pagina->bit_referido == 0 && pagina->bit_modificado == 0){
                    t_clock *clockOld = list_replace(tabla_paginas->Clock,tabla_paginas->ultimo_Clock,paginaNueva);
                    t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                    numeroMarco = paginaVieja->marco_asignado;
                    reemplazo_CLOCK->ultimo = index + 1;

                    list_iterator_destroy(list_iterator);
                    return numeroMarco;
                }
                index++;
            }

            list_iterator_destroy(list_iterator);

            //Termine de iterar la lista -> vuelvo a empezar hasta ultimo

            list_iterator = list_iterator_create(tabla_paginas->Clock);
            pagina = list_iterator_next(list_iterator);
            while(pagina->pid != ultimoClock->pid && pagina->numero_pagina != ultimoClock->numero_pagina){

                if(pagina->bit_referido == 0 && pagina->bit_modificado == 0){
                    t_clock *clockOld = list_replace(tabla_paginas->Clock,tabla_paginas->ultimo_Clock,paginaNueva);
                    t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                    numeroMarco = paginaVieja->marco_asignado;
                    reemplazo_CLOCK->ultimo = index + 1;

                    list_iterator_destroy(list_iterator);
                    return numeroMarco;
                }

                pagina = list_iterator_next(list_iterator);
                index++;

            }

            list_iterator_destroy(list_iterator);
            index = 0;

            //Di primera vuelta y no pude agarrar a nadie
            list_iterator = list_iterator_create(tabla_paginas->Clock);

            while(list_iterator_has_next(list_iterator))
            {
                pagina = list_iterator_next(list_iterator);
                ultimoClock = list_get(tabla_paginas->Clock,tabla_paginas->ultimo_Clock);
                while(pagina->pid != ultimoClock->pid && pagina->numero_pagina != ultimoClock->numero_pagina){
                    pagina = list_iterator_next(list_iterator);
                    index++;
                }
                //Me ubico en el ultimo clock para empezar a recorrer
                if(pagina->bit_referido == 0){
                    t_clock *clockOld = list_replace(tabla_paginas->Clock,tabla_paginas->ultimo_Clock,paginaNueva);
                    t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                    numeroMarco = paginaVieja->marco_asignado;
                    reemplazo_CLOCK->ultimo = index + 1;

                    list_iterator_destroy(list_iterator);
                    return numeroMarco;
                }

                modificar = list_get(tabla_paginas->Clock,index);
                modificar->bit_referido = 0;
                modificar = list_replace(tabla_paginas->Clock,index,modificar);

                index++;
            }

            list_iterator_destroy(list_iterator);
            index = 0;

            list_iterator = list_iterator_create(tabla_paginas->Clock);
            pagina = list_iterator_next(list_iterator);
            while(pagina->pid != ultimoClock->pid && pagina->numero_pagina != ultimoClock->numero_pagina){

                if(pagina->bit_referido == 0){
                    t_clock *clockOld = list_replace(tabla_paginas->Clock,tabla_paginas->ultimo_Clock,paginaNueva);
                    t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                    numeroMarco = paginaVieja->marco_asignado;
                    reemplazo_CLOCK->ultimo = index + 1;

                    list_iterator_destroy(list_iterator);
                    return numeroMarco;
                }

                modificar = list_get(tabla_paginas->Clock,index);
                modificar->bit_referido = 0;
                modificar = list_replace(tabla_paginas->Clock,index,modificar);

                pagina = list_iterator_next(list_iterator);
                index++;

            }

            index = 0;
            list_iterator_destroy(list_iterator);

        }

    }else
    {
        
        //Clock para asignacion dinamica -> Supongo que ya estan todas las paginas en la lista
        t_clock* pagina = malloc(sizeof(t_clock));
        t_list_iterator *list_iterator = list_iterator_create(reemplazo_CLOCK->paginas);
        index = reemplazo_CLOCK->ultimo;
        while(numeroMarco == -1){
        while (list_iterator_has_next(list_iterator))
        {
            pagina = list_iterator_next(list_iterator);
            ultimoClock = list_get(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo);
            while(pagina->pid != ultimoClock->pid && pagina->numero_pagina != ultimoClock->numero_pagina){
                pagina = list_iterator_next(list_iterator);
            }
            //Primera vuelta
            if(pagina->bit_referido == 0 && pagina->bit_modificado == 0){
                t_clock *clockOld = list_replace(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo,paginaNueva);
                t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                numeroMarco = paginaVieja->marco_asignado;
                reemplazo_CLOCK->ultimo = index + 1;

                list_iterator_destroy(list_iterator);
                return numeroMarco;
            }
            index++;
        }

        index = 0;
        list_iterator_destroy(list_iterator);

        //Llegue al final de la lista
        //Tengo que volver a empezar en lista hasta que llegue a ultimo y ahi termino primera vuelta
        t_list_iterator *list_iterator = list_iterator_create(reemplazo_CLOCK->paginas);

        while (list_iterator_has_next(list_iterator))
        {
            pagina = list_iterator_next(list_iterator);
            ultimoClock = list_get(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo);

            //Primera vuelta
            if(pagina->bit_referido == 0 && pagina->bit_modificado == 0){
                t_clock *clockOld = list_replace(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo,paginaNueva);
                t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                numeroMarco = paginaVieja->marco_asignado;
                reemplazo_CLOCK->ultimo = index + 1;

                list_iterator_destroy(list_iterator);
                return numeroMarco;
            }

            if(pagina->pid == ultimoClock->pid && pagina->numero_pagina == ultimoClock->numero_pagina){
                while (list_iterator_has_next(list_iterator)){
                    pagina = list_iterator_next(list_iterator);
                }
            }

            index++;
        }

        index = 0;
        list_iterator_destroy(list_iterator);

        //Termine primera vuelta, ahora segunda vuelta es cambiando el bit de uso
        list_iterator = list_iterator_create(reemplazo_CLOCK->paginas);
        while (list_iterator_has_next(list_iterator))
        {
            pagina = list_iterator_next(list_iterator);
            ultimoClock = list_get(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo);
            while(pagina->pid != ultimoClock->pid && pagina->numero_pagina != ultimoClock->numero_pagina){
                pagina = list_iterator_next(list_iterator);
            }
            //Primera vuelta
            if(pagina->bit_referido == 0){
                t_clock *clockOld = list_replace(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo,paginaNueva);
                t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                numeroMarco = paginaVieja->marco_asignado;
                reemplazo_CLOCK->ultimo = index + 1;

                list_iterator_destroy(list_iterator);
                return numeroMarco;
            }
            pagina->bit_referido = 0;
            pagina = list_replace(reemplazo_CLOCK->paginas,index,pagina);


            index++;
        }

        //Termino primera parte de la segunda vuelta
        index = 0;
        list_iterator_destroy(list_iterator);

        list_iterator = list_iterator_create(reemplazo_CLOCK->paginas);

        while (list_iterator_has_next(list_iterator))
        {
            pagina = list_iterator_next(list_iterator);
            ultimoClock = list_get(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo);

            //Primera vuelta
            if(pagina->bit_referido == 0){
                t_clock *clockOld = list_replace(reemplazo_CLOCK->paginas,reemplazo_CLOCK->ultimo,paginaNueva);
                t_pagina* paginaVieja = getPaginaByNumero(clockOld->numero_pagina,clockOld->pid);
                numeroMarco = paginaVieja->marco_asignado;
                reemplazo_CLOCK->ultimo = index + 1;

                list_iterator_destroy(list_iterator);
                return numeroMarco;
            }

            pagina->bit_referido = 0;
            pagina = list_replace(reemplazo_CLOCK->paginas,index,pagina);

            if(pagina->pid == ultimoClock->pid && pagina->numero_pagina == ultimoClock->numero_pagina){
                while (list_iterator_has_next(list_iterator)){
                    pagina = list_iterator_next(list_iterator);
                }
            }

            index++;
        }

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
