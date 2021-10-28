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
    tamanio_archivo = config_swap->TAMANIO_SWAP/config_swap->TAMANIO_PAGINA;

    //Se inicializa el servidor
    socket_server = iniciar_servidor(config_swap->IP, string_itoa(config_swap->PUERTO), logger_swap);

    //Se inicializan los archivos
    log_info(logger_swap, "Aguarde un momento... Generando archivos...");
    for(int i=1; i<=(tamanio_archivo); i++) {
        crear_archivo_swap(i);
    }
    archivo_seleccionado = 1;

    /*Hardcodeo lectura de una página desde el archivo*/
    t_marco *marco_prueba = malloc(sizeof(t_marco));
    marco_prueba->numero_marco = rand();
    t_pagina *pagina_prueba = malloc(sizeof(t_pagina));
    pagina_prueba->numero_pagina = rand();
    pagina_prueba->marco = marco_prueba;
    insertar_pagina_en_archivo(pagina_prueba);

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
    archivo_seleccionado++;
    if(archivo_seleccionado > tamanio_archivo) {
        archivo_seleccionado = 1;
    }
}

void crear_archivo_swap(int numero_particion) {
    char *path = string_new();
    string_append_with_format(&path, "files/%s.bin", string_itoa(numero_particion));
    list_add(config_swap->ARCHIVOS_SWAP, path);

    FILE *file;
    file = fopen(path, "wb");
    fwrite(&path, sizeof(tamanio_archivo), 1, file);
}

void insertar_pagina_en_archivo(t_pagina *pagina) {
    bool pagina_guardada = 0;

    do {
        char *path_archivo_actual = list_get(config_swap->ARCHIVOS_SWAP, archivo_seleccionado);
        FILE *archivo = fopen(path_archivo_actual, "wb+");
        
        if(bytes_pagina(*pagina) <= tamanio_archivo) {
            //Asumo que el contenido de la página se reemplaza ya que solo podrá haber una página por archivo
            fwrite(&pagina, sizeof(t_pagina), 1, archivo);
            fclose(archivo);
            free(pagina);

            pagina_guardada = 1;
        } else {
            fclose(archivo);
            archivo_seleccionado += 1;
        }
    } while(pagina_guardada == 0 && archivo_seleccionado < tamanio_archivo);

    if(pagina_guardada == 0) {
        log_error(logger_swap, "No se ha podido guardar la pagina en ningun archivo dado que no hay espacio suficiente");
    } else {
        log_info(logger_swap, "Pagina almacenada en el archivo %d.bin", archivo_seleccionado);
    }
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