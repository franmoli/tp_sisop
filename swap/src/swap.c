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
    
    //Prueba de serialización
    t_marco *marco_prueba = malloc(sizeof(t_marco));
    marco_prueba->numero_marco = 340;

    t_pagina *pagina_prueba = malloc(sizeof(t_pagina));
    pagina_prueba->numero_pagina = 200;
    pagina_prueba->marco = marco_prueba;
    pagina_prueba->esta_vacia = false;

    void *stream_prueba = malloc(bytes_pagina(*pagina_prueba));
    serializar_pagina(*pagina_prueba, &stream_prueba, 0);
    t_pagina pagina_deserializada = deserializar_pagina(stream_prueba, 0);

    printf("\n\n\n");

    printf("Pagina %d\n", pagina_prueba->numero_pagina);
    printf("\tMarco: %d\n", pagina_prueba->marco->numero_marco);
    printf("\tVacia: %d\n", pagina_prueba->esta_vacia);
    printf("---------------------------\n");
    printf("Pagina %d\n", pagina_deserializada.numero_pagina);
    printf("\tMarco: %d\n", pagina_deserializada.marco->numero_marco);
    printf("\tVacia: %d\n", pagina_deserializada.esta_vacia);

    printf("\n\n\n");

    free(marco_prueba);
    free(pagina_prueba);
    free(stream_prueba);

    //Se esperan conexiones
    pthread_t hilo_client;
    log_info(logger_swap, "Esperando conexiones por parte de clientes");
    while(1) {
        socket_client = esperar_cliente(socket_server, logger_swap);
        if(socket_client != -1) {
            pthread_create(&hilo_client, NULL, (void *) ejecutar_operacion, (void *) socket_client);
        }
        //TODO: terminar la ejecución cuando el cliente se desconecta
    }

    //Fin del programa
    liberar_memoria_y_finalizar();
    return 1;
}

static void *ejecutar_operacion(int client) {
	while(1) {
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

        //Salgo del ciclo
        break; 
	}

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