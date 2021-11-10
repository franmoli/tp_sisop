#include "memoria.h"

int main(int argc, char **argv)
{
    logger_memoria = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    log_info(logger_memoria, "Programa inicializado correctamente");
    pthread_t hilo_client;
	t_config* config = leer_config_file("./cfg/memoria.cfg");
    config_memoria = generarConfigMemoria(config);
    log_info(logger_memoria, "Configuración cargada correctamente");
    tamanio_memoria = malloc(sizeof(config_memoria->TAMANIO));
	
	tabla_paginas = malloc(sizeof(t_tabla_paginas));
    tabla_paginas->paginas = list_create();

    tabla_tlb = malloc(sizeof(tabla_tlb));
    tabla_tlb->tlb = list_create();
	tabla_paginas->paginas_totales_maximas =config_memoria->TAMANIO / config_memoria->TAMANIO_PAGINA;
    int i = 0;
    while( i< tabla_paginas->paginas_totales_maximas){
        t_pagina *pagina = malloc(sizeof(t_pagina));
        pagina->tamanio_ocupado = 0;
        pagina->numero_pagina = i;
        pagina->cantidad_contenidos= 0;
        pagina->contenidos_pagina = list_create();
        t_contenidos_pagina *contenido =  malloc(sizeof(t_contenido));
        contenido->recorrido = 0;
        list_add(pagina->contenidos_pagina, contenido);
        list_add(tabla_paginas->paginas, pagina);
        i++;
    }
    //Conectar a swap
    socket_cliente_swap = crear_conexion("127.0.0.1", "5001");
    if(socket_cliente_swap == -1){
        log_info(logger_memoria, "Fallo en la conexion a swap");
    }

   /* uint32_t inicio = tamanio_memoria;

    log_info(logger_memoria, "Inicio memoria: %d", inicio);

    memAlloc(5);
    t_heap_metadata* data = memRead(tamanio_memoria);

    log_info(logger_memoria, "PrevAlloc: %d", data->prevAlloc);
    log_info(logger_memoria, "NextAlloc: %d", data->nextAlloc);
    log_info(logger_memoria, "isFree: %d", data->isFree);

    log_info(logger_memoria, "---------------");

    t_heap_metadata* data2 = memRead(data->nextAlloc);

    log_info(logger_memoria, "PrevAlloc: %d", data2->prevAlloc);
    log_info(logger_memoria, "NextAlloc: %d", data2->nextAlloc);
    log_info(logger_memoria, "isFree: %d", data2->isFree);

    log_info(logger_memoria, "---------------");

    memAlloc(6);
    t_heap_metadata* data3 = memRead(data->nextAlloc);
    log_info(logger_memoria, "otro alloc: %d", data3->prevAlloc);
    log_info(logger_memoria, "ultimo alloc empieza en: %d", data3->nextAlloc);*/


    //CASO PRUEBA DE MEMALLOC
    t_paquete *paquete1 = serializar_alloc(5);
    guardarMemoria(paquete1);
    free(paquete1);

    paquete1 = serializar_alloc(10);
    guardarMemoria(paquete1);
    free(paquete1);

    paquete1 = serializar_alloc(3);
    guardarMemoria(paquete1);
    free(paquete1);



    //PROGRAMA NORMAL
    socket_server = iniciar_servidor("127.0.0.1", string_itoa(config_memoria->PUERTO), logger_memoria);
    while(1){
        socket_client = esperar_cliente(socket_server, logger_memoria);
		if (socket_client != -1) {
			pthread_create(&hilo_client, NULL, (void *)ejecutar_operacion, (void *)socket_client);
		}
    }
	log_info(logger_memoria, "Programa finalizado con éxito");
    log_destroy(logger_memoria);
	liberar_config(config);
}
static void *ejecutar_operacion(int client)
{
	while(1) {
		t_paquete *paquete = recibir_paquete(client);

        //Analizo el código de operación recibido y ejecuto acciones según corresponda
        switch(paquete->codigo_operacion) {
            case CLIENTE_TEST:
                log_info(logger_memoria, "Mensaje de prueba recibido correctamente por el cliente %d", client);
                break;
            case MEMALLOC:
                log_info(logger_memoria, "recibi orden de almacenar memoria del cliente %d", client);
                guardarMemoria(paquete);
                break;
            case MEMWRITE:
                log_info(logger_memoria, "recibi orden de guardar en memoria del cliente %d", client);
                guardarMemoria(paquete);
                break;
            default:
                log_error(logger_memoria, "Codigo de operacion desconocido");
                break;
        }

        //Libero la memoria ocupada por el paquete
		free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);

        //Salgo del ciclo
        break; 
	}
	close(client);
	log_info(logger_memoria, "Se desconecto el cliente [%d]", client);
	return NULL;
}