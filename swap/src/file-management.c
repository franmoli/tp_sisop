#include "file-management.h"

/*
    Inicialización de archivos de SWAP
*/
void crear_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        //Creo un archivo
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        char *init_value = '\0';

        FILE *file = fopen(path, "wb");
        fwrite(&init_value, 0, 1, file);

        fclose(file);

        //Agrego el archivo creado a la lista de archivos abiertos
        t_informacion_archivo *informacion_archivo = malloc(sizeof(t_informacion_archivo));
        informacion_archivo->numero_archivo = i;
        informacion_archivo->espacio_disponible = config_swap->TAMANIO_SWAP;
        list_add(archivos_abiertos, informacion_archivo);
    }
}

/*
    Escritura en archivos de SWAP
*/
void escribir_en_archivo_swap(int archivo, void *mapping, int offset_final) {
    //Almaceno la página mapeada en el archivo
    ssize_t flag_escritura = write(archivo, mapping, offset_final);
    if(flag_escritura != offset_final){
        log_error(logger_swap, "No se pudo realizar la escritura en el archivo, revise el codigo. Abortando ejecucion.");
        exit(-1);
    }
}

/*
    Selección del archivo que se escribirá/leerá
*/
int seleccionar_archivo_escritura(int proceso_a_guardar, int bytes_a_guardar) {
    int archivo_seleccionado = -1;
    int tamanio_archivo_seleccionado = 0;

    //Analizo si la página pertenece a un proceso que ya fue almacenado previamente
    int archivo_paginas_con_mismo_proceso = -1;
    for(int i=0; i<list_size(lista_paginas_almacenadas); i++) {
        t_pagina_almacenada *pagina = list_get(lista_paginas_almacenadas, i);
        if(pagina->id_proceso == proceso_a_guardar) {
            archivo_paginas_con_mismo_proceso = pagina->marco->file;
            break;
        }
    }

    //Si se encontró un archivo que contenga páginas del mismo proceso, valido en el mismo
    if(archivo_paginas_con_mismo_proceso >= 0) {
        t_informacion_archivo *archivo = list_get(archivos_abiertos, archivo_paginas_con_mismo_proceso);
        //Valido que el archivo cuente con mas bytes de los que necesito almacenar
        if(bytes_a_guardar <= archivo->espacio_disponible) {
            archivo_seleccionado = archivo->numero_archivo;
        } else {
            archivo_seleccionado = -2;
        }
    } else {
        for(int i=0; i<list_size(archivos_abiertos); i++) {
            t_informacion_archivo *archivo = list_get(archivos_abiertos, i);
            //Valido que el archivo cuente con mas bytes de los que necesito almacenar
            if(bytes_a_guardar <= archivo->espacio_disponible) {
                //Si el espacio disponible es mayor al de los archivos previamente recorridos, lo asigno
                if(archivo->espacio_disponible > tamanio_archivo_seleccionado) {
                    tamanio_archivo_seleccionado = archivo->espacio_disponible;
                    archivo_seleccionado = archivo->numero_archivo;
                }
            }
        }
    }

    return archivo_seleccionado;
}

/*
    Inserción de página en archivo de SWAP
*/
void insertar_pagina_en_archivo(t_pagina *pagina) {
    //Selecciono el archivo
    int bytes_a_guardar = config_swap->TAMANIO_PAGINA;
    int posicion_archivo_obtenido = seleccionar_archivo_escritura(pagina->id_carpincho, bytes_a_guardar);

    if(posicion_archivo_obtenido >= 0) {
        //Abro el archivo seleccionado
        char *path_archivo = list_get(config_swap->ARCHIVOS_SWAP, posicion_archivo_obtenido);
        int archivo = open(path_archivo, O_RDWR);
        if(archivo < 0){
            log_error(logger_swap, "No pudo encontrarse el archivo en la ruta especificada (%s). Abortando ejecucion.", path_archivo);
            exit(-1);
        }

        //Obtengo datos del archivo
        struct stat statbuf;
        int archivo_cargado_correctamente = fstat(archivo, &statbuf);
        if(archivo_cargado_correctamente == -1) {
            log_error(logger_swap, "No pudo cargarse el archivo correctamente, revise el codigo. Abortando ejecucion.");
            exit(-1);
        }

        //Analizo el tipo de asignación
        if(strcmp(config_swap->TIPO_ASIGNACION, "GLOBAL") == 0) {
            asignacion_global_de_pagina(statbuf.st_size, posicion_archivo_obtenido, path_archivo, archivo, pagina);
        }

        //Actualizo la estructura administrativa de los archivos
        t_informacion_archivo *informacion_archivo = list_get(archivos_abiertos, posicion_archivo_obtenido);
        informacion_archivo->espacio_disponible = informacion_archivo->espacio_disponible - config_swap->TAMANIO_PAGINA;

        //Cierro el archivo y libero la memoria de la página
        close(archivo);

        log_info(logger_swap, "Pagina %d almacenada en %s", pagina->numero_pagina, path_archivo);
        free(pagina);
    } else if(posicion_archivo_obtenido == -2) {
        log_error(logger_swap, "No se ha podido guardar la pagina %d en el archivo dado que no hay espacio suficiente. No puede almacenarse en otros archivos dado que se encontraron paginas asociadas al mismo proceso (%d) en este archivo.", pagina->numero_pagina, pagina->id_carpincho);
    } else if(posicion_archivo_obtenido == -1) {
        log_error(logger_swap, "No se ha podido guardar la pagina %d en ningun archivo dado que no hay espacio suficiente", pagina->numero_pagina);
    }
}

/*
    Lectura de una página de un archivo de SWAP
*/
void leer_pagina_de_archivo(int numero_pagina) {
    //Me fijo si la página solicitada es una de las páginas almacenadas
    t_pagina_almacenada *informacion_almacenamiento = NULL;
    for(int i=0; i<list_size(lista_paginas_almacenadas); i++) {
        t_pagina_almacenada *pagina_almacenada = list_get(lista_paginas_almacenadas, i);
        if(numero_pagina == pagina_almacenada->numero_pagina) {
            informacion_almacenamiento = pagina_almacenada;
            break;
        }
    }

    //Si la página fue encontrada la voy a buscar, sino lanzo mensaje de aviso
    if(informacion_almacenamiento != NULL) {
        //Lectura del archivo
        char *path_archivo = list_get(config_swap->ARCHIVOS_SWAP, informacion_almacenamiento->marco->file);
        int archivo = open(path_archivo, O_RDONLY);

        struct stat statbuf;
        fstat(archivo, &statbuf);

        //Obtengo la página dentro del archivo
        void *paginas_obtenidas = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, archivo, 0);
        t_pagina pagina_obtenida;
        t_marco marco_pagina;

        int offset_actual = informacion_almacenamiento->marco->base;
        memcpy(&pagina_obtenida.numero_pagina, paginas_obtenidas + offset_actual, sizeof(pagina_obtenida.numero_pagina));
        offset_actual += sizeof(pagina_obtenida.numero_pagina);
        memcpy(&marco_pagina, paginas_obtenidas + offset_actual, sizeof(marco_pagina));
        offset_actual += sizeof(marco_pagina);
        pagina_obtenida.marco = malloc(bytes_marco(marco_pagina));
        pagina_obtenida.marco->numero_marco = marco_pagina.numero_marco;

        printf("Numero de pagina: %d\n", pagina_obtenida.numero_pagina);
        printf("Numero de marco: %d\n", pagina_obtenida.marco->numero_marco);

        close(archivo);
    } else {
        log_error(logger_swap, "La pagina %d no se encuentra almacenada en los archivos de swap", numero_pagina);
    }
}

/*
    Eliminación de una página
    Se utiliza cuando un proceso finaliza su ejecución
*/
void eliminar_pagina(int numero_pagina) {
    t_pagina_almacenada *pagina = NULL;

    //Busco la página entre aquellas que están almacenadas en archivos
    for(int i=0; i<list_size(lista_paginas_almacenadas); i++) {
        t_pagina_almacenada *pagina_almacenada = list_get(lista_paginas_almacenadas, i);
        if(pagina_almacenada->numero_pagina == numero_pagina) {
            pagina = pagina_almacenada;
            break;
        }
    }

    //Si la página fue encontrada, la elimino
    if(pagina != NULL) {
        pagina->marco->esta_libre = 1;

        //Elimino la página
        for(int i=0; i<list_size(lista_paginas_almacenadas); i++) {
            t_pagina_almacenada *pagina_almacenada = list_get(lista_paginas_almacenadas, i);
            if(pagina_almacenada->numero_pagina == numero_pagina) {
                list_remove(lista_paginas_almacenadas, i);
                break;
            }
        }

        //Actualizo la información del archivo
        t_informacion_archivo *archivo = list_get(archivos_abiertos, pagina->marco->file);
        archivo->espacio_disponible = archivo->espacio_disponible + config_swap->TAMANIO_PAGINA;

        free(pagina);
    } else {
        log_error(logger_swap, "La pagina %d no se encuentra almacenada en los archivos de swap. No pudo completarse el proceso de eliminacion.", numero_pagina);
    }
}

/*
    Eliminación de archivos de SWAP
    Utilizado cuando finaliza la ejecución
*/
void borrar_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        remove(path);
    }
}