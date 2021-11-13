#include "swap.h"

int main(int argc, char **argv) {
    //Inicio del programa
    system("clear");
    logger_swap = log_create("./cfg/swap.log", "SWAP", true, LOG_LEVEL_INFO);
    lista_paginas_almacenadas = list_create();
    lista_mapeos = list_create();
    archivos_abiertos = list_create();
    sem_init(&mutex_operacion, 0, 1);
    log_info(logger_swap, "Programa inicializado correctamente");

    //Se carga la configuración
    log_info(logger_swap, "Iniciando carga del archivo de configuración");
    config_file = leer_config_file("./cfg/swap.cfg");
    config_swap = generar_config_swap(config_file);
    log_info(logger_swap, "Configuración cargada correctamente");

    //Valido que el tamaño de SWAP sea múltiplo de la página
    if(config_swap->TAMANIO_SWAP % config_swap->TAMANIO_PAGINA != 0) {
        log_error(logger_swap, "El tamanio de los archivos de swap debe ser multiplo del tamanio de la pagina");
        exit(-1);
    }

    //Se inicializa el servidor
    socket_server = iniciar_servidor(config_swap->IP, string_itoa(config_swap->PUERTO), logger_swap);

    //Se inicializan los archivos
    log_info(logger_swap, "Aguarde un momento... Generando archivos...");
    crear_archivos_swap();

    /*Hardcodeo lectura de una página desde el archivo*/
    t_marco *marco_prueba = malloc(sizeof(t_marco));
    marco_prueba->numero_marco = 500;
    t_pagina *pagina_prueba = malloc(sizeof(t_pagina));
    pagina_prueba->numero_pagina = 500;
    pagina_prueba->marco = marco_prueba;
    t_pagina *pagina_prueba_2 = malloc(sizeof(t_pagina));
    pagina_prueba_2->numero_pagina = 12;
    pagina_prueba_2->marco = marco_prueba;
    t_pagina *pagina_prueba_3 = malloc(sizeof(t_pagina));
    pagina_prueba_3->numero_pagina = 1256;
    pagina_prueba_3->marco = marco_prueba;

    leer_pagina_de_archivo(23);
    insertar_pagina_en_archivo(pagina_prueba);
    insertar_pagina_en_archivo(pagina_prueba_2);
    insertar_pagina_en_archivo(pagina_prueba_3);
    leer_pagina_de_archivo(1256);
    leer_pagina_de_archivo(500);
    leer_pagina_de_archivo(12);

    //Se esperan conexiones
    log_info(logger_swap, "Esperando conexiones por parte de un cliente");
    bool cliente_recibido = 0;
    do {
        socket_client = esperar_cliente(socket_server, logger_swap);
        if(socket_client != -1) {
            sem_wait(&mutex_operacion);
            printf("\nEntra un procesardo\n");
            cliente_recibido = 1;
            ejecutar_operacion(socket_client);
            sem_post(&mutex_operacion);
        }

        //Espero el tiempo de retardo
        sleep(config_swap->RETARDO_SWAP);
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