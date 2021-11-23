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
        if(entrada->file == archivo_seleccionado && entrada->esta_libre) {
            marco = entrada;
            break;
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
    log_info(logger_swap, "Se ha instanciado el marco %d correctamente con base %dB", numero_marco, base);

    return nuevo_marco;
}

/*
    Almacenamiento de una página según el esquema de asignación global
*/
void asignacion_global_de_pagina(int current_file_size, int posicion_archivo, char *path_archivo, int archivo, t_pagina *pagina) {
    //Almaceno el tamaño disponible del archivo
    int tamanio_disponible = config_swap->TAMANIO_SWAP - current_file_size;
    log_info(logger_swap, "Archivo seleccionado: %s (%dB de espacio disponible)", path_archivo, tamanio_disponible);

    //Si es la primera pagina a insertar en el archivo, genero el mapeo del archivo correspondiente
    if(tamanio_disponible == config_swap->TAMANIO_SWAP) {
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
    pagina->marco->numero_marco = marco_seleccionado->numero_marco;

    //Mapeo los datos de la pagina
    log_info(logger_swap, "Escribiendo la pagina %d en el archivo %s en el marco %d", pagina->numero_pagina, path_archivo, marco_seleccionado->numero_marco);
    memcpy(mapping + offset, &(pagina->numero_pagina), sizeof(pagina->numero_pagina));
    offset += sizeof(pagina->numero_pagina);
    memcpy(mapping + offset, &(pagina->marco->numero_marco), sizeof(pagina->marco->numero_marco));
    offset += sizeof(pagina->marco->numero_marco);

    //Añado los datos de la página a la estructura administrativa
    t_pagina_almacenada *pagina_almacenada = malloc(sizeof(t_pagina_almacenada));
    pagina_almacenada->numero_pagina = pagina->numero_pagina;
    pagina_almacenada->id_proceso = pagina->id_carpincho;
    pagina_almacenada->marco = marco_seleccionado;
    list_add(lista_paginas_almacenadas, pagina_almacenada);

    //Escribo el archivo
    escribir_en_archivo_swap(archivo, mapping, offset);
}