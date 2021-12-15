#include "asignaciones.h"

/*
    Selección de un marco que ya se encuentra instanciado
*/
t_entrada_tabla_marcos* seleccionar_marco(int numero_marco) {
    t_entrada_tabla_marcos *marco = NULL;

    for(int i=0; i<list_size(tabla_marcos); i++) {
        t_entrada_tabla_marcos *entrada = list_get(tabla_marcos, i);
        if(entrada->numero_marco == numero_marco) {
            marco = entrada;
            break;
        }
    }

    return marco;
}

/*
    Selección de un marco libre dentro de un determinado archivo
*/
t_entrada_tabla_marcos* seleccionar_marco_libre(int archivo_seleccionado) {
    t_entrada_tabla_marcos *marco = NULL;

    for(int i=0; i<list_size(tabla_marcos); i++) {
        t_entrada_tabla_marcos *entrada = list_get(tabla_marcos, i);
        if(entrada->file == archivo_seleccionado && entrada->esta_libre && entrada->proceso_asignado == -1) {
            marco = entrada;
            break;
        }
    }

    return marco;
}

/*
    Selección de un marco libre de un determinado archivo
    Útil en el caso de asignación fija, ya que reserva los marcos que correspondan para el proceso
*/
t_entrada_tabla_marcos* seleccionar_marco_libre_fija(int proceso, int archivo_seleccionado) {
    t_entrada_tabla_marcos *marco = NULL;

    //Analizo la cantidad de paginas que tiene asignadas un proceso
    int marcos_proceso = 0;
    for(int i=0; i<list_size(lista_paginas_almacenadas); i++) {
        t_pagina_almacenada *pagina = list_get(lista_paginas_almacenadas, i);
        if(pagina->id_proceso == proceso) {
            marcos_proceso++;
        }
    }

    if(marcos_proceso < config_swap->MARCOS_MAXIMOS) {
        //Analizo la cantidad de marcos libres totales y marcos habilitados para el proceso
        int marcos_libres = 0;
        for(int i=0; i<list_size(tabla_marcos); i++) {
            t_entrada_tabla_marcos *entrada = list_get(tabla_marcos, i);
            if(entrada->file == archivo_seleccionado && entrada->esta_libre && entrada->proceso_asignado == proceso) {
                marco = entrada;
                break;
            }
            if(entrada->file == archivo_seleccionado && entrada->esta_libre) {
                marcos_libres++;
            }
        }

        //Analizo si se asignó un marco, si no se asignó, busco el primer marco libre y reservo los próximos para que únicamente el proceso pueda acceder a los mismos
        if(marco == NULL && marcos_libres >= (config_swap->MARCOS_MAXIMOS - marcos_proceso)) {
            marco = seleccionar_marco_libre(archivo_seleccionado);
            marcos_libres--;
            
            while(marcos_libres > config_swap->MARCOS_MAXIMOS) {
                t_entrada_tabla_marcos *marco_libre = seleccionar_marco_libre(archivo_seleccionado);
                marco_libre->proceso_asignado = proceso;

                marcos_libres--;
            }

            marco->esta_libre = 0;
        }
    }

    return marco;
}

/*
    Creación de un marco según el esquema de asignación global
    La disponibilidad se setea en 0 al comienzo, ya que se instanciaran los marcos a demanda
*/
t_entrada_tabla_marcos* instanciar_marco_global(int file) {
    t_entrada_tabla_marcos *nuevo_marco = malloc(sizeof(t_entrada_tabla_marcos));
    nuevo_marco->file = file;
    nuevo_marco->esta_libre = 0;
    nuevo_marco->proceso_asignado = -1;
    
    //Seteo el numero de marco
    int numero_marco = contador_marcos++;
    nuevo_marco->numero_marco = numero_marco;
    
    //Obtengo la posición base del marco
    int base = 0;
    for(int i=0; i<list_size(tabla_marcos); i++) {
        t_entrada_tabla_marcos *marco = list_get(tabla_marcos, i);
        if(marco->file == file && (marco->base + config_swap->TAMANIO_PAGINA) > base) {
            base = marco->base + config_swap->TAMANIO_PAGINA;
        }
    }
    nuevo_marco->base = base;

    list_add(tabla_marcos, nuevo_marco);
    log_info(logger_swap, "Se ha instanciado el marco %d correctamente con base %dB (file %d)", numero_marco, base, file);

    return nuevo_marco;
}

/*
    Creación de marcos según el esquema de asignación fija
    La disponibilidad se seta en 1, ya que se irán asignando las páginas a medida que lleguen
*/
void instanciar_marcos_fija(int file) {
    int base_actual = 0;

    for(int i=0; i<(config_swap->TAMANIO_SWAP/config_swap->TAMANIO_PAGINA); i++) {
        t_entrada_tabla_marcos *nuevo_marco = malloc(sizeof(t_entrada_tabla_marcos));
        nuevo_marco->file = file;
        nuevo_marco->esta_libre = 1;
        nuevo_marco->numero_marco = contador_marcos++;
        nuevo_marco->base = base_actual;
        nuevo_marco->proceso_asignado = -1;

        list_add(tabla_marcos, nuevo_marco);
        log_info(logger_swap, "Se ha instanciado el marco %d correctamente con base %dB (file %d)", contador_marcos, base_actual, file);

        base_actual = base_actual + config_swap->TAMANIO_PAGINA;
    }
}

/*
    Almacenamiento de una página según el esquema de asignación global
*/
bool asignacion_global_de_pagina(int posicion_archivo, char *path_archivo, int archivo, t_pagina_enviada_swap *pagina) {
    bool operacion_concretada = 1;

    //Almaceno el tamaño disponible del archivo
    t_informacion_archivo *informacion_archivo = list_get(archivos_abiertos, posicion_archivo);
    log_info(logger_swap, "Archivo seleccionado: %s (%dB de espacio disponible)", path_archivo, informacion_archivo->espacio_disponible);

    //Si es la primera pagina a insertar en el archivo, genero el mapeo del archivo correspondiente
    if(informacion_archivo->espacio_disponible == config_swap->TAMANIO_SWAP) {
        void *mapeo_archivo = mmap(NULL, config_swap->TAMANIO_SWAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, archivo, 0);
        list_add(lista_mapeos, mapeo_archivo);
    }

    //Obtengo el mapeo correspondiente al archivo
    void *mapping = list_get(lista_mapeos, posicion_archivo);
    if(mapping == MAP_FAILED){
        log_error(logger_swap, "El mapeado de la pagina fallo, revise el codigo. Abortanto ejecucion.");
        exit(-1);
    }

    //Selecciono el offset de inicio y de final
    int offset;
    t_entrada_tabla_marcos *marco_seleccionado = seleccionar_marco_libre(posicion_archivo);
    if(marco_seleccionado != NULL) {
        offset = marco_seleccionado->base;
    } else {
        marco_seleccionado = instanciar_marco_global(posicion_archivo);
        offset = marco_seleccionado->base;
    }

    //Mapeo los datos de la pagina
    log_info(logger_swap, "Escribiendo la pagina %d en el archivo %s en el marco %d", pagina->numero_pagina, path_archivo, marco_seleccionado->numero_marco);
    
    t_list *sizes_contenidos = list_create();
    for(int i=0; i<list_size(pagina->heap_contenidos); i++) {
        t_heap_contenido_enviado *contenido_heap = list_get(pagina->heap_contenidos, i);

        memcpy(mapping + offset, &(contenido_heap->prevAlloc), sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(mapping + offset, &(contenido_heap->nextAlloc), sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(mapping + offset, &(contenido_heap->isFree), sizeof(uint8_t));
        offset += sizeof(uint8_t);

        int strlen_contenido = strlen(contenido_heap->contenido);
        list_add(sizes_contenidos, strlen_contenido);
        memcpy(mapping + offset, contenido_heap->contenido, strlen_contenido);
        offset += strlen_contenido;
    }

    //Añado los datos de la página a la estructura administrativa
    t_pagina_almacenada *pagina_almacenada = malloc(sizeof(t_pagina_almacenada));
    pagina_almacenada->numero_pagina = pagina->numero_pagina;
    pagina_almacenada->id_proceso = pagina->pid;
    pagina_almacenada->marco = marco_seleccionado;
    pagina_almacenada->cantidad_contenidos = list_size(pagina->heap_contenidos);
    pagina_almacenada->sizes_contenidos = sizes_contenidos;
    list_add(lista_paginas_almacenadas, pagina_almacenada);

    //Escribo el archivo
    escribir_en_archivo_swap(archivo, mapping, offset);

    //Actualizo la estructura administrativa de los archivos
    informacion_archivo->espacio_disponible = informacion_archivo->espacio_disponible - config_swap->TAMANIO_PAGINA;

    return operacion_concretada;
}

/*
    Almacenamiento de una página según el esquema de asignación fija
*/
bool asignacion_fija_de_pagina(int posicion_archivo, char *path_archivo, int archivo, t_pagina_enviada_swap *pagina) {
    bool operacion_concretada = 1;

    //Almaceno el tamaño disponible del archivo
    t_informacion_archivo *informacion_archivo = list_get(archivos_abiertos, posicion_archivo);
    log_info(logger_swap, "Archivo seleccionado: %s (%dB de espacio disponible)", path_archivo, informacion_archivo->espacio_disponible);

    //Si es la primera pagina a insertar en el archivo, genero el mapeo del archivo correspondiente
    if(informacion_archivo->espacio_disponible == config_swap->TAMANIO_SWAP) {
        void *mapeo_archivo = mmap(NULL, config_swap->TAMANIO_SWAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, archivo, 0);
        list_add(lista_mapeos, mapeo_archivo);
    }

    //Obtengo el mapeo correspondiente al archivo
    void *mapping = list_get(lista_mapeos, posicion_archivo);
    if(mapping == MAP_FAILED){
        log_error(logger_swap, "El mapeado de la pagina fallo, revise el codigo. Abortanto ejecucion.");
        exit(-1);
    }

    //Selecciono el offset de inicio y de final
    int offset;
    int id_carpincho = 0;
    t_entrada_tabla_marcos *marco_seleccionado = seleccionar_marco_libre_fija(id_carpincho, posicion_archivo);
    if(marco_seleccionado != NULL) {
        offset = marco_seleccionado->base;

        //Mapeo los datos de la pagina
        log_info(logger_swap, "Escribiendo la pagina %d en el archivo %s en el marco %d", pagina->numero_pagina, path_archivo, marco_seleccionado->numero_marco);
    
        t_list *sizes_contenidos = list_create();
        for(int i=0; i<list_size(pagina->heap_contenidos); i++) {
            t_heap_contenido_enviado *contenido_heap = list_get(pagina->heap_contenidos, i);

            memcpy(mapping + offset, &(contenido_heap->prevAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(mapping + offset, &(contenido_heap->nextAlloc), sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(mapping + offset, &(contenido_heap->isFree), sizeof(uint8_t));
            offset += sizeof(uint8_t);

            int strlen_contenido = strlen(contenido_heap->contenido);
            list_add(sizes_contenidos, strlen_contenido);
            memcpy(mapping + offset, &(contenido_heap->contenido), strlen_contenido);
            offset += strlen_contenido;
        }

        //Añado los datos de la página a la estructura administrativa
        t_pagina_almacenada *pagina_almacenada = malloc(sizeof(t_pagina_almacenada));
        pagina_almacenada->numero_pagina = pagina->numero_pagina;
        pagina_almacenada->id_proceso = pagina->pid;
        pagina_almacenada->marco = marco_seleccionado;
        pagina_almacenada->cantidad_contenidos = list_size(pagina->heap_contenidos);
        pagina_almacenada->sizes_contenidos = sizes_contenidos;
        list_add(lista_paginas_almacenadas, pagina_almacenada);

        //Escribo el archivo
        escribir_en_archivo_swap(archivo, mapping, offset);

        //Actualizo la estructura administrativa de los archivos
        informacion_archivo->espacio_disponible = informacion_archivo->espacio_disponible - config_swap->TAMANIO_PAGINA;
    } else {
        log_error(logger_swap, "No se ha podido almacenar la pagina debido a que los marcos disponibles no son suficientes para reservarle %d marcos al proceso.", config_swap->MARCOS_MAXIMOS);
        operacion_concretada = 0;
    }

    return operacion_concretada;
}