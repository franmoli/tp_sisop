#include "swap.h"

int main(int argc, char **argv) {
    //Inicio del programa
    system("clear");
    contador_marcos = 0;
    logger_swap = log_create("./cfg/swap.log", "SWAP", true, LOG_LEVEL_INFO);
    lista_paginas_almacenadas = list_create();
    lista_mapeos = list_create();
    archivos_abiertos = list_create();
    tabla_marcos = list_create();
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

    //Se inicializan el servidor y la conexión con memoria
    socket_server = iniciar_servidor(config_swap->IP, string_itoa(config_swap->PUERTO), logger_swap);

    //Se inicializan los archivos
    log_info(logger_swap, "Aguarde un momento... Generando archivos...");
    crear_archivos_swap();

    //Se esperan conexiones
    log_info(logger_swap, "Esperando conexiones por parte de un cliente");
    socket_client = esperar_cliente(socket_server, logger_swap);

    if(socket_client != -1) {
        int resultado_operacion = 0;
        while(resultado_operacion == 0) {
            sem_wait(&mutex_operacion);
            resultado_operacion = ejecutar_operacion(socket_client);
            sem_post(&mutex_operacion);

            //Espero el tiempo de retardo
            sleep(config_swap->RETARDO_SWAP);
        }
    }

    //Fin del programa
    close(socket_client);
    close(socket_server);
    liberar_memoria_y_finalizar();
    return 1;
}

int ejecutar_operacion(int client) {
    t_paquete *paquete = recibir_paquete(client);
    //Analizo el código de operación recibido y ejecuto acciones según corresponda
    if(paquete->codigo_operacion == SWAPSAVE) {
        //Deserializo la página enviada por Memoria
        t_pagina_enviada_swap *pagina = deserializar_pagina(paquete->buffer->stream);
        t_heap_contenido_enviado *c = list_get(pagina->heap_contenidos,0);
        
        //Inserto la página en los archivos de swap
        int op_code = insertar_pagina_en_archivo(pagina);

        //Envío respuesta de la operación a memoria
        t_buffer *buffer = malloc(sizeof(t_buffer));
        buffer->size = 0;

        t_paquete *paquete_respuesta = malloc(sizeof(t_paquete));
        paquete_respuesta->codigo_operacion = op_code ? PAGINA_GUARDADA:PAGINA_NO_GUARDADA;
        paquete_respuesta->buffer = buffer;

        enviar_paquete(paquete_respuesta, socket_client);
    } else if(paquete->codigo_operacion == SWAPFREE) {
        //Deserializo el pedido enviado por Memoria
        int pagina_solicitada;
        memcpy(&pagina_solicitada, paquete->buffer->stream + 0, sizeof(int));
	    
        //Busco la página y la envío en caso correcto
        t_pagina_enviada_swap pagina = leer_pagina_de_archivo(pagina_solicitada);

        if(pagina.numero_pagina >= 0) {
            void *pagina_serializada = serializar_pagina(&pagina);

            t_buffer *buffer = malloc(sizeof(t_buffer));
            buffer->size = bytes_pagina(&pagina);
            buffer->stream = pagina_serializada;

            t_paquete *paquete_respuesta = malloc(sizeof(t_paquete));
            paquete_respuesta->codigo_operacion = RECEPCION_PAGINA;
            paquete_respuesta->buffer = buffer;

            enviar_paquete(paquete_respuesta, socket_client);
        }
    } else {
        log_info(logger_swap, "Memoria se esta preparando para finalizar, apagando memoria virtual");
        
        free(paquete->buffer);
        free(paquete); 
        
        return -1;
    }

    //Libero la memoria ocupada por el paquete
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete); 

    return 0;
}

/* Liberado de memoria */
void liberar_memoria_y_finalizar(){
    log_info(logger_swap, "Programa finalizado con éxito");
    
    destruir_mapeos();
    borrar_archivos_swap();
    
    config_destroy(config_file);
    log_destroy(logger_swap);
    sem_destroy(&mutex_operacion);
    
    list_destroy_and_destroy_elements(config_swap->ARCHIVOS_SWAP, (void *) destruir_elemento_lista);
    list_destroy_and_destroy_elements(archivos_abiertos, (void *) destruir_elemento_lista);
    list_destroy_and_destroy_elements(lista_paginas_almacenadas, (void *) destruir_elemento_lista);
    list_destroy_and_destroy_elements(tabla_marcos, (void *) destruir_elemento_lista);
    
    free(config_swap);
}

void destruir_elemento_lista(void *elemento){
    free(elemento);
}

void destruir_mapeos(){
    for(int i=0; i<list_size(lista_mapeos); i++) {
        list_remove(lista_mapeos, i);
    }
    list_destroy(lista_mapeos);
}