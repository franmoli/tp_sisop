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
    borrar_archivos_swap();
    liberar_memoria_y_finalizar();
    return 1;
}

int ejecutar_operacion(int client) {
    t_paquete *paquete = recibir_paquete(client);

    //Analizo el código de operación recibido y ejecuto acciones según corresponda
    if(paquete->codigo_operacion == SWAPSAVE) {
        //Deserializo la página enviada por Memoria
        t_pagina_swap *pagina = malloc(sizeof(t_pagina_swap));
        deserializar(paquete,10,INT,&(pagina->tipo_contenido),INT,&(pagina->pid),INT,&(pagina->numero_pagina),LIST,&(pagina->contenido_heap_info),LIST,&(pagina->contenido_carpincho_info));

        //Inserto la página en los archivos de swap
        int op_code = insertar_pagina_en_archivo(pagina);

        //Envío respuesta de la operación a memoria
        t_paquete *paquete_respuesta = malloc(sizeof(paquete));
        paquete_respuesta->codigo_operacion = op_code ? PAGINA_GUARDADA:PAGINA_NO_GUARDADA;

        enviar_paquete(paquete_respuesta, socket_client);
    } else if(paquete->codigo_operacion == SWAPFREE) {
        //Deserializo el pedido enviado por Memoria
        int pagina_solicitada;
        memcpy(&pagina_solicitada, paquete->buffer->stream + 0, sizeof(int));
	    
        //Busco la página y la envío en caso correcto
        t_pagina_swap pagina = leer_pagina_de_archivo(pagina_solicitada);

        if(pagina.numero_pagina >= 0) {
            t_paquete *paquete_respuesta = serializar(SWAPSAVE,12,INT,pagina.tipo_contenido,INT,pagina.pid,INT,pagina.numero_pagina,LIST,SWAP_PAGINA_HEAP,(pagina.contenido_heap_info),LIST,SWAP_PAGINA_CONTENIDO,(pagina.contenido_carpincho_info));
            enviar_paquete(paquete_respuesta, socket_client);
        }
    } else {
        log_info(logger_swap, "Memoria se esta preparando para finalizar, apagando memoria virtual");
        
        free(paquete->buffer->stream);
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
    config_destroy(config_file);
    list_destroy_and_destroy_elements(config_swap->ARCHIVOS_SWAP, (void *) destruir_elementos_lista);
    free(config_swap);
    log_info(logger_swap, "Programa finalizado con éxito");
    log_destroy(logger_swap);
}

void destruir_elementos_lista(void *elemento){
    free(elemento);
}