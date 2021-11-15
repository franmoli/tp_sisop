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
    /*while( i< tabla_paginas->paginas_totales_maximas){
        t_pagina *pagina = malloc(sizeof(t_pagina));
        pagina->tamanio_ocupado = 0;
        pagina->numero_pagina = i;
        pagina->cantidad_contenidos= 0;
        pagina->listado_de_contenido = list_create();
        t_contenidos_pagina *contenido =  malloc(sizeof(t_contenido));
        contenido->recorrido = 0;
        list_add(pagina->listado_de_contenido, contenido);
        list_add(tabla_paginas->paginas, pagina);
        i++;
    }*/
    //Conectar a swap
    socket_cliente_swap = crear_conexion("127.0.0.1", "5001");
    if(socket_cliente_swap == -1){
        log_info(logger_memoria, "Fallo en la conexion a swap");
    }

    signal(SIGINT, imprimirMetricas);
    signal(SIGUSR1, generarDump);
    signal(SIGUSR2, limpiarTlb);

    //CASO PRUEBA DE MEMALLOC
    t_paquete *paquete1 = serializar_alloc(5);
    memAlloc(paquete1);
    free(paquete1);

    paquete1 = serializar_alloc(10);
    memAlloc(paquete1);
    free(paquete1);

    paquete1 = serializar_alloc(134542270);
    freeAlloc(paquete1);
    free(paquete1);
    
    paquete1 = serializar_alloc(3);
    memAlloc(paquete1);
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
                memAlloc(paquete);
                break;
            case MEMWRITE:
                log_info(logger_memoria, "recibi orden de guardar en memoria del cliente %d", client);
                memAlloc(paquete);
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
void limpiarTlb(int signal){
    
}
void generarDump(int signal){

}
void imprimirMetricas(int signal){
    log_info(logger_memoria,"SEÑAL RECIBIDA");
}