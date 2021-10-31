#include "swap.h"

int main(int argc, char **argv) {
    //Inicio del programa
    system("clear");
    logger_swap = log_create("./cfg/swap.log", "SWAP", true, LOG_LEVEL_INFO);
    log_info(logger_swap, "Programa inicializado correctamente");

    //Se carga la configuración
    log_info(logger_swap, "Iniciando carga del archivo de configuración");
    config_file = leer_config_file("./cfg/swap.cfg");
    config_swap = generar_config_swap(config_file);
    log_info(logger_swap, "Configuración cargada correctamente");

    //Se inicializa el servidor
    socket_server = iniciar_servidor(config_swap->IP, string_itoa(config_swap->PUERTO), logger_swap);

    //Se inicializan los archivos
    log_info(logger_swap, "Aguarde un momento... Generando archivos...");
    crear_archivos_swap();
    archivo_seleccionado = 1;

    /*Hardcodeo lectura de una página desde el archivo*/
    t_marco *marco_prueba = malloc(sizeof(t_marco));
    marco_prueba->numero_marco = 500;
    t_pagina *pagina_prueba = malloc(sizeof(t_pagina));
    pagina_prueba->numero_pagina = 500;
    pagina_prueba->marco = marco_prueba;

    printf("********** ARCHIVO NORMAL **********\n");
    printf("Numero de pagina: %d\n", pagina_prueba->numero_pagina);
    printf("Numero de marco: %d\n\n", pagina_prueba->marco->numero_marco);

    insertar_pagina_en_archivo(pagina_prueba);
    leer_pagina_de_archivo();

    //Se esperan conexiones
    log_info(logger_swap, "Esperando conexiones por parte de un cliente");
    bool cliente_recibido = 0;
    do {
        socket_client = esperar_cliente(socket_server, logger_swap);
        if(socket_client != -1) {
            cliente_recibido = 1;
            ejecutar_operacion(socket_client);
        }
    } while(cliente_recibido);

    //Fin del programa
    borrar_archivos_swap();
    liberar_memoria_y_finalizar();
    return 1;
}

static void *ejecutar_operacion(int client) {
    t_paquete *paquete = recibir_paquete(client);

    //Analizo el código de operación recibido y ejecuto acciones según corresponda
    switch(paquete->codigo_operacion) {
        case CLIENTE_TEST:
            log_info(logger_swap, "Mensaje de prueba recibido correctamente por el cliente %d", client);
            break;
        default:
            log_error(logger_swap, "Codigo de operacion desconocido");
            break;
    }

    //Libero la memoria ocupada por el paquete
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete); 

    //Cierro el cliente
	close(client);
	log_info(logger_swap, "El cliente %d ha finalizado su ejecución y se desconecto", client);
	
    return NULL;
}

/* Manejo de archivos */
void siguiente_archivo() {
    if(archivo_seleccionado == -1) {
        archivo_seleccionado = 0;
    }

    archivo_seleccionado++;

    if(archivo_seleccionado > list_size(config_swap->ARCHIVOS_SWAP)) {
        archivo_seleccionado = -1;
    }
}

void crear_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        char *init_value = '\0';

        FILE *file = fopen(path, "wb");
        fwrite(&init_value, sizeof(config_swap->TAMANIO_SWAP), 1, file);

        fclose(file);
    }
}

void insertar_pagina_en_archivo(t_pagina *pagina) {
    bool pagina_guardada = 0;
    struct stat statbuf;

    do {
        //Abro el archivo
        char *path_archivo_actual = list_get(config_swap->ARCHIVOS_SWAP, archivo_seleccionado-1);
        int archivo = open(path_archivo_actual, O_WRONLY);
        if(archivo < 0){
            log_error(logger_swap, "No pudo encontrarse el archivo en la ruta especificada (%s). Abortando ejecucion.", path_archivo_actual);
            exit(-1);
        }

        //Obtengo datos del archivo
        int archivo_cargado_correctamente = fstat(archivo, &statbuf);
        if(archivo_cargado_correctamente == -1) {
            log_error(logger_swap, "No pudo cargarse el archivo correctamente, revise el codigo. Abortando ejecucion.");
            exit(-1);
        }

        if(bytes_pagina(*pagina) <= (config_swap->TAMANIO_SWAP - statbuf.st_size)) {
            //Asumo que el contenido de la página se reemplaza ya que solo podrá haber una página por archivo
            t_pagina *mapping = mmap(NULL, bytes_pagina(*pagina), PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, archivo, 0);
            if(mapping == MAP_FAILED){
                log_error(logger_swap, "El mapeado de la pagina fallo, revise el codigo. Abortanto ejecucion.");
                exit(-1);
            }
            mapping->numero_pagina = pagina->numero_pagina;
            mapping->marco = pagina->marco;

            ssize_t flag_escritura = write(archivo, mapping, bytes_pagina(*pagina));
            if(flag_escritura != bytes_pagina(*pagina)){
                log_error(logger_swap, "No se pudo realizar la escritura en el archivo, revise el codigo. Abortando ejecucion.");
                exit(-1);
            }

            close(archivo);
            free(pagina);
            pagina_guardada = 1;
        } else {
            close(archivo);
            siguiente_archivo();
        }
    } while(pagina_guardada == 0 && archivo_seleccionado == -1);

    if(pagina_guardada == 0) {
        log_error(logger_swap, "No se ha podido guardar la pagina en ningun archivo dado que no hay espacio suficiente");
    } else {
        log_info(logger_swap, "Pagina almacenada en el archivo %d.bin", archivo_seleccionado);
    }
}

void leer_pagina_de_archivo() {
    //TODO: hacer algoritmo de busqueda de página a través de archivos
    char *path_archivo_pagina = list_get(config_swap->ARCHIVOS_SWAP, archivo_seleccionado-1);
    int archivo = open(path_archivo_pagina, O_RDONLY);

    struct stat buffer_stat;
    fstat(archivo, &buffer_stat);
    int size_archivo = buffer_stat.st_size;

    t_pagina *pagina_obtenida = malloc(sizeof(t_pagina));
    
    pagina_obtenida = mmap(0, size_archivo, PROT_READ, MAP_PRIVATE, archivo, 0);

    printf("Numero de pagina: %d\n", pagina_obtenida->numero_pagina);
    printf("Numero de marco: %d\n", pagina_obtenida->marco->numero_marco);

    close(archivo);
}

void borrar_archivos_swap() {
    for(int i=0; i<list_size(config_swap->ARCHIVOS_SWAP); i++) {
        char *path = list_get(config_swap->ARCHIVOS_SWAP, i);
        remove(path);
    }
}

/* Liberado de memoria */
void liberar_memoria_y_finalizar(){
    config_destroy(config_file);
    list_destroy_and_destroy_elements(config_swap->ARCHIVOS_SWAP, (void *) destruir_elementos_lista);
    free(config_swap);
    log_info(logger_swap, "Programa finalizado con éxito");
    log_destroy(logger_swap);
}

void destruir_elementos_lista(void *elemento){
    free(elemento);
}