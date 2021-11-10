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
    Selección del archivo que se escribirá/leerá
*/
int seleccionar_archivo_escritura(int bytes_a_guardar) {
    int archivo_seleccionado = -1;
    int tamanio_archivo_seleccionado = 0;

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

    return archivo_seleccionado;
}

/*
    Inserción de página en archivo de SWAP
*/
void insertar_pagina_en_archivo(t_pagina *pagina) {
    //Selecciono el archivo
    int bytes_a_guardar = bytes_pagina(*pagina);
    int posicion_archivo_obtenido = seleccionar_archivo_escritura(bytes_a_guardar);

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
        int offset_inicial = statbuf.st_size;
        int offset_final = offset_inicial;
        log_info(logger_swap, "Archivo seleccionado: %s (%dB de espacio disponible)", path_archivo, config_swap->TAMANIO_SWAP - statbuf.st_size);

        //Si es la primera pagina a insertar en el archivo, genero el mapeo del archivo correspondiente
        if(offset_inicial == 0) {
            void *mapeo_archivo = mmap(NULL, config_swap->TAMANIO_SWAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, archivo, 0);
            list_add(lista_mapeos, mapeo_archivo);
        }

        //Obtengo el mapeo correspondiente al archivo
        void *mapping = list_get(lista_mapeos, posicion_archivo_obtenido);
        if(mapping == MAP_FAILED){
            log_error(logger_swap, "El mapeado de la pagina fallo, revise el codigo. Abortanto ejecucion.");
            exit(-1);
        }

        //Mapeo los datos de la pagina
        log_info(logger_swap, "Escribiendo la pagina %d en el archivo %s en el offset %d", pagina->numero_pagina, path_archivo, offset_inicial);
        memcpy(mapping + offset_final, &(pagina->numero_pagina), sizeof(pagina->numero_pagina));
        offset_final += sizeof(pagina->numero_pagina);
        memcpy(mapping + offset_final, &(pagina->marco->numero_marco), sizeof(pagina->marco->numero_marco));
        offset_final += sizeof(pagina->marco->numero_marco);

        //Almaceno la página mapeada en el archivo
        ssize_t flag_escritura = write(archivo, mapping, offset_final);
        if(flag_escritura != offset_final){
            log_error(logger_swap, "No se pudo realizar la escritura en el archivo, revise el codigo. Abortando ejecucion.");
            exit(-1);
        }

        //Añado los datos de la página a la estructura administrativa
        t_pagina_almacenada *pagina_almacenada = malloc(sizeof(t_pagina_almacenada));
        pagina_almacenada->numero_pagina = pagina->numero_pagina;
        pagina_almacenada->base = offset_inicial;
        pagina_almacenada->size = bytes_a_guardar;
        pagina_almacenada->file = path_archivo;
        list_add(lista_paginas_almacenadas, pagina_almacenada);

        //Actualizo la estructura administrativa de los archivos
        t_informacion_archivo *informacion_archivo = list_get(archivos_abiertos, posicion_archivo_obtenido);
        informacion_archivo->espacio_disponible = config_swap->TAMANIO_SWAP - (statbuf.st_size + bytes_a_guardar);

        //Cierro el archivo y libero la memoria de la página
        close(archivo);

        log_info(logger_swap, "Pagina %d almacenada en %s", pagina->numero_pagina, path_archivo);
        free(pagina);
    } else {
        log_error(logger_swap, "No se ha podido guardar la pagina en ningun archivo dado que no hay espacio suficiente");
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
        int archivo = open(informacion_almacenamiento->file, O_RDONLY);

        struct stat statbuf;
        fstat(archivo, &statbuf);

        //Obtengo la página dentro del archivo
        void *paginas_obtenidas = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, archivo, 0);
        t_pagina pagina_obtenida;
        t_marco marco_pagina;

        int offset_actual = informacion_almacenamiento->base;
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
    Eliminación de archivos de SWAP
    Utilizado cuando finaliza la ejecución
*/
void borrar_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        remove(path);
    }
}